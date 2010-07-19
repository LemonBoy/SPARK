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

char *app;

#ifdef __WIN32__
static void switch_slashes(char *tmp)
{
	for(; *tmp != 0; tmp++) {
		if(*tmp == '/')
			*tmp = '\\';
	}
}
#else
static void switch_slashes(char *tmp)
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

void usage()
{
	fprintf(stderr,
		"Usage:\n"
		"	%s <file>\n\n", app);
}

