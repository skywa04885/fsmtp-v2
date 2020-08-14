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

#include "Email.src.h"


namespace FSMTP::Models
{
  /**
   * Variable constructor for the EmailAddres
   *
   * @Param {const std::string &} e_Name
   * @Param {const std::string &} e_Address
   * @Return {void}
   */
	EmailAddress::EmailAddress(
		const std::string &e_Name,
		const std::string &e_Address
	):
		e_Name(e_Name), e_Address(e_Address)
	{}

  /**
   * Parse constructor for the email address
   * - basically calls EmailAddress::parse()
   *
   * @Param {const std::string &} raw
   * @Return {void}
   */
	EmailAddress::EmailAddress(const std::string &raw)
	{
		this->parse(raw);
	}

  /**
   * The empty constructor for the EmailAddress
   *
   * @Param {void}
   * @Return {void}
   */
	EmailAddress::EmailAddress()
	{

	}

  /**
   * Returns the string version of the email address
   *
   * @Param {void}
   * @Return {std::string}
   */
  std::string EmailAddress::toString(void) const
  {
		std::string res;
		if (!this->e_Name.empty()) {
			res += this->e_Name;
			res += ' ';
		}
    res += "<" + this->e_Address + ">";
    return res;
  }

  /**
   * Parses an raw string into an EmailAddress
   *
   * @Param {const std::string &} raw
   * @Return {void}
   */
	void EmailAddress::parse(const std::string &raw)
	{
		// Checks the email format type, since there may
		// - be multiple formats, and we need to parse it
		// - in the correct way
		std::size_t openBracket = raw.find_first_of('<'), closeBracket = raw.find_first_of('>');
		if (openBracket != std::string::npos && closeBracket != std::string::npos) {
			
			this->e_Address = raw.substr(openBracket + 1, closeBracket - openBracket - 1);
			this->e_Name = raw.substr(0, openBracket);

			removeFirstAndLastWhite(this->e_Name);
			removeStringQuotes(this->e_Name);
		} else if (
			openBracket == std::string::npos && closeBracket != std::string::npos ||
			openBracket != std::string::npos && closeBracket == std::string::npos
		) {
			throw std::runtime_error("Only one bracket found, address ""should contain both opening and closing.");
		}
		else {
			this->e_Address = raw;
		}

		// Validates the email address by checking for the at symbol
		if (raw.find_first_of('@') == std::string::npos)
			throw std::runtime_error("Could not find @");

		// Removes the non-required whitespace, this is always required
		// - so used as last, while the name whitespace is only required when
		// - the name was parsed
		removeFirstAndLastWhite(this->e_Address);
	}


  /**
   * Parses an raw string into multiple addresses
   *
   * @Param {const std::string &} raw
   * @Return {std::vector<EmailAddress>}
   */
  std::vector<EmailAddress> EmailAddress::parseAddressList(const std::string &raw)
  {
    std::stringstream stream(raw);
    std::string token;
    std::vector<EmailAddress> result = {};

    // Loops over the separate email addresses,
    // - and parses them, and then pushes
    // - them to the result vector
    while (std::getline(stream, token, ','))
      result.push_back(EmailAddress(token));

    return result;
  }

  /**
   * Turns an vector of addresses into an string
   *
   * @Param {const std::vector<EmailAddress> &} addresses
   * @Return {std::string}
   */
  std::string EmailAddress::addressListToString(const std::vector<EmailAddress> &addresses)
  {
    std::string res;

    // Loops over the addresses and appends them to the result
    std::size_t i = 0;
    for (const EmailAddress &a : addresses)
    {
      if (!a.e_Name.empty()) res += '"' + a.e_Name + "\" ";
      res += '<' + a.e_Address + '>';
      if (++i < addresses.size()) res += ", ";
    }

    return res;
  }

  /**
   * Parses the domain name from the address
   *
   * @Param {void}
   * @Return {std::string}
   */
	std::string EmailAddress::getDomain(void) const
	{
		std::size_t index = this->e_Address.find_first_of('@');
		if (index == std::string::npos)
			throw std::runtime_error("Invalid email address");
		return this->e_Address.substr(index + 1);
	}

  /**
   * parses the username from the address
   *
   * @Param {void}
   * @Return {std::string}
   */
	std::string EmailAddress::getUsername(void) const
	{
		std::size_t index = this->e_Address.find_first_of('@');
		if (index == std::string::npos)
			throw std::runtime_error("Invalid email address");
		return this->e_Address.substr(0, index);
	}

