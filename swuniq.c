/*
*  swuniq - sliding window uniq
*  Copyright (C) Miguel Terron 2018
*
*  GPL v2 License
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

int notSeen(const unsigned long long hash, const UT_ringbuffer* rbuffer)
{
	int out = 0;

	if (utringbuffer_len(rbuffer) == 0) return(out);
	else
	{
		unsigned long long *item;
		for (int i=0; i < utringbuffer_len(rbuffer); i++) {
			item = utringbuffer_eltptr(rbuffer, i);
			out = (out || (*item == hash));
		}
		return(out);
	}	
}

/* ********************************************************
*  Main
**********************************************************/
int main (int argc, char *argv[]){
	int wsize = 100; // Default window size
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
				fprintf (stderr, "Usage: swuniq [-w N]\n\nFilters matching lines from stdin (within a configurable window)\nwriting to stdout.\n\nBy default swuniq will filter out lines that match any of the\nprevious 100 lines.\n\n\t-w N Size of the sliding window to use for deduplication\n", optopt);
				exit(1);
		}
	}

	char *buffer;
	size_t bufsize = 6000;

	UT_ringbuffer *history;
	UT_icd ut_long_long_icd = {sizeof(long long), NULL, NULL, NULL };
	utringbuffer_new(history, wsize, &ut_long_long_icd);
	unsigned long long fingerprint;

	buffer = (char *)malloc(bufsize * sizeof(char));
	if( buffer == NULL )
	{
		perror("Unable to allocate buffer");
		exit(1);
	}

	while( -1 != getline(&buffer, &bufsize, stdin) )
	{
		fingerprint = hashString(buffer, strlen(buffer));
		if (notSeen(fingerprint,history) == 0)
		{
			utringbuffer_push_back(history, &fingerprint);
			printf("%s",buffer);
			fflush(stdout);
		}
	}
	
	utringbuffer_free(history);
	fclose(stdin);
	exit(0);
}
