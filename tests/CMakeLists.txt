cmake_minimum_required(VERSION 3.10)
project(
	poc_http_parser_tests
	LANGUAGES C CXX
	DESCRIPTION "HTTP parsers test"
)
include_directories(../)
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
