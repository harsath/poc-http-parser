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
#ifndef HTTP_STATE_MACHINE
#define HTTP_STATE_MACHINE
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define poc_str3cmp_macro(ptr, c0, c1, c2) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2
static inline bool poc_str3cmp(const char* ptr, const char* cmp){
		return poc_str3cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2));
}
#define poc_str4cmp_macro(ptr, c0, c1, c2, c3) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2 && *(ptr+3) == c3
static inline bool poc_str4cmp(const char* ptr, const char* cmp){
		return poc_str4cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2),  *(cmp+3));
}
#define HEADER_VALUE_BUFFER_SIZE 100
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
	HEADER_VALUE_LF,
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

enum RequestType {
	GET, POST, HEAD, PUT, DELETE
};

struct poc_Header_Pair{
	char header_name[HEADER_NAME_BUFFER_SIZE];
	char header_value[HEADER_VALUE_BUFFER_SIZE];
};

struct poc_Header{
	struct poc_Header_Pair* http_header_pairs;
	size_t remaining_memory;
	size_t current_index = 0;
};

struct poc_Buffer{
	char* buffer;
	size_t remaining_memory;
	size_t current_index = 0;
};

struct poc_HTTP_Request_Message{
	struct poc_Header http_header;
	struct poc_Buffer http_message_body;
	struct poc_Buffer http_request_resource;
	char http_version[8];
	RequestType http_request_type;
};

static inline bool poc_http_message_add_header(
		struct poc_HTTP_Request_Message* http_message, struct poc_Header_Pair* http_header){
	if(((http_message->http_header.remaining_memory - http_message->http_header.current_index) <= 0))
		return false;
	memcpy(http_message->http_header.http_header_pairs[http_message->http_header.current_index].header_name,
		http_header->header_name, HEADER_NAME_BUFFER_SIZE);
	memcpy(http_message->http_header.http_header_pairs[http_message->http_header.current_index].header_value,
		http_header->header_value, HEADER_VALUE_BUFFER_SIZE);
	http_message->http_header.current_index++;
	return true;
}

static inline bool poc_http_message_append_raw_body_bytes(
		struct poc_HTTP_Request_Message* http_message, const char* value){
	size_t remaining_memory = http_message->http_message_body.remaining_memory - http_message->http_message_body.current_index;
	size_t value_length = strlen(value);
	if((remaining_memory <= 0) && (value_length > remaining_memory))
		return false;
	memcpy(&http_message->http_message_body.buffer[http_message->http_message_body.current_index], value, value_length);
	return true;
}

static inline bool poc_http_message_append_request_resource_bytes(
		struct poc_HTTP_Request_Message* http_message, const char* value){
	size_t remaining_memory = http_message->http_request_resource.remaining_memory 
					- http_message->http_request_resource.current_index;
	size_t value_length = strlen(value);
	if((remaining_memory <= 0) && (value_length > remaining_memory))
		return false;
	memcpy(&http_message->http_request_resource.buffer[http_message->http_request_resource.current_index], value, value_length);
	return true;
}

__attribute__((always_inline))
static inline void poc_http_message_initilize_http_version(struct poc_HTTP_Request_Message* http_message){
	const char* http_version = "HTTP/1.1";
	memcpy(http_message->http_version, http_version, 8);
}

static inline struct poc_Header* poc_allocate_http_header(size_t num_header){
	struct poc_Header* http_header = (struct poc_Header*)malloc(sizeof(poc_Header));
	http_header->http_header_pairs = (struct poc_Header_Pair*)malloc(sizeof(poc_Header_Pair)*num_header);
	http_header->remaining_memory = num_header;
	for(size_t i = 0; i < num_header; i++){
		memset(http_header->http_header_pairs[i].header_name, 0, HEADER_NAME_BUFFER_SIZE);
		memset(http_header->http_header_pairs[i].header_value, 0, HEADER_VALUE_BUFFER_SIZE);
	}
	return http_header;
}

__attribute__((always_inline))
static inline bool _poc_is_char(char value){
	return (unsigned)value <= 127;
}

__attribute__((always_inline))
static inline bool _poc_is_control(char value){
	return (value >= 0 && value <= 31) || (value == 127);
}

__attribute__((always_inline))
static inline bool _poc_is_seperator(char value){
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
static inline bool _poc_is_token(char value){
	return _poc_is_char(value) && !(_poc_is_control(value) || _poc_is_seperator(value));
}

__attribute__((always_inline))
static inline bool _poc_is_text(char value){
	// Any bytes except Control characters
	return !_poc_is_control(value) || (value == (char)SP) || (value == HT);
}

__attribute__((always_inline))
static inline char* poc_get_header_value(){}

// Things which are pre-fixed with 'p' means, it will be changed by the parser
//  'http_state_machine_parser' assumes that there will be enough memory to hold message data
static inline void poc_http_state_machine_parser(
		const char* raw_client_buffer_start, const char* raw_client_buffer_end, ParserState* current_state, 
		struct poc_Header** header_container_p, size_t* header_container_start_index_p, char* message_append_buffer_p, 
		size_t* message_buffer_start_index_p, char* request_method_p, char* request_resource_p, char* http_version_p,
		size_t* header_name_index, size_t* header_value_index){
	bool is_protocol_fail = false;
	while(!is_protocol_fail && (raw_client_buffer_start != raw_client_buffer_end)){
		switch(*current_state){
			case REQUEST_LINE_BEGIN:
				if(_poc_is_token(*raw_client_buffer_start)){
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
				}else if(_poc_is_token(*raw_client_buffer_start)){
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
				}else if(isprint(*raw_client_buffer_start)){
					*request_resource_p = *raw_client_buffer_start;
					request_resource_p++;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_BEGIN:
				if(*raw_client_buffer_start == (char)SP){
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_H:
				if(*raw_client_buffer_start == 'H'){
					*http_version_p = 'H';
					*current_state = REQUEST_PROTOCOL_T1;
					http_version_p++;
					raw_client_buffer_end++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_T1:
				if(*raw_client_buffer_start == 'T'){
					*http_version_p = 'T';
					*current_state = REQUEST_PROTOCOL_T2;
					http_version_p++;
					raw_client_buffer_end++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_T2:
				if(*raw_client_buffer_start == 'T'){
					*http_version_p = 'T';
					*current_state = REQUEST_PROTOCOL_P;
					http_version_p++;
					raw_client_buffer_end++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_P:
				if(*raw_client_buffer_start == 'P'){
					*http_version_p = 'P';
					*current_state = REQUEST_PROTOCOL_SLASH;
					http_version_p++;
					raw_client_buffer_end++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_SLASH:
				if(*raw_client_buffer_start == '/'){
					*http_version_p = '/';
					*current_state = REQUEST_PROTOCOL_VERSION_MAJOR;
					http_version_p++;
					raw_client_buffer_end++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_VERSION_MAJOR:
				if(isdigit(*raw_client_buffer_start)){
					*http_version_p = *raw_client_buffer_start;
					http_version_p++;
					raw_client_buffer_end++;
				}else if(*raw_client_buffer_start == '.'){
					*http_version_p = '.';
					*current_state = REQUEST_PROTOCOL_VERSION_MINOR;
					http_version_p++;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_PROTOCOL_VERSION_MINOR:
				if(isdigit(*raw_client_buffer_start)){
					*http_version_p = *raw_client_buffer_start;
					http_version_p++;
					raw_client_buffer_start++;
				}else if(*raw_client_buffer_start == (char)CR){
					*current_state = REQUEST_LINE_LF;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case REQUEST_LINE_LF:
				if(*raw_client_buffer_start == (char)LF){
					*current_state = HEADER_NAME_BEGIN;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case HEADER_NAME_BEGIN:
				if(_poc_is_token(*raw_client_buffer_start)){
					_poc_append_byte_to_header(header_container_p, header_container_start_index_p,
							*raw_client_buffer_start, true, header_name_index, header_value_index);
					*current_state = HEADER_NAME;
					raw_client_buffer_start++;
				}else if(*raw_client_buffer_start == (char)CR){
					*current_state = HEADER_END_LF;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case HEADER_NAME:
				if(_poc_is_token(*raw_client_buffer_start)){
					_poc_append_byte_to_header(header_container_p, header_container_start_index_p,
							*raw_client_buffer_start, true, header_name_index, header_value_index);
					raw_client_buffer_start++;
				}else if(*raw_client_buffer_start == ':'){
					*current_state = HEADER_VALUE_BEGIN;
					*header_name_index = 0;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case HEADER_VALUE_BEGIN:
				if(_poc_is_text(*raw_client_buffer_start)){
					_poc_append_byte_to_header(header_container_p, header_container_start_index_p,
							*raw_client_buffer_start, false, header_name_index, header_value_index);
					*current_state = HEADER_VALUE;
					raw_client_buffer_start++;
				}else if(*raw_client_buffer_start == (char)CR){
					*current_state = HEADER_END_LF;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case HEADER_VALUE:
				if(*raw_client_buffer_start == (char)CR){
					*current_state = HEADER_VALUE_LF;
					raw_client_buffer_start++;
				}else if(_poc_is_text(*raw_client_buffer_start)){
					_poc_append_byte_to_header(header_container_p, header_container_start_index_p,
							*raw_client_buffer_start, false, header_name_index, header_value_index);
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case HEADER_VALUE_LF:
				if(*raw_client_buffer_start == (char)LF){
					*current_state = HEADER_VALUE_END;
					raw_client_buffer_start++;
				}else{ *current_state = PROTOCOL_ERROR; }
				break;
			case HEADER_VALUE_END:
				(*header_container_start_index_p)++;
				*header_name_index = 0;		
				*header_value_index = 0;
				*current_state = HEADER_NAME_BEGIN;
				break;
			case HEADER_END_LF:
				if(*raw_client_buffer_start == (char)LF){
					if(poc_str3cmp(request_method_p, "GET")){
						*current_state = PARSING_DONE;
					}else if(poc_str4cmp(request_method_p, "POST")){

					}
				}
				
		}
	}
}

#endif
