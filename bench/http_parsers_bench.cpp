#include "../http_SIMD_zero_mem_alloc_parser.h"
#include "../http_parser_meth_two.h"
#include "../http_state_machine.h"
#include "../http_zero_mem_alloc_parser.h"
#include <benchmark/benchmark.h>
#include <cstring>

static void Http_State_Machine_Parser(benchmark::State &state) {
	const char *sample_get_request =
	    "GET /some_page.php HTTP/1.1\r\n"
	    "Host: www.somesite.ca\r\n"
	    "User-Agent: Mozilla/5.0 (Windows NT 6.1 31.0) Gecko/20100101 "
	    "Firefox/31.0\r\n"
	    "Accept: "
	    "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
	    "*;q=0.8\r\n"
	    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"
	    "Keep-Alive: 115\r\n"
	    "Cookie: some_inter_key=2; "
	    "some_internal_value_php=LJldkfjalkasdfl;\r\n"
	    "X-Powered-By: Proxygen/FB-CXX\r\n"
	    "Connection: keep-alive\r\n\r\n";
	size_t sample_request_size = strlen(sample_get_request);
	size_t num_header = 15;
	size_t message_body_buffer_size = 2048;
	size_t request_resource_buffer_size = 1024;
	size_t http_version_buffer_size = 10;
	size_t http_method_buffer_size = 10;
	for (auto _ : state) {
		bool is_protocol_failur = false;
		ParserState_sm parser_state = REQUEST_LINE_BEGIN_sm;
		poc_HTTP_Request_Message_sm *http_request_message =
		    poc_allocate_http_request_message_sm(
			num_header, message_body_buffer_size,
			request_resource_buffer_size, http_version_buffer_size,
			http_method_buffer_size);
		bool return_parser = poc_http_state_machine_parser_sm(
		    http_request_message, &parser_state, sample_get_request,
		    sample_request_size, &is_protocol_failur);
		benchmark::DoNotOptimize(is_protocol_failur);
		benchmark::DoNotOptimize(parser_state);
		benchmark::DoNotOptimize(http_request_message);
		benchmark::DoNotOptimize(return_parser);
		poc_free_http_request_message_sm(http_request_message);
	}
}

static void Http_Parsing_Method_Two(benchmark::State &state) {
	const char *sample_get_request =
	    "GET /some_page.php HTTP/1.1\r\n"
	    "Host: www.somesite.ca\r\n"
	    "User-Agent: Mozilla/5.0 (Windows NT 6.1 31.0) Gecko/20100101 "
	    "Firefox/31.0\r\n"
	    "Accept: "
	    "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
	    "*;q=0.8\r\n"
	    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"
	    "Keep-Alive: 115\r\n"
	    "Cookie: some_inter_key=2; "
	    "some_internal_value_php=LJldkfjalkasdfl;\r\n"
	    "X-Powered-By: Proxygen/FB-CXX\r\n"
	    "Connection: keep-alive\r\n\r\n";
	size_t sample_request_size = strlen(sample_get_request);
	for (auto _ : state) {
		poc_Buffer_mt *http_body_buffer =
		    poc_allocate_http_buffer_mt(2048);
		poc_Header_mt *http_header = poc_allocate_http_header_mt(10);
		poc_HTTP_Version_mt http_version;
		poc_HTTP_Method_mt http_method;
		char *request_resource_buffer = (char *)calloc(1, 1024);
		poc_Parser_Error_mt http_parser_error;
		bool parser_return = poc_http_parser_mt(
		    http_body_buffer, http_header, sample_get_request,
		    sample_request_size, &http_method, request_resource_buffer,
		    &http_version, &http_parser_error);
		poc_free_buffer_mt(http_body_buffer);
		poc_free_http_header_mt(http_header);
		benchmark::DoNotOptimize(http_body_buffer);
		benchmark::DoNotOptimize(http_header);
		benchmark::DoNotOptimize(http_version);
		benchmark::DoNotOptimize(http_method);
		benchmark::DoNotOptimize(request_resource_buffer);
		benchmark::DoNotOptimize(http_parser_error);
		benchmark::DoNotOptimize(parser_return);
		free(request_resource_buffer);
	}
}

