/*
	SPARK - Dead or Alive Paradise Archive Extractor

Copyright (C) 2010              Giuseppe "The Lemon Man"
Copyright (C) 2010              Alex Marshall "trap15" <trap15@raidenii.net>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <sys/stat.h>

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;

extern char *app;

char *follow(char *path);
u32 decompress(u8 *inBuf, u32 len, u8 **outBuf);
void usage();

#ifdef __WIN32__
#  define compat_mkdir(path)	mkdir(path);
#else
#  define compat_mkdir(path)	mkdir(path, S_IRWXU);
#endif

#endif
