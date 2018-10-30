/*
*  swuniq - sliding window uniq
*  
*  MIT License
*  
*  Copyright (c) 2018 Miguel Terron
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*
*/

/* swuniq :
 * TODO: Description
 */


/* ************************************
 *  Includes
 **************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>

#include <unistd.h>
#include <inttypes.h>
#include <ctype.h>

#define XXH_PRIVATE_API
#include "xxhash.h"

#include "utringbuffer.h"

/********************************************************************************************************************/

unsigned long long hashString(const void* buffer, size_t length)
{
	unsigned long long const seed = 1029384756;
	unsigned long long const hash = XXH64(buffer, length, seed);
	return hash;
}

/* returns 1 if the hash already exists on the ringbuffer */
int lookup(const unsigned long long hash, const UT_ringbuffer* rbuffer)
{
	int out = 0;

	if (utringbuffer_len(rbuffer) == 0) return(out);
	else
	{
		//unsigned long long *item;
		for (int i=0; i < utringbuffer_len(rbuffer); i++) {
			unsigned long long *item = utringbuffer_eltptr(rbuffer, i);
			out = (hash == *item);
			if (out) break;
		}
		return(out);
	}
}

/**********************************************************
*  Main
**********************************************************/
int main (int argc, char *argv[]){
	int wsize = 10; // Default window size
	int c;

	while ((c = getopt (argc, argv, "hw:")) != -1)
	{
		switch (c)
		{
			case 'w':
				wsize = strtoumax(optarg, NULL, 10);
				break;
			case 'h':
			default:
				fprintf(stderr,"Usage: swuniq [-w N]\nFilter matching lines (within a configurable window) from INPUT\n(or standard input), writing to standard output.\n\n\t-w N Size of the sliding window to use for deduplication\nNote: By default swuniq will use a window of 10 lines.\n\n");
				exit(1);
		}
	}

	// Open file if filename is provided
	if(optind < argc) {
		if ( freopen(argv[optind], "r", stdin) == NULL)
		{
			fprintf(stderr,"Can't open file %s",argv[optind]);
			exit(1);
		}
	}

	char *buffer;
	size_t bufsize = 6000;

	UT_ringbuffer *history;
	UT_icd ut_long_long_icd = {sizeof(long long), NULL, NULL, NULL };
	utringbuffer_new(history, wsize, &ut_long_long_icd);
	unsigned long long digest;

	buffer = (char *)malloc(bufsize * sizeof(char));
	if( buffer == NULL )
	{
		perror("Unable to allocate buffer");
		exit(1);
	}

	while( -1 != getline(&buffer, &bufsize, stdin) )
	{
		digest = hashString(buffer, strlen(buffer));
		if (!lookup(digest,history))
		{
			utringbuffer_push_back(history, &digest);
			printf("%s",buffer);
			fflush(stdout);
		}
	}

	utringbuffer_free(history);
	fclose(stdin);
	exit(0);
}
