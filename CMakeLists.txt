# ! IGNORE ! Only used for CLANG

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
