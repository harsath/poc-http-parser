#include <cstring>
#include <http_state_machine.h>
#include <gtest/gtest.h>

TEST(Test_HTTP_Parser, Test_HTTP_Parser_state_machine){
	{
		const char* sample_http_get_request = "GET /index.php HTTP/1.1\r\n"
						      "Host: www.foo.com\r\n"
						      "User-Agent: curl/19.2\r\n\r\n";
		size_t sample_http_get_request_size = strlen(sample_http_get_request);
		size_t num_header = 5;
		size_t message_body_buf_size = 2048;
		size_t req_resource_buf_size = 1024;
		size_t http_version_buf_size = 10;
		size_t http_method_buf_size = 10;
		poc_HTTP_Request_Message* http_request_message = 
			poc_allocate_http_request_message(num_header, message_body_buf_size, req_resource_buf_size,
					http_version_buf_size, http_method_buf_size);
		ParserState parser_state = REQUEST_LINE_BEGIN;
		bool is_protocol_fail = false;
		bool parser_return = poc_http_state_machine_parser(http_request_message, &parser_state, 
				sample_http_get_request, sample_http_get_request_size, &is_protocol_fail);
		ASSERT_TRUE(poc_str3cmp(http_request_message->http_method->buffer, (unsigned char*)"GET") && !is_protocol_fail);
		ASSERT_TRUE(strncmp((char*)http_request_message->http_request_resource->buffer, "/index.php", strlen("/index.php")) == 0);
		ASSERT_TRUE(strncmp((char*)http_request_message->http_version->buffer, "HTTP/1.1", strlen("HTTP/1.1")) == 0);
		ASSERT_TRUE(strncmp((char*)http_request_message->http_headers->http_header_pairs[0].header_name,
					"Host", strlen("Host")) == 0);
		ASSERT_TRUE(strncmp((char*)http_request_message->http_headers->http_header_pairs[1].header_name,
					"User-Agent", strlen("User-Agent")) == 0);
		ASSERT_TRUE(strncmp((char*)http_request_message->http_headers->http_header_pairs[0].header_value,
					"www.foo.com", strlen("www.foo.com")) == 0);
		ASSERT_TRUE(strncmp((char*)http_request_message->http_headers->http_header_pairs[1].header_value,
					"curl/19.2", strlen("curl/19.2")) == 0);
		ASSERT_TRUE(parser_state == PARSING_DONE);
		poc_free_http_request_message(http_request_message);
	}

	{
		const char* sample_http_post_request = "POST /index.php HTTP/1.1\r\n"
						       "Host: www.foo.com\r\n"
						       "User-Agent: curl/19.2\r\n"
						       "Content-Type: application/www-x-form-urlencoded\r\n";
		size_t sample_http_get_request_size = strlen(sample_http_get_request);
		size_t num_header = 5;
		size_t message_body_buf_size = 2048;
		size_t req_resource_buf_size = 1024;
		size_t http_version_buf_size = 10;
		size_t http_method_buf_size = 10;
		poc_HTTP_Request_Message* http_request_message = 
			poc_allocate_http_request_message(num_header, message_body_buf_size, req_resource_buf_size,
					http_version_buf_size, http_method_buf_size);
		ParserState parser_state = REQUEST_LINE_BEGIN;
		bool is_protocol_fail = false;
		bool parser_return = poc_http_state_machine_parser(http_request_message, &parser_state, 
				sample_http_get_request, sample_http_get_request_size, &is_protocol_fail);
	}
}
