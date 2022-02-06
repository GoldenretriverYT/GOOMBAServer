/***[superhash.h]***************************************************[TAB=4]**\
*                                                                            *
* GOOMBAServer                                                               *
*                                                                            *
* Copyright 2021,2022 GoombaProgrammer & Computa.me                          *
*                                                                            *
*  This program is free software; you can redistribute it and/or modify      *
*  it under the terms of the GNU General Public License as published by      *
*  the Free Software Foundation; either version 2 of the License, or         *
*  (at your option) any later version.                                       *
*                                                                            *
*  This program is distributed in the hope that it will be useful,           *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*  GNU General Public License for more details.                              *
*                                                                            *
*  You should have received a copy of the GNU General Public License         *
*  along with this program; if not, write to the Free Software               *
*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                 *
*                                                                            *
\****************************************************************************/
/* Superhash is the custom hash for GOOMBAServer. */
#ifndef _shash_h
#define _shash_h

#include "global.h"

/* Superhash context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[4];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[128];                         /* input buffer */
} SH_CTX;

void SUPERHInit PROTO_LIST ((SH_CTX *));
void SUPERHUpdate PROTO_LIST
  ((MD5_CTX *, unsigned char *, unsigned int));
void SUPERHFinal PROTO_LIST ((unsigned char [16], SH_CTX *));

#endif
