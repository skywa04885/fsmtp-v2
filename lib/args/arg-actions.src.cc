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

#include "arg-actions.src.h"

namespace FSMTP::ARG_ACTIONS
{
  /**
   * Performs an default set of tests, which will check if the Server
   * - is capable of connecting to the database etcetera
   *
   * @Param {void}
   * @Return {void}
   */
  void testArgAction(void)
  {
    std::size_t fail = 0, total = 4;
    Logger logger("test", LoggerLevel::INFO);
    logger << "FSMTP gaat nu beginnen met testen !" << ENDL;

    // ===================================
    // Performs database Checks
    //
    // Checks if the server can connect
    // - to redis and cassandra
    // ===================================

    // Performs attemt to connect with cassandra
    logger << _BASH_UNKNOWN_MARK << "Start van Apache Cassandra || Datastax Enterprise database verbindings test ..." << ENDL;
    std::unique_ptr<CassandraConnection> cassandra;
    try {
      cassandra = std::make_unique<CassandraConnection>(_CASSANDRA_DATABASE_CONTACT_POINTS);
      logger << _BASH_SUCCESS_MARK << "Verbinding met Apache Cassandra || Datastax Enterprise is geslaagd !" << ENDL;
    }
    catch (const std::runtime_error &e)
    {
      logger << _BASH_FAIL_MARK << "Verbinding met Cassandra || Datastax Enterprise is niet gelukt" << ENDL;
      fail++;
    }

    // Creates the redis client
    logger << _BASH_UNKNOWN_MARK << "Start van de Redis verbindings test ..." << ENDL;
    std::unique_ptr<RedisConnection> redis;
    try {
      redis = std::make_unique<RedisConnection>(_REDIS_CONTACT_POINTS, _REDIS_PORT);
      logger << _BASH_SUCCESS_MARK << "Verbonden met Redis" << ENDL;
    } catch (const std::runtime_error &e)
    {
      logger << _BASH_FAIL_MARK << "Verbinding met Redis is niet gelukt" << ENDL;
      fail++;
    }

    // ===================================
    // Performs hosting checks
    //
    // Checks if the server can listen
    // - on the specified ports
    // ===================================

    // Performs the check if we may create an socket
    // - this will throw an error if we're not allowed to do it
    logger << _BASH_UNKNOWN_MARK << "Start van server socket test ..." << ENDL;
    std::unique_ptr<SMTPServer> smtpServer;
    try {
      smtpServer = std::make_unique<SMTPServer>(25, true, _REDIS_PORT, _REDIS_CONTACT_POINTS);
      logger << _BASH_SUCCESS_MARK << "SMTPServer successvol" << ENDL;
      smtpServer->shutdownServer();
    }
    catch (const std::runtime_error &e)
    {
      logger << _BASH_FAIL_MARK << "Kon SMTP Server niet opstarten:" << e.what() << ENDL;
      fail++;
    }

    // Performs the check if we may create an pop3 server
    logger << _BASH_UNKNOWN_MARK << "Start van POP3 Server socket test ..." << ENDL;
    std::unique_ptr<POP3::P3Server> pop3server;
    try {
      pop3server = std::make_unique<POP3::P3Server>(false);
      logger << _BASH_SUCCESS_MARK << "POP3 Server successvol" << ENDL;
      pop3server->shutdown();
    } catch (const SocketInitializationException &e)
    {
      logger << _BASH_FAIL_MARK << "Kon de POP3 server niet opstarten: " << e.what() << ENDL;
      fail++;
    }

    // Prints the errors versus the total
    if (fail > 0) logger << _BASH_FAIL_MARK << fail << '/' << total << " zijn gefaald, probeer deze op te lossen voordat u de server start." << ENDL;
    else logger << _BASH_SUCCESS_MARK << total << '/' << total << " zijn geslaagd, u kunt de server starten !" << ENDL;

    // Closes the application since the tests are finished
    std::exit(0);
  }

