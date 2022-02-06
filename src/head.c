/***[head.c]******************************************************[TAB=4]****\
*                                                                            *
* GOOMBAServer                                                               *
*                                                                            *
* Copyright 2022 GoombaProgrammer                                            *
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
 head.c,v 1.40 2022/10/23 09:39:08 ssb Exp $ */
#include "GOOMBAServer.h"
#include "parse.h"
#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#if APACHE
#include "http_protocol.h"
#endif

static int HeaderPrinted=0;
static int PrintHeader=1;
static CookieList *top = NULL;
static char *cont_type = NULL;

void GOOMBAServer_init_head(void) {
	HeaderPrinted=0;
	PrintHeader=1;
	top = NULL;
	cont_type = NULL;
}

void NoHeader(void) {
	PrintHeader=0;
	SetHeaderCalled();
}

void Header(void) {
	Stack *s;
	char *r;
#if APACHE
	char *rr=NULL;
#endif
#if GOOMBAServer_SAFE_MODE
	char *temp=NULL;
	char temp2[32];
	long myuid=0L;
#endif

	s = Pop();
	if(!s) {
		Error("Stack error in header\n");
		return;
	}
#if APACHE
	if(HeaderPrinted==1) {
#if DEBUG
		Debug("Woops, call to Header - already gone\n");
#endif
		return; /* too late, already sent */
	}
	ParseEscapes(s->strval);
	/*
	 * Not entirely sure this is the right way to support the header
	 * command in the Apache module.  Comments?
	 */
	r = strchr(s->strval,':');
	if(r) {
		*r='\0';
		if(!strcasecmp(s->strval,"content-type")) {
			if(*(r+1)==' ') 
				GOOMBAServer_rqst->content_type = estrdup(0,r+2);
			else
				GOOMBAServer_rqst->content_type = estrdup(0,r+1);
		} else {
			if(*(r+1)==' ') 
				rr=r+2;
			else
				rr=r+1;
#if GOOMBAServer_SAFE_MODE
			if(!strcasecmp(s->strval,"WWW-authenticate")) {
				myuid=getmyuid();
				sprintf(temp2,"realm=\"%ld ",myuid);
				temp=_ERegReplace("realm=\"",temp2,rr,1);
				if(!strcmp(temp,rr)) {
					sprintf(temp2,"realm=%ld",myuid);
					temp=_ERegReplace("realm=",temp2,rr,1);
					if(!strcmp(temp,rr)) {
						sprintf(temp2," realm=%ld",myuid);
						temp=_ERegReplace("$",temp2,rr,1);
					}
				}
#if DEBUG
				Debug("header to [%s]\n",temp);
#endif
				table_set(GOOMBAServer_rqst->headers_out,s->strval,temp);
			} else 
#endif
				table_set(GOOMBAServer_rqst->headers_out,s->strval,rr);
		}
		if(!strcasecmp(s->strval,"location")) {
			GOOMBAServer_rqst->status = REDIRECT;
#if DEBUG
			Debug("Redirecting to: %s\n",s->strval);
#endif
		}
		*r=':';
		HeaderPrinted = 2;
	}		
	if(!strncasecmp(s->strval,"http/",5)) {
		if(strlen(s->strval) > 9) {
			GOOMBAServer_rqst->status = atoi(&((s->strval)[9]));
		}
		GOOMBAServer_rqst->status_line = estrdup(0,&((s->strval)[9]));
	}	
#else
	r = strchr(s->strval,':');
	if(r) {
		*r='\0';
		if(!strcasecmp(s->strval,"content-type")) {
			cont_type = estrdup(0,r+1);;
		} else {
			*r=':';
			ParseEscapes(s->strval);
			PUTS(s->strval);
			PUTS("\015\012");
		}
	} else {
		ParseEscapes(s->strval);
		PUTS(s->strval);
		PUTS("\015\012");
	}
#endif
}

/*
 * GOOMBAServer_header() flushes the header info built up using calls to
 * the Header() function.  If type is 1, a redirect to str is done.
 * Otherwise type should be 0 and str NULL.
 */
void GOOMBAServer_header(int type,char *str) {
	CookieList *cookie;
	char *tempstr;
	time_t t;
	int len=0;

#if APACHE
    if((PrintHeader && !HeaderPrinted) || (PrintHeader && HeaderPrinted==2)) { 
		if(type==1) {
			table_set(GOOMBAServer_rqst->headers_out,"Location",str);
			GOOMBAServer_rqst->status = REDIRECT;
		}
#if APACHE_NEWAPI
		cookie = PopCookieList();
		while(cookie) {
			if(cookie->name) len+=strlen(cookie->name);	
			if(cookie->value) len+=strlen(cookie->value);	
			if(cookie->path) len+=strlen(cookie->path);	
			if(cookie->domain) len+=strlen(cookie->domain);	
			if(cookie->domain) len+=strlen(cookie->domain);	
			tempstr = emalloc(0,len+100);
			if(!cookie->value || (cookie->value && !*cookie->value)) {
				/* 
				 * MSIE doesn't delete a cookie when you set it to a null value
				 * so in order to force cookies to be deleted, even on MSIE, we
				 * pick an expiry date 1 year and 1 second in the past
				 */
				sprintf(tempstr,"%s=deleted",cookie->name);
				t = time(NULL)-31536001;
				strcat(tempstr,"; expires=");
				strcat(tempstr,std_date(t));	
			} else {
				sprintf(tempstr,"%s=%s",cookie->name,cookie->value?GOOMBAServer_urlencode(cookie->value):"");
				if(cookie->expires>0) {
					strcat(tempstr,"; expires=");
					strcat(tempstr,std_date(cookie->expires));	
				}
			}
			if(cookie->path && strlen(cookie->path)) {
				strcat(tempstr,"; path=");
				strcat(tempstr,cookie->path);
			}
			if(cookie->domain && strlen(cookie->domain)) {
				strcat(tempstr,"; domain=");
				strcat(tempstr,cookie->domain);
			}
			if(cookie->secure) {
				strcat(tempstr,"; secure");
			}	
#if APACHE_NEWAPI
			table_add(GOOMBAServer_rqst->headers_out,"Set-Cookie",tempstr);
#else
			/* Without the table_add function, you cannot set multiple cookies */
			table_set(GOOMBAServer_rqst->headers_out,"Set-Cookie",tempstr);
#endif
			cookie = PopCookieList();
		}
#endif
        HeaderPrinted = 1;
		SetHeaderCalled();
#if DEBUG
		Debug("Sending header\n");
#endif
        send_http_header(GOOMBAServer_rqst);
		if(GOOMBAServer_rqst->header_only) {
			Exit(0);
		}
    }
#else
    if(PrintHeader && !HeaderPrinted) { 
		cookie = PopCookieList();
		while(cookie) {
			if(cookie->name) len+=strlen(cookie->name);	
			if(cookie->value) len+=strlen(cookie->value);	
			if(cookie->path) len+=strlen(cookie->path);	
			if(cookie->domain) len+=strlen(cookie->domain);	
			if(cookie->domain) len+=strlen(cookie->domain);	
			tempstr = emalloc(0,len+100);
			if(!cookie->value || (cookie->value && !*cookie->value)) {
				/* 
				 * MSIE doesn't delete a cookie when you set it to a null value
				 * so in order to force cookies to be deleted, even on MSIE, we
				 * pick an expiry date 1 year and 1 second in the past
				 */
				sprintf(tempstr,"%s=deleted",cookie->name);
				strcat(tempstr,"; expires=");
				t = time(NULL)-31536001;
				strcat(tempstr,std_date(t));	
			} else {
				sprintf(tempstr,"%s=%s",cookie->name,cookie->value?GOOMBAServer_urlencode(cookie->value):"");
				if(cookie->expires>0) {
					strcat(tempstr,"; expires=");
					strcat(tempstr,std_date(cookie->expires));	
				}
			}
			if(cookie->path && strlen(cookie->path)) {
				strcat(tempstr,"; path=");
				strcat(tempstr,cookie->path);
			}
			if(cookie->domain && strlen(cookie->domain)) {
				strcat(tempstr,"; domain=");
				strcat(tempstr,cookie->domain);
			}
			if(cookie->secure) {
				strcat(tempstr,"; secure");
			}	
			fputs("Set-Cookie: ",stdout);
			fputs(tempstr,stdout);
			fputs("\015\012",stdout);
			cookie = PopCookieList();
		}
		if(type==1) {
			fputs("Location: ",stdout);
			fputs(str,stdout);
			fputs("\015\012\015\012",stdout);
		} else {
			if(!cont_type) {
	        	fputs("Content-type: text/html\015\012\015\012",stdout); 
			} else {
				fputs("Content-type:",stdout);
				fputs(cont_type,stdout);
				fputs("\015\012\015\012",stdout);
			}
		}
        HeaderPrinted = 1;
		SetHeaderCalled();
    }
#endif
}

