#include <string.h>
#include "http_state_machine.h"
#include "test-helper.h"

int main(int argc, const char*const argv[]){
	{ // GET Request parsing
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
		POC_ASSERT_TRUE(poc_str3cmp(http_request_message->http_method->buffer, (unsigned char*)"GET") && !is_protocol_fail,
					"GET method");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message->http_request_resource->buffer, "/index.php", strlen("/index.php")) == 0,
					"GET request resource");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message->http_version->buffer, "HTTP/1.1", strlen("HTTP/1.1")) == 0,
					"GET http version");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message->http_headers->http_header_pairs[0].header_name,
					"Host", strlen("Host")) == 0, "GET http header name 0");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message->http_headers->http_header_pairs[1].header_name,
					"User-Agent", strlen("User-Agent")) == 0, "GET http header name 1");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message->http_headers->http_header_pairs[0].header_value,
					"www.foo.com", strlen("www.foo.com")) == 0, "GET http header value 0");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message->http_headers->http_header_pairs[1].header_value,
					"curl/19.2", strlen("curl/19.2")) == 0, "GET http header value 1");
		POC_ASSERT_TRUE(parser_state == PARSING_DONE, "GET parser state");
		poc_free_http_request_message(http_request_message);
	}

	{ // POST Request parsing
		const char* sample_http_post_request = "POST /index.php HTTP/1.1\r\n"
						       "Host: www.foo.com\r\n"
						       "User-Agent: curl/19.2\r\n"
						       "Content-Type: application/www-x-form-urlencoded\r\n"
						       "Content-Length: 35\r\n\r\n"
						       "key_one=value_one&key_two=value_two";
		size_t sample_http_post_request_size = strlen(sample_http_post_request);
		size_t num_header = 10;
		size_t message_body_buf_size = 2048;
		size_t req_resource_buf_size = 1024;
		size_t http_version_buf_size = 10;
		size_t http_method_buf_size = 10;
		poc_HTTP_Request_Message* http_request_message_post = 
			poc_allocate_http_request_message(num_header, message_body_buf_size, req_resource_buf_size,
					http_version_buf_size, http_method_buf_size);
		ParserState parser_state_post = REQUEST_LINE_BEGIN;
		bool is_protocol_fail = false;
		bool parser_return = poc_http_state_machine_parser(http_request_message_post, &parser_state_post, 
				sample_http_post_request, sample_http_post_request_size, &is_protocol_fail);
		POC_ASSERT_TRUE(poc_str3cmp(http_request_message_post->http_method->buffer, (unsigned char*)"POST") && !is_protocol_fail,
					"POST method");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_request_resource->buffer, "/index.php",
					strlen("/index.php")) == 0, "POST request resource");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_version->buffer, "HTTP/1.1", strlen("HTTP/1.1")) == 0,
					"POST http version");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[0].header_name,
					"Host", strlen("Host")) == 0, "POST header name 0");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[1].header_name,
					"User-Agent", strlen("User-Agent")) == 0, "POST header name 1");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[0].header_value,
					"www.foo.com", strlen("www.foo.com")) == 0, "POST header value 0");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[1].header_value,
					"curl/19.2", strlen("curl/19.2")) == 0, "POST header value 1");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[2].header_name,
					"Content-Type", strlen("Content-Type")) == 0, "POST header name 2");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[2].header_value,
					"application/www-x-form-urlencoded", strlen("application/www-x-form-urlencoded")) == 0,
					"POST header value 2");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[3].header_name,
					"Content-Length", strlen("Content-Length")) == 0, "POST header name 3");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_headers->http_header_pairs[3].header_value,
					"35", strlen("35")) == 0, "POST header value 3");
		POC_ASSERT_TRUE(memcmp((char*)http_request_message_post->http_message_body->buffer,
					"key_one=value_one&key_two=value_two", strlen("key_one=value_one&key_two=value_two")) == 0, "POST body");
		POC_ASSERT_EQ(parser_state_post, PARSING_DONE, "POST parser state");
		poc_free_http_request_message(http_request_message_post);
	}

	POC_TEST_STATS();
}
