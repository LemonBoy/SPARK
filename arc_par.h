/*
	SPARK - Dead or Alive Paradise Archive Extractor

Copyright (C) 2010              Giuseppe "The Lemon Man"
Copyright (C) 2010              Alex Marshall "trap15" <trap15@raidenii.net>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef ARC_PAR_H
#define ARC_PAR_H

#include <stdio.h>
#include "common.h"

#define MAGIC_PAR	(0x50415200)

int par_unarc(FILE* fp);

#endif