  /**
   * Adds an user to the database
   *
   * @Param {void}
   * @Return {void}
   */
  void addUserArgAction(void)
  {
    Logger logger("AccountCreator", LoggerLevel::INFO);
    std::string username, domain, password, fullName;

    logger << "[Gebruiker aanmaken]: " << ENDL;

    // Requests the required information about the user
    // - then we will create and store the user
    logger << "Vul naam in: " << FLUSH;
    std::getline(std::cin, fullName);
    logger << "Vul gebruikersnaam in: " << FLUSH;
    std::getline(std::cin, username);
    logger << "Vul domein in: " << FLUSH;
    std::getline(std::cin, domain);
    logger << "Vul wachtwoord in: " << FLUSH;
    std::getline(std::cin, password);

    // Prints that the user is being created, and
    // - hashes the password
    std::string hash = passwordHash(password);
    logger << "Wachtwoord hash aangemaakt: " << hash << ENDL;

    // Prints the message
    logger << "Gebruiker wordt aangemaakt, een ogenblik gedult ..." << ENDL;

    // Connects to cassandra
    logger << _BASH_UNKNOWN_MARK << "Verbinding maken met Apache Cassandra ..." << ENDL;
    std::unique_ptr<CassandraConnection> cassandra;
    try {
      cassandra = std::make_unique<CassandraConnection>(_CASSANDRA_DATABASE_CONTACT_POINTS);
      logger << _BASH_SUCCESS_MARK << "Verbonden met apache cassandra !" << ENDL;
    } catch (const std::runtime_error &e)
    {
      logger << _BASH_FAIL_MARK << "Kon geen verbinding maken met Cassandra: " << e.what() << ENDL;
      std::exit(-1);
    }

    // Connects to Redis
    logger << _BASH_UNKNOWN_MARK << "Verbinding maken met Redis ..." << ENDL;
    std::unique_ptr<RedisConnection> redis;
    try {
      redis = std::make_unique<RedisConnection>(_REDIS_CONTACT_POINTS, _REDIS_PORT);
      logger << _BASH_SUCCESS_MARK << "Verbonden met Redis !" << ENDL;
    } catch (const std::runtime_error &e)
    {
      logger << _BASH_FAIL_MARK << "Kon geen verbinding maken met redis: " << e.what() << ENDL;
      std::exit(-1);
    }

    // Creates the user and adds the asked variables
    // - into it
    Account account;
    account.a_Bucket = Account::getBucket();
    account.a_Username = username;
    account.a_Password = hash;
    account.a_Domain = domain;
    account.a_FullName = fullName;

    // Generates an random user id for the user
    // - and stores it
    CassUuidGen *gen = cass_uuid_gen_new();
    cass_uuid_gen_time(gen, &account.a_UUID);
    cass_uuid_gen_free(gen);

    // Generates an new RSA keypair for the user, and prints
    // - it to the console
    account.generateKeypair();
    logger << "Keypair generated ..." << ENDL;
    logger << "RSA 2048 public: \033[41m\r\n" << account.a_RSAPublic << "\033[0m" << ENDL;
    logger << "RSA 2048 private: \033[41m\r\n" << account.a_RSAPrivate << "\033[0m" << ENDL;

    // Encrypts the rsa private key
    account.a_RSAPrivate = AES256::encrypt(account.a_RSAPrivate, password);

    // Saves the full account in cassandra, and prints the message
    account.save(cassandra.get());
    logger << "Account stored in cassandra" << ENDL;

    // Creates the account shortcut
    AccountShortcut accountShortcut(
      account.a_Bucket,
      account.a_Domain,
      account.a_Username,
      account.a_UUID
    );
    accountShortcut.save(cassandra.get());
    accountShortcut.saveRedis(redis.get());

    try
    {
      // Adds the mailboxes
      Mailbox inbox(
        account.a_Bucket, account.a_Domain, account.a_UUID,
        "INBOX", true,
        0,
        0, true
      );
      inbox.save(cassandra.get());

      Mailbox sent(
        account.a_Bucket, account.a_Domain, account.a_UUID,
        "INBOX.Sent", true,
        0,
        _MAILBOX_FLAG_UNMARKED | _MAILBOX_FLAG_SENT, true
      );
      sent.save(cassandra.get());

      Mailbox spam(
        account.a_Bucket, account.a_Domain, account.a_UUID,
        "INBOX.Spam",
        true,
        0,
        _MAILBOX_FLAG_MARKED | _MAILBOX_FLAG_JUNK, true
      );
      spam.save(cassandra.get());

      Mailbox archive(
        account.a_Bucket, account.a_Domain, account.a_UUID,
        "INBOX.Archive",
        true,
        0,
        _MAILBOX_FLAG_UNMARKED | _MAILBOX_FLAG_ARCHIVE, true
      );
      archive.save(cassandra.get());

      Mailbox drafts(
        account.a_Bucket, account.a_Domain, account.a_UUID,
        "INBOX.Drafts",
        true,
        0,
        _MAILBOX_FLAG_UNMARKED | _MAILBOX_FLAG_DRAFT, true
      );
      drafts.save(cassandra.get());

      Mailbox trash(
        account.a_Bucket, account.a_Domain, account.a_UUID,
        "INBOX.Trash",
        true,
        0,
        _MAILBOX_FLAG_MARKED | _MAILBOX_FLAG_TRASH, true
      );
      trash.save(cassandra.get());
    } catch (const DatabaseException &e)
    {
      logger << FATAL << "Could not create mailboxes: " << e.what() << ENDL;
    }

    std::exit(0);
  }

