/*
	Copyright [2020] [Luke A.C.A. Rieff]

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#include "data.src.h"

namespace FSMTP::SMTP::Server::Handlers {
	bool dataHandler(
		shared_ptr<ClientSocket> client,
        shared_ptr<SMTPServerSession> session,
		Logger &clogger
    ) {
		char buffer[2048];

		// ========================================
		// Reads the message from socket
		// ========================================

		// Reads the message from the socket, while storing the
		//  start and end time, so we can calculate the speed
		uint64_t start = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
		session->s_RawBody = client->readToDelim("\r\n.\r\n");
		uint64_t end = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

		// Calculates the time difference so we can calculate the speed
		//  of transmission
		double timeDifference = static_cast<double>(end - start) / 1000 / 1000;
		double kbsec = static_cast<float>(session->s_RawBody.length()) / timeDifference;
		DEBUG_ONLY(clogger << DEBUG << "MIME-Message received in " << timeDifference
			<< " seconds, with " << kbsec << "kb/sec" << ENDL << CLASSIC);

		// ========================================
		// Performs the security checks
		// ========================================

		// Creates the DMARC resolve query
		string dmarcQuery = "_dmarc.";
		dmarcQuery += session->s_TransportMessage.e_TransportFrom.getDomain();

		// Resolves the DMARC record, if this fails we set the
		//  check dmarc false, so that we will not verify further 
		//  actions on the system
		DMARC::DMARCRecord dmarcRecord;
		bool dmarcFound = false;
		try {
			dmarcRecord = DMARC::DMARCRecord::fromDNS(dmarcQuery.c_str());
			dmarcFound = true;
			DEBUG_ONLY(dmarcRecord.print(clogger));
		} catch (const runtime_error &e) {
			DEBUG_ONLY(clogger << ERROR << "Failed to resolve dmarc from query: '"
				<< dmarcQuery << "', error: " << e.what() << ENDL << CLASSIC);
		} catch (...) {
			DEBUG_ONLY(clogger << ERROR << "Failed to resolve dmarc from query: '"
				<< dmarcQuery << "', error unknown" << ENDL << CLASSIC);
		}

		// Performs the SPF validation
		SPF::SPFValidator spfValidator;
		spfValidator.validate(session->s_TransportMessage.e_TransportFrom.getDomain(), 
			client->getPrefix());

		// Checks the outcome of the spf validation, and sets the SPF valid boolean
		//  if the outcome is valid
		bool spfValid;
		switch (spfValidator.getResult().type) {
			case SPF::SPFValidatorResultType::ResultTypeDenied:
			case SPF::SPFValidatorResultType::ResultTypeSystemFailure:
				DEBUG_ONLY(clogger << "Server not found in SPF records" << ENDL);
				spfValid = false;
				break;
			case SPF::SPFValidatorResultType::ResultTypeAllowed:
				DEBUG_ONLY(clogger << "Server found in SPF records" << ENDL);
				spfValid = true;
				break;
		}

		// Checks if the dmarc record exists, and if so check if the policy says
		//  anything about how we should treat the message
		bool dmarcSpfValid;
		if (dmarcFound && !spfValid) {
			switch (dmarcRecord.getPolicy()) {
				case DMARC::DMARCPolicy::PolicyNone:
					dmarcSpfValid = true;
					break;
				case DMARC::DMARCPolicy::PolicyQuarantine:
					session->s_PossSpam = true;
					dmarcSpfValid = false;
					break;
				case DMARC::DMARCPolicy::PolicyReject: {
					client->write(ServerResponse(SMTPResponseType::SRC_SPF_REJECT).build());
					return true;
					break;
				}
			}
		}

		// Validates the DKIM record
		DKIM::DKIMValidator dkimValidator;
		dkimValidator.validate(session->s_RawBody);

		// Checks the result of the dkim validator, and prints it to
		//  the console, while setting the valid boolean based on the result
		bool dkimValid;
		switch (dkimValidator.getResult().type) {
			case DKIM::DKIMValidatorResultType::DKIMValidationPass:
				DEBUG_ONLY(clogger << DEBUG << "Validator found one or more valid signatures" << ENDL << CLASSIC);
				dkimValid = true;
				break;
			case DKIM::DKIMValidatorResultType::DKIMValidationNeutral:
				DEBUG_ONLY(clogger << DEBUG << "Validator returned neutral" << ENDL << CLASSIC);
				dkimValid = true;
				break;
			case DKIM::DKIMValidatorResultType::DKIMValidationSystemError:
				DEBUG_ONLY(clogger << DEBUG << "An system error occured while validating DKIM" << ENDL << CLASSIC);
				dkimValid = false;
				break;
			case DKIM::DKIMValidatorResultType::DKIMValidationFail:
				DEBUG_ONLY(clogger << DEBUG << "All of the signatures are invalid" << ENDL << CLASSIC);
				dkimValid = false;
				break;
		}

		// Checks the result of the DKIM validator, and how we should
		//  treat it according to the dmarc record
		bool dmarcDkimValid;
		switch (dkimValidator.getResult().type) {
			case DMARC::DMARCPolicy::PolicyNone:
				dmarcDkimValid = true;
				break;
			// TODO: Make DKIM reliable to reject clients if this fails
			case DMARC::DMARCPolicy::PolicyQuarantine:
			case DMARC::DMARCPolicy::PolicyReject:
				dmarcDkimValid = false;
				break;
		}

		// ========================================
		// Parses the headers from the message
		// ========================================

		// Splits the raw body into lines, so we can perform
		//  further processing on it later
		vector<string> lines = Parsers::getMIMELines(session->s_RawBody);

		// Splits the headers and the body, so we can append our
		//  own headers to it
		strvec_it headersBegin, headersEnd, bodyBegin, bodyEnd;
		tie(
			headersBegin, headersEnd,
			bodyBegin, bodyEnd
		) = Parsers::splitMIMEBodyAndHeaders(lines.begin(), lines.end());

		// Joins the header lines, so they will not contain any non-wantend
		//  indentions, after which we parse them
		vector<string> joinedHeaders = Parsers::joinHeaders(headersBegin, headersEnd);

		// ========================================
		// Builds the su/dkim/dmarc/spf result
		// ========================================

		// Builds the dmarc result header value, this will not be generated
		//  since the validation of dmarc happens here;

		if (dmarcFound) {
			// Prints the policy, subdomain policy and the query into the buffer
			//  which will be inserted into the final headers
			sprintf(buffer, "%s (p=%s sp=%s) query=%s", 
				(!dmarcSpfValid || !dmarcDkimValid ? "fail" : "pass"), dmarcRecord.getPolicyString(),
				dmarcRecord.getSubdomainPolicyString(), dmarcQuery.c_str());
		} else {
			// Prints that dmarc is neutral, since there was no record found
			sprintf(buffer, "neutral (no record found) query=%s", dmarcQuery.c_str());
		}

		// Builds the auth result header map, which will contain
		//  some basic auth results
		map<string, string> authResults = {
			make_pair("spf", spfValidator.getResultString()),
			make_pair("dkim", dkimValidator.getResultString()),
			make_pair("dmarc", buffer)
		};

		// Checks if the client was using using SU, if so add the SU
		//  header to the authResults
		if (session->getFlag(_SMTP_SERV_SESSION_SU))
			authResults.insert(make_pair("su", "pass (to=" + client->getPrefix() + ")"));

		// ========================================
		// Builds the default message headers
		// ========================================

		// Generates the received header, this will indicate that the message
		//  went through the FSMTP-V2 Server
		joinedHeaders.push_back("Received: " + SMTP::Server::Headers::buildReceived(
			DNS::getHostnameByAddress(client->getAddress()), client->getPrefix(),
			session->s_TransportMessage.e_TransportFrom.e_Address,
			spfValidator.getResultString(), client->getPort()
		));

		// ========================================
		// Starts generating the text MIME
		// ========================================

		// Starts generating the folded headers, these will all be folded
		//  in our own way
		session->s_RawBody.clear();
		for_each(joinedHeaders.begin(), joinedHeaders.end(), [&](const string &header) {
			session->s_RawBody += Builders::foldHeader(header, 98) + "\r\n";
		});

		// Appends the message body itself
		session->s_RawBody += "\r\n\r\n" + Parsers::getStringFromLines(bodyBegin, bodyEnd);
		
		// ========================================
		// Parses the MIME message
		// ========================================

		// Parses the mime message into the final transport message
		//  this will include things such as the subject, body etcetera
		Parsers::parseMIME(session->s_RawBody, session->s_TransportMessage);

		// ========================================
		// Adds the message to the specified queues
		// ========================================

		if (session->s_StorageTasks.size() > 0) Workers::DatabaseWorker::push(session);
		if (session->s_RelayTasks.size() > 0) Workers::TransmissionWorker::push(session);

		// ========================================
		// Sends the response
		// ========================================
		
		// Builds the server response message
		sprintf(buffer, "%s %d bytes in %lf, %lf KB/sec queued for delivery.",
			session->s_TransportMessage.e_MessageID, session->s_RawBody.length(), 
			timeDifference, kbsec);

		// Builds the server response, after which we immediately transfer
		//  the message to the client, to indicate that we're done
		client->write(ServerResponse(SMTPResponseType::SRC_DATA_END, buffer, nullptr, nullptr).build());

		return false;
	}
};