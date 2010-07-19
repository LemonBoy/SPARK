/*
	SPARK - Dead or Alive Paradise Archive Extractor

Copyright (C) 2010              Giuseppe "The Lemon Man"
Copyright (C) 2010              Alex Marshall "trap15" <trap15@raidenii.net>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

#include "common.h"
#include "arc_paa.h"
#include "arc_par.h"

#define VERSION_MAJOR		0
#define VERSION_MINOR		2

int main(int argc, char *argv[])
{
	FILE* fp;
	u32 magic;
	int ret = EXIT_FAILURE;
	app = argv[0];
	fprintf(stderr, 
		"SPARK v%d.%d - Dead or Alive Paradise Archive Extractor\n"
		"By The Lemon Man and trap15\n",
		VERSION_MAJOR, VERSION_MINOR);
	
	if(argc < 2) {
		usage();
		return EXIT_FAILURE;
	}
	
	fp = fopen(argv[1], "rb");
	if(fp == NULL) {
		fprintf(stderr, "Can't open %s", argv[1]);
		return EXIT_FAILURE;
	}
	fread(&magic, 1, sizeof(u32), fp);
	fseek(fp, 0, SEEK_SET);
	magic = ntohl(magic);
	switch(magic) {
		case MAGIC_PAA: /* PAA\0 */
			ret = paa_unarc(fp);
			break;
		case MAGIC_PAR: /* PAR\0 */
			ret = par_unarc(fp);
			break;
		default:
			fprintf(stderr, "Unknown magic %08X\n", magic);
			break;
	}
	
	fclose(fp);
	fprintf(stderr, "\n");
	return ret;
}
