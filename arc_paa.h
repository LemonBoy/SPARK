/*
	SPARK - Dead or Alive Paradise Archive Extractor

Copyright (C) 2010              Giuseppe "The Lemon Man"
Copyright (C) 2010              Alex Marshall "trap15" <trap15@raidenii.net>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef ARC_PAA_H
#define ARC_PAA_H

#include <stdio.h>
#include "common.h"

#define MAGIC_PAA	(0x50414100)

int paa_unarc(FILE* fp);

#endif