	// ======================================================
	// The email stuff, instead of the address stuff
	// ======================================================
  
  FullEmail::FullEmail()
  {}

  /**
	 * Turns an string into an enum value
	 * - of email content type
	 *
	 * @Param {const std::string &} raw
	 * @Return {EmailContentType}
	 */
	EmailContentType stringToEmailContentType(const std::string &raw)
	{
		if (raw == "multipart/alternative") return EmailContentType::ECT_MULTIPART_ALTERNATIVE;
		else if (raw == "multipart/mixed") return EmailContentType::ECT_MULTIPART_MIXED;
		else if (raw == "text/plain") return EmailContentType::ECT_TEXT_PLAIN;
		else if (raw == "text/html") return EmailContentType::ECT_TEXT_HTML;
		else return EmailContentType::ECT_NOT_FUCKING_KNOWN;
	}


  /**
   * Turns an enum value into an string
   *
   * @Param {const EmailContentType} type
   * @Return {const char *}
   */
  const char *contentTypeToString(const EmailContentType type)
  {
    switch (type)
    {
      case EmailContentType::ECT_TEXT_PLAIN: return "text/plain";
      case EmailContentType::ECT_TEXT_HTML: return "text/html";
      case EmailContentType::ECT_MULTIPART_ALTERNATIVE: return "multipart/alternative";
      case EmailContentType::ECT_MULTIPART_MIXED: return "multipart/mixed";
      default: return "application/octet-stream";
    }
  }

	/**
	 * Turns an string into an enum value of
	 * - EmailTransferEncoding type
	 *
	 * @Param {const std::string &} raw
	 * @Return {EmailContentType}
	 */
	EmailTransferEncoding stringToEmailTransferEncoding(const std::string &raw)
	{
		if (raw == "7bit") return EmailTransferEncoding::ETE_7BIT;
		else if (raw == "8bit") return EmailTransferEncoding::ETE_8BIT;
		else if (raw == "base64") return EmailTransferEncoding::ETE_QUOTED_PRINTABLE;
    else if (raw == "quoted-printable") return EmailTransferEncoding::ETE_BASE64;
		else return EmailTransferEncoding::ETE_NOT_FUCKING_KNOWN;
	}

  /**
   * Turns an enum into an string
   *
   * @Param {const EmailTransferEncoding} enc
   * @Return {const char *}
   */
  const char *contentTransferEncodingToString(const EmailTransferEncoding enc)
  {
    switch (enc)
    {
      case EmailTransferEncoding::ETE_8BIT: return "8bit";
      case EmailTransferEncoding::ETE_BASE64: return "base64";
      case EmailTransferEncoding::ETE_7BIT: return "7bit";
      case EmailTransferEncoding::ETE_QUOTED_PRINTABLE: return "quoted-printable";
      default: return "text/plain";
    }
  }


  /**
   * Prints an full email to the console
   *
   * @Param {FullEmail &} email
   * @Param {Logger &logger} logger
   * @Return {void}
   */
  void FullEmail::print(FullEmail &email, Logger &logger)
  {
  	logger << DEBUG;

  	// ========================================
  	// Prints the basic email data
  	//
  	// These are just some variables we
  	// - want to see in debug mode
  	// ========================================

  	// Prints the already finished variables
  	logger << "FullEmail:" << ENDL;
  	logger << " - Transport From: " << email.e_TransportFrom.e_Name << '<' << email.e_TransportFrom.e_Address << '>' << ENDL;
  	logger << " - Transport To: " << ENDL;
		for_each(email.e_TransportTo.begin(), email.e_TransportTo.end(), [&](auto &to) {
			logger << "\t - \"" << to.e_Name << "\" <" << to.e_Address << '>' << ENDL;
		});
  	logger << " - Subject: " << email.e_Subject << ENDL;
  	logger << " - Date: " << email.e_Date << ENDL;
  	logger << " - Message ID: " << email.e_MessageID << ENDL;
  	logger << " - Headers (Without Microsoft Bullshit): " << ENDL;

  	// ========================================
  	// Prints the arrays
  	//
  	// These print the vector data inside
  	// - of the email
  	// ========================================
  	
  	std::size_t c = 0;

  	// Loops over the headers and displays them
  	for (const EmailHeader &h : email.e_Headers)
  	{
  		logger << "\t - Header[no: " << c++ << "]: <"
  			<< h.e_Key << "> - <" << h.e_Value << ">" << ENDL;
  	}


  	// Loops over the body sections and displays them
    c = 0;
  	logger << " - Body sections: " << ENDL;
  	for (const EmailBodySection &s : email.e_BodySections)
  	{
  		logger << "\t - Body Section[no: " << c++ << ", cType: "
  			<< s.e_Type << ", cTransEnc: " 
  			<< s.e_TransferEncoding << "]: " << ENDL;
  		logger << "\t\t" << (s.e_Content.size() < 56 ? s.e_Content : s.e_Content.substr(0, 56) + " ...") << ENDL;
  	}

    // Prints the addresses, for example from and
    // - to, this is also an vector
    c = 0;
    logger << " - From: " << ENDL;
    for (const EmailAddress &address : email.e_From)
      logger << "\t - Email[no: " << c++ << "]: " 
        << address.e_Name << " | " << address.e_Address << ENDL;

    c = 0;
    logger << " - To: " << ENDL;
    for (const EmailAddress &address : email.e_To)
      logger << "\t - Email[no: " << c++ << "]: " 
        << address.e_Name << " | " << address.e_Address << ENDL;
  	logger << CLASSIC;
  }

