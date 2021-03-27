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
#ifndef HTTP_ZERO_ALLOC_PARSER_H
#define HTTP_ZERO_ALLOC_PARSER_H
#include <emmintrin.h>
#include <immintrin.h>
#include <nmmintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// NOTE: 'zm' suffix denotes 'Zero-Memory(allocation)', it's named that way to
//       avoid collision in benchmarks/tests

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef enum {
	CR_zm = 0x0D,
	LF_zm = 0x0A,
	SP_zm = 0x20,
	HT_zm = 0x09
} LexConst_zm;

typedef struct {
	const char *header_name;
	size_t header_name_len;
	const char *header_value;
	size_t header_value_len;
} poc_header_pair_zm;

#define POC_INIT_HEADER_PAIR_TO_ZERO_ZM(header_pair_ptr, pair_size)            \
	do {                                                                   \
		for (size_t i = 0; i < pair_size; i++) {                       \
			header_pair_ptr[i].header_name = NULL;                 \
			header_pair_ptr[i].header_name_len = 0;                \
			header_pair_ptr[i].header_value = NULL;                \
			header_pair_ptr[i].header_value_len = 0;               \
		}                                                              \
	} while (0)
#define POC_IS_SEPERATOR_ZM(CHAR_VALUE)                                        \
	((CHAR_VALUE == '(') || (CHAR_VALUE == ')') || (CHAR_VALUE == '<') ||  \
	 (CHAR_VALUE == '>') || (CHAR_VALUE == '@') || (CHAR_VALUE == ',') ||  \
	 (CHAR_VALUE == ';') || (CHAR_VALUE == ':') || (CHAR_VALUE == '\\') || \
	 (CHAR_VALUE == '"') || (CHAR_VALUE == '/') || (CHAR_VALUE == '[') ||  \
	 (CHAR_VALUE == ']') || (CHAR_VALUE == '?') || (CHAR_VALUE == '=') ||  \
	 (CHAR_VALUE == '{') || (CHAR_VALUE == '}') ||                         \
	 (CHAR_VALUE == (char)SP_zm) || (CHAR_VALUE == (char)HT_zm))
#define POC_IS_PRINTABLE_CHAR_ZM(CHAR_VALUE)                                   \
	(((unsigned)CHAR_VALUE >= 0x20) && ((unsigned)CHAR_VALUE < 0x7F))
#define POC_IS_CHAR_ZM(CHAR_VALUE) ((unsigned)CHAR_VALUE <= 127)
#define POC_IS_CONTROL_ZM(CHAR_VALUE)                                          \
	((CHAR_VALUE >= 0 && CHAR_VALUE <= 31) || (CHAR_VALUE == 127))
#define POC_IS_TOKEN_ZM(CHAR_VALUE)                                            \
	(POC_IS_CHAR_ZM(CHAR_VALUE) &&                                         \
	 !(POC_IS_CONTROL_ZM(CHAR_VALUE) || POC_IS_SEPERATOR_ZM(CHAR_VALUE)))
#define POC_IS_TEXT_ZM(CHAR_VALUE)                                             \
	(!POC_IS_CONTROL_ZM(CHAR_VALUE) || (CHAR_VALUE) == (char)SP_zm ||      \
	 (CHAR_VALUE) == HT_zm)
#endif // !DOXYGEN_SHOULD_SKIP_THIS

/**
 * Parse a HTTP 1.x Request-Message with Zero dynamic memory allocation
 *
 * @param message_buffer Raw HTTP requet bytes read from wire.
 * @param message_buffer_size Size of the input bytes
 * @param request_method Pointer to store the start of request-method
 * @param request_method_len Size of the pointer which holds request-method within the message buffer.
 * @param request_resource Pointer to store the start of request-resource
 * @param request_resource_len Size of the pointer which holds request-resouce within the message buffer.
 * @param major_version_num Pointer to store the HTTP message's HTTP major version number as int
 * @param minor_version_num Pointer to store the HTTP message's HTTP minor version number as int
 * @param headers Buffer(which have pointer members) to point to HTTP headers within the message-buffer along with their size.
 * @param num_header Total number of headers parsed in the raw HTTP request bytes
 * @param message_body Pointer to start of the HTTP message's body
 * @param message_body_size Size of the pointer which points to the message body within the HTTP request read buffer
 * @param failed Boolean flag, True if parser failed to parse the raw bytes or else False.
 */
static void http_parse_request_zm(
    char *message_buffer, size_t message_buffer_size, char **request_method,
    size_t *request_method_len, char **request_resource,
    size_t *request_resource_len, int *major_version_num,
    int *minor_version_num, poc_header_pair_zm *headers, size_t *num_header,
    char **message_body, size_t *message_body_size, bool *failed) {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define POC_INCREMENT_BUFFER_OFFSET_ZM(OFFSET_LENGTH)                          \
	do {                                                                   \
		message_buffer += OFFSET_LENGTH;                               \
		current_buffer_index += OFFSET_LENGTH;                         \
		if (current_buffer_index > message_buffer_size) {              \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
	} while (0)
#define POC_EXPECT_CHAR_ZM(CHAR_VALUE)                                         \
	do {                                                                   \
		if (*message_buffer != CHAR_VALUE) {                           \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
		POC_INCREMENT_BUFFER_OFFSET_ZM(1);                             \
	} while (0)
#define POC_EXPECT_CRLF_ZM(MESSAGE_BUFFER)                                     \
	do {                                                                   \
		if (*MESSAGE_BUFFER != (char)CR_zm &&                          \
		    *(MESSAGE_BUFFER + 1) != (char)LF_zm) {                    \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
		POC_INCREMENT_BUFFER_OFFSET_ZM(2);                             \
	} while (0)
#define POC_GET_INT_FROM_CHAR_ZM(CHAR_VALUE) (CHAR_VALUE - '0')
#endif // !DOXYGEN_SHOULD_SKIP_THIS

	// parsing HTTP request method
	size_t current_buffer_index = 0;
	*request_method = message_buffer;
	while (*message_buffer != (char)SP_zm &&
	       POC_IS_PRINTABLE_CHAR_ZM(*message_buffer)) {
		(*request_method_len) += 1;
		POC_INCREMENT_BUFFER_OFFSET_ZM(1);
	}
	POC_INCREMENT_BUFFER_OFFSET_ZM(1); // skip the SP

	*request_resource = message_buffer;
	// parsing HTTP request resource
	while (*message_buffer != (char)SP_zm &&
	       POC_IS_PRINTABLE_CHAR_ZM(*message_buffer)) {
		(*request_resource_len) += 1;
		POC_INCREMENT_BUFFER_OFFSET_ZM(1);
	}
	POC_INCREMENT_BUFFER_OFFSET_ZM(1);

	// parsing HTTP message version(SSE)
	__m128i version_pack_one =
	    _mm_setr_epi8('H', 'T', 'T', 'P', '/', '1', '.', '1', '\r', '\n',
			  '\0', '\0', '\0', '\0', '\0', '\0');
	__m128i version_pack_two = _mm_setr_epi8(
	    message_buffer[0], message_buffer[1], message_buffer[2],
	    message_buffer[3], message_buffer[4], message_buffer[5],
	    message_buffer[6], message_buffer[7], message_buffer[8],
	    message_buffer[9], '\0', '\0', '\0', '\0', '\0', '\0');
	__m128i cmp_result = _mm_cmpeq_epi8(version_pack_one, version_pack_two);
	unsigned int cmp_result_uint = _mm_movemask_epi8(cmp_result);
	if (cmp_result_uint == 65535) {
		*major_version_num = 1;
		*minor_version_num = 1;
		POC_INCREMENT_BUFFER_OFFSET_ZM(10);
	} else {
		*failed = true;
		return;
	}

	// let's parse headers
	// internal state machine for parsing headers
	typedef enum {
		HEADER_NAME,
		HEADER_NAME_ACCEPT,
		HEADER_VALUE,
		HEADER_VALUE_ACCEPT,
		HEADER_LF
	} header_parser_state;
	header_parser_state current_state = HEADER_NAME;
	size_t current_header_index_processing = 0;
	while (1) {
		switch (current_state) {
		case HEADER_NAME:
			if (*message_buffer == (char)CR_zm) {
				current_state = HEADER_LF;
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
			} else if (POC_IS_TOKEN_ZM(*message_buffer)) {
				headers[current_header_index_processing]
				    .header_name = message_buffer;
				current_state = HEADER_NAME_ACCEPT;
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_NAME_ACCEPT:
			if (*message_buffer == (char)CR_zm) {
				current_state = HEADER_LF;
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
			} else if (POC_IS_TOKEN_ZM(*message_buffer)) {
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
				headers[current_header_index_processing]
				    .header_name_len++;
			} else if (*message_buffer == ':') {
				current_state = HEADER_VALUE;
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_VALUE:
			if (*message_buffer == (char)CR_zm) {
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
				current_state = HEADER_LF;
			} else if (*message_buffer == (char)SP_zm &&
				   *(message_buffer - 1) == ':') {
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
			} else if (POC_IS_TEXT_ZM(*message_buffer)) {
				headers[current_header_index_processing]
				    .header_value = message_buffer;
				current_state = HEADER_VALUE_ACCEPT;
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_VALUE_ACCEPT:
			if (*message_buffer == (char)CR_zm) {
				current_state = HEADER_LF;
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
			} else if (POC_IS_TEXT_ZM(*message_buffer)) {
				headers[current_header_index_processing]
				    .header_value_len++;
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_LF:
			if (*message_buffer == (char)LF_zm &&
			    *(message_buffer + 1) == (char)CR_zm &&
			    *(message_buffer + 2) == (char)LF_zm) {
				POC_INCREMENT_BUFFER_OFFSET_ZM(3);
				*num_header =
				    (current_header_index_processing + 1);
				goto PROCESS_HTTP_MESSAGE_BODY;
			} else if (*message_buffer == (char)LF_zm &&
				   POC_IS_TOKEN_ZM(*(message_buffer + 1))) {
				POC_INCREMENT_BUFFER_OFFSET_ZM(1);
				current_state = HEADER_NAME;
				++current_header_index_processing;
			} else {
				goto ERROR_FAIL;
			}
			break;
		}
	}

ERROR_FAIL:
	*failed = true;
	return;

PROCESS_HTTP_MESSAGE_BODY:;
	ssize_t total_buffer_remaining =
	    (message_buffer_size - current_buffer_index);
	if (total_buffer_remaining > 0) {
		*message_body = message_buffer;
		*message_body_size = total_buffer_remaining;
	}
	*failed = false;
}

#endif // !HTTP_ZERO_ALLOC_PARSER_H
