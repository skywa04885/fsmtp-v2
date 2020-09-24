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

namespace FSMTP::ARG_ACTIONS {
  /**
   * Registers an new domain available for usage
   */
  void addDomain() {
    Logger logger("DOMAIN_ADDER", LoggerLevel::INFO);

    // Asks the admin for the domain name which needs to be added to the database
    //  we do not have a default one tho
    string domain2add;
    logger << "Enter domain: " << FLUSH;
    getline(cin, domain2add);

    // Connects to apache cassandra and redis, so we can store the domain in permanent
    //  storage and the cache
    unique_ptr<CassandraConnection> cassandra;
    try {
      cassandra = Global::getCassandra();
    } catch (const runtime_error &e) {
      logger << FATAL << "Could not connect to cassandra: " << e.what() << ENDL << CLASSIC;
      exit(-1);
    }

    unique_ptr<RedisConnection> redis;
    try {
      redis = Global::getRedis();
    } catch (const runtime_error &e) {
      logger << FATAL << "Could not connect to redis: " << e.what() << ENDL << CLASSIC;
      exit(-1);
    }

    // Creates the local domain instance, and stores it
    LocalDomain localDomain(domain2add);
    localDomain.saveRedis(redis.get());
    localDomain.saveCassandra(cassandra);

    exit(0);
  }

  void testArgAction(void) {
    Logger logger("test", LoggerLevel::INFO);

    size_t successes, failures;
    successes = failures = 0;
    
    auto failure = [&](const string &message, const string &error) {
      logger << ERROR << _BASH_FAIL_MARK << message << ": " << error << ENDL << CLASSIC;
      ++failures;
    };

    auto success = [&](const string &message) {
      logger << _BASH_SUCCESS_MARK << message << ENDL;
      ++successes;
    };

    // =======================
    // Database tests
    // =======================

    try {
      auto cass = Global::getCassandra();
      success("Connected to cassandra");
    } catch (const runtime_error &err) {
      failure("Could not connect to cassandra", err.what());
    }

    try {
      auto redis = Global::getRedis();
      success("Connected to redis");
    } catch (const runtime_error &err) {
      failure("Could not connect to redis", err.what());
    }

    // =======================
    // Socket tests
    // =======================

    unique_ptr<SSLContext> ctx;
    try {
      ctx = Global::getSSLContext(SSLv23_server_method());
      success("Created new SSLContext");
    } catch (const runtime_error &err) {
      failure("Could not create SSLContext", err.what());
    }

    try {
      ServerSocket().queue(250).listenServer(25);
      success("Created plain server socket");
    } catch (const runtime_error &err) {
      failure("Could not create plain server socket", err.what());
    }

    try {
      ServerSocket().useSSL(ctx.get()).queue(250).listenServer(587);
      success("Created SSL socket server socket");
    } catch (const runtime_error &err) {
      failure("Could not create SSL server socket", err.what());
    }

    int32_t ports[] = {25, 587, 110, 143, 995, 993};
    for (int32_t port : ports) {
      string message = "port ";
      message += to_string(port);
      
      try {
        ServerSocket().queue(250).listenServer(25);
        
        message += " is available";
        success(message);
      } catch (const runtime_error &err) {
        message += " is not available";
        failure(message, err.what());
      }
    }

    // =======================
    // Final message
    // =======================

    logger << successes << '/' << successes + failures << " tests completed successfully." << ENDL;

    exit(0);
  }

  void mailTestArgAction(void) {
    Logger logger("Mail", LoggerLevel::INFO);
    MailComposerConfig mailComposerConfig;

    // Gets the user inputs for the from, to etcetera.
    //  after this we put default values if the user specified
    //  the default option

    string type, from, to, subject;

    logger << "Enter type [c: custom, d: default]: " << FLUSH;
    getline(cin, type);
    if (type[0] != 'd') {
      logger << "Enter subject: " << FLUSH;
      getline(cin, subject);
      logger << "Enter from: " << FLUSH;
      getline(cin, from);
      logger << "Enter to: " << FLUSH;
      getline(cin, to);

      mailComposerConfig.m_To.push_back(EmailAddress(to));
      mailComposerConfig.m_From.push_back(EmailAddress(from));
      mailComposerConfig.m_Subject = subject;
    } else {
      mailComposerConfig.m_From.push_back(EmailAddress("webmaster", "webmaster@fannst.nl"));
      mailComposerConfig.m_To.push_back(EmailAddress("Contabo Support", "support@contabo.com"));
      mailComposerConfig.m_Subject = "Testing TLS, since your system is outdated, great test";
    }

    // Sends the email to the target server, if this fails
    //  we print an error message to the console

    try {
      SMTPClient client(true);
      client.prepare(mailComposerConfig).beSocial();
      logger << ERROR << "Errorlog: " << client.s_ErrorLog << ENDL << CLASSIC;
    } catch (const runtime_error &e) {
      logger << FATAL << "Could not send email: " << e.what() << ENDL << CLASSIC;
    }

    exit(0);
  }
}
