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
#include <pmmintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09 } LexConst;

typedef struct {
	const char *header_name;
	size_t header_name_len;
	const char *header_value;
	size_t header_value_len;
} poc_header_pair;

// clang-format off
// Use AVX2 256-bit version if processor supports AVX2
#if defined(__AVX2__)
static const char *fast_find_char(const char *buffer_start, const char *buffer_end, char find){
	__m256i char_fill = _mm256_set1_epi8(find);
	for(; buffer_start+32 < buffer_end; buffer_start += 32){
		__m256i buff_pack = _mm256_lddqu_si256((__m256i*)buffer_start);
		__m256i result = _mm256_cmpeq_epi8(buff_pack, char_fill);
		int mask = _mm256_movemask_epi8(result);
		if(mask) return buffer_start + __builtin_ffs(mask) - 1;
	}
	for(; buffer_start < buffer_end; ++buffer_start)
		if(*buffer_start == find) return buffer_start;
	return buffer_start;
}
// or else, use SSE4.2 128-bit version as fallback
#elif defined(__SSE4_2__)
static const char *fast_find_char(const char *buffer_start, const char *buffer_end, char find){
	__m128i char_fill = _mm_set1_epi8(find);
	for(; buffer_start+32 < buffer_end; buffer_start += 32){
		__m128i buff_pack = _mm_lddqu_si128((__m128i*)buffer_start);
		__m128i result = _mm_cmpeq_epi8(buff_pack, char_fill);
		int mask = _mm_movemask_epi8(result);
		if(mask) return buffer_start + __builtin_ffs(mask) - 1;
	}
	for(; buffer_start < buffer_end; ++buffer_start)
		if(*buffer_start == find) return buffer_start;
	return buffer_start;
}
#endif
// clang-format on

#define POC_INIT_HEADER_PAIR_TO_ZERO(header_pair_ptr, pair_size)               \
	do {                                                                   \
		for (size_t i = 0; i < pair_size; i++) {                       \
			header_pair_ptr[i].header_name = NULL;                 \
			header_pair_ptr[i].header_name_len = 0;                \
			header_pair_ptr[i].header_value = NULL;                \
			header_pair_ptr[i].header_value_len = 0;               \
		}                                                              \
	} while (0)
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
#define POC_CHAR_VECTOR_SIZE 16
#define POC_SIMD_FOR_HTTP_VERSION 1

static void http_parse_request(
    char *message_buffer, size_t message_buffer_size, char **request_method,
    size_t *request_method_len, char **request_resource,
    size_t *request_resource_len, int *major_version_num,
    int *minor_version_num, poc_header_pair *headers, size_t *num_header,
    char **message_body, size_t *message_body_size, bool *failed) {

#define POC_INCREMENT_BUFFER_OFFSET(OFFSET_LENGTH)                             \
	do {                                                                   \
		message_buffer += OFFSET_LENGTH;                               \
		current_buffer_index += OFFSET_LENGTH;                         \
		if (current_buffer_index > message_buffer_size) {              \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
	} while (0)
#define POC_EXPECT_CHAR(CHAR_VALUE)                                            \
	do {                                                                   \
		if (*message_buffer != CHAR_VALUE) {                           \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
		POC_INCREMENT_BUFFER_OFFSET(1);                                \
	} while (0)
#define POC_EXPECT_CRLF(MESSAGE_BUFFER)                                        \
	do {                                                                   \
		if (*MESSAGE_BUFFER != (char)CR &&                             \
		    *(MESSAGE_BUFFER + 1) != (char)LF) {                       \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
		POC_INCREMENT_BUFFER_OFFSET(2);                                \
	} while (0)
#define POC_GET_INT_FROM_CHAR(CHAR_VALUE) (CHAR_VALUE - '0')
#define POC_CHECK_EOF_VECTOR()                                                 \
	do {                                                                   \
		if ((current_buffer_index + POC_CHAR_VECTOR_SIZE) >=           \
		    message_buffer_size) {                                     \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
	} while (0)

	// parsing HTTP request method
	size_t current_buffer_index = 0;
	*request_method = message_buffer;
	while (*message_buffer != (char)SP &&
	       POC_IS_PRINTABLE_CHAR(*message_buffer)) {
		(*request_method_len) += 1;
		POC_INCREMENT_BUFFER_OFFSET(1);
	}
	POC_INCREMENT_BUFFER_OFFSET(1); // skip the SP

	// use SIMD here
	*request_resource = message_buffer;
	__m128i space_in_vector = _mm_set1_epi8(' ');
	// clang-format off
	for (;;) {
		POC_CHECK_EOF_VECTOR();
		__m128i message_buffer_vector = _mm_loadu_si128((__m128i*)message_buffer);
		__m128i matches = _mm_cmpeq_epi8(message_buffer_vector, space_in_vector);
		uint16_t eq_mask = _mm_movemask_epi8(matches);
		if (eq_mask != 0) {
			int clz = __builtin_ctz(eq_mask);
			(*request_resource_len) += clz;
			POC_INCREMENT_BUFFER_OFFSET(clz);
			goto PARSE_HTTP_VERSION;
		}
		(*request_resource_len) += POC_CHAR_VECTOR_SIZE;
		POC_INCREMENT_BUFFER_OFFSET(16);
	}
	// clang-format on
PARSE_HTTP_VERSION:
	POC_INCREMENT_BUFFER_OFFSET(1);
#ifdef POC_SIMD_FOR_HTTP_VERSION
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
		POC_INCREMENT_BUFFER_OFFSET(10);
	} else {
		*failed = true;
		return;
	}
#endif
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
			if (*message_buffer == (char)CR) {
				current_state = HEADER_LF;
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else if (POC_IS_TOKEN(*message_buffer)) {
				headers[current_header_index_processing]
				    .header_name = message_buffer;
				current_state = HEADER_NAME_ACCEPT;
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_NAME_ACCEPT:
			if (*message_buffer == (char)CR) {
				current_state = HEADER_LF;
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else if (POC_IS_TOKEN(*message_buffer)) {
				POC_INCREMENT_BUFFER_OFFSET(1);
				headers[current_header_index_processing]
				    .header_name_len++;
			} else if (*message_buffer == ':') {
				current_state = HEADER_VALUE;
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_VALUE:
			if (*message_buffer == (char)CR) {
				POC_INCREMENT_BUFFER_OFFSET(1);
				current_state = HEADER_LF;
			} else if (*message_buffer == (char)SP &&
				   *(message_buffer - 1) == ':') {
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else if (POC_IS_TEXT(*message_buffer)) {
				headers[current_header_index_processing]
				    .header_value = message_buffer;
				current_state = HEADER_VALUE_ACCEPT;
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_VALUE_ACCEPT:
			if (*message_buffer == (char)CR) {
				current_state = HEADER_LF;
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else if (POC_IS_TEXT(*message_buffer)) {
				headers[current_header_index_processing]
				    .header_value_len++;
				POC_INCREMENT_BUFFER_OFFSET(1);
			} else {
				goto ERROR_FAIL;
			}
			break;
		case HEADER_LF:
			if (*message_buffer == (char)LF &&
			    *(message_buffer + 1) == (char)CR &&
			    *(message_buffer + 2) == (char)LF) {
				POC_INCREMENT_BUFFER_OFFSET(3);
				*num_header =
				    (current_header_index_processing + 1);
				goto PROCESS_HTTP_MESSAGE_BODY;
			} else if (*message_buffer == (char)LF &&
				   POC_IS_TOKEN(*(message_buffer + 1))) {
				POC_INCREMENT_BUFFER_OFFSET(1);
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
