#include "test-helper.h"
#include <http_parser_meth_two.h>

void test_http_meth_two(void){
	{ // HTTP GET Message parsing test
		const char* sample_get_request = "GET /some_page.php HTTP/1.1\r\n"
						 "Host: www.somesite.ca\r\n"
						 "User-Agent: Mozilla/5.0 (Windows NT 6.1 31.0)\r\n"
						 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
						 "Cookie: some_inter_key=2; some_internal_value_php=LJldkfjalkasdfl;\r\n"
						 "X-Powered-By: Proxygen/FB-CXX\r\n\r\n";
		size_t sample_request_size = strlen(sample_get_request);
		poc_Buffer_mt* http_body = poc_allocate_http_buffer_mt(2048);	
		poc_Header_mt* http_header = poc_allocate_http_header_mt(10);
		poc_HTTP_Version_mt http_version;
		poc_HTTP_Method_mt http_method;
		char* request_resource = calloc(sizeof(char), 1000);
		poc_Parser_Error_mt http_parser_error;
		bool return_parser = poc_http_parser_mt(http_body, http_header, sample_get_request, sample_request_size,
				&http_method, request_resource, &http_version, &http_parser_error);
		POC_ASSERT_TRUE(return_parser, "Meth-2 Parser's return value");
		POC_ASSERT_TRUE(http_method == POC_HTTP_GET_mt, "Meth-2 HTTP Method");
		POC_ASSERT_TRUE(memcmp(request_resource, "/some_page.php", strlen("some_page.php")) == 0, "Meth-2 HTTP Request Res");
		POC_ASSERT_TRUE(http_version == HTTP_1_1, "Meth-2 HTTP Version");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[0].header_name, "Host", strlen("Host")) == 0,
				"Meth-2 HTTP Header name 0");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[0].header_value, "www.somesite.ca", 
					strlen("www.somesite.ca")) == 0, "Meth-2 HTTP Header value 0");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[1].header_name, "User-Agent", strlen("User-Agent")) == 0, 
				"Meth-2 HTTP Header name 1");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[1].header_value, "Mozilla/5.0 (Windows NT 6.1 31.0)",
					strlen("Mozilla/5.0 (Windows NT 6.1 31.0)")) == 0, "Meth-2 HTTP Header value 1");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[2].header_name, "Accept", strlen("Accept")) == 0,
				"Meth-2 HTTP Header name 2");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[2].header_value, 
				"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
				strlen("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8")) == 0,
				"Meth-2 HTTP Header value 2");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[3].header_name, "Cookie",strlen("Cookie")) == 0,
				"Meth-2 HTTP Header name 3");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[3].header_value, 
				"some_inter_key=2; some_internal_value_php=LJldkfjalkasdfl;",
				strlen("some_inter_key=2; some_internal_value_php=LJldkfjalkasdfl;")) == 0,
				"Meth-2 HTTP Header value 3");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[4].header_name, "X-Powered-By", strlen("X-Powered-By")) == 0,
				"Meth-2 HTTP Header name 4");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[4].header_value, "Proxygen/FB-CXX", 
					strlen("Proxygen/FB-CXX")) == 0, "Meth-2 HTTP Header value 4");
		POC_ASSERT_TRUE(http_header->current_index == 5, "Meth-3 internal current_index check");
		poc_free_http_header_mt(http_header);
		poc_free_buffer_mt(http_body);
		free(request_resource);
	}

	{ // HTTP POST Message parsing test
		const char* sample_post_request = "POST /some_endpoint HTTP/1.1\r\n"
						  "Host: www.foobar.com\r\n"
						  "X-Powered-By: Proxygen/FB-CXX\r\n"
						  "Content-Type: application/www-x-form-urlencoded\r\n"
						  "Content-Length: 35\r\n\r\n"
						  "key_one=value_one&key_two=value_two";
		size_t sample_request_size = strlen(sample_post_request);
		poc_Buffer_mt* http_body = poc_allocate_http_buffer_mt(2048);	
		poc_Header_mt* http_header = poc_allocate_http_header_mt(10);
		poc_HTTP_Version_mt http_version;
		poc_HTTP_Method_mt http_method;
		char* request_resource = calloc(sizeof(char), 1000);
		poc_Parser_Error_mt http_parser_error;
		bool return_parser = poc_http_parser_mt(http_body, http_header, sample_post_request, sample_request_size,
				&http_method, request_resource, &http_version, &http_parser_error);
		POC_ASSERT_TRUE(return_parser, "Meth-2 POST HTTP parser return");
		POC_ASSERT_TRUE(http_method == POC_HTTP_POST_mt, "Meth-2 POST HTTP Method");
		POC_ASSERT_TRUE(memcmp(request_resource, "/some_endpoint", strlen("/some_endpoint")) == 0,
				"Meth-2 POST HTTP request resource");
		POC_ASSERT_TRUE(http_version == HTTP_1_1, "Meth-2 POST HTTP Version");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[0].header_name, "Host", strlen("Host")) == 0,
				"Meth-2 POST HTTP header 0 name");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[0].header_value, "www.foobar.com", strlen("www.foobar.com"))
				== 0, "Meth-2 POST HTTP header value 0");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[1].header_name, "X-Powered-By", strlen("X-Powered-By"))
				== 0, "Meth-2 POST HTTP header name 1");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[1].header_value, "Proxygen/FB-CXX", strlen("Proxygen/FB-CXX"))
				== 0, "Meth-2 POST HTTP header value 1");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[2].header_name, "Content-Type", strlen("Content-Type")) == 0,
				"Meth-2 POST HTTP header name 2");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[2].header_value, "application/www-x-form-urlencoded",
					strlen("application/www-x-form-urlencoded")) == 0, "Meth-2 POST HTTP header value 2");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[3].header_name, "Content-Length", strlen("Content-Length"))
				== 0, "Meth-2 POST HTTP header name 3");
		POC_ASSERT_TRUE(memcmp(http_header->http_header_pairs[3].header_value, "35", 2 /* len of string */) == 0,
				"Meth-2 POST HTTP header value 3");
		POC_ASSERT_TRUE(memcmp(http_body->buffer, "key_one=value_one&key_two=value_two", 
					strlen("key_one=value_one&key_two=value_two")) == 0, "Meth-2 POST HTTP message body");
		POC_ASSERT_TRUE(http_body->current_index == strlen("key_one=value_one&key_two=value_two")-1, 
				"Meth-2 POST HTTP body current_index");
		poc_free_http_header_mt(http_header);
		poc_free_buffer_mt(http_body);
		free(request_resource);
	}
}
