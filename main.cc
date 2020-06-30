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

int main(const int argc, const char **argv)
{
	int32_t opts = 0x0;

	opts |= _SERVER_OPT_ENABLE_AUTH;
	opts |= _SERVER_OPT_ENABLE_TLS;

	SMTPServer server(3000, true, opts);

	std::cin.get();
	server.shutdown();

	return 0;
}