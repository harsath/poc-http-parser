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
#ifndef HTTP_SIMD_ZERO_ALLOC_PARSER_H
#define HTTP_SIMD_ZERO_ALLOC_PARSER_H
#include <emmintrin.h>
#include <immintrin.h>
#include <nmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// NOTE: 'simd' suffix denotes the 'SIMD' implementation, it's named this way to avoid collision in benchmarks/tests 

#define str5cmp_macro_simd(ptr, c0, c1, c2, c3, c4)                                 \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3 &&*(ptr + 4) == c4

static bool str5cmp_simd(const char *ptr, const char *cmp) {
	return str5cmp_macro_simd(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
			     *(cmp + 3), *(cmp + 4));
}

typedef struct {
	const char *header_name;
	size_t header_name_len;
	const char *header_value;
	size_t header_value_len;
} poc_header_pair_simd;

// clang-format off
// Very useful/fast helpers. AVX2 256-bit version if processor supports AVX2
// #if defined(__AVX__)
static char *fast_find_char_avx2(char *buffer_start, char *buffer_end, char find){
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
// #elif defined(__SSE4_2__)
static char *fast_find_char_sse4_2(char *buffer_start, char *buffer_end, char find){
	__m128i char_fill = _mm_set1_epi8(find);
	for(; buffer_start+16 < buffer_end; buffer_start += 16){
		__m128i buff_pack = _mm_lddqu_si128((__m128i*)buffer_start);
		__m128i result = _mm_cmpeq_epi8(buff_pack, char_fill);
		int mask = _mm_movemask_epi8(result);
		if(mask) return buffer_start + __builtin_ffs(mask) - 1;
	}
	for(; buffer_start < buffer_end; ++buffer_start)
		if(*buffer_start == find) return buffer_start;
	return buffer_start;
}
// #endif
// clang-format on

#define POC_INIT_HEADER_PAIR_TO_ZERO_SIMD(header_pair_ptr, pair_size)               \
	do {                                                                   \
		for (size_t i = 0; i < pair_size; i++) {                       \
			header_pair_ptr[i].header_name = NULL;                 \
			header_pair_ptr[i].header_name_len = 0;                \
			header_pair_ptr[i].header_value = NULL;                \
			header_pair_ptr[i].header_value_len = 0;               \
		}                                                              \
	} while (0)
#define POC_IS_SEPERATOR_SIMD(CHAR_VALUE)                                           \
	((CHAR_VALUE == '(') || (CHAR_VALUE == ')') || (CHAR_VALUE == '<') ||  \
	 (CHAR_VALUE == '>') || (CHAR_VALUE == '@') || (CHAR_VALUE == ',') ||  \
	 (CHAR_VALUE == ';') || (CHAR_VALUE == ':') || (CHAR_VALUE == '\\') || \
	 (CHAR_VALUE == '"') || (CHAR_VALUE == '/') || (CHAR_VALUE == '[') ||  \
	 (CHAR_VALUE == ']') || (CHAR_VALUE == '?') || (CHAR_VALUE == '=') ||  \
	 (CHAR_VALUE == '{') || (CHAR_VALUE == '}') ||                         \
	 (CHAR_VALUE == ' ') || (CHAR_VALUE == '\t'))
#define POC_IS_PRINTABLE_CHAR_SIMD(CHAR_VALUE)                                      \
	(((unsigned)CHAR_VALUE >= 0x20) && ((unsigned)CHAR_VALUE < 0x7F))
#define POC_IS_CHAR_SIMD(CHAR_VALUE) ((unsigned)CHAR_VALUE <= 127)
#define POC_IS_CONTROL_SIMD(CHAR_VALUE)                                             \
	((CHAR_VALUE >= 0 && CHAR_VALUE <= 31) || (CHAR_VALUE == 127))
#define POC_IS_TOKEN_SIMD(CHAR_VALUE)                                               \
	(POC_IS_CHAR_SIMD(CHAR_VALUE) &&                                            \
	 !(POC_IS_CONTROL_SIMD(CHAR_VALUE) || POC_IS_SEPERATOR_SIMD(CHAR_VALUE)))
#define POC_IS_TEXT_SIMD(CHAR_VALUE)                                                \
	(!POC_IS_CONTROL_SIMD(CHAR_VALUE) || (CHAR_VALUE) == ' ' ||            \
	 (CHAR_VALUE) == '\t')
#define POC_SIMD_FOR_HTTP_VERSION 1

static void http_parse_request_simd(
    char *message_buffer, size_t message_buffer_size, char **request_method,
    size_t *request_method_len, char **request_resource,
    size_t *request_resource_len, int *major_version_num,
    int *minor_version_num, poc_header_pair_simd *headers, size_t *num_header,
    char **message_body, size_t *message_body_size, bool *failed) {

#define POC_INCREMENT_BUFFER_OFFSET_SIMD(OFFSET_LENGTH)                             \
	do {                                                                   \
		message_buffer += OFFSET_LENGTH;                               \
		current_buffer_index += OFFSET_LENGTH;                         \
		if (current_buffer_index > message_buffer_size) {              \
			*failed = true;                                        \
			return;                                                \
		}                                                              \
	} while (0)

	// parsing HTTP request method
	size_t current_buffer_index = 0;
	*request_method = message_buffer;
	while (*message_buffer != ' ' &&
	       POC_IS_PRINTABLE_CHAR_SIMD(*message_buffer)) {
		(*request_method_len) += 1;
		POC_INCREMENT_BUFFER_OFFSET_SIMD(1);
	}
	POC_INCREMENT_BUFFER_OFFSET_SIMD(1); // skip the SP

	// clang-format off
	*request_resource = message_buffer;
	const char *original_ptr = message_buffer;
	message_buffer = fast_find_char_sse4_2(message_buffer, message_buffer + (message_buffer_size - 1), ' ');
	if (*message_buffer != ' ') {
		*failed = true;
		return;
	}
	*request_resource_len = (message_buffer - original_ptr);
	current_buffer_index += *request_resource_len;
	POC_INCREMENT_BUFFER_OFFSET_SIMD(1);
	// clang-format on
#if POC_SIMD_FOR_HTTP_VERSION
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
		POC_INCREMENT_BUFFER_OFFSET_SIMD(10);
	} else {
		*failed = true;
		return;
	}
#else
	if (!str5cmp_simd(message_buffer, "HTTP/")) {
		*failed = true;
		return;
	}
	POC_INCREMENT_BUFFER_OFFSET_SIMD(5);
	*major_version_num = *message_buffer - '0';
	POC_INCREMENT_BUFFER_OFFSET_SIMD(2);
	*minor_version_num = *message_buffer - '0';
	POC_INCREMENT_BUFFER_OFFSET_SIMD(3); // Skip EOL marker
#endif
	// clang-format off
	for (;; ++*num_header) {
		if (*message_buffer == '\r' && *(message_buffer + 1) == '\n') {
			POC_INCREMENT_BUFFER_OFFSET_SIMD(2);
			break;
		}
		char *colon_ptr = fast_find_char_sse4_2(message_buffer, message_buffer + (message_buffer_size-1), ':');
		// Do not allow space before colon in HTTP header. HTTP response smuggling vulnerability mitigation.
		// https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2006-2786
		if(*(colon_ptr-1) == ' '){
			*failed = true;
			return;
		}
		if(*colon_ptr != ':'){
			*failed = true;
			return;
		}
		headers[*num_header].header_name = message_buffer;
		headers[*num_header].header_name_len = (colon_ptr - message_buffer);
		current_buffer_index += (colon_ptr - message_buffer);
		message_buffer = colon_ptr;
		POC_INCREMENT_BUFFER_OFFSET_SIMD(1);
		if(*message_buffer == ' '){ POC_INCREMENT_BUFFER_OFFSET_SIMD(1); }
		if(!POC_IS_TEXT_SIMD(*message_buffer) && *message_buffer != ' '){
			*failed = true;
			return;
		}
		headers[*num_header].header_value = message_buffer;
		char *newline_ptr = fast_find_char_sse4_2(message_buffer, message_buffer + (message_buffer_size-1), '\n');
		if(*newline_ptr != '\n' && *(newline_ptr-1) != '\r'){
			*failed = true;
			return;
		}
		//                                      (Skipping EOL marker in size)
		headers[*num_header].header_value_len = (newline_ptr-1) - message_buffer;
		current_buffer_index += (newline_ptr - message_buffer);
		message_buffer = newline_ptr;
		POC_INCREMENT_BUFFER_OFFSET_SIMD(1);
	}
	// clang-format on

	ssize_t total_buffer_remaining =
	    (message_buffer_size - current_buffer_index);
	if (total_buffer_remaining > 0) {
		*message_body = message_buffer;
		*message_body_size = total_buffer_remaining;
	}
	*failed = false;
}

#endif // !HTTP_SIMD_ZERO_ALLOC_PARSER_H
