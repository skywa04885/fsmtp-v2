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

set(sources "main.cc")
set(headers "main.h")
set(args "-lpthread")

add_library(
	networking STATIC

	${networking_headers}
	${networking_sources}
)

add_library(
	smtp STATIC

	${smtp_headers}
	${smtp_sources}
)


add_library(
	server STATIC

	${server_sources}
	${server_headers}
)

add_library(
	general STATIC

	${general_sources}
	${general_headers}
)

add_executable(
	${PROJECT_NAME}

	${headers}
	${sources}
)

target_link_libraries(${PROJECT_NAME} networking general server smtp ${args})