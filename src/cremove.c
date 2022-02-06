#include <stdio.h>
#include "GOOMBAServer.h"
#include "parse.h"

void cremove(void) {
    Stack *s;
	s = Pop();
	if(!s) {
		Error("Stack error in Fdelete");
		return;
	}
  remove(s->strval);
}
