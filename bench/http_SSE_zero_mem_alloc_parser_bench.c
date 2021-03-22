// poc-http-parser
// Copyright © Harsath
// The software is licensed under the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "../http_SSE_zero_mem_alloc_parser.h"
#include <stdio.h>
#include <time.h>

int main(int argc, const char *argv[]){
	char* sample_get_request = "GET /some_pageasdjflasdjflkasjflaksjdlfdksj.php HTTP/1.1\r\n"
				   "Host: www.somesite.ca\r\n"
				   "User-Agent: Mozilla/5.0 (Windows NT 6.1 31.0) Gecko/20100101 Firefox/31.0\r\n"
				   "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
				   "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"
				   "Keep-Alive: 115\r\n"
				   "Cookie: some_inter_key=2; some_internal_value_php=LJldkfjalkasdfl;\r\n"
				   "X-Powered-By: Proxygen/FB-CXX\r\n"
				   "Connection: keep-alive\r\n\r\n";
	size_t sample_http_get_request_size = strlen(sample_get_request);
	float start_time, end_time;
	size_t total_header = 10;
	start_time = (float)clock()/CLOCKS_PER_SEC;
	for(size_t i = 0; i < 10000000; i++){
		char* request_method = NULL;
		size_t request_method_len = 0;
		char* request_resource = NULL;
		size_t request_resource_len = 0;
		int http_version_minor;
		int http_version_major;
		poc_header_pair header_pair[total_header];
		POC_INIT_HEADER_PAIR_TO_ZERO(header_pair, total_header);
		size_t num_header = 0;
		char* http_message_body = NULL;
		size_t http_message_body_len = 0;
		bool is_failed;
		http_parse_request(sample_get_request, sample_http_get_request_size, &request_method, &request_method_len, &request_resource,
				   &request_resource_len, &http_version_major, &http_version_minor, header_pair, &num_header, &http_message_body,
				   &http_message_body_len, &is_failed);
	}
	end_time = (float)clock()/CLOCKS_PER_SEC;
	printf("Elapsed time is: %f secs\n", (end_time - start_time));

	return 0;
}
