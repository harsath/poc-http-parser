#include <http_SIMD_zero_mem_alloc_parser.h>
#include "test-helper.h"

void test_SIMD_zero_mem_alloc(void){
	{ // test GET request message
		char* sample_http_get_request = "GET /index.php HTTP/1.1\r\n"
						"Host: www.foo.com\r\n"
						"User-Agent: curl/19.2\r\n\r\n";
		size_t sample_http_get_request_size = strlen(sample_http_get_request);
		size_t total_header = 10;
		char* request_method = NULL;
		size_t request_method_len = 0;
		char* request_resource = NULL;
		size_t request_resource_len = 0;
		int http_version_minor;
		int http_version_major;
		poc_header_pair_simd header_pair[total_header];			
		POC_INIT_HEADER_PAIR_TO_ZERO_SIMD(header_pair, total_header);
		size_t num_header = 0;
		char* http_message_body = NULL;
		size_t http_message_body_len = 0;
		bool is_failed;
		http_parse_request_simd(sample_http_get_request, sample_http_get_request_size, &request_method, &request_method_len, &request_resource,
				   &request_resource_len, &http_version_major, &http_version_minor, header_pair, &num_header, &http_message_body,
				   &http_message_body_len, &is_failed);
		POC_ASSERT_TRUE(memcmp(request_method, "GET", request_method_len) == 0, "SIMD_ZERO_ALLOC request_method cmp");
		POC_ASSERT_TRUE(request_method_len == 3, "SIMD_ZERO_ALLOC request_resource_len cmp");
		POC_ASSERT_TRUE(memcmp(request_resource, "/index.php", request_resource_len) == 0, "SIMD_ZERO_ALLOC request resource");
		POC_ASSERT_TRUE(request_resource_len == strlen("/index.php"), "SIMD_ZERO_ALLOC request_method_len cmp");
		POC_ASSERT_TRUE(http_version_major == 1 && http_version_minor == 1, "SIMD_ZERO_ALLOC http version_{major, minor}");
		POC_ASSERT_TRUE(header_pair[0].header_name_len == 4, "SIMD_ZERO_ALLOC header one's name length");
		POC_ASSERT_TRUE(header_pair[0].header_value_len == strlen("www.foo.com"), "SIMD_ZERO_ALLOC header one's value length");
		POC_ASSERT_TRUE(header_pair[1].header_name_len == strlen("User-Agent"), "SIMD_ZERO_ALLOC header two's name length");
		POC_ASSERT_TRUE(header_pair[1].header_value_len == strlen("curl/19.2"), "SIMD_ZERO_ALLOC header two's value length");
		POC_ASSERT_TRUE(header_pair[2].header_name_len == 0 && header_pair[2].header_value_len == 0, "SIMD_ZERO_ALLOC uninit name, value's length");
		POC_ASSERT_TRUE(memcmp(header_pair[0].header_name, "Host", header_pair[0].header_name_len) == 0, "SIMD_ZERO_ALLOC header one name cmp");
		POC_ASSERT_TRUE(memcmp(header_pair[0].header_value, "www.foo.com", header_pair[0].header_name_len) == 0, "SIMD_ZERO_ALLOC header one value cmp");
		POC_ASSERT_TRUE(memcmp(header_pair[1].header_name, "User-Agent", header_pair[1].header_name_len) == 0, "SIMD_ZERO_ALLOC header two name cmp");
		POC_ASSERT_TRUE(memcmp(header_pair[1].header_value, "curl/19.2", header_pair[1].header_value_len) == 0, "SIMD_ZERO_ALLOC header two value cmp");
		POC_ASSERT_TRUE(header_pair[2].header_name == NULL && header_pair[2].header_value == NULL, "SIMD_ZERO_ALLOC uninit name & value check");
		POC_ASSERT_TRUE(!is_failed, "SIMD_ZERO_ALLOC parser 'is_failed' flag");
		POC_ASSERT_TRUE(http_message_body == NULL && http_message_body_len == 0, "SIMD_ZERO_ALLOC GET request message body & len check");
	}

	{ // test POST request message
		char* sample_post_request = "POST /some_endpoint HTTP/1.1\r\n"
					    "Host: www.foobar.com\r\n"
					    "X-Powered-By: Proxygen/FB-CXX\r\n"
					    "Content-Type: application/www-x-form-urlencoded\r\n"
					    "Content-Length: 35\r\n\r\n"
					    "key_one=value_one&key_two=value_two";
		size_t sample_request_size = strlen(sample_post_request);
		size_t total_header = 10;
		char* request_method = NULL;
		size_t request_method_len = 0;
		char* request_resource = NULL;
		size_t request_resource_len = 0;
		int http_version_minor;
		int http_version_major;
		poc_header_pair_simd header_pair[total_header];			
		POC_INIT_HEADER_PAIR_TO_ZERO_SIMD(header_pair, total_header);
		size_t num_header = 0;
		char* http_message_body = NULL;
		size_t http_message_body_len = 0;
		bool is_failed;
		http_parse_request_simd(sample_post_request, sample_request_size, &request_method, &request_method_len, &request_resource,
				   &request_resource_len, &http_version_major, &http_version_minor, header_pair, &num_header, &http_message_body,
				   &http_message_body_len, &is_failed);
		POC_ASSERT_TRUE(memcmp(request_method, "POST", request_method_len) == 0, "SIMD_ZERO_ALLOC POST request method");
		POC_ASSERT_TRUE(request_method_len == 4, "SIMD_ZERO_ALLOC POST request_method_len");
		POC_ASSERT_TRUE(memcmp(request_resource, "/some_endpoint", request_resource_len) == 0, "SIMD_ZERO_ALLOC POST request_resource");
		POC_ASSERT_TRUE(request_resource_len == strlen("/some_endpoint"), "SIMD_ZERO_ALLOC POST request_resource_len");
		POC_ASSERT_TRUE(http_version_major == 1 && http_version_minor == 1, "SIMD_ZERO_ALLOC POST http_version{major,minor}");
		POC_ASSERT_TRUE(memcmp(header_pair[0].header_name, "Host", strlen("Host")) == 0, "SIMD_ZERO_ALLOC POST header one name");
		POC_ASSERT_TRUE(header_pair[0].header_name_len == strlen("Host"), "SIMD_ZERO_ALLOC POST header one's name length");
		POC_ASSERT_TRUE(memcmp(header_pair[1].header_name, "X-Powered-By", strlen("X-Powered-By")) == 0, "SIMD_ZERO_ALLOC POST header two name");
		POC_ASSERT_TRUE(header_pair[1].header_name_len == strlen("X-Powered-By"), "SIMD_ZERO_ALLOC POST header two name len");
		POC_ASSERT_TRUE(memcmp(header_pair[2].header_name, "Content-Type", strlen("Content-Type")) == 0, "SIMD_ZERO_ALLOC POST header three name");
		POC_ASSERT_TRUE(header_pair[2].header_name_len == strlen("Content-Type"), "SIMD_ZERO_ALLOC POST header three name len");
		POC_ASSERT_TRUE(memcmp(header_pair[3].header_name, "Content-Length", strlen("Content-Length")) == 0, "SIMD_ZERO_ALLOC POST header four name");
		POC_ASSERT_TRUE(header_pair[3].header_name_len == strlen("Content-Length"), "SIMD_ZERO_ALLOC POST header four name len");
		POC_ASSERT_TRUE(memcmp(header_pair[0].header_value, "www.foobar.com", strlen("www.foobar.com")) == 0, "SIMD_ZERO_ALLOC POST header one value");
		POC_ASSERT_TRUE(header_pair[0].header_value_len == strlen("www.foobar.com"), "SIMD_ZERO_ALLOC POST header one value len");
		POC_ASSERT_TRUE(memcmp(header_pair[1].header_value, "Proxygen/FB-CXX", strlen("Proxygen/FB-CXX")) == 0, "SIMD_ZERO_ALLOC POST header two value");
		POC_ASSERT_TRUE(http_message_body_len == strlen("key_one=value_one&key_two=value_two"), "SIMD_ZERO_ALLOC POST http message body length");
		POC_ASSERT_TRUE(memcmp(http_message_body, "key_one=value_one&key_two=value_two", strlen("key_one=value_one&key_two=value_two")) == 0, 
				"SIMD_ZERO_ALLOC POST http message body");
		POC_ASSERT_TRUE(!is_failed, "SIMD_ZERO_ALLOC POST parser 'is_failed' flag");
	}
}
