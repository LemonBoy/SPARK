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

#include "common.h"
#include "arc_par.h"

typedef struct {
	u32 magic;
	u32 version;
	u32 fCount;
	u32 unk1;
} __attribute__((packed)) arc_par_hdr;

#define round(n, x)	((n + (x - 1)) & ~(x - 1))

static FILE *doaPar;
static arc_par_hdr hdr;
static u32 *fOff;
static char **fName;

static int read_header()
{
	fread(&hdr, 1, sizeof(arc_par_hdr), doaPar);
	
	printf("Magic : %08x\n", hdr.magic);
	printf("Ver   : %08x\n", hdr.version);
	printf("Files : %08x\n", hdr.fCount);
	printf("Unk1  : %08x\n", hdr.unk1);
	return 0;
}

static int read_offsets()
{
	int i;
	fOff = malloc(sizeof(u32) * (hdr.fCount+1));
	if(fOff == NULL)
		return 1;
	
	for(i = 0; i < hdr.fCount; i++) {
		printf("[%02i/%02i] Offsets\n", i, hdr.fCount);
		fread(&fOff[i], 1, sizeof(u32), doaPar);
		printf("%08x\n", fOff[i]);
	}
	return 0;
}

static int read_name_table()
{
	int i;
	u32 skip = round(hdr.fCount, 4) - hdr.fCount;
	fseek(doaPar, skip * sizeof(u32), SEEK_CUR);
	
	fName = malloc(sizeof(char *) * hdr.fCount);
	if(fName == NULL)
		return 1;
	
	for(i = 0; i < hdr.fCount; i++) {
		printf("[%02i/%02i]\n", i, hdr.fCount);
		fName[i] = malloc(0x20);
		fread(fName[i], 1, 0x20, doaPar);
		printf("%s\n", fName[i]);
	}
	return 0;
}

static int extract_files()
{
	int i;
	u8 *tmp;
	FILE *xtractd;
	fseek(doaPar, 0, SEEK_END);
	
	fOff[hdr.fCount] = (u32)ftell(doaPar);
	
	for(i = 0; i < hdr.fCount; i++) {
		printf("[%02i/%02i]\n", i, hdr.fCount);
		printf("Extracting... %x %x\n",fOff[i+1],fOff[i]);
		tmp = malloc(fOff[i+1] - fOff[i]);
		if(tmp == NULL)
			return 1;
		xtractd = fopen(fName[i], "wb+");
		if(xtractd == NULL)
			return 1;
		fseek(doaPar, fOff[i], SEEK_SET);
		fread(tmp, 1, fOff[i+1] - fOff[i], doaPar);
		fwrite(tmp, 1, fOff[i+1] - fOff[i], xtractd);
		fclose(xtractd);
		free(tmp);
	}
	
	return 0;
}

int par_unarc(FILE* fp)
{
	doaPar = fp;
	
	if(read_header()) {
		perror("Can't read header");
		return EXIT_FAILURE;
	}
	
	if(read_offsets()) {
		perror("Can't read offsets");
		return EXIT_FAILURE;
	}
	
	if(read_name_table()) {
		perror("Can't read name table");
		return EXIT_FAILURE;
	}
	
	if(extract_files()) {
		perror("Can't extract files");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
