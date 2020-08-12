CMAKE_MINiMUM_REQUIRED(VERSION 3.10)

project(fsmtp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE sources "lib/*.src.cc")
file(GLOB_RECURSE headers "lib/*.src.h")

add_executable(
	${PROJECT_NAME}

	main.cc
	main.h

	${sources}
	${headers}
)

target_link_libraries(${PROJECT_NAME} -lssl -lcassandra -ljsoncpp -lresolv -lcrypto -lhiredis -lpthread)