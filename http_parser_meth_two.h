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
#ifndef HTTP_PARSER_TWO_H
#define HTTP_PARSER_TWO_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HEADER_NAME_BUFFER_SIZE 1024
#define HEADER_VALUE_BUFFER_SIZE 1024
#define POC_ALLOCATOR(type, num_of_elements)                                   \
	calloc(num_of_elements, sizeof(type))

// Fast strcmp helpers
#define str4cmp_macro(ptr, c0, c1, c2, c3)                                     \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3
static bool str4cmp(const char *ptr, const char *cmp) {
	return str4cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
			     *(cmp + 3));
}
#define str5cmp_macro(ptr, c0, c1, c2, c3, c4)                                 \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3 &&*(ptr + 4) == c4
static bool str5cmp(const char *ptr, const char *cmp) {
	return str5cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
			     *(cmp + 3), *(cmp + 4));
}
#define str7cmp_macro(ptr, c0, c1, c2, c3, c4, c5, c6)                         \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3 &&*(ptr + 4) == c4 &&*(ptr + 5) == c5 &&*(ptr + 6) == c6
static bool str7cmp(const char *ptr, const char *cmp) {
	return str7cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
			     *(cmp + 3), *(cmp + 4), *(cmp + 5), *(cmp + 6));
}
#define str10cmp_macro(ptr, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9)            \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3 &&*(ptr + 4) == c4 &&*(ptr + 5) == c5 &&*(ptr + 6) ==           \
	    c6 &&*(ptr + 7) == c7 &&*(ptr + 8) == c8 &&*(ptr + 9) == c9
static bool str10cmp(const char *ptr, const char *cmp) {
	return str10cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
			      *(cmp + 3), *(cmp + 4), *(cmp + 5), *(cmp + 6),
			      *(cmp + 7), *(cmp + 8), *(cmp + 9));
}

typedef enum {
	POC_HTTP_GET = 10,
	POC_HTTP_POST,
	POC_HTTP_PUT,
	POC_HTTP_DELETE,
	POC_HTTP_HEAD,
} poc_HTTP_Method;

typedef enum { CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09 } LexConst;

typedef enum {
	ERROR_UNSUPPORTED_HTTP_METHOD = 10,
	ERROR_UNSUPPORTED_HTTP_VERSION,
	ERROR_PROTOCOL_ERROR,
	ERROR_BUFFER_OVERFLOW
} poc_Parser_Error;

typedef enum { HTTP_1_1 = 10, HTTP_1_0 } poc_HTTP_Version;

typedef struct {
	char header_name[HEADER_NAME_BUFFER_SIZE];
	char header_value[HEADER_VALUE_BUFFER_SIZE];
	size_t header_name_current_index;
	size_t header_value_current_index;
} poc_Header_Pair;

typedef struct {
	poc_Header_Pair *http_header_pairs;
	size_t total_header_pair;
	size_t current_index;
} poc_Header;

typedef struct {
	unsigned char *buffer;
	size_t total_memory;
	size_t current_index;
} poc_Buffer;

static poc_Buffer *poc_allocate_http_buffer(size_t buffer_size) {
	poc_Buffer *http_buffer = (poc_Buffer *)POC_ALLOCATOR(poc_Buffer, 1);
	http_buffer->buffer =
	    (unsigned char *)POC_ALLOCATOR(unsigned char, buffer_size);
	http_buffer->current_index = 0;
	http_buffer->total_memory = buffer_size;
	return http_buffer;
}

static poc_Header *poc_allocate_http_header(size_t number_of_headers) {
	poc_Header *http_header = (poc_Header *)POC_ALLOCATOR(poc_Header, 1);
	http_header->http_header_pairs = (poc_Header_Pair *)POC_ALLOCATOR(
	    poc_Header_Pair, number_of_headers);
	http_header->total_header_pair = number_of_headers;
	http_header->current_index = 0;
	for (size_t i = 0; i < number_of_headers; i++) {
		memset(http_header->http_header_pairs[i].header_name, 0,
		       HEADER_NAME_BUFFER_SIZE);
		memset(http_header->http_header_pairs[i].header_value, 0,
		       HEADER_VALUE_BUFFER_SIZE);
		http_header->http_header_pairs[i].header_name_current_index = 0;
		http_header->http_header_pairs[i].header_value_current_index =
		    0;
	}
	return http_header;
}

static void poc_free_buffer(poc_Buffer *buffer) {
	free(buffer->buffer);
	free(buffer);
}

static void poc_free_http_header(poc_Header *http_header) {
	free(http_header->http_header_pairs);
	free(http_header);
}

#define POC_IS_SEPERATOR(CHAR_VALUE)                                           \
	((CHAR_VALUE == '(') || (CHAR_VALUE == ')') || (CHAR_VALUE == '<') ||  \
	 (CHAR_VALUE == '>') || (CHAR_VALUE == '@') || (CHAR_VALUE == ',') ||  \
	 (CHAR_VALUE == ';') || (CHAR_VALUE == ':') || (CHAR_VALUE == '\\') || \
	 (CHAR_VALUE == '"') || (CHAR_VALUE == '/') || (CHAR_VALUE == '[') ||  \
	 (CHAR_VALUE == ']') || (CHAR_VALUE == '?') || (CHAR_VALUE == '=') ||  \
	 (CHAR_VALUE == '{') || (CHAR_VALUE == '}') ||                         \
	 (CHAR_VALUE == (char)SP) || (CHAR_VALUE == (char)HT))
#define POC_IS_PRINTABLE_CHAR(CHAR_VALUE)                                      \
	(((unsigned)CHAR_VALUE >= 0x20) && ((unsigned)CHAR_VALUE < 0x7F))
#define POC_IS_CHAR(CHAR_VALUE) ((unsigned)CHAR_VALUE <= 127)
#define POC_IS_CONTROL(CHAR_VALUE)                                             \
	((CHAR_VALUE >= 0 && CHAR_VALUE <= 31) || (CHAR_VALUE == 127))
#define POC_IS_TOKEN(CHAR_VALUE)                                               \
	(POC_IS_CHAR(CHAR_VALUE) &&                                            \
	 !(POC_IS_CONTROL(CHAR_VALUE) || POC_IS_SEPERATOR(CHAR_VALUE)))
#define POC_IS_TEXT(CHAR_VALUE)                                                \
	(!POC_IS_CONTROL(CHAR_VALUE) || (CHAR_VALUE) == (char)SP ||            \
	 (CHAR_VALUE) == HT)

static bool poc_http_parser(poc_Buffer *http_body, poc_Header *http_header,
			    const char *raw_input_buffer,
			    size_t input_buffer_size,
			    poc_HTTP_Method *http_method,
			    char *request_resource,
			    poc_HTTP_Version *http_version,
			    poc_Parser_Error *http_parser_error) {
	size_t current_buffer_index = 0;
#define POC_INCREMENT_BUFFER_OFFSET(offset_length)                             \
	raw_input_buffer += offset_length;                                     \
	current_buffer_index += offset_length

#define CHECK_IF_GET(input_buffer)                                             \
	do {                                                                   \
		if (str4cmp(input_buffer, "GET ")) {                           \
			*http_method = POC_HTTP_GET;                           \
			POC_INCREMENT_BUFFER_OFFSET(4);                        \
			goto PARSE_HTTP_REQUEST_METHOD;                        \
		}                                                              \
	} while (0)

#define CHECK_IF_POST(input_buffer)                                            \
	do {                                                                   \
		if (str5cmp(input_buffer, "POST ")) {                          \
			*http_method = POC_HTTP_POST;                          \
			POC_INCREMENT_BUFFER_OFFSET(5);                        \
			goto PARSE_HTTP_REQUEST_METHOD;                        \
		}                                                              \
	} while (0)

#define CHECK_IF_HEAD(input_buffer)                                            \
	do {                                                                   \
		if (str5cmp(input_buffer, "HEAD ")) {                          \
			*http_method = POC_HTTP_HEAD;                          \
			POC_INCREMENT_BUFFER_OFFSET(5);                        \
			goto PARSE_HTTP_REQUEST_METHOD;                        \
		}                                                              \
	} while (0)

#define CHECK_IF_PUT(input_buffer)                                             \
	do {                                                                   \
		if (str4cmp(input_buffer, "PUT ")) {                           \
			*http_method = POC_HTTP_PUT;                           \
			POC_INCREMENT_BUFFER_OFFSET(4);                        \
			goto PARSE_HTTP_REQUEST_METHOD;                        \
		}                                                              \
	} while (0)

#define CHECK_IF_DELETE(input_buffer)                                          \
	do {                                                                   \
		if (str7cmp(input_buffer, "DELETE ")) {                        \
			*http_method = POC_HTTP_PUT;                           \
			POC_INCREMENT_BUFFER_OFFSET(7);                        \
			goto PARSE_HTTP_REQUEST_METHOD;                        \
		}                                                              \
	} while (0)

#define HANDLE_UNSUPPORTED_METHOD()                                            \
	do {                                                                   \
		*http_parser_error = ERROR_UNSUPPORTED_HTTP_METHOD;            \
		return false;                                                  \
	} while (0)

#define CHECK_IF_HTTP_VERSION_SUPPORTED(input_buffer)                          \
	do {                                                                   \
		if (str10cmp(input_buffer, "HTTP/1.1\r\n")) {                  \
			*http_version = HTTP_1_1;                              \
			POC_INCREMENT_BUFFER_OFFSET(10);                       \
			goto PARSE_HTTP_HEADERS;                               \
		} else {                                                       \
			*http_parser_error = ERROR_UNSUPPORTED_HTTP_VERSION;   \
			return false;                                          \
		}                                                              \
	} while (0)

#define POC_APPEND_CHAR_TO_HEADER_NAME(CHAR_VALUE, header)                     \
	do {                                                                   \
		size_t header_name_current_index =                             \
		    header->http_header_pairs[header->current_index]           \
			.header_name_current_index++;                          \
		if (header->current_index >= header->total_header_pair) {      \
			*http_parser_error = ERROR_BUFFER_OVERFLOW;            \
			return false;                                          \
		}                                                              \
		if (header_name_current_index >= HEADER_NAME_BUFFER_SIZE) {    \
			*http_parser_error = ERROR_BUFFER_OVERFLOW;            \
			return false;                                          \
		}                                                              \
		header->http_header_pairs[header->current_index]               \
		    .header_name[header_name_current_index] = CHAR_VALUE;      \
	} while (0)

#define POC_APPEND_CHAR_TO_HEADER_VALUE(CHAR_VALUE, header)                    \
	do {                                                                   \
		size_t header_value_current_index =                            \
		    header->http_header_pairs[header->current_index]           \
			.header_value_current_index++;                         \
		if (header->current_index >= header->total_header_pair) {      \
			*http_parser_error = ERROR_BUFFER_OVERFLOW;            \
			return false;                                          \
		}                                                              \
		if (header_value_current_index >= HEADER_VALUE_BUFFER_SIZE) {  \
			*http_parser_error = ERROR_BUFFER_OVERFLOW;            \
			return false;                                          \
		}                                                              \
		header->http_header_pairs[header->current_index]               \
		    .header_value[header_value_current_index] = CHAR_VALUE;    \
	} while (0)

#define POC_PROTOCOL_ERROR_HANDLER()                                           \
	*http_parser_error = ERROR_PROTOCOL_ERROR;                             \
	return false

	CHECK_IF_GET(raw_input_buffer);
	CHECK_IF_POST(raw_input_buffer);
	CHECK_IF_HEAD(raw_input_buffer);
	CHECK_IF_PUT(raw_input_buffer);
	CHECK_IF_DELETE(raw_input_buffer);
	HANDLE_UNSUPPORTED_METHOD();

PARSE_HTTP_REQUEST_METHOD:
	while (*raw_input_buffer != (char)SP &&
	       POC_IS_PRINTABLE_CHAR(*raw_input_buffer)) {
		*request_resource++ = *raw_input_buffer;
		POC_INCREMENT_BUFFER_OFFSET(1);
	}
	if (*raw_input_buffer == (char)SP) {
		POC_INCREMENT_BUFFER_OFFSET(1);
		goto PARSE_HTTP_VERSION;
	}

PARSE_HTTP_VERSION:
	CHECK_IF_HTTP_VERSION_SUPPORTED(raw_input_buffer);

PARSE_HTTP_HEADERS:;
	// Small internal state-machine for parsing headers
	typedef enum {
		_HEADER_NAME,
		_HEADER_VALUE,
		_HEADER_LF,
		_HEADER_END_LF
	} _Header_Parsing_State;
	_Header_Parsing_State current_state = _HEADER_NAME;
	for (;;) {
		switch (current_state) {
		case _HEADER_NAME:
			if (*raw_input_buffer == (char)CR) {
				POC_INCREMENT_BUFFER_OFFSET(1);
				current_state = _HEADER_END_LF;
			} else if (POC_IS_TOKEN(*raw_input_buffer)) {
				POC_APPEND_CHAR_TO_HEADER_NAME(
				    *raw_input_buffer, http_header);
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else if (*raw_input_buffer == ':') {
				current_state = _HEADER_VALUE;
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else {
				POC_PROTOCOL_ERROR_HANDLER();
			}
			break;
		case _HEADER_VALUE:
			if (*raw_input_buffer == (char)CR) {
				POC_INCREMENT_BUFFER_OFFSET(1);
				current_state = _HEADER_LF;
			} else if (*raw_input_buffer == (char)SP &&
				   *(raw_input_buffer - 1) == ':') {
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else if (POC_IS_TEXT(*raw_input_buffer)) {
				POC_APPEND_CHAR_TO_HEADER_VALUE(
				    *raw_input_buffer, http_header);
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else {
				POC_PROTOCOL_ERROR_HANDLER();
			}
			break;
		case _HEADER_LF:
			if (*raw_input_buffer == (char)LF) {
				POC_INCREMENT_BUFFER_OFFSET(1);
				current_state = _HEADER_NAME;
				http_header->current_index++;
			} else {
				POC_PROTOCOL_ERROR_HANDLER();
			}
			break;
		case _HEADER_END_LF:
			if (*raw_input_buffer == (char)LF) {
				POC_INCREMENT_BUFFER_OFFSET(1);
				goto PARSE_HTTP_BODY;
			} else {
				POC_PROTOCOL_ERROR_HANDLER();
			}
			break;
		}
	}

PARSE_HTTP_BODY:;
	size_t total_remaining_size = (input_buffer_size)-current_buffer_index;
	if (total_remaining_size >= http_body->total_memory) {
		*http_parser_error = ERROR_BUFFER_OVERFLOW;
		return false;
	}
	memcpy(http_body->buffer, raw_input_buffer, total_remaining_size);
	http_body->current_index = (total_remaining_size - 1);
	return true;
}

#endif // !HTTP_PARSER_TWO_H
