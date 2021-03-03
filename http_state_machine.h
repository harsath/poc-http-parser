/*
 * The software is licensed either the MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef HTTP_STATE_MACHINE
#define HTTP_STATE_MACHINE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#define str3cmp_macro(ptr, c0, c1, c2) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2
static inline bool str3cmp(const char* ptr, const char* cmp){
		return str3cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2));
}
#define str4cmp_macro(ptr, c0, c1, c2, c3) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2 && *(ptr+3) == c3
static inline bool str4cmp(const char* ptr, const char* cmp){
		return str4cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2),  *(cmp+3));
}
#define HEADER_VALUE_BUFFER_SIZE 1024
#define HEADER_NAME_BUFFER_SIZE 1024
enum ParserState { 
	PROTOCOL_ERROR,
	// Request-Line
	REQUEST_LINE_BEGIN = 100,
	REQUEST_METHOD,
	REQUEST_RESOURCE_BEGIN,
	REQUEST_RESOURCE,
	REQUEST_PROTOCOL_BEGIN,
	REQUEST_PROTOCOL_H,
	REQUEST_PROTOCOL_T1,
	REQUEST_PROTOCOL_T2,
	REQUEST_PROTOCOL_P,
	REQUEST_PROTOCOL_SLASH,
	REQUEST_PROTOCOL_VERSION_MAJOR,
	REQUEST_PROTOCOL_VERSION_MINOR,
	REQUEST_LINE_LF,
	// Message-Header
	HEADER_NAME_BEGIN,
	HEADER_NAME,
	HEADER_NAME_COLON,
	HEADER_VALUE_BEGIN,
	HEADER_VALUE,
	HEADER_VALUE_END,
	HEADER_END_LF,
	// Message-Content
	MESSAGE_BODY,
	// Final state
	PARSING_DONE
};
enum LexConst { 
	CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09
};
struct Header{
	char header_name[HEADER_NAME_BUFFER_SIZE];
	char header_value[HEADER_VALUE_BUFFER_SIZE];
};
__attribute__((always_inline))
static inline bool is_char(char value){
	return (unsigned)value <= 127;
}
__attribute__((always_inline))
static inline bool is_control(char value){
	return (value >= 0 && value <= 31) || (value == 127);
}
__attribute__((always_inline))
static inline bool is_seperator(char value){
	switch(value){
		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '\\':
		case '"':
		case '/':
		case '[':
		case ']':
		case '?':
		case '=':
		case '{':
		case '}':
		case (char)SP:
		case (char)HT:
			return true;
		default:
			return false;
	}
}
__attribute__((always_inline))
static inline bool is_token(char value){
	return is_char(value) && !(is_control(value) || is_seperator(value));
}
__attribute__((always_inline))
static inline bool is_text(char value){
	// Any bytes except Control characters
	return !is_control(value) || (value == (char)SP) || (value == HT);
}
__attribute__((always_inline))
struct Header* get_header_for_storing_helper(struct Header** header_container, size_t* current_header_index){
	return header_container[*current_header_index];
}
// Things which are pre-fixed with 'p' means, it will be changed by the parser
//  'http_state_machine_parser' assumes that there will be enough memory to hold message data
static inline void http_state_machine_parser(
		const char* raw_client_buffer_start, const char* raw_client_buffer_end, ParserState* current_state, 
		struct Header** header_container_p, size_t* header_container_start_index_p, char* message_append_buffer_p, 
		size_t* message_buffer_start_index_p, char* request_method_p, char* request_resource_p, char* http_version_p){
	bool is_protocol_fail = false;
	while(!is_protocol_fail && (raw_client_buffer_start != raw_client_buffer_end)){
		switch(*current_state){
			case REQUEST_LINE_BEGIN:
				if(is_token(*raw_client_buffer_start)){
					*current_state = REQUEST_METHOD;
					*request_method_p = *raw_client_buffer_start;
					request_method_p++;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_METHOD:
				if(*raw_client_buffer_start == (char)SP){
					*current_state = REQUEST_RESOURCE_BEGIN;
					raw_client_buffer_start++;
				}else if(is_token(*raw_client_buffer_start)){
					*request_method_p = *raw_client_buffer_start;
					request_method_p++;
					raw_client_buffer_start++;
				}else{
					*current_state = PROTOCOL_ERROR;
				}
				break;
			case REQUEST_RESOURCE_BEGIN:
				if(isprint(*raw_client_buffer_start)){
					*current_state = REQUEST_RESOURCE;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_RESOURCE:
				if(*raw_client_buffer_start == (char)SP){
					// We parsed the request-resource, Let's parse the protocol version
					*current_state = REQUEST_PROTOCOL_BEGIN;
					raw_client_buffer_start++;
				}else if(isprint(*raw_client_buffer_start)){
					*request_resource_p = *raw_client_buffer_start;
					request_resource_p++;
					raw_client_buffer_start++;
				}else{
					*current_state = PROTOCOL_ERROR;
				}
				break;
			case REQUEST_PROTOCOL_BEGIN:
				

		}
	}
}



#endif