  /**
   * Gets the current message bucket, basically
   * - the current time in milliseconds / 1000 / 1000 / 1000
   *
   * @Param {void}
   * @Return {int64_t}
   */
  int64_t FullEmail::getBucket(void)
  {
  	int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
  		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  	return now / 1000 / 1000 / 10;
  }

	/* ! DEPRECATED !
  void FullEmail::save(CassandraConnection *conn)
  {
  	const CassDataType *udtEmailAddress = nullptr;
  	const CassDataType *udtEmailBodySection = nullptr;
  	const CassDataType *udtEmailHeader = nullptr;

  	CassUserType *transportTo = nullptr;
  	CassUserType *transportFrom = nullptr;
  	CassCollection *from = nullptr;
  	CassCollection *to = nullptr;
		CassCollection *headers = nullptr;
  	CassCollection *bodySections = nullptr;

  	// ========================================
  	// Requests the keyspace meta
  	//
  	// This allows us to use custom types
  	// - because the cassandra client is
  	// - retarded, we need to do it ourselves
  	// ========================================

  	const CassSchemaMeta *schemaMeta = cass_session_get_schema_meta(conn->c_Session);
  	const CassKeyspaceMeta *keyspaceMeta = cass_schema_meta_keyspace_by_name(schemaMeta, "fannst");
  	if (keyspaceMeta == nullptr)
  		throw DatabaseException("Could not retreive custom data types");

  	udtEmailAddress = cass_keyspace_meta_user_type_by_name(keyspaceMeta, "email_address");
  	udtEmailHeader = cass_keyspace_meta_user_type_by_name(keyspaceMeta, "email_header");
  	udtEmailBodySection = cass_keyspace_meta_user_type_by_name(keyspaceMeta, "email_body_section");

  	// ========================================
  	// Sets the basic variables
  	//
  	// Sets the basic stuff such as transport
  	// - from / to
  	// ========================================

  	// Sets the transport to
		transportTo = cass_user_type_new_from_data_type(udtEmailAddress);
  	cass_user_type_set_string_by_name(transportTo, "a_address", this->e_TransportTo.e_Address.c_str());
  	cass_user_type_set_string_by_name(transportTo, "a_name", this->e_TransportTo.e_Name.c_str());

  	// Sets the transport from
  	transportFrom = cass_user_type_new_from_data_type(udtEmailAddress);
  	cass_user_type_set_string_by_name(transportFrom, "a_address", this->e_TransportFrom.e_Address.c_str());
  	cass_user_type_set_string_by_name(transportFrom, "a_name", this->e_TransportFrom.e_Name.c_str());

  	// ========================================
  	// Sets the basic variables
  	//
  	// Sets the advanced address
  	// - stuff such as transport from / to (full) 
  	// ========================================

  	// Creates the collections
  	to = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->e_To.size());
  	from = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->e_From.size());

  	// Loops over the to addresses and adds
  	// - the custom types to the collection
  	for (const EmailAddress &address : this->e_To)
  	{
  		CassUserType *toAddress = cass_user_type_new_from_data_type(udtEmailAddress);
  		cass_user_type_set_string_by_name(toAddress, "a_address", address.e_Address.c_str());
  		cass_user_type_set_string_by_name(toAddress, "a_name", address.e_Name.c_str());
  		cass_collection_append_user_type(to, toAddress);
  		cass_user_type_free(toAddress);
  	}

		// Loops over the from addresses and adds
  	// - the custom types to the collection
  	for (const EmailAddress &address : this->e_From)
  	{
  		CassUserType *fromAddress = cass_user_type_new_from_data_type(udtEmailAddress);
  		cass_user_type_set_string_by_name(fromAddress, "a_address", address.e_Address.c_str());
  		cass_user_type_set_string_by_name(fromAddress, "a_name", address.e_Name.c_str());
  		cass_collection_append_user_type(from, fromAddress);
  		cass_user_type_free(fromAddress);
  	}

  	// ========================================
  	// Sets the headers and body
  	//
  	// In our case just the headers and body
  	// - lol, this comment is just to separate
  	// - the code
  	// ========================================

  	// Creates the collections
		headers = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->e_Headers.size());
  	bodySections = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->e_BodySections.size());

  	// Loops over the headers and puts
  	// - them into the collection
  	for (const EmailHeader &header : this->e_Headers)
  	{
  		CassUserType *emailHeader = cass_user_type_new_from_data_type(udtEmailHeader);
  		cass_user_type_set_string_by_name(emailHeader, "h_key", header.e_Key.c_str());
  		cass_user_type_set_string_by_name(emailHeader, "h_value", header.e_Value.c_str());
  		cass_collection_append_user_type(headers, emailHeader);
  		cass_user_type_free(emailHeader);
  	}

  	// Loops over the body sections
  	// - and puts them into the collection
  	for (const EmailBodySection &section : this->e_BodySections)
  	{
  		CassUserType *emailSection = nullptr;
  		CassCollection *emailSectionHeaders = nullptr;

  		// Initializes the section and sets 
  		// - the basic variable
  		emailSection = cass_user_type_new_from_data_type(udtEmailBodySection);
  		cass_user_type_set_string_by_name(emailSection, "e_content", section.e_Content.c_str());
  		cass_user_type_set_int32_by_name(emailSection, "e_index", section.e_Index);
  		cass_user_type_set_int32_by_name(emailSection, "e_type", section.e_Type);
  		cass_user_type_set_int32_by_name(emailSection, "e_transfer_encoding", section.e_TransferEncoding);

  		// Loops over the headers and adds them to the
  		// - collection of the section headers
  		emailSectionHeaders = cass_collection_new(CASS_COLLECTION_TYPE_LIST, section.e_Headers.size());
  		for (const EmailHeader &header : section.e_Headers)
  		{
  			CassUserType *emailSectionHeader = cass_user_type_new_from_data_type(udtEmailHeader);
  			cass_user_type_set_string_by_name(emailSectionHeader, "h_key", header.e_Key.c_str());
  			cass_user_type_set_string_by_name(emailSectionHeader, "h_value", header.e_Value.c_str());
  			cass_collection_append_user_type(emailSectionHeaders, emailSectionHeader);
  			cass_user_type_free(emailSectionHeader);
  		}
      cass_user_type_set_collection_by_name(emailSection, "e_header", emailSectionHeaders);

  		// Frees the memory
  		cass_collection_append_user_type(bodySections, emailSection);
  		cass_user_type_free(emailSection);
  		cass_collection_free(emailSectionHeaders);
  	}

  	// ========================================
  	// Creates the statement and executes
  	//
  	// Creates the statement and adds the final
  	// - variables, then we execute
  	// ========================================

  	CassError rc;
  	CassStatement *statement = nullptr;
  	CassFuture *future = nullptr;

  	const char *query = R"(INSERT INTO fannst.full_emails (
  		e_transport_from, e_transport_to, e_from,
  		e_to, e_domain, e_subject,
  		e_message_id, e_body_sections, e_headers,
  		e_encrypted, e_date, e_owners_uuid, 
  		e_email_uuid, e_type, e_bucket
  	) VALUES (
  		?, ?, ?,
  		?, ?, ?,
  		?, ?, ?,
  		?, ?, ?,
  		?, ?, ?
  	))";

  	// Prepares the statement and then binds
  	// - the values
  	statement = cass_statement_new(query, 15);
    cass_statement_bind_user_type(statement, 0, transportFrom);
    cass_statement_bind_user_type(statement, 1, transportTo);
    cass_statement_bind_collection(statement, 2, from);
    cass_statement_bind_collection(statement, 3, to);
    cass_statement_bind_string(statement, 4, this->e_OwnersDomain.c_str());
    cass_statement_bind_string(statement, 5, this->e_Subject.c_str());
    cass_statement_bind_string(statement, 6, this->e_MessageID.c_str());
    cass_statement_bind_collection(statement, 7, bodySections);
    cass_statement_bind_collection(statement, 8, headers);
    cass_statement_bind_bool(statement, 9, (this->e_Encryped == true ? cass_true : cass_false));
    cass_statement_bind_int64(statement, 10, this->e_Date);
    cass_statement_bind_uuid(statement, 11, this->e_OwnersUUID);
    cass_statement_bind_uuid(statement, 12, this->e_EmailUUID);
    cass_statement_bind_int32(statement, 13, this->e_Type);
    cass_statement_bind_int64(statement, 14, this->e_Bucket);

  	// Executes the query, and waits for execution to finish
  	future = cass_session_execute(conn->c_Session, statement);
  	cass_future_wait(future);

  	// Frees the memory, since we don't want
  	// - to leak anything if error occurs
  	cass_statement_free(statement);
  	cass_collection_free(from);
  	cass_collection_free(to);
  	cass_collection_free(headers);
  	cass_collection_free(bodySections);
  	cass_user_type_free(transportFrom);
  	cass_user_type_free(transportTo);
  	cass_schema_meta_free(schemaMeta);

  	// Checks for errors, and throw
  	// - error message if so
  	rc = cass_future_error_code(future);
  	if (rc != CASS_OK)
  	{
  		// Gets the error message, and frees the memory
  		// - before throwing it
			const char *err = nullptr;
			std::size_t errLen;

			cass_future_error_message(future, &err, &errLen);

			std::string errString(err, errLen);
			std::string message = "cass_session_execute() failed: ";
			message += errString;

      cass_future_free(future);
			throw DatabaseException(message);
  	}

  	// Frees the memory ( if no error )
  	cass_future_free(future);
  }
	*/

