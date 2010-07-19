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

typedef struct {
	u32	magic;
	u32	zero1;
	u32	fileCount;
	u32	tableStart;
	u32	offTblStart;
	u32	offTblLen;
	u32	zero2;
	u32	zero3;
} __attribute__((packed)) arc_paa_hdr;

typedef struct {
	u32	nameOffset;
	u32	fileLen;
	u32	unk2;
	u32	unk3;
} __attribute__((packed)) arc_paa_file;

static arc_paa_hdr hdr;
static arc_paa_file *fEnt;
static u32 *fOff;
static char **fName;
static FILE* doapBin;
static FILE* doapArc;

static int parse_header()
{
	fread(&hdr, 1, sizeof(arc_paa_hdr), doapBin);
	
	if (hdr.magic != 0x00414150)
		return 0;
	
	printf("Magic : %08x\n", hdr.magic);
	printf("Zero  : %08x\n", hdr.zero1);
	printf("Fcnt  : %08x\n", hdr.fileCount);
	printf("Tstrt : %08x\n", hdr.tableStart);
	printf("OTS   : %08x\n", hdr.offTblStart);
	printf("OTL   : %08x\n", hdr.offTblLen);
	printf("Zero  : %08x\n", hdr.zero2);
	printf("Zero  : %08x\n", hdr.zero3);
	
	fseek(doapBin, hdr.tableStart, SEEK_SET);
	
	fEnt  = calloc(sizeof(arc_paa_file), hdr.fileCount);
	if(fEnt == NULL)
		return 1;
	fOff  = calloc(sizeof(u32), hdr.fileCount);
	if(fOff == NULL)
		return 1;
	fName = calloc(sizeof(char*), hdr.fileCount);
	if(fName == NULL)
		return 1;
	return 0;
}

static int parse_file_table()
{
	int i;
	for(i = 0; i < hdr.fileCount; i++) {
		printf("[%02i/%02i] Table struct\n", i, hdr.fileCount);
		fread(&(fEnt[i]), 1, sizeof(arc_paa_file), doapBin);
		
		printf("Noff  : %08x\n", fEnt[i].nameOffset);
		printf("Flen  : %08x\n", fEnt[i].fileLen);
		/* if unk1 and unk2 are both 0xFFFFFFFF the file is uncompressed */
		printf("Unk2  : %08x\n", fEnt[i].unk2);
		printf("Unk3  : %08x\n", fEnt[i].unk3);
	}
	return 0;
}

static int parse_file_offsets()
{
	int i;
	for(i = 0; i < hdr.fileCount; i++) {
		printf("[%04i/%04i]\n", i, hdr.fileCount);
		fread(&(fOff[i]), 1, sizeof(u32), doapBin);
		printf("doap.arc @ %08x\n", fOff[i]);
	}
	return 0;
}

static int parse_file_names()
{
	int i;
	int fNameLen = 0;
	for(i = 0; i < hdr.fileCount; i++) {
		printf("[%04i/%04i]\n", i, hdr.fileCount);
		fNameLen = 0;
		fseek(doapBin, fEnt[i].nameOffset, SEEK_SET);
		fName[i] = malloc(1);
		if(fName[i] == NULL)
			return 1;
		fread(fName[i] + fNameLen, 1, 1, doapBin);
		while(fName[i][fNameLen] != 0x00) {
			fNameLen++;
			fName[i] = (char *)realloc(fName[i], fNameLen + 1);
			if(fName[i] == NULL)
				return 1;
			fread(fName[i] + fNameLen, 1, 1, doapBin);
		}
		printf("%s\n", fName[i]);
	}
	return 0;
}

static int file_extract()
{
	FILE* xtractd;
	u8 *tmp;
	u8 *dTmp = NULL;
	int i;
	for(i = 0; i < hdr.fileCount; i++) {
		printf("[%04i/%04i]\n", i, hdr.fileCount);
		printf("Extracting...\n");
		xtractd = fopen(follow(fName[i]), "wb+");
		if(xtractd == NULL)
			return 1;
		fseek(doapArc, fOff[i], SEEK_SET);
		tmp = malloc(fEnt[i].fileLen);
		if(tmp == NULL)
			return 1;
		fread(tmp, 1, fEnt[i].fileLen, doapArc);
		if(((tmp[0] == 0x1F) && (tmp[1] == 0x8B) && (tmp[2] == 0x08)) && \
		   !((fEnt[i].unk2 == 0xFFFFFFFF) && (fEnt[i].unk3 == 0xFFFFFFFF))) {
			u32 dSz = decompress(tmp, fEnt[i].fileLen, &dTmp);
			if(dTmp == NULL)
				return 1;
			fwrite(dTmp, 1, dSz, xtractd);
			free(dTmp);
		}else{
			fwrite(tmp, 1, fEnt[i].fileLen, xtractd);
		}
		free(tmp);
		fclose(xtractd);
	}
	return 0;
}

int paa_unarc(FILE* fp)
{
	int i;
	
	doapBin = fp;
	
	doapArc = fopen("doap.arc", "rb");
	if(doapArc == NULL) {
		perror("Can't open doap.arc");
		return EXIT_FAILURE;
	}
	
	/* Pass 1 - Header */
	if(parse_header()) {
		perror("Can't parse header");
		return EXIT_FAILURE;
	}
	
	/* Pass 2 - File table */
	if(parse_file_table()) {
		perror("Can't parse file table");
		return EXIT_FAILURE;
	}
	
	/* Pass 3 - File offsets */
	if(parse_file_offsets()) {
		perror("Can't parse file offsets");
		return EXIT_FAILURE;
	}
	
	/* Pass 4 - File names */
	if(parse_file_names()) {
		perror("Can't parse file names");
		return EXIT_FAILURE;
	}
	
	/* Pass 5 - File extraction */
	if(file_extract()) {
		perror("Can't extract files");
		return EXIT_FAILURE;
	}
	
	/* Pass 6 - Cleaning up the kitchen */
	for(i = 0; i < hdr.fileCount; i++) {
		free(fName[i]);
	}
	
	free(fOff);
	free(fEnt);
	free(fName);
	
	fclose(doapArc);
	
	return EXIT_SUCCESS;
}
