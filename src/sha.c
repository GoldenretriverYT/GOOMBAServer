/***[sha.c]******************************************************[TAB=4]*****\
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
#include "sha.h"
#include "GOOMBAServer.h"
#include "parse.h"

int sha()
{  
  Stack *s;
	char *md5str;
	MD5_CTX context;
	unsigned int len;
	unsigned char digest[16];
	int i;
	char *r;
  s = Pop();
	if(!s) {
		Error("Stack error in Md5");
		return;
	}
  const unsigned char str[] = s->strval;
  unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
  SHA1(str, sizeof(str) - 1, hash);
  Push(hash,STRING);
}
