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

#include "Global.src.h"

using namespace FSMTP;

Json::Value _global_config;

void Global::configure() {
	// Loads the things openssl needs, idk why they did it like this
	//  but it is required
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	// Disables cassandra's anoying warnings, which we do not
	//  care about, and only see the critical ones now
	cass_log_set_level(CASS_LOG_CRITICAL);
}

void Global::readConfig(const char *config, const char *fallbackConfig) {
	// Checks if the specified config file exists, if not
	//  use the fallback one, which will allow immediate
	//  run after build.
  const char  *finalConfig = nullptr;
  if (filesystem::exists(config)) finalConfig = config;
	else finalConfig = fallbackConfig;

	// Creates the binary stream and reads the config
	//  file, either the normal or fallback one
	ifstream stream(finalConfig, ios::binary);
	DEFER(stream.close());
	stream >> _global_config;
}

Json::Value &Global::getConfig() noexcept {
	return _global_config;
}

unique_ptr<CassandraConnection> Global::getCassandra() {
	const Json::Value &conf = _global_config;

	return make_unique<CassandraConnection>(
		conf["database"]["cassandra_hosts"].asCString(),
		conf["database"]["cassandra_username"].asCString(),
		conf["database"]["cassandra_password"].asCString()
	);
}
unique_ptr<RedisConnection> Global::getRedis() {
	const Json::Value &conf = _global_config;

	return make_unique<RedisConnection>(
		conf["database"]["redis_hosts"].asCString(),
		conf["database"]["redis_port"].asInt()
	);
}

unique_ptr<SSLContext> Global::getSSLContext(const SSL_METHOD *method) {
	const Json::Value &conf = _global_config;

	const char *key = conf["ssl_key"].asCString();
	const char *cert = conf["ssl_cert"].asCString();
	const char *pass = conf["ssl_pass"].asCString();
	const char *bundle = conf["ssl_bundle"] ? conf["ssl_bundle"].asCString() : nullptr;

	return make_unique<SSLContext>(method, key, cert, pass, bundle);
}