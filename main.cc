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

#include "main.h"

bool _forceLoggerNCurses = false;

int main(const int argc, const char **argv)
{
	// Initializes OpenSSL stuff
	SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();

	// Parses the arguments into an vector after this
	// - we define the anonymous function which will compare
	// - an string and an argument, this will allow all the possible
	// - formats such as -<A> --<A> -<A[0]>,
	// - after this we start an loop which checks for specific args
	std::vector<std::string> args(argv, argv+argc);
	auto compareArg = [](const std::string &a, const std::string &b)
	{
		if (a == "--" + b || a == "-" + b.substr(0, 1)) return true;
		else return false;	
	};
	for (const std::string &arg : args)
	{
		if (compareArg(arg, "help"))
		{
			std::cout << "Gebruik: " << std::endl;
			std::cout << "sudo fsmtp [arguments]" << std::endl;

			std::cout << std::endl << "Opdrachten: " << std::endl;
			std::cout << "-h, --help: " << "\tPrint de lijst met beschikbare opdrachten." << std::endl;
			std::cout << "-t, --test: " << "\tVoer tests uit op de vitale functies van de server, zoals database verbinding." << std::endl;
			std::cout << "-s, --sync: " << "\tSynchroniseerd de redis database met die van cassandra" << std::endl;
			std::cout << "-n, --ncurses: " << "\tGebruik NCurses in plaats van klassieke terminal." << std::endl;
			std::cout << "-m, --mailtest: " << "\tVerstuurd een test email." << std::endl; 
			return 0;
		}

		if (compareArg(arg, "test"))
		{
			// Creates the logger and starts performing the tests
			Logger logger("test", LoggerLevel::INFO);
			logger << "FSMTP gaat nu beginnen met testen !" << ENDL;

			// Performs the database connection tests, so we 
			// - can check if we may reach apache cassandra
			logger << _BASH_UNKNOWN_MARK << "Start van Apache Cassandra || Datastax Enterprise database verbindings test ..." << ENDL;
			try { CassandraConnection connection(_CASSANDRA_DATABASE_CONTACT_POINTS); }
			catch (const std::runtime_error &e)
			{
				// Closes since the database connections messes everything up if we wont
				return -1;
			}
			logger << _BASH_SUCCESS_MARK << "Verbinding met Apache Cassandra || Datastax Enterprise is geslaagd !" << ENDL;

			// Creates the redis client
			logger << _BASH_UNKNOWN_MARK << "Start van de Redis verbindings test ..." << ENDL;
			std::unique_ptr<RedisConnection> redis;
			try {
				redis = std::make_unique<RedisConnection>(_REDIS_CONTACT_POINTS, _REDIS_PORT);
			} catch (const std::runtime_error &e)
			{
				logger << _BASH_FAIL_MARK << "Verbinding met Redis is niet gelukt" << ENDL;
			}
			logger << _BASH_SUCCESS_MARK << "Verbonden met Redis" << ENDL;

			// Performs the check if we may create an socket
			// - this will throw an error if we're not allowed to do it
			logger << _BASH_UNKNOWN_MARK << "Start van server socket test ..." << ENDL;
			try { SMTPSocket socket(25); }
			catch (const std::runtime_error &e)
			{
				logger << _BASH_FAIL_MARK << "Het starten en stoppen van een server socket is fout gegaan, probeer de server met 'sudo' te starten." << ENDL;
			}
			logger << _BASH_SUCCESS_MARK << "Het starten en stoppen van een server socket is geslaagd !" << ENDL;

			// Closes the application since the tests are finished
			return 0;
		}

		if (compareArg(arg, "sync"))
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
				return -1;
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
				return -1;
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


			return 0;
		}

		if (compareArg(arg, "ncurses"))
		{
			_forceLoggerNCurses = true;
		}

		if (compareArg(arg, "mailtest"))
		{
			MailComposerConfig config;
			config.m_Subject = "Hello";
			config.m_To.emplace_back("Example", "test@gmail.com");
			config.m_From.emplace_back("Test", "test@fannst.nl");
			SMTPClient client(false);
			client.prepare(config);
			client.beSocial();

			return 0;
		}
	}

	// =====================================
	// Initializes NCursesDisplay
	//
	// This is an pretty replacement for the
	// - default console
	// =====================================

	if (_forceLoggerNCurses)
	{
		NCursesDisplay::init();
		NCursesDisplay::setStatus(NCursesDisplayStatus::NDS_STARTING);
	}

	// =====================================
	// Starts the workers
	//
	// Due to memory leaks, certain
	// - tasks will be performed in workers
	// - such as Cassamdra access
	// =====================================

	// Creates and starts the database worker
	std::unique_ptr<DatabaseWorker> dbWorker = std::make_unique<DatabaseWorker>(_CASSANDRA_DATABASE_CONTACT_POINTS);
	if (!dbWorker->start(nullptr))
		std::exit(-1);

	std::unique_ptr<TransmissionWorker> transWorker = std::make_unique<TransmissionWorker>(_CASSANDRA_DATABASE_CONTACT_POINTS);
	if (!transWorker->start(nullptr))
		std::exit(-1);

	// =====================================
	// Creates the SMTP Server itself
	//
	// Sets the config and starts
	// - the smtp server
	// =====================================

	// Runs the server
	Logger logger("Main", LoggerLevel::INFO);

	int32_t opts = 0x0;

	opts |= _SERVER_OPT_ENABLE_AUTH;
	opts |= _SERVER_OPT_ENABLE_TLS;

	SMTPServer server(25, true, opts, _REDIS_PORT, _REDIS_CONTACT_POINTS);

	if (_forceLoggerNCurses)
	{
		NCursesDisplay::setStatus(NCursesDisplayStatus::NDS_RUNNING);
		NCursesDisplay::listenForQuit();
		NCursesDisplay::setStatus(NCursesDisplayStatus::NDS_SHUTDOWN);
	}
	else std::cin.get();

	server.shutdownServer();
	dbWorker->stop();
	NCursesDisplay::die();

	return 0;
}