void PushCookieList(char *name, char *value, time_t expires, char *path, char *domain, int secure) {
    CookieList *new;

    new = emalloc(0,sizeof(CookieList));
    new->next=top;
	new->name = name;
	new->value = value;
	new->expires = expires;
	new->path = path;
	new->domain = domain;
	new->secure = secure;	
    top = new;
}

CookieList *PopCookieList(void) {
	CookieList *ret;

	ret = top;
	if(top) top = top->next;
	return(ret);
}

/* SetCookie(name,value,expires,path,domain,secure) */
void SetCookie(int args) {
	Stack *s;
	char *name=NULL, *value=NULL, *path=NULL, *domain=NULL;
	time_t expires=0;
	int secure=0;

	if(HeaderPrinted==1) {
		Error("Oops, SetCookie called after header has been sent\n");
		return;
	}

	switch(args) {
		case 6:
			s = Pop();
			if(!s) {
				Error("Stack error in SetCookie");
				return;
			}
			secure = s->intval;
		case 5:
			s = Pop();
			if(!s) {
				Error("Stack error in SetCookie");
				return;
			}
			domain = estrdup(0,s->strval);
		case 4:	
			s = Pop();
			if(!s) {
				Error("Stack error in SetCookie");
				return;
			}
			path = estrdup(0,s->strval);
		case 3:	
			s = Pop();
			if(!s) {
				Error("Stack error in SetCookie");
				return;
			}
			expires = s->intval;
		case 2:	
			s = Pop();
			if(!s) {
				Error("Stack error in SetCookie");
				return;
			}
			value = estrdup(0,s->strval);
		case 1:
			s = Pop();
			if(!s) {
				Error("Stack error in SetCookie");
				return;
			}
			name = estrdup(0,s->strval);
	}
	PushCookieList(name,value,expires,path,domain,secure);
}


void
GetAllHeaders()
{
#if APACHE
	array_header *env_arr;
	table_entry *tenv;
	int i;
	VarTree *var;

	var = GetVar("__headertmp__", NULL, 0);
	if (var) {
		deletearray(var);
	}
	
	env_arr = table_elts(GOOMBAServer_rqst->headers_in);
	tenv = (table_entry *)env_arr->elts;
	for (i = 0; i < env_arr->nelts; ++i) {
		if (!tenv[i].key
#if GOOMBAServer_SAFE_MODE
			|| !strncasecmp(tenv[i].key, "authorization", 13)
#endif
			) {
			continue;
		}
		Push(AddSlashes(tenv[i].key, 0), STRING);
		Push(AddSlashes(tenv[i].val, 0), STRING);
		SetVar("__headertmp__", 2, 0);
	}
	Push("__headertmp__", VAR);
#else
	Error("GetAllHeaders() only available in Apache module.");
	Push("-1", LNUMBER);
#endif
}


/*
 * Local variables:
 * tab-width: 4
 * End:
 */
