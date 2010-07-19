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

typedef uint8_t		u8;
typedef uint32_t	u32;

typedef struct {
	u32	magic;
	u32	zero1;
	u32	fileCount;
	u32	tableStart;
	u32	offTblStart;
	u32	offTblLen;
	u32	zero2;
	u32	zero3;
} __attribute__((packed)) doap_arc_hdr;

typedef struct {
	u32	nameOffset;
	u32	fileLen;
	u32	unk2;
	u32	unk3;
} __attribute__((packed)) doap_arc_file;

doap_arc_hdr hdr;
doap_arc_file *fEnt;
u32 *fOff;
char **fName;
FILE* doapBin;
FILE* doapArc;


#ifdef __WIN32__
#  define compat_mkdir(path)	mkdir(path);
#else
#  define compat_mkdir(path)	mkdir(path, S_IRWXU);
#endif

#ifdef __WIN32__
void switch_slashes(char *tmp)
{
	for(; *tmp != 0; tmp++) {
		if(*tmp == '/')
			*tmp = '\\';
	}
}
#else
void switch_slashes(char *tmp)
{
	for(; *tmp != 0; tmp++) {
		if(*tmp == '\\')
			*tmp = '/';
	}
}
#endif

char *follow(char *path)
{
	char *tmp;
	char *tmp2;
	char *p;
	char *olddir;
	char oldp;
	size_t len;
	switch_slashes(path);
	tmp = strdup(path);
	tmp2 = tmp;
	len = strlen(tmp);
	olddir = getcwd(NULL, 0);
	printf("Expanding %s\n", path);
	if((tmp[len - 1] == '/') || (tmp[len - 1] == '\\'))
		tmp[len - 1] = 0;
	for(p = tmp + 1; *p; p++) {
		if((*p == '/') || (*p == '\\')) {
			oldp = *p;
			*p = 0;
			printf("Making %s\n", tmp);
			compat_mkdir(tmp);
			chdir(tmp);
			*p = oldp;
			tmp = p + 1;
		}
	}
	chdir(olddir);
	free(tmp2);
	return path;
}

u32 decompress(u8 *inBuf, u32 len, u8 **outBuf)
{
	u32 res;
	
	z_stream zStr;
	
	zStr.zalloc = Z_NULL;
	zStr.zfree = Z_NULL;
	zStr.opaque = Z_NULL;
	zStr.avail_in = len;
	zStr.next_in = inBuf;
	
	*outBuf = malloc(len * 2);
	
	zStr.avail_out = len * 2;
	zStr.next_out = *outBuf;
	
	inflateInit2(&zStr, 16 + MAX_WBITS);
	
	if((res = inflate(&zStr, Z_NO_FLUSH)) != Z_OK)
		printf("Inflate error %i\n", res);
	
	inflateEnd(&zStr);
	
	return len * 2;
}

int parse_header()
{
	fread(&hdr, 1, sizeof(doap_arc_hdr), doapBin);
	
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
	
	fEnt  = calloc(sizeof(doap_arc_file), hdr.fileCount);
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

int parse_file_table()
{
	int i;
	for(i = 0; i < hdr.fileCount; i++) {
		printf("[%02i/%02i] Table struct\n", i, hdr.fileCount);
		fread(&(fEnt[i]), 1, sizeof(doap_arc_file), doapBin);
		
		printf("Noff  : %08x\n", fEnt[i].nameOffset);
		printf("Flen  : %08x\n", fEnt[i].fileLen);
		/* if unk1 and unk2 are both 0xFFFFFFFF the file is uncompressed */
		printf("Unk2  : %08x\n", fEnt[i].unk2);
		printf("Unk3  : %08x\n", fEnt[i].unk3);
	}
	return 0;
}

int parse_file_offsets()
{
	int i;
	for(i = 0; i < hdr.fileCount; i++) {
		printf("[%04i/%04i]\n", i, hdr.fileCount);
		fread(&(fOff[i]), 1, sizeof(u32), doapBin);
		printf("doap.arc @ %08x\n", fOff[i]);
	}
	return 0;
}

int parse_file_names()
{
	int i;
	int fNameLen = 0;
	for(i = 0; i < hdr.fileCount; i++) {
		printf("[%04i/%04i]\n", i, hdr.fileCount);
		fNameLen = 0;
		fseek(doapBin, fEnt[i].nameOffset, SEEK_SET);
		fName[i] = malloc(0x10);
		if(fName[i] == NULL)
			return 1;
		fread(fName[i] + fNameLen, 1, 0x10, doapBin);
		fNameLen += 0x10;
		while(fName[i][fNameLen] != 0x00) {
			fName[i] = (char *)realloc(fName[i], (fNameLen + 1) * 0x10);
			if(fName[i] == NULL)
				return 1;
			fread(fName[i] + fNameLen, 1, 0x10, doapBin);
			fNameLen += 0x10;
		}
		printf("%s\n", fName[i]);
	}
	return 0;
}

int file_extract()
{
	FILE* xtractd;
	u8 *tmp;
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
		if((tmp[0] == 0x1F) && (tmp[1] == 0x8B) && (tmp[2] == 0x08)) {
			u8 *dTmp = NULL;
			u32 dSz = decompress(tmp, fEnt[i].fileLen, &dTmp);
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

int main(int argc, char *argv[])
{
	int fDone;
	
	doapBin = fopen("doap.bin", "rb");
	if(doapBin == NULL) {
		perror("Can't open doap.bin");
		return EXIT_FAILURE;
	}
	
	doapArc = fopen("doap.arc", "rb");
	if(doapArc == NULL) {
		perror("Can't open doap.arc");
		return EXIT_FAILURE;
	}
	
	/* Pass 1 - Header */
	if(parse_header()) {
		printf("Can't parse header.\n");
		return EXIT_FAILURE;
	}
	
	/* Pass 2 - File table */
	if(parse_file_table()) {
		printf("Can't parse file table.\n");
		return EXIT_FAILURE;
	}
	
	/* Pass 3 - File offsets */
	if(parse_file_offsets()) {
		printf("Can't parse file offsets.\n");
		return EXIT_FAILURE;
	}
	
	/* Pass 4 - File names */
	if(parse_file_names()) {
		printf("Can't parse file names.\n");
		return EXIT_FAILURE;
	}
	
	/* Pass 5 - File extraction */
	if(file_extract()) {
		printf("Can't extract files.\n");
		return EXIT_FAILURE;
	}
	
	/* Pass 6 - Cleaning up the kitchen */
	for(fDone = 0; fDone < hdr.fileCount; fDone++) {
		free(fName[fDone]);
	}
	
	free(fOff);
	free(fEnt);
	free(fName);
	
	fclose(doapBin);
	fclose(doapArc);
	
	return EXIT_SUCCESS;
}
