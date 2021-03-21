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
#ifndef HTTP_STATE_MACHINE_H
#define HTTP_STATE_MACHINE_H
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define poc_str3cmp_macro(ptr, c0, c1, c2)                                     \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2
static bool poc_str3cmp(const unsigned char *ptr, const unsigned char *cmp) {
	return poc_str3cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2));
}
#define poc_str4cmp_macro(ptr, c0, c1, c2, c3)                                 \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3
static bool poc_str4cmp(const unsigned char *ptr, const unsigned char *cmp) {
	return poc_str4cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
				 *(cmp + 3));
}
#define str5cmp_macro(ptr, c0, c1, c2, c3, c4)                                 \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3 &&*(ptr + 4) == c4
static bool str5cmp(const unsigned char *ptr, const unsigned char *cmp) {
	return str5cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
			     *(cmp + 3), *(cmp + 4));
}
#define POC_ALLOCATOR(num_of_elements, size_of_single_type)                    \
	calloc(num_of_elements, size_of_single_type)
#define HEADER_VALUE_BUFFER_SIZE 100
#define HEADER_NAME_BUFFER_SIZE 1024
#define HTTP_VERSION_BUFFER_SIZE 8

typedef enum {
	PROTOCOL_ERROR,
	// Request-Line
	REQUEST_LINE_BEGIN = 100,
	REQUEST_METHOD,
	REQUEST_RESOURCE,
	REQUEST_PROTOCOL_H,
	REQUEST_PROTOCOL_T1,
	REQUEST_PROTOCOL_T2,
	REQUEST_PROTOCOL_P,
	REQUEST_PROTOCOL_SLASH,
	REQUEST_PROTOCOL_VERSION_MAJOR,
	REQUEST_PROTOCOL_VERSION_MINOR,
	REQUEST_LINE_LF,
	// Message-Header
	HEADER_NAME,
	HEADER_VALUE,
	HEADER_VALUE_LF,
	HEADER_VALUE_END,
	HEADER_END_LF,
	// Message-Content
	MESSAGE_BODY,
	// Final state
	PARSING_DONE
} ParserState;

typedef enum { CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09 } LexConst;

typedef struct {
	char header_name[HEADER_NAME_BUFFER_SIZE];
	char header_value[HEADER_VALUE_BUFFER_SIZE];
	size_t _header_name_current_index;
	size_t _header_value_current_index;
} poc_Header_Pair;

typedef struct {
	poc_Header_Pair *http_header_pairs;
	size_t _total_headers_pairs;
	size_t _current_header_index;
} poc_Header;

typedef struct {
	unsigned char *buffer;
	size_t _remaining_memory;
	size_t _current_index;
} poc_Buffer;

typedef struct {
	poc_Header *http_headers;
	poc_Buffer *http_request_resource;
	poc_Buffer *http_version;
	poc_Buffer *http_method;
	poc_Buffer *http_message_body;
} poc_HTTP_Request_Message;

static poc_Header *poc_allocate_http_header(size_t num_header) {
	poc_Header *http_header =
	    (poc_Header *)POC_ALLOCATOR(1, sizeof(poc_Header));
	http_header->http_header_pairs = (poc_Header_Pair *)POC_ALLOCATOR(
	    num_header, sizeof(poc_Header_Pair));
	http_header->_total_headers_pairs = num_header;
	return http_header;
}

static poc_Buffer *poc_allocate_buffer(size_t buffer_size) {
	poc_Buffer *buffer = (poc_Buffer *)POC_ALLOCATOR(1, sizeof(poc_Buffer));
	buffer->buffer = (unsigned char *)POC_ALLOCATOR(
	    1, sizeof(unsigned char) * buffer_size);
	buffer->_remaining_memory = buffer_size;
	return buffer;
}

static poc_HTTP_Request_Message *poc_allocate_http_request_message(
    size_t num_header, size_t message_body_buffer_size,
    size_t request_resource_buffer_Size, size_t http_version_buffer_size,
    size_t http_method_buffer_size) {
	poc_HTTP_Request_Message *http_request_message =
	    (poc_HTTP_Request_Message *)POC_ALLOCATOR(
		1, sizeof(poc_HTTP_Request_Message));
	http_request_message->http_headers =
	    poc_allocate_http_header(num_header);
	http_request_message->http_message_body =
	    poc_allocate_buffer(message_body_buffer_size);
	http_request_message->http_request_resource =
	    poc_allocate_buffer(request_resource_buffer_Size);
	http_request_message->http_version =
	    poc_allocate_buffer(http_version_buffer_size);
	http_request_message->http_method =
	    poc_allocate_buffer(http_method_buffer_size);
	return http_request_message;
}

static void poc_free_buffer(poc_Buffer *buffer) {
	free(buffer->buffer);
	free(buffer);
}

static void poc_free_header(poc_Header *http_header) {
	free(http_header->http_header_pairs);
	free(http_header);
}

static void
poc_free_http_request_message(poc_HTTP_Request_Message *http_message) {
	poc_free_buffer(http_message->http_message_body);
	poc_free_buffer(http_message->http_method);
	poc_free_buffer(http_message->http_request_resource);
	poc_free_buffer(http_message->http_version);
	poc_free_header(http_message->http_headers);
	free(http_message);
}

static bool _poc_is_seperator(char value) {
	switch (value) {
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

#define POC_IS_CHAR(CHAR_VALUE) ((unsigned)CHAR_VALUE <= 127)
#define POC_IS_CONTROL(CHAR_VALUE)                                             \
	((CHAR_VALUE >= 0 && CHAR_VALUE <= 31) || (CHAR_VALUE == 127))
#define POC_IS_TOKEN(CHAR_VALUE)                                               \
	(POC_IS_CHAR(CHAR_VALUE) &&                                            \
	 !(POC_IS_CONTROL(CHAR_VALUE) || _poc_is_seperator(CHAR_VALUE)))
#define POC_IS_TEXT(CHAR_VALUE)                                                \
	(!POC_IS_CONTROL(CHAR_VALUE) || (CHAR_VALUE == (char)SP) ||            \
	 (CHAR_VALUE == HT))

static bool poc_http_state_machine_parser(
    poc_HTTP_Request_Message *http_message, ParserState *current_state,
    const char *input_buffer, size_t buffer_size, bool *is_protocol_failur) {
#define POC_APPEND_CHAR_MESSAGE_BUFFER(HTTP_MESSAGE, BUFFER_TYPE, CHAR_VALUE)  \
	do {                                                                   \
		if (HTTP_MESSAGE->BUFFER_TYPE->_current_index >=               \
		    HTTP_MESSAGE->BUFFER_TYPE->_remaining_memory)              \
			return false;                                          \
		HTTP_MESSAGE->BUFFER_TYPE                                      \
		    ->buffer[HTTP_MESSAGE->BUFFER_TYPE->_current_index] =      \
		    CHAR_VALUE;                                                \
		HTTP_MESSAGE->BUFFER_TYPE->_current_index++;                   \
	} while (0)

#define POC_APPEND_CHAR_HEADER_NAME(HTTP_MESSAGE, CHAR_VALUE)                  \
	do {                                                                   \
		size_t current_header_index =                                  \
		    HTTP_MESSAGE->http_headers->_current_header_index;         \
		if (current_header_index >=                                    \
		    HTTP_MESSAGE->http_headers->_total_headers_pairs)          \
			return false;                                          \
		size_t current_header_name_index =                             \
		    HTTP_MESSAGE->http_headers                                 \
			->http_header_pairs[current_header_index]              \
			._header_name_current_index;                           \
		if (current_header_index >=                                    \
		    HTTP_MESSAGE->http_headers->_total_headers_pairs)          \
			return false;                                          \
		if (current_header_name_index >= HEADER_NAME_BUFFER_SIZE)      \
			return false;                                          \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    .header_name[current_header_name_index] = CHAR_VALUE;      \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    ._header_name_current_index++;                             \
	} while (0)

#define POC_APPEND_CHAR_HEADER_VALUE(HTTP_MESSAGE, CHAR_VALUE)                 \
	do {                                                                   \
		size_t current_header_index =                                  \
		    HTTP_MESSAGE->http_headers->_current_header_index;         \
		if (current_header_index >=                                    \
		    HTTP_MESSAGE->http_headers->_total_headers_pairs)          \
			return false;                                          \
		size_t current_header_value_index =                            \
		    HTTP_MESSAGE->http_headers                                 \
			->http_header_pairs[current_header_index]              \
			._header_value_current_index;                          \
		if (current_header_index >=                                    \
		    HTTP_MESSAGE->http_headers->_total_headers_pairs)          \
			return false;                                          \
		if (current_header_value_index >= HEADER_NAME_BUFFER_SIZE)     \
			return false;                                          \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    .header_value[current_header_value_index] = CHAR_VALUE;    \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    ._header_value_current_index++;                            \
	} while (0)

#define POC_INCREMENT_CURRENT_HEADER_PAIR_INDEX(HTTP_MESSAGE)                  \
	HTTP_MESSAGE->http_headers->_current_header_index++

	const char *buffer_end = &input_buffer[buffer_size];
	while (input_buffer != buffer_end) {
		switch (*current_state) {
		case REQUEST_LINE_BEGIN:
			if (POC_IS_TOKEN(*input_buffer)) {
				*current_state = REQUEST_METHOD;
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_method, *input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_METHOD:
			if (*input_buffer == (char)SP) {
				*current_state = REQUEST_RESOURCE;
				input_buffer++;
			} else if (POC_IS_TOKEN(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_method, *input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_RESOURCE:
			if (*input_buffer == (char)SP) {
				*current_state = REQUEST_PROTOCOL_H;
				input_buffer++;
			} else if (isprint(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_request_resource,
				    *input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_PROTOCOL_H:
			if (*input_buffer == 'H') {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, 'H');
				*current_state = REQUEST_PROTOCOL_T1;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_PROTOCOL_T1:
			if (*input_buffer == 'T') {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, 'T');
				*current_state = REQUEST_PROTOCOL_T2;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_PROTOCOL_T2:
			if (*input_buffer == 'T') {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, 'T');
				*current_state = REQUEST_PROTOCOL_P;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_PROTOCOL_P:
			if (*input_buffer == 'P') {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, 'P');
				*current_state = REQUEST_PROTOCOL_SLASH;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_PROTOCOL_SLASH:
			if (*input_buffer == '/') {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, '/');
				*current_state = REQUEST_PROTOCOL_VERSION_MAJOR;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_PROTOCOL_VERSION_MAJOR:
			if (isdigit(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, *input_buffer);
				input_buffer++;
			} else if (*input_buffer == '.') {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, '.');
				*current_state = REQUEST_PROTOCOL_VERSION_MINOR;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_PROTOCOL_VERSION_MINOR:
			if (isdigit(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER(
				    http_message, http_version, *input_buffer);
				input_buffer++;
			} else if (*input_buffer == (char)CR) {
				*current_state = REQUEST_LINE_LF;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case REQUEST_LINE_LF:
			if (*input_buffer == (char)LF) {
				*current_state = HEADER_NAME;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case HEADER_NAME:
			if (POC_IS_TOKEN(*input_buffer)) {
				POC_APPEND_CHAR_HEADER_NAME(http_message,
							    *input_buffer);
				input_buffer++;
			} else if (*input_buffer == ':') {
				*current_state = HEADER_VALUE;
				input_buffer++;
			} else if (*input_buffer == (char)CR) {
				*current_state = HEADER_END_LF;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case HEADER_VALUE:
			if (*input_buffer == (char)CR) {
				*current_state = HEADER_VALUE_LF;
				input_buffer++;
			} else if (*input_buffer == (char)SP &&
				   *(input_buffer - 1) == ':') {
				input_buffer++;
			} else if (POC_IS_TEXT(*input_buffer)) {
				POC_APPEND_CHAR_HEADER_VALUE(http_message,
							     *input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case HEADER_VALUE_LF:
			if (*input_buffer == (char)LF) {
				*current_state = HEADER_VALUE_END;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR;
			}
			break;
		case HEADER_VALUE_END:
			POC_INCREMENT_CURRENT_HEADER_PAIR_INDEX(http_message);
			*current_state = HEADER_NAME;
			break;
		case HEADER_END_LF:
			if (*input_buffer == (char)LF) {
				if (poc_str3cmp(
					http_message->http_method->buffer,
					(unsigned char *)"GET")) {
					*current_state = PARSING_DONE;
					input_buffer++;
					// clang-format off
				} else if (poc_str4cmp(
					       http_message->http_method->buffer,
					       (unsigned char *)"POST")) {
					// clang-format on
					*current_state = MESSAGE_BODY;
					input_buffer++;
				} else {
					input_buffer++;
					goto FINISH;
				}
			}
			break;
		case MESSAGE_BODY:
			if (input_buffer + 1 == buffer_end) {
				*current_state = PARSING_DONE;
			}
			POC_APPEND_CHAR_MESSAGE_BUFFER(
			    http_message, http_message_body, *input_buffer);
			input_buffer++;
			break;
		case PARSING_DONE:
			goto FINISH;
		case PROTOCOL_ERROR:
			*is_protocol_failur = true;
			return false;
		default:
			printf("\nUnsupported\n");
			return false;
		}
	}
FINISH:
	return true;
}

#endif
