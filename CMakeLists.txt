#
#	Copyright [2020] [Luke A.C.A. Rieff]
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#	http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

cmake_minimum_required(VERSION 3.0.0)
project(fsmtp VERSION 1.0)

set (CMAKE_CXX_STANDARD 17)

file(GLOB networking_sources "lib/networking/*.cc")
file(GLOB networking_headers "lib/networking/*.h")

file(GLOB server_sources "lib/smtp/server/*.cc")
file(GLOB server_headers "lib/smtp/server/*.h")

file(GLOB general_sources "lib/general/*.cc")
file(GLOB general_headers "lib/general/*.h")

file(GLOB smtp_sources "lib/smtp/*.cc")
file(GLOB smtp_headers "lib/smtp/*.h")

file(GLOB models_sources "lib/models/*.cc")
file(GLOB models_headers "lib/models/*.h")

file(GLOB parsers_sources "lib/parsers/*.cc")
file(GLOB parsers_headers "lib/parsers/*.h")

file(GLOB workers_sources "lib/workers/*.cc")
file(GLOB workers_headers "lib/workers/*.h")

file(GLOB client_sources "lib/smtp/client/*.cc")
file(GLOB client_headers "lib/client/*.h")

file(GLOB dkim_sources "lib/dkim/*.cc")
file(GLOB dkim_headers "lib/dkim/*.h")

file(GLOB pop3_sources "lib/pop3/*.cc")
file(GLOB pop3_headers "lib/pop3/*.h")

set(sources "main.cc")
set(headers "main.h")
set(args "-lpthread -lssl -lresolv -lcrypto -lcassandra -lcurses -lhiredis")

add_executable(
	${PROJECT_NAME}

	${headers}
	${sources}

	${workers_sources}
	${workers_headers}

	${general_headers}
	${general_sources}

	${server_sources}
	${server_headers}

	${smtp_headers}
	${smtp_sources}

	${networking_headers}
	${networking_sources}

	${models_sources}
	${models_headers}

	${parsers_sources}
	${parsers_headers}

	${client_sources}
	${client_headers}

	${dkim_sources}
	${dkim_headers}

	${pop3_sources}
	${pop3_headers}
)

target_link_libraries(${PROJECT_NAME} ${args})