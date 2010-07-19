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

typedef uint8_t		u8;
typedef uint32_t	u32;

typedef struct {
	u32 magic;
	u32 version;
	u32 fCount;
	u32 unk1;
} doap_par_hdr;

#define roundNtoX(N,X) ((N + (X-1)) & ~(X-1))

int main (int argc, char **argv)
{
	FILE *doaPar;
	doap_par_hdr hdr;
	u32 *fOff;
	char **fName;
	
	printf("PAR extractor\n(C) 2010 The Lemon Man & trap15\n");
	
	if (argc != 2)
	{
		printf("Pass the par archive name as parameter kthx.\n");
		return 0;
	}
	
	doaPar = fopen(argv[1], "rb");
	
	if (!doaPar)
		return 0;
	
	fread(&hdr, 1, sizeof(doap_par_hdr), doaPar);
	
	if (hdr.magic != 0x00524150)
	{
		printf("Not a PAR archive\n");
		return 0;
	}
	
	printf("Magic : %08x\n", hdr.magic);
	printf("Ver   : %08x\n", hdr.version);
	printf("Files : %08x\n", hdr.fCount);
	printf("Unk1  : %08x\n", hdr.unk1);
	
	fOff = malloc(sizeof(u32) * (hdr.fCount+1));
	
	int fDone;
	
	for (fDone = 0; fDone < hdr.fCount; fDone++)
	{
		printf("[%02i/%02i] Offsets\n", fDone, hdr.fCount);
		fread(&fOff[fDone], 1, sizeof(u32), doaPar);
		printf("%08x\n", fOff[fDone]);
	}
	
	u32 skip = roundNtoX(hdr.fCount, 0x4) - hdr.fCount;
	
	fseek(doaPar, skip * sizeof(u32), SEEK_CUR);
	
	fName = malloc(sizeof(char *) * hdr.fCount);
	
	for (fDone = 0; fDone < hdr.fCount; fDone++)
	{
		printf("[%02i/%02i] Name table\n", fDone, hdr.fCount);
		fName[fDone] = malloc(0x20);
		fread(fName[fDone], 1, 0x20, doaPar);
		printf("%s\n", fName[fDone]);
	}
	
	fseek(doaPar, 0, SEEK_END);
	
	fOff[hdr.fCount] = (u32)ftell(doaPar);
	
	for (fDone = 0; fDone < hdr.fCount; fDone++)
	{
		printf("[%02i/%02i] Name table\n", fDone, hdr.fCount);
		printf("Extracting...%x %x\n",fOff[fDone+1],fOff[fDone]);
		u8 *tmp = malloc(fOff[fDone+1] - fOff[fDone]);
		FILE *xtractd = fopen(fName[fDone], "w+b");
		fseek(doaPar, fOff[fDone], SEEK_SET);
		fread(tmp, 1, fOff[fDone+1] - fOff[fDone], doaPar);
		fwrite(tmp, 1, fOff[fDone+1] - fOff[fDone], xtractd);
		fclose(xtractd);
		free(tmp);
	}	
	
	fclose(doaPar);
	
	return 1;
}