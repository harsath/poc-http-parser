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
#ifndef DOXYGEN_SHOULD_SKIP_THIS
// NOTE: 'sm' suffix referes to 'State-Machine', it's named that way due to name
// 	 collision in benchmarks/tests

#define poc_str3cmp_macro_sm(ptr, c0, c1, c2)                                  \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2
static bool poc_str3cmp_sm(const unsigned char *ptr, const unsigned char *cmp) {
	return poc_str3cmp_macro_sm(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2));
}
#define poc_str4cmp_macro_sm(ptr, c0, c1, c2, c3)                              \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3
static bool poc_str4cmp_sm(const unsigned char *ptr, const unsigned char *cmp) {
	return poc_str4cmp_macro_sm(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
				    *(cmp + 3));
}
#define str5cmp_macro_sm(ptr, c0, c1, c2, c3, c4)                              \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3 &&*(ptr + 4) == c4
static bool str5cmp_sm(const unsigned char *ptr, const unsigned char *cmp) {
	return str5cmp_macro_sm(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
				*(cmp + 3), *(cmp + 4));
}
#define POC_ALLOCATOR_SM(num_of_elements, size_of_single_type)                 \
	calloc(num_of_elements, size_of_single_type)
#define HEADER_VALUE_BUFFER_SIZE_SM 100
#define HEADER_NAME_BUFFER_SIZE_SM 1024
#define HTTP_VERSION_BUFFER_SIZE_SM 8

typedef enum {
	PROTOCOL_ERROR_sm,
	// Request-Line
	REQUEST_LINE_BEGIN_sm = 100,
	REQUEST_METHOD_sm,
	REQUEST_RESOURCE_sm,
	REQUEST_PROTOCOL_H_sm,
	REQUEST_PROTOCOL_T1_sm,
	REQUEST_PROTOCOL_T2_sm,
	REQUEST_PROTOCOL_P_sm,
	REQUEST_PROTOCOL_SLASH_sm,
	REQUEST_PROTOCOL_VERSION_MAJOR_sm,
	REQUEST_PROTOCOL_VERSION_MINOR_sm,
	REQUEST_LINE_LF_sm,
	// Message-Header
	HEADER_NAME_sm,
	HEADER_VALUE_sm,
	HEADER_VALUE_LF_sm,
	HEADER_VALUE_END_sm,
	HEADER_END_LF_sm,
	// Message-Content
	MESSAGE_BODY_sm,
	// Final state
	PARSING_DONE_sm
} ParserState_sm;

typedef enum {
	CR_sm = 0x0D,
	LF_sm = 0x0A,
	SP_sm = 0x20,
	HT_sm = 0x09
} LexConst_sm;

typedef struct {
	char header_name[HEADER_NAME_BUFFER_SIZE_SM];
	char header_value[HEADER_VALUE_BUFFER_SIZE_SM];
	size_t _header_name_current_index;
	size_t _header_value_current_index;
} poc_Header_Pair_sm;

typedef struct {
	poc_Header_Pair_sm *http_header_pairs;
	size_t _total_headers_pairs;
	size_t _current_header_index;
} poc_Header_sm;

typedef struct {
	unsigned char *buffer;
	size_t _remaining_memory;
	size_t _current_index;
} poc_Buffer_sm;

typedef struct {
	poc_Header_sm *http_headers;
	poc_Buffer_sm *http_request_resource;
	poc_Buffer_sm *http_version;
	poc_Buffer_sm *http_method;
	poc_Buffer_sm *http_message_body;
} poc_HTTP_Request_Message_sm;
#endif // !DOXYGEN_SHOULD_SKIP_THIS

/**
 * Allocates a poc_Header_sm object for storing parsed HTTP message' headers
 *
 * @param num_header Pre-defined size of headers
 * @return Dynamically allocated poc_Header_sm object.
 */
static poc_Header_sm *poc_allocate_http_header_sm(size_t num_header) {
	poc_Header_sm *http_header =
	    (poc_Header_sm *)POC_ALLOCATOR_SM(1, sizeof(poc_Header_sm));
	http_header->http_header_pairs = (poc_Header_Pair_sm *)POC_ALLOCATOR_SM(
	    num_header, sizeof(poc_Header_Pair_sm));
	http_header->_total_headers_pairs = num_header;
	return http_header;
}

/**
 * Allocates a poc_Buffer_sm object for storing parsed data from HTTP message.
 *
 * @param buffer_size Size of the poc_Buffer_sm to allocate
 * @return Dynamically allocated poc_Buffer_sm object.
 */
static poc_Buffer_sm *poc_allocate_buffer_sm(size_t buffer_size) {
	poc_Buffer_sm *buffer =
	    (poc_Buffer_sm *)POC_ALLOCATOR_SM(1, sizeof(poc_Buffer_sm));
	buffer->buffer = (unsigned char *)POC_ALLOCATOR_SM(
	    1, sizeof(unsigned char) * buffer_size);
	buffer->_remaining_memory = buffer_size;
	return buffer;
}

/**
 * Allocate a poc_HTTP_Request_Message_sm for storing a parsed version of
 * HTTP 1.x message representation
 *
 * @param num_header Pre-define size of headers
 * @param message_body_buffer_size Pre-define size of HTTP 1.x message's message
 * body size in bytes
 * @param request_resource_buffer_Size Size of HTTP 1.x request message's
 * request-resource size.
 * @param http_version_buffer_size Buffer size for storing HTTP message's
 * version
 * @param http_method_buffer_size Buffer size for storing HTTP message's method.
 * @return Dynamically allocated poc_HTTP_Request_Message_sm for representing a
 * HTTP 1.x request message
 */
static poc_HTTP_Request_Message_sm *poc_allocate_http_request_message_sm(
    size_t num_header, size_t message_body_buffer_size,
    size_t request_resource_buffer_Size, size_t http_version_buffer_size,
    size_t http_method_buffer_size) {
	poc_HTTP_Request_Message_sm *http_request_message =
	    (poc_HTTP_Request_Message_sm *)POC_ALLOCATOR_SM(
		1, sizeof(poc_HTTP_Request_Message_sm));
	http_request_message->http_headers =
	    poc_allocate_http_header_sm(num_header);
	http_request_message->http_message_body =
	    poc_allocate_buffer_sm(message_body_buffer_size);
	http_request_message->http_request_resource =
	    poc_allocate_buffer_sm(request_resource_buffer_Size);
	http_request_message->http_version =
	    poc_allocate_buffer_sm(http_version_buffer_size);
	http_request_message->http_method =
	    poc_allocate_buffer_sm(http_method_buffer_size);
	return http_request_message;
}

/**
 * Frees a poc_Buffer_sm previously allocated through the helper function
 *
 * @param buffer poc_Buffer_sm object to be freed.
 */
static void poc_free_buffer_sm(poc_Buffer_sm *buffer) {
	free(buffer->buffer);
	free(buffer);
}

/**
 * Frees the HTTP header object, poc_Header_sm
 *
 * @param http_header poc_Header_sm object to be freed
 */
static void poc_free_header_sm(poc_Header_sm *http_header) {
	free(http_header->http_header_pairs);
	free(http_header);
}

/**
 * Free a whole poc_HTTP_Request_Message_sm object
 *
 * @param http_message Object to free.
 */
static void
poc_free_http_request_message_sm(poc_HTTP_Request_Message_sm *http_message) {
	poc_free_buffer_sm(http_message->http_message_body);
	poc_free_buffer_sm(http_message->http_method);
	poc_free_buffer_sm(http_message->http_request_resource);
	poc_free_buffer_sm(http_message->http_version);
	poc_free_header_sm(http_message->http_headers);
	free(http_message);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static bool _poc_is_seperator_sm(char value) {
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
	case (char)SP_sm:
	case (char)HT_sm:
		return true;
	default:
		return false;
	}
}

#define POC_IS_CHAR_SM(CHAR_VALUE) ((unsigned)CHAR_VALUE <= 127)
#define POC_IS_CONTROL_SM(CHAR_VALUE)                                          \
	((CHAR_VALUE >= 0 && CHAR_VALUE <= 31) || (CHAR_VALUE == 127))
#define POC_IS_TOKEN_SM(CHAR_VALUE)                                            \
	(POC_IS_CHAR_SM(CHAR_VALUE) &&                                         \
	 !(POC_IS_CONTROL_SM(CHAR_VALUE) || _poc_is_seperator_sm(CHAR_VALUE)))
#define POC_IS_TEXT_SM(CHAR_VALUE)                                             \
	(!POC_IS_CONTROL_SM(CHAR_VALUE) || (CHAR_VALUE == (char)SP_sm) ||      \
	 (CHAR_VALUE == HT_sm))
#endif // !DOXYGEN_SHOULD_SKIP_THIS

/**
 * State machine parser for parsing HTTP 1.x Request-Message
 *
 * @param http_message poc_HTTP_Request_Message_sm object buffer for storing the
 * parsed HTTP message bytes.
 * @param current_state Current state of the HTTP state-machine(will be updated
 * as we incrementally parse)
 * @param input_buffer Raw HTTP request bytes for parsing read through wire
 * @param buffer_size Size of the raw bytes
 * @param is_protocol_failur Boolean indicator if the HTTP request bytes does
 * not follow HTTP 1.x protocol specification
 * @return True if state-machine parsed the raw bytes successfully, if not
 * returns False.
 */
static bool poc_http_state_machine_parser_sm(
    poc_HTTP_Request_Message_sm *http_message, ParserState_sm *current_state,
    const char *input_buffer, size_t buffer_size, bool *is_protocol_failur) {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define POC_APPEND_CHAR_MESSAGE_BUFFER_SM(HTTP_MESSAGE, BUFFER_TYPE,           \
					  CHAR_VALUE)                          \
	do {                                                                   \
		if (HTTP_MESSAGE->BUFFER_TYPE->_current_index >=               \
		    HTTP_MESSAGE->BUFFER_TYPE->_remaining_memory)              \
			return false;                                          \
		HTTP_MESSAGE->BUFFER_TYPE                                      \
		    ->buffer[HTTP_MESSAGE->BUFFER_TYPE->_current_index] =      \
		    CHAR_VALUE;                                                \
		HTTP_MESSAGE->BUFFER_TYPE->_current_index++;                   \
	} while (0)

#define POC_APPEND_CHAR_HEADER_NAME_SM(HTTP_MESSAGE, CHAR_VALUE)               \
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
		if (current_header_name_index >= HEADER_NAME_BUFFER_SIZE_SM)   \
			return false;                                          \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    .header_name[current_header_name_index] = CHAR_VALUE;      \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    ._header_name_current_index++;                             \
	} while (0)

#define POC_APPEND_CHAR_HEADER_VALUE_SM(HTTP_MESSAGE, CHAR_VALUE)              \
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
		if (current_header_value_index >= HEADER_NAME_BUFFER_SIZE_SM)  \
			return false;                                          \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    .header_value[current_header_value_index] = CHAR_VALUE;    \
		HTTP_MESSAGE->http_headers                                     \
		    ->http_header_pairs[current_header_index]                  \
		    ._header_value_current_index++;                            \
	} while (0)

#define POC_INCREMENT_CURRENT_HEADER_PAIR_INDEX_SM(HTTP_MESSAGE)               \
	HTTP_MESSAGE->http_headers->_current_header_index++
#endif // !DOXYGEN_SHOULD_SKIP_THIS
	const char *buffer_end = &input_buffer[buffer_size];
	while (input_buffer != buffer_end) {
		switch (*current_state) {
		case REQUEST_LINE_BEGIN_sm:
			if (POC_IS_TOKEN_SM(*input_buffer)) {
				*current_state = REQUEST_METHOD_sm;
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_method, *input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_METHOD_sm:
			if (*input_buffer == (char)SP_sm) {
				*current_state = REQUEST_RESOURCE_sm;
				input_buffer++;
			} else if (POC_IS_TOKEN_SM(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_method, *input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_RESOURCE_sm:
			if (*input_buffer == (char)SP_sm) {
				*current_state = REQUEST_PROTOCOL_H_sm;
				input_buffer++;
			} else if (isprint(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_request_resource,
				    *input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_PROTOCOL_H_sm:
			if (*input_buffer == 'H') {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, 'H');
				*current_state = REQUEST_PROTOCOL_T1_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_PROTOCOL_T1_sm:
			if (*input_buffer == 'T') {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, 'T');
				*current_state = REQUEST_PROTOCOL_T2_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_PROTOCOL_T2_sm:
			if (*input_buffer == 'T') {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, 'T');
				*current_state = REQUEST_PROTOCOL_P_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_PROTOCOL_P_sm:
			if (*input_buffer == 'P') {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, 'P');
				*current_state = REQUEST_PROTOCOL_SLASH_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_PROTOCOL_SLASH_sm:
			if (*input_buffer == '/') {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, '/');
				*current_state =
				    REQUEST_PROTOCOL_VERSION_MAJOR_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_PROTOCOL_VERSION_MAJOR_sm:
			if (isdigit(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, *input_buffer);
				input_buffer++;
			} else if (*input_buffer == '.') {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, '.');
				*current_state =
				    REQUEST_PROTOCOL_VERSION_MINOR_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_PROTOCOL_VERSION_MINOR_sm:
			if (isdigit(*input_buffer)) {
				POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
				    http_message, http_version, *input_buffer);
				input_buffer++;
			} else if (*input_buffer == (char)CR_sm) {
				*current_state = REQUEST_LINE_LF_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case REQUEST_LINE_LF_sm:
			if (*input_buffer == (char)LF_sm) {
				*current_state = HEADER_NAME_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case HEADER_NAME_sm:
			if (POC_IS_TOKEN_SM(*input_buffer)) {
				POC_APPEND_CHAR_HEADER_NAME_SM(http_message,
							       *input_buffer);
				input_buffer++;
			} else if (*input_buffer == ':') {
				*current_state = HEADER_VALUE_sm;
				input_buffer++;
			} else if (*input_buffer == (char)CR_sm) {
				*current_state = HEADER_END_LF_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case HEADER_VALUE_sm:
			if (*input_buffer == (char)CR_sm) {
				*current_state = HEADER_VALUE_LF_sm;
				input_buffer++;
			} else if (*input_buffer == (char)SP_sm &&
				   *(input_buffer - 1) == ':') {
				input_buffer++;
			} else if (POC_IS_TEXT_SM(*input_buffer)) {
				POC_APPEND_CHAR_HEADER_VALUE_SM(http_message,
								*input_buffer);
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case HEADER_VALUE_LF_sm:
			if (*input_buffer == (char)LF_sm) {
				*current_state = HEADER_VALUE_END_sm;
				input_buffer++;
			} else {
				*current_state = PROTOCOL_ERROR_sm;
			}
			break;
		case HEADER_VALUE_END_sm:
			POC_INCREMENT_CURRENT_HEADER_PAIR_INDEX_SM(
			    http_message);
			*current_state = HEADER_NAME_sm;
			break;
		case HEADER_END_LF_sm:
			if (*input_buffer == (char)LF_sm) {
				if (poc_str3cmp_sm(
					http_message->http_method->buffer,
					(unsigned char *)"GET")) {
					*current_state = PARSING_DONE_sm;
					input_buffer++;
					// clang-format off
				} else if (poc_str4cmp_sm(
					       http_message->http_method->buffer,
					       (unsigned char *)"POST")) {
					// clang-format on
					*current_state = MESSAGE_BODY_sm;
					input_buffer++;
				} else {
					input_buffer++;
					goto FINISH;
				}
			}
			break;
		case MESSAGE_BODY_sm:
			if (input_buffer + 1 == buffer_end) {
				*current_state = PARSING_DONE_sm;
			}
			POC_APPEND_CHAR_MESSAGE_BUFFER_SM(
			    http_message, http_message_body, *input_buffer);
			input_buffer++;
			break;
		case PARSING_DONE_sm:
			goto FINISH;
		case PROTOCOL_ERROR_sm:
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
