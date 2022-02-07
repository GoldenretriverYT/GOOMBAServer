/***[goombcrypt.c]*************************************************[TAB=4]****\
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
#if APACHE
#include "http_protocol.h"
#endif
#include <stdarg.h>

char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

void goombencrypt(void){
Stack *s;
	char *t;
	int i;
	char *r;
	
	s = Pop();
	if(!s) {
		Error("Stack error in GoombCRYPT");
		return;
	}
	t = s->strval;
      /* Yes, I know this code is bad, but I am still learning C, so stop (: */
      t = str_replace(t, "a", "%%#"); 
      t = str_replace(t, "b", "w%%w#w"); 
      t = str_replace(t, "c", "%q%#q"); 
      t = str_replace(t, "h", "%2%2#"); 
      t = str_replace(t, "2", "%1%1#"); 
      t = str_replace(t, "!", "!!!*"); 
      t = str_replace(t, "g", "0&*"); 
      t = str_replace(t, "0", "0&"); 
      t = str_replace(t, "p", "2@0&"); 
      t = str_replace(t, "ee", "2@0~~"); 
      t = str_replace(t, "e", "209~"); 
      t = str_replace(t, "xx", "420069"); 
      t = str_replace(t, "x", "46"); 

	Push(t,STRING);
}
