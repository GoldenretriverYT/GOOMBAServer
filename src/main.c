/***[main.c]******************************************************[TAB=4]****\
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
/* $Id: main.c,v 1.26 2022/05/23 14:18:29 rasmus Exp $ */
#include <stdlib.h>
#include <GOOMBAServer.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SETLOCALE
#include <locale.h>
#endif
#if APACHE
#include "http_protocol.h"
#endif
#if GOOMBAServerFASTCGI
#include "fcgi_stdio.h"
#endif

#if APACHE
request_rec *GOOMBAServer_rqst;
#endif

#ifndef APACHE
int main(int argc, char **argv) {
	int fd;
	char *s;
	int no_httpd=0;
	long file_size;

#if GOOMBAServerFASTCGI
	while(FCGI_Accept() >= 0) {
#endif
	s = getenv("PATH_TRANSLATED");
	if(!s) no_httpd=1;

#if HAVE_SETLOCALE
	setlocale(LC_ALL,"");
#endif

	/* Init all components */
	GOOMBAServer_init_pool();
	GOOMBAServer_init_log();
	GOOMBAServer_init_acc();
	GOOMBAServer_init_yacc();
	GOOMBAServer_init_lex();
	GOOMBAServer_init_error();
	GOOMBAServer_init_stack();
	GOOMBAServer_init_symbol_tree();
	GOOMBAServer_init_switch();
	GOOMBAServer_init_db();
	GOOMBAServer_init_while();
	GOOMBAServer_init_msql();
	GOOMBAServer_init_pg95();
	GOOMBAServer_init_file();
	GOOMBAServer_init_head();
	GOOMBAServer_init_dir();
#if HAVE_LIBGD
	GOOMBAServer_init_gd();
#endif
	GOOMBAServer_init_cond();

	if(argc>1) {
		if(!strcasecmp(argv[argc-1],"info")) {
			Info();
			exit(0);
#if ACCESS_CONTROL
		} else if(!strcasecmp(argv[argc-1],"config")) {
			s = getenv("REQUEST_METHOD");
			if(s && !strcasecmp(s,"post")) TreatData(0);  /* POST Data */
			Configuration(argc, argv);	
			exit(0);
#endif
#if TEXT_MAGIC
		} else if(!strcasecmp(argv[argc-1],"text_magic")) {
			set_text_magic(1);
#endif
		}
	}
	s = getenv("REQUEST_METHOD");
	if(s && !strcasecmp(s,"post")) TreatData(0);  /* POST Data */
	TreatData(1); /* GET Data */

	if(no_httpd && argv[1]) fd=OpenFile(argv[1],1,&file_size);
	else fd=OpenFile(NULL,1,&file_size);
	if(fd==-1) return(-1);
	ParserInit(fd,file_size,no_httpd,NULL);	
	yyparse();
	Exit(1);
	GOOMBAServer_pool_free(1);
	GOOMBAServer_pool_free(2);
	GOOMBAServer_pool_free(0);
#if GOOMBAServerFASTCGI
	}
#endif
	return(0);
}
#else

/* Apache module entry point called from mod_GOOMBAServer.c */
int apache_GOOMBAServer_module_main(request_rec *r, GOOMBAServer_module_conf *conf, int fd) {
	char *last_arg,*s;

#if HAVE_SETLOCALE
	setlocale(LC_ALL,"");
#endif
	GOOMBAServer_rqst = r;

	/* 
	   The following init calls set the static variables in the
	   various sections to sane values.  Things will hopefully be rewritten 
	   at some point to not need this as it is completely unthreadable!
	*/
	GOOMBAServer_init_pool(conf);
	GOOMBAServer_init_log(conf);
	GOOMBAServer_init_acc(conf);
	GOOMBAServer_init_yacc();
	GOOMBAServer_init_lex();
	GOOMBAServer_init_error();
	GOOMBAServer_init_stack();
	GOOMBAServer_init_symbol_tree();
	GOOMBAServer_init_switch();
	GOOMBAServer_init_db();
	GOOMBAServer_init_while();
#if HAVE_LIBMSQL
	GOOMBAServer_init_msql();
#endif
#if HAVE_LIBPQ
	GOOMBAServer_init_pg95();
#endif
	GOOMBAServer_init_file();
	GOOMBAServer_init_head();
	GOOMBAServer_init_dir();
	GOOMBAServer_init_mime(conf);
#if HAVE_LIBGD
	GOOMBAServer_init_gd();
#endif
	GOOMBAServer_init_cond();

	if(r->args) {
		last_arg = strrchr(r->args,'&');
		if(!last_arg) last_arg = r->args;	
		if(!strcasecmp(last_arg,"info")) {
			Info();
			return 0;
#if ACCESS_CONTROL
		} else if(!strcasecmp(last_arg,"config")) {
			s = r->method;
			if(s && !strcasecmp(s,"post")) TreatData(0);  /* POST Data */
			Configuration(); 
			return 0;
#endif
		}
	}
	SetCurrentFilename(r->filename);
	SetCurrentFileSize(r->finfo.st_size);
	SetCurrentPI(r->uri);
	SetStatInfo(&(r->finfo));

	s = r->method;
	if(s && !strcasecmp(s,"post")) TreatData(0);  /* POST Data */
	TreatData(1); /* GET Data */
#if ACCESS_CONTROL
	ParserInit(fd,r->finfo.st_size,0,NULL);	
	if(CheckAccess(r->uri,r->finfo.st_uid)>-1) {
		yyparse();
	}
#else
	ParserInit(fd,r->finfo.st_size,0,NULL);	
	yyparse();
#endif
	Exit(1); 
	return(OK);
}
#endif
