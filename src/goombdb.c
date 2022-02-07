#include <GOOMBAServer.h>
#include <parse.h>
#if APACHE
#include "http_protocol.h"
#endif
#include <stdio.h>
#include <assert.h>

char *char_replace(char *orig, char *rep, char *with) {
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

bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

void SelectGDB(void){
Stack *s;
	char *t;
	int i;
	char *r;
	char *k;
	k = "Null";
	s = Pop();
	if(!s) {
		Error("Stack error in goombdb");
		return;
	}
	t = s->strval;
     
    FILE *file = fopen("goombata.cgi", "r");
    char currentline[512];

    assert(file != NULL);

    while (fgets(currentline, sizeof(currentline), file) != NULL) {
        if(prefix(strcat(t, "::"), currentline)){
			k = char_replace(currentline, strcat(t, "::"), "");
		}
    }

    fclose(file);

	Push(k,STRING);
}

int split (const char *txt, char delim, char ***tokens)
{
    int *tklen, *t, count = 1;
    char **arr, *p = (char *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = calloc (count, sizeof (int));
    for (p = (char *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = malloc (count * sizeof (char *));
    t = tklen;
    p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
            txt++;
        }
        else *p++ = *txt++;
    }
    free (tklen);
    return count;
}

void InsertGDB(void){
Stack *s;
	char *t;
	int i;
	char *r;
	char *k;
	k = "Null";
	s = Pop();
	if(!s) {
		Error("Stack error in goombdb");
		return;
	}
	t = s->strval;
     
    FILE *file = fopen("goombata.cgi", "a");
    fprintf(file, s);

    fclose(file);

	Push(k,STRING);
}

void PutGDB(void){
Stack *s;
	const char *t;
	int i;
	char *r;
	char *k;
	k = "Null";
	s = Pop();
	if(!s) {
		Error("Stack error in goombdb");
		return;
	}
	t = s->strval;
    FILE *filer = fopen("goombata.cgi", "r");
	char line[512]; 
          while ( fgets( line, 512, stdin ) != null ) 
            { 
              strcat(t, line); 
            } 
    fclose(filer);
	char **tokens;
	int count, i;
    FILE *file = fopen("goombata.cgi", "w");
	split(t, '^', &tokens);
	for (i = 0; i < count; i++) strcat(t, tokens[i]);
	for (i = 0; i < count; i++) free (tokens[i]);
	free (tokens);
    fprintf(file, t);

    fclose(file);

	Push(k,STRING);
}
