cmake_minimum_required(VERSION 3.10)
project(
	poc_http_parser_tests
	LANGUAGES C CXX
	DESCRIPTION "HTTP parsers test"
)
include_directories(../)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(
	SOURCE_FILES
	test-http-parser-state-machine.cpp
)
add_subdirectory(vendor/googletest)
add_executable(
	${PROJECT_NAME}
	${SOURCE_FILES}
)
target_link_libraries(
	${PROJECT_NAME}
	gtest_main
)