static void
Http_Zero_Dynamic_Memory_Allocation_Parser(benchmark::State &state) {
	char *sample_get_request =
	    "GET /some_page.php HTTP/1.1\r\n"
	    "Host: www.somesite.ca\r\n"
	    "User-Agent: Mozilla/5.0 (Windows NT 6.1 31.0) Gecko/20100101 "
	    "Firefox/31.0\r\n"
	    "Accept: "
	    "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
	    "*;q=0.8\r\n"
	    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"
	    "Keep-Alive: 115\r\n"
	    "Cookie: some_inter_key=2; "
	    "some_internal_value_php=LJldkfjalkasdfl;\r\n"
	    "X-Powered-By: Proxygen/FB-CXX\r\n"
	    "Connection: keep-alive\r\n\r\n";
	size_t sample_request_size = strlen(sample_get_request);
	size_t total_header = 15;
	for (auto _ : state) {
		char *request_method = NULL;
		size_t request_method_len = 0;
		char *request_resource = NULL;
		size_t request_resource_len = 0;
		int http_version_minor;
		int http_version_major;
		poc_header_pair_zm header_pair[total_header];
		POC_INIT_HEADER_PAIR_TO_ZERO_ZM(header_pair, total_header);
		size_t num_header = 0;
		char *http_message_body = NULL;
		size_t http_message_body_len = 0;
		bool is_failed;
		http_parse_request_zm(
		    sample_get_request, sample_request_size, &request_method,
		    &request_method_len, &request_resource,
		    &request_resource_len, &http_version_major,
		    &http_version_minor, header_pair, &num_header,
		    &http_message_body, &http_message_body_len, &is_failed);
	}
}

static void Http_SIMD_SSE42_AVX2_Zero_Dynamic_Memory_Allocation_Parser(
    benchmark::State &state) {
	char *sample_get_request =
	    "GET /some_page.php HTTP/1.1\r\n"
	    "Host: www.somesite.ca\r\n"
	    "User-Agent: Mozilla/5.0 (Windows NT 6.1 31.0) Gecko/20100101 "
	    "Firefox/31.0\r\n"
	    "Accept: "
	    "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
	    "*;q=0.8\r\n"
	    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"
	    "Keep-Alive: 115\r\n"
	    "Cookie: some_inter_key=2; "
	    "some_internal_value_php=LJldkfjalkasdfl;\r\n"
	    "X-Powered-By: Proxygen/FB-CXX\r\n"
	    "Connection: keep-alive\r\n\r\n";
	size_t sample_request_size = strlen(sample_get_request);
	size_t total_header = 15;
	for (auto _ : state) {
		char *request_method = NULL;
		size_t request_method_len = 0;
		char *request_resource = NULL;
		size_t request_resource_len = 0;
		int http_version_minor;
		int http_version_major;
		poc_header_pair_simd header_pair[total_header];
		POC_INIT_HEADER_PAIR_TO_ZERO_SIMD(header_pair, total_header);
		size_t num_header = 0;
		char *http_message_body = NULL;
		size_t http_message_body_len = 0;
		bool is_failed;
		http_parse_request_simd(
		    sample_get_request, sample_request_size, &request_method,
		    &request_method_len, &request_resource,
		    &request_resource_len, &http_version_major,
		    &http_version_minor, header_pair, &num_header,
		    &http_message_body, &http_message_body_len, &is_failed);
	}
}

BENCHMARK(Http_State_Machine_Parser);
BENCHMARK(Http_Parsing_Method_Two);
BENCHMARK(Http_Zero_Dynamic_Memory_Allocation_Parser);
BENCHMARK(Http_SIMD_SSE42_AVX2_Zero_Dynamic_Memory_Allocation_Parser);

BENCHMARK_MAIN();