  /**
   * Syncs cassandra with redis
   *
   * @Param {void}
   * @Return {void}
   */
  void syncArgAction(void)
  {
    char uuidBuffer[64];

    // ====================================
    // Connects to database
    //
    // Just connects to redis and Cassandra
    // - so our stuff will be synced
    // ====================================

    // Creates the logger and starts
    Logger logger("DBSync", LoggerLevel::INFO);
    logger << "FSMTP Gaat nu Redis en Cassandra synchroniseren" << ENDL;

    // Creates the redis client
    std::unique_ptr<RedisConnection> redis;

    logger << "Verbinding maken met Redis ..." << ENDL;
    try {
      redis = std::make_unique<RedisConnection>(_REDIS_CONTACT_POINTS, _REDIS_PORT);
    } catch (const std::runtime_error &e)
    {
      logger << FATAL << "Kon geen verbinding met Redis maken: " << e.what() << ENDL;
      std::exit(-1);
    }

    logger << "Verbinding met Redis gemaakt !" << ENDL;

    // Connects to apache cassandra
    logger << "Verbinding maken met Apache Cassandra || Datastax Cassandra ..." << ENDL;

    std::unique_ptr<CassandraConnection> cassandra;
    try {
      cassandra = std::make_unique<CassandraConnection>(_CASSANDRA_DATABASE_CONTACT_POINTS);
    } catch (const std::runtime_error &e)
    {
      logger << FATAL << "Kon geen verbinding met Cassandra maken: " << e.what() << ENDL;
      std::exit(-1);
    }

    logger << "Verbinding met Apache Cassandra || Datastax Cassandra gemaakt !" << ENDL;

    // ====================================
    // Starts synchronizing the domains
    //
    // These are quick checks if user
    // - is on our server
    // ====================================

    // Gets all the domains, loops over them and adds them
    // - to the redis database
    std::vector<LocalDomain> domains = LocalDomain::findAllCassandra(cassandra.get());
    for (LocalDomain &domain : domains)
    {
      // Performs the command and idk why but
      // - we need to reinterpret the reply, since
      // - it is an void pointer
      std::string command = "SET domain:";
      command += domain.l_Domain;
      command += ' ';

      cass_uuid_string(domain.l_UUID, uuidBuffer);
      command += uuidBuffer;

      DEBUG_ONLY(logger << DEBUG
        << "Uitvoeren van: '" << command << '\'' << ENDL << CLASSIC);

      redisReply *reply = reinterpret_cast<redisReply *>(
        redisCommand(redis->r_Session, command.c_str())
      );
      if (reply->type == REDIS_REPLY_ERROR)
      {
        // Gets the error message and throws it
        std::string message = "redisCommand() failed: ";
        message += std::string(reply->str, reply->len);
        throw std::runtime_error(message);
      }

      // Frees the reply object and continues
      freeReplyObject(reply);
    }

    // ====================================
    // Starts synchronizing the users
    //
    // Allows the server to check if
    // - and user is on our server or not
    // - and what is uuid is
    // ====================================

    {
      CassStatement *statement = nullptr;
      cass_bool_t hasMorePages = cass_false;
      const char *query = "SELECT * FROM fannst.account_shortcuts";

      // Prepares the statement, and sets the paging size
      statement = cass_statement_new(query, 0);
      cass_statement_set_paging_size(statement, 50);

      // Starts performing the query, and inserting them
      // - into redis
      do
      {
        CassError rc;
        CassIterator *iterator = nullptr;
        CassFuture *future = nullptr;

        // Executes the query, and checks for any errors
        future = cass_session_execute(cassandra->c_Session, statement);
        cass_future_wait(future);

        rc = cass_future_error_code(future);
        if (rc != CASS_OK)
        {
          std::string message = "cass_session_execute() failed: ";
          message += CassandraConnection::getError(future);

          cass_future_free(future);
          cass_statement_free(statement);
          throw DatabaseException(message);
        }

        // Gets the result and creates the iterator,
        // - after that we start looping over the users
        const CassResult *result = cass_future_get_result(future);
        iterator = cass_iterator_from_result(result);

        while (cass_iterator_next(iterator))
        {
          const char *username = nullptr;
          std::size_t usernameLen;
          const char *domain = nullptr;
          std::size_t domainLen;
          CassUuid uuid;
          int64_t bucket;

          // Gets the row, and then gets the values
          // - that we store in the memory
          const CassRow *row = cass_iterator_get_row(iterator);
          cass_value_get_string(
            cass_row_get_column_by_name(row, "a_username"),
            &username,
            &usernameLen
          );
          cass_value_get_string(
            cass_row_get_column_by_name(row, "a_domain"),
            &domain,
            &domainLen
          );
          cass_value_get_uuid(
            cass_row_get_column_by_name(row, "a_uuid"),
            &uuid
          );
          cass_value_get_int64(
            cass_row_get_column_by_name(row, "a_bucket"),
            &bucket
          );

          // Gets the UUID string
          cass_uuid_string(uuid, uuidBuffer);

          // Prepares the redis command
          std::string command = "HMSET acc:";
          command += std::string(username, usernameLen);
          command += '@';
          command += std::string(domain, domainLen);
          command += " v1 ";
          command += std::to_string(bucket);
          command += " v2 ";
          command += uuidBuffer;

          DEBUG_ONLY(logger << DEBUG << "Uitvoeren van: '" << command
            << "' op Redis Server" << ENDL << CLASSIC);

          // Performs the command on the redis server,
          // - and throws error if not working idk
          redisReply *reply = reinterpret_cast<redisReply *>(
            redisCommand(redis->r_Session, command.c_str())
          );
          if (reply->type == REDIS_REPLY_ERROR)
          {
            // Gets the error message and throws it
            std::string message = "redisCommand() failed: ";
            message += std::string(reply->str, reply->len);
            throw std::runtime_error(message);
          }
        }

        // Frees the round memory
        cass_future_free(future);
        cass_statement_free(statement);
        cass_result_free(result);
        cass_iterator_free(iterator);
      } while(hasMorePages);
    }

    std::exit(0);
  }

  /**
   * Sends an email custom or default
   *
   * @Param {void}
   * @Return {void}
   */
  void mailTestArgAction(void)
  {
    Logger logger("Mail", LoggerLevel::INFO);
    MailComposerConfig mailComposerConfig;

    // ==================================
    // Gets the user input
    //
    // Stuff like subject and target
    // ==================================

    std::string type, from, to, subject;

    logger << "Enter type [c: custom, d: default]: " << FLUSH;
    std::getline(std::cin, type);
    if (type[0] != 'd')
    {
      logger << "Enter subject: " << FLUSH;
      std::getline(std::cin, subject);
      logger << "Enter from: " << FLUSH;
      std::getline(std::cin, from);
      logger << "Enter to: " << FLUSH;
      std::getline(std::cin, to);

      mailComposerConfig.m_To.push_back(EmailAddress(to));
      mailComposerConfig.m_From.push_back(EmailAddress(from));
      mailComposerConfig.m_Subject = subject;
    } else
    {
      mailComposerConfig.m_From.push_back(EmailAddress("Luke Rieff", "luke.rieff@gmail.com"));
      mailComposerConfig.m_To.push_back(EmailAddress("Luke Rieff", "lr@missfunfitt.com"));
      mailComposerConfig.m_Subject = "Test email";
    }

    // ==================================
    // Sends the email
    //
    // Sends the email with the signature
    // - etcetra
    // ==================================

    mailComposerConfig.m_Headers.push_back(EmailHeader{"X-Test", "true"});
    SMTPClient client(false);
    client.prepare(mailComposerConfig);
    client.beSocial();

    std::exit(0);
  }
}
