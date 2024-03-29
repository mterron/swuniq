/*
 *  swuniq - sliding window uniq
 *
 *  MIT License
 *
 *  Copyright (c) 2018 Miguel Terron
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/* swuniq :
 * TODO: Description
 */

#include <ctype.h>
#include <err.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "uthash/src/utringbuffer.h"


#define XXH_STATIC_LINKING_ONLY   /* *_state_t */
#include "xxHash/xxhash.h"

/* makes the next part easier */
#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
#   define ARCH_X64 1
#   define ARCH_X86 "x86_64"
#elif defined(__i386__) || defined(_M_IX86) || defined(_M_IX86_FP)
#   define ARCH_X86 "i386"
#endif
/* Try to detect the architecture. */
#if defined(ARCH_X86)
#  if defined(X86DISPATCH)
#    include "xxHash/xxh_x86dispatch.h"
#    define ARCH ARCH_X86 " autoVec"
#  elif defined(__AVX512F__)
#    define ARCH ARCH_X86 " + AVX512"
#  elif defined(__AVX2__)
#    define ARCH ARCH_X86 " + AVX2"
#  elif defined(__AVX__)
#    define ARCH ARCH_X86 " + AVX"
#  elif defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) \
      || defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP == 2)
#     define ARCH ARCH_X86 " + SSE2"
#  else
#     define ARCH ARCH_X86
#  endif
#elif defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
#  define ARCH "aarch64 + NEON"
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__) || defined(_M_ARM)
/* ARM has a lot of different features that can change xxHash significantly. */
#  if defined(__thumb2__) || (defined(__thumb__) && (__thumb__ == 2 || __ARM_ARCH >= 7))
#    define ARCH_THUMB " Thumb-2"
#  elif defined(__thumb__)
#    define ARCH_THUMB " Thumb-1"
#  else
#    define ARCH_THUMB ""
#  endif
/* ARMv7 has unaligned by default */
#  if defined(__ARM_FEATURE_UNALIGNED) || __ARM_ARCH >= 7 || defined(_M_ARMV7VE)
#    define ARCH_UNALIGNED " + unaligned"
#  else
#    define ARCH_UNALIGNED ""
#  endif
#  if defined(__ARM_NEON) || defined(__ARM_NEON__)
#    define ARCH_NEON " + NEON"
#  else
#    define ARCH_NEON ""
#  endif
#  define ARCH "ARMv" __ARM_ARCH ARCH_THUMB ARCH_NEON ARCH_UNALIGNED
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__)
#  if defined(__GNUC__) && defined(__POWER9_VECTOR__)
#    define ARCH "ppc64 + POWER9 vector"
#  elif defined(__GNUC__) && defined(__POWER8_VECTOR__)
#    define ARCH "ppc64 + POWER8 vector"
#  else
#    define ARCH "ppc64"
#  endif
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
#  define ARCH "ppc"
#elif defined(__AVR)
#  define ARCH "AVR"
#elif defined(__mips64)
#  define ARCH "mips64"
#elif defined(__mips)
#  define ARCH "mips"
#elif defined(__s390x__)
#  define ARCH "s390x"
#elif defined(__s390__)
#  define ARCH "s390"
#else
#  define ARCH "unknown"
#endif

#define VERSION "1.0"

/********************************************************************************************************************/

/* returns 1 if the hash already exists on the ringbuffer */
// XXH64_hash_t is long unsigned int
bool lookup(XXH64_hash_t digest, const UT_ringbuffer *rbuffer) {
	bool out = false;

	if (utringbuffer_empty(rbuffer))
		return (out);
	else {
		// long int index = utringbuffer_eltidx(rbuffer, digest);
		// printf("Index is: %ld\n",index),
		for (unsigned int i = 0; i < utringbuffer_len(rbuffer); i++) {
			XXH64_hash_t *item = utringbuffer_eltptr(rbuffer, i);
			out = (digest == *item);
			if (out)
				break;
		}
		return (out);
	}
}

/**********************************************************
 *  Main
 **********************************************************/
int main(int argc, char *argv[]) {
	size_t window_size = 10; // Default window size
	int c;

	UT_ringbuffer *history;
	UT_icd ut_xxh64_hash_t_icd = {sizeof(XXH64_hash_t), NULL, NULL, NULL};
	char *line = NULL;
	size_t pagesize = (size_t) sysconf(_SC_PAGESIZE);
	XXH64_hash_t digest;

	while ((c = getopt(argc, argv, "hw:")) != -1) {
		switch (c) {
		case 'w':
			window_size = strtoumax(optarg, NULL, 10);
			break;
		case 'h':
		default:
			#define HELP_MESSAGE "swuniq %s (%s) by Miguel Terron\nFilter matching lines (within a configurable window) from INPUT\n(or stdin), writing to stdout.\n\nUsage: swuniq [-w N] INPUT\n\t-w N Size of the sliding window to use for deduplication\nNote: By default swuniq will use a window of 10 lines.\n", VERSION, ARCH
			fprintf(stderr, HELP_MESSAGE);
			exit(1);
		}
	}

	// Open file if filename is provided
	if (optind < argc) {
		if (freopen(argv[optind], "r", stdin) == NULL) {
			fprintf(stderr, "Can't open file %s", argv[optind]);
			exit(1);
		}
	}

	utringbuffer_new(history, window_size, &ut_xxh64_hash_t_icd);
	while (getline(&line, &pagesize, stdin) != -1) {
		digest = XXH3_64bits(line, strlen(line));
		// printf("Digest is: %lu\n", digest);
		if (!lookup(digest, history)) {
			utringbuffer_push_back(history, &digest);
			printf("%s", line);
			fflush(stdout);
		}
	}

	fclose(stdin);
	free(line);
	utringbuffer_free(history);
	exit(0);
}