  /**
   * Generates an UUID for the current message
   *
   * @Param {void}
   * @Return {void}
   */
  CassUuid FullEmail::generateMessageUUID(void)
  {
		CassUuid uuid;
    CassUuidGen *gen = cass_uuid_gen_new();
    cass_uuid_gen_time(gen, &uuid);
    cass_uuid_gen_free(gen);
		return uuid;
  }

  /**
   * Deletes one email from the database
   *
   * @Param {CassandraConnection *} cassandra
   * @Param {const std::string &} domain
   * @Param {const CassUuid &} ownersUuid
   * @Param {const CassUuid &} emailUuid;
   * @Param {const int64_t} bucket
   * @Return {void} 
   */
  void FullEmail::deleteOne(
    CassandraConnection *cassandra,
    const std::string &domain,
    const CassUuid &ownersUuid,
    const CassUuid &emailUuid,
    const int64_t bucket
  )
  {
    const char *query = "DELETE FROM fannst.full_emails WHERE e_bucket=? AND e_domain=? AND e_owners_uuid=? AND e_email_uuid=? AND e_type=?";
    CassStatement *statement = nullptr;
    CassFuture *future = nullptr;
    CassError rc;

    // Creates the statement and binds the values
    statement = cass_statement_new(query, 5);
    cass_statement_bind_int64(statement, 0, bucket);
    cass_statement_bind_string(statement, 1, domain.c_str());
    cass_statement_bind_uuid(statement, 2, ownersUuid);
    cass_statement_bind_uuid(statement, 3, emailUuid);
    cass_statement_bind_int32(statement, 4, EmailType::ET_INCOMMING);

    // Executes the query and checks for errors
    future = cass_session_execute(cassandra->c_Session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK)
    {
      std::string error = "Could not delete full email: ";
      error += CassandraConnection::getError(future);
      cass_future_free(future);
      cass_statement_free(statement);
      throw DatabaseException(error);
    }

    // Frees the memory
    cass_future_free(future);
    cass_statement_free(statement);
  }
}
