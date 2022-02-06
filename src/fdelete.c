#include <stdio.h>
#include "GOOMBAServer.h"
#include "parse.h"

int fdelete() {
    Stack *s;
	s = Pop();
	if(!s) {
		Error("Stack error in Fdelete");
		return;
	}
  remove(s->strval);
}
