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
#include "../http_state_machine.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, const char*const argv[]){
	{ // bench GET request
		const char* sample_get_request = "GET /some_page.php HTTP/1.1\r\n"
						 "Host: www.somesite.ca\r\n"
						 "User-Agent: Mozilla/5.0 (Windows NT 6.1 31.0) Gecko/20100101 Firefox/31.0\r\n"
						 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
						 "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"
						 "Keep-Alive: 115\r\n"
						 "Cookie: some_inter_key=2; some_internal_value_php=LJldkfjalkasdfl;\r\n"
						 "X-Powered-By: Proxygen/FB-CXX\r\n"
						 "Connection: keep-alive\r\n\r\n";
		size_t sample_request_size = strlen(sample_get_request);
		size_t num_header = 15;
		size_t message_body_buffer_size = 2048;
		size_t request_resource_buffer_size = 1024;
		size_t http_version_buffer_size = 10;
		size_t http_method_buffer_size = 10;
		float start_time, end_time;
		start_time = (float)clock()/CLOCKS_PER_SEC;
		for(size_t i = 0; i < 10000000; i++){
		  bool is_protocol_failur = false;
		  ParserState parser_state = REQUEST_LINE_BEGIN;
		  poc_HTTP_Request_Message* http_request_message =
			  poc_allocate_http_request_message(num_header, message_body_buffer_size, request_resource_buffer_size,
					  http_version_buffer_size, http_method_buffer_size);
		  bool return_parser = poc_http_state_machine_parser(http_request_message, &parser_state, 
				  sample_get_request, sample_request_size, &is_protocol_failur);
		  poc_free_http_request_message(http_request_message);
		}
		end_time = (float)clock()/CLOCKS_PER_SEC;
		printf("Elapsed time is: %f secs\n", (end_time - start_time));
	}
	
	return 0;
}
