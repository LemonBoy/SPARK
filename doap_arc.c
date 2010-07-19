/*
 *
 * Dead or Alive Paradise Archive Extractor
 * The Lemon Man (C) 2010
 *
 */

#include <stdio.h>
#include <zlib.h>

typedef unsigned char u8;
typedef unsigned int u32;

typedef struct {
	u32 magic;
	u32 zero1;
	u32 fileCount;
	u32 tableStart;
	u32 offTblStart;
	u32 offTblLen;
	u32 zero2;
	u32 zero3;
} doap_arc_hdr;

typedef struct {
	u32 nameOffset;
	u32 fileLen;
	u32 unk2;
	u32 unk3;
} doap_arc_file;

inline u32 bswap(u32 val)
{
	asm volatile ("bswap %0" : "=r" (val) : "0" (val));
	return val;
}

/* Should follow the path, mkdir the folders and return the filename.
 * todo
 */
 
char *follow (char *path)
{
	char *folderChain = strdup(path);
	char *lSlash = strrchr(folderChain, '/');
	
	if (!lSlash)
	{
		free(folderChain);
		return path;
	}
	
	*lSlash = 0;
	
	do {
		lSlash = strrchr(folderChain, '/');
		if (!lSlash)
			break;
		*lSlash = '\\';
	} while (1);
			
	mkdir(folderChain);
	free(folderChain);
	return path;
}

u32 decompress (u8 *gzBuf, u32 gzLen, u8 **gzOut)
{
	z_stream zStr;
	
    zStr.zalloc = Z_NULL;
    zStr.zfree = Z_NULL;
    zStr.opaque = Z_NULL;
    zStr.avail_in = gzLen;
    zStr.next_in = gzBuf;
	
	*gzOut = malloc(gzLen * 2);
	
	zStr.avail_out = gzLen * 2;
	zStr.next_out = (u8 *)*gzOut;
	
    inflateInit2(&zStr, 16+MAX_WBITS);
	
	u32 res;
	
	if ((res = inflate(&zStr, Z_NO_FLUSH)) != Z_OK)
		printf("Inflate error %i\n", res);

	inflateEnd(&zStr);
	
	return gzLen * 2;
}

int main ()
{
	doap_arc_hdr hdr; 
	doap_arc_file *fEnt;
	u32 *fOff;
	char **fName;
	FILE * doapBin = fopen("doap.bin", "rb");
	FILE * doapArc = fopen("doap.arc", "rb");
	FILE * xtractd = NULL;
	u8 *tmp = NULL;
	
	fread(&hdr, 1, sizeof(doap_arc_hdr), doapBin);
	
	/* Pass 1 - Header */
	
	printf("Magic : %08x\n", hdr.magic);
	printf("Zero  : %08x\n", hdr.zero1);
	printf("Fcnt  : %08x\n", hdr.fileCount);
	printf("Tstrt : %08x\n", hdr.tableStart);
	printf("OTS   : %08x\n", hdr.offTblStart);
	printf("OTL   : %08x\n", hdr.offTblLen);
	printf("Zero  : %08x\n", hdr.zero2);
	printf("Zero  : %08x\n", hdr.zero3);
	
	fseek(doapBin, hdr.tableStart, SEEK_SET);
	
	fEnt = malloc(sizeof(doap_arc_file) * hdr.fileCount);
	fOff = malloc(sizeof(u32) * hdr.fileCount);
	fName = malloc(sizeof(char *) * hdr.fileCount);
	
	int fDone;
	
	/* Pass 2 - File table */
	
	for (fDone = 0; fDone < hdr.fileCount; fDone++)
	{
		printf("[%02i/%02i] Table struct\n", fDone, hdr.fileCount);
		fread(&fEnt[fDone], 1, sizeof(doap_arc_file), doapBin);
		
		printf("Noff  : %08x\n", fEnt[fDone].nameOffset);
		printf("Flen  : %08x\n", fEnt[fDone].fileLen);
		printf("Unk2  : %08x\n", fEnt[fDone].unk2); // if unk1 and unk2 are both 0xFFFFFFFF the file is uncompressed
		printf("Unk3  : %08x\n", fEnt[fDone].unk3);
	}
	
	/* Pass 3 - File offsets */

	for (fDone = 0; fDone < hdr.fileCount; fDone++)
	{
		printf("[%04i/%04i]\n", fDone, hdr.fileCount);
		fread(&fOff[fDone], 1, sizeof(u32), doapBin);
		printf("doap.arc @ %08x\n", fOff[fDone]);
	}
	
	int fNameLen = 0;
	
	/* Pass 4 - File names */
	
	for (fDone = 0; fDone < hdr.fileCount; fDone++)
	{
		printf("[%04i/%04i]\n", fDone, hdr.fileCount);
		fNameLen = 0;
		fseek(doapBin, fEnt[fDone].nameOffset, SEEK_SET);
		fName[fDone] = malloc(0x10);
		fread(fName[fDone] + fNameLen, 1, 0x10, doapBin);
		fNameLen += 0x10;
		while(fName[fDone][fNameLen] != 0x00)
		{
			fName[fDone] = (char *)realloc(fName[fDone], (fNameLen+1) * 0x10);
			fread(fName[fDone] + fNameLen, 1, 0x10, doapBin);
			fNameLen += 0x10;
		} 
		printf("%s\n", fName[fDone]);
	}
	
	/* Pass 5 - File extraction */
	
	for (fDone = 0; fDone < hdr.fileCount; fDone++)
	{
		printf("[%04i/%04i]\n", fDone, hdr.fileCount);
		printf("Extracting...\n");
		xtractd = fopen(follow(fName[fDone]), "w+b");
		fseek(doapArc, fOff[fDone], SEEK_SET);
		tmp = malloc(fEnt[fDone].fileLen);
		fread(tmp, 1, fEnt[fDone].fileLen, doapArc);
		if (tmp[0] == 0x1F && tmp[1] == 0x8B && tmp[2] == 0x08)
		{
			u8 *dTmp = NULL;
			u32 dSz = decompress(tmp, fEnt[fDone].fileLen, &dTmp);
			fwrite(dTmp, 1, dSz, xtractd);
			free(dTmp);
		} else {
			fwrite(tmp, 1, fEnt[fDone].fileLen, xtractd);
		}
		free(tmp);
		fclose(xtractd);
	}
	
	/* Pass 6 - Cleaning up the kitchen */
	
	for (fDone = 0; fDone < hdr.fileCount; fDone++)
	{
		free(fEnt[fDone]);
		free(fOff[fDone]);
		free(fName[fDone]);
	}
	
	free(fName);
	
	fclose(doapBin);
	fclose(doapArc);
	
	return 1;
}
