#ifndef HTTP_STATE_MACHINE
#define HTTP_STATE_MACHINE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define str3cmp_macro(ptr, c0, c1, c2) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2
static inline bool str3cmp(const char* ptr, const char* cmp){
		return str3cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2));
}
#define str4cmp_macro(ptr, c0, c1, c2, c3) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2 && *(ptr+3) == c3
static inline bool str4cmp(const char* ptr, const char* cmp){
		return str4cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2),  *(cmp+3));
}
enum ParserState { 
	PROTOCOL_ERROR,
	// Request-Line
	REQUEST_LINE_BEGIN = 100,
	REQUEST_METHOD,
	REQUEST_RESOURCE_BEGIN,
	REQUEST_RESOURCE,
	REQUEST_PROTOCOL_BEGIN,
	REQUEST_PROTOCOL,
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
	char header_name[1024];
	char header_value[1024];
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
// Things which are pre-fixed with 'p' means, it will be changed by the parser
//  'http_state_machine_parser' assumes that there will be enough memory to hold message data
static inline void http_state_machine_parser(
		const char* raw_client_buffer_start, const char* raw_client_buffer_end, ParserState* current_state, 
		struct Header** header_container_p, size_t* header_container_start_index_p, char* message_append_buffer_p, 
		size_t* message_buffer_start_index_p, char* request_method_p, char* request_resource, char* http_version_p){
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
		}
	}
}



#endif
