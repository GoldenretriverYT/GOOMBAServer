/***[rand.c]******************************************************[TAB=4]****\
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

#include <stdlib.h>
#include <GOOMBAServer.h>
#include <parse.h>

#ifndef RAND_MAX 
#define RAND_MAX	32767
#endif

void Srand(void) {
	Stack *s;

	s = Pop();
	if(!s) {
		Error("Stack error in srand");
		return;
	}
#if HAVE_SRAND48
	srand48((unsigned int)s->intval);	
#else
	srand((unsigned int)s->intval);	
#endif
}

void Rand(void) {
	char temp[32];

#if HAVE_LRAND48
	sprintf(temp,"%ld",lrand48());
#else
	sprintf(temp,"%d",rand());
#endif
	Push(temp,LNUMBER);
}

void GetRandMax(void) {
	char temp[32];

#if HAVE_LRAND48
	strcpy(temp,"2147483648");
#else
	sprintf(temp,"%d",RAND_MAX);
#endif
	Push(temp,LNUMBER);
}
