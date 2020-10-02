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
		try {
			session->raw() = client->readToDelim("\r\n.\r\n", 10000000);
		} catch (const SocketReadLimit &e) {

		}
		exit(0);
		uint64_t end = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

		// Calculates the time difference so we can calculate the speed
		//  of transmission
		double timeDifference = static_cast<double>(end - start) / 1000 / 1000;
		double kbsec = static_cast<float>(session->raw().length()) / timeDifference;
		DEBUG_ONLY(clogger << DEBUG << "MIME-Message received in " << timeDifference
			<< " seconds, with " << kbsec << "kb/sec" << ENDL << CLASSIC);

		// ========================================
		// Performs the security checks
		// ========================================

		vector<pair<string, string>> authResults = {};
		if (!session->getFlag(_SMTP_SERV_SESSION_AUTH_FLAG)) {
			// Creates the DMARC resolve query
			string dmarcQuery = "_dmarc.";
			dmarcQuery += session->getTransportFrom().getDomain();

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
			spfValidator.safeValidate(session->getTransportFrom().getDomain(), client->getPrefix());

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
						session->setPossibleSpam(true);
						dmarcSpfValid = true;
						break;
					case DMARC::DMARCPolicy::PolicyQuarantine:
						session->setPossibleSpam(true);
						dmarcSpfValid = false;
						break;
					case DMARC::DMARCPolicy::PolicyReject: {
						client->write(ServerResponse(SMTPResponseType::SRC_SPF_REJECT).build());
						return true;
						break;
					}
				}
			} else if (!spfValid && !dmarcFound) session->setPossibleSpam(true);
			else if(dmarcFound) dmarcSpfValid = true;

			// Validates the DKIM record
			DKIM::DKIMValidator dkimValidator;
			dkimValidator.validate(session->raw());

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
			if (dmarcFound && !dkimValid) {
				switch (dmarcRecord.getPolicy()) {
					case DMARC::DMARCPolicy::PolicyNone:
						dmarcDkimValid = true;
						break;
					// TODO: Make DKIM reliable to reject clients if this fails
					case DMARC::DMARCPolicy::PolicyQuarantine:
					case DMARC::DMARCPolicy::PolicyReject:
						dmarcDkimValid = false;
						break;
				}
			} else if (dmarcFound) dmarcDkimValid = true;


			// Builds the dmarc result header value, this will not be generated
			//  since the validation of dmarc happens here;
			if (dmarcFound) {
				// Prints the policy, subdomain policy and the query into the buffer
				//  which will be inserted into the final headers
				sprintf(buffer, "%s (p:%s sp:%s) query:%s", 
					(!dmarcSpfValid || !dmarcDkimValid ? "fail" : "pass"), dmarcRecord.getPolicyString(),
					dmarcRecord.getSubdomainPolicyString(), dmarcQuery.c_str());
			} else {
				// Prints that dmarc is neutral, since there was no record found
				sprintf(buffer, "fail (no record found) query:%s", dmarcQuery.c_str());
			}

			// Builds the auth result header map, which will contain
			//  some basic auth results
			authResults.push_back(make_pair("spf", spfValidator.getResultString()));
			authResults.push_back(make_pair("dkim", dkimValidator.getResultString()));
			authResults.push_back(make_pair("dmarc", buffer));

			// Checks if the client was using using SU, if so add the SU
			//  header to the authResults
			if (session->getFlag(_SMTP_SERV_SESSION_SU))
				authResults.push_back(make_pair("su", "pass (to:" + client->getPrefix() + ")"));
		} else authResults.push_back(make_pair("auth", "pass"));

		// ========================================
		// Parses the headers from the message
		// ========================================

		// Splits the raw body into lines, so we can perform
		//  further processing on it later
		vector<string> lines = Parsers::getMIMELines(session->raw());

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
		// Builds the headers
		// ========================================

		// Inserts the auth header into the joined headers
		//  vector, which will later be used to build message
		joinedHeaders.push_back(Builders::buildHeaderFromSegments("X-Fannst-Auth", authResults));

		// ========================================
		// Builds the default message headers
		// ========================================

		// Generates the received header, this will indicate that the message
		//  went through the FSMTP-V2 Server
		joinedHeaders.push_back("Received: " + SMTP::Server::Headers::buildReceived(
			client->getReverseLookup(), client->getPrefix(),
			session->getTransportFrom().e_Address, client->getPort()
		));

		// ========================================
		// Starts generating the text MIME
		// ========================================

		// Starts generating the folded headers, these will all be folded
		//  in our own way
		session->raw().clear();
		for_each(joinedHeaders.begin(), joinedHeaders.end(), [&](const string &header) {
			session->raw() += Builders::foldHeader(header, 128) + "\r\n";
		});

		// Appends the message body itself
		session->raw() += "\r\n\r\n" + Parsers::getStringFromLines(bodyBegin, bodyEnd);
		
		// ========================================
		// Parses the MIME message
		// ========================================

		// Parses the MIME email, so we can later
		//  start getting the content from it
		FullEmail email;
		Parsers::parseMIME(session->raw(), email);
		for_each(email.e_Headers.begin(), email.e_Headers.end(), [&](const EmailHeader &header) {
			string lower = header.e_Key;
			transform(lower.begin(), lower.end(), lower.begin(), [](const char c) { return tolower(c); });

			if (lower == "x-fannst-flags") {
				try {
					DEBUG_ONLY(clogger << DEBUG << "Found 'X-Fannst-Flags' header, parsing .." << ENDL << CLASSIC);
					session->xfannst().parse(header.e_Value);
					DEBUG_ONLY(session->xfannst().print(clogger));
				} catch (const runtime_error &e) {
					clogger << ERROR << "Parsing failed for 'X-Fannst-Flags', runtime_error: " << e.what() << ENDL << CLASSIC;
				} catch (...) {
					clogger << ERROR << "Parsing failed for 'X-Fannst-Flags', error unknown" << ENDL << CLASSIC;
				}
			}
		});

		// Starts storing the basic values of the email inside of the current session
		session->setMessageID(email.e_MessageID);
		session->setSubject(email.e_Subject);
		session->setFrom(email.e_From[0]);

		// Finds an useful section of the message body for the snippet
		//  we want it to be text/plain
		any_of(email.e_BodySections.begin(), email.e_BodySections.end(), [&](const EmailBodySection &b) {
			if (b.e_Type == EmailContentType::ECT_TEXT_PLAIN) {
				session->generateSnippet(b.e_Content);
				return false;
			} else return true;
		});

		// ========================================
		// Modifies targets if required
		// ========================================

		// If the message is classified as spam we will change
		//  the INBOX targets to the SPAM targets
		if (session->getPossibleSpam()) session->storageTasksToSpam();

		// Checks if the fannst flags state we should not store the
		//  message as sent, if so remove the Sent target from the storage tasks
		if (session->xfannst().getNoStore()) session->removeSentTasks();

		// ========================================
		// Sends the response
		// ========================================
		
		// Builds the server response message
		sprintf(buffer, "%s %ld bytes in %lf, %lf KB/sec queued for delivery.",
			session->getMessageID().c_str(), session->raw().length(), 
			timeDifference, kbsec);

		// Builds the server response, after which we immediately transfer
		//  the message to the client, to indicate that we're done
		client->write(ServerResponse(SMTPResponseType::SRC_DATA_END, buffer, nullptr, nullptr).build());

		if (session->getStorageTasks().size() > 0) Workers::DatabaseWorker::push(session);
		if (session->getRelayTasks().size() > 0) Workers::TransmissionWorker::push(session);

		return false;
	}
};