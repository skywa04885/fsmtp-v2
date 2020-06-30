cmake_minimum_required(VERSION 3.0.0)
project(fsmtp VERSION 1.0)

file(GLOB networking_sources "lib/networking/*.cc")
file(GLOB networking_headers "lib/networking/*.h")

file(GLOB server_sources "lib/server/*.cc")
file(GLOB server_headers "lib/server/*.h")

file(GLOB general_sources "lib/general/*.cc")
file(GLOB general_headers "lib/general/*.h")

file(GLOB smtp_sources "lib/smtp/*.cc")
file(GLOB smtp_headers "lib/smtp/*.h")

file(GLOB models_sources "lib/models/*.cc")
file(GLOB models_headers "lib/models/*.h")

set(sources "main.cc")
set(headers "main.h")
set(args "-lpthread -lssl -lcrypto")

add_executable(
	${PROJECT_NAME}

	${headers}
	${sources}

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
)

target_link_libraries(${PROJECT_NAME} ${args})