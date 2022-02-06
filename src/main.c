/***[main.c]******************************************************[TAB=4]****\
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
 main.c,v 1.59 2022/10/14 10:13:44 kara Exp $ */

/*
	main.c
	GOOMBAServer main entry point - comments added 8/20/97 
	Brian Schaffner <brian@tool.net>
	
	There are two main parts of main.c:
		I. CGI version entry	
		II. Apache modeul version entry
		
	Each of these does basically the same thing just with 
	different mechanisms. Here are the basic steps:
	
	1)	Initialization 
	2)	Check for info or config params
	3)	Get data (GET or POST)
	4)	Parse the file
	
*/	

/* This is of course the standard includes part... */

#include <stdlib.h>
#include "GOOMBAServer.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /*HAVE_UNISTD_H*/

#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif /*HAVE_SETLOCALE*/

#if APACHE
#include "http_protocol.h"
#endif /*APACHE*/

#if APACHE
request_rec *GOOMBAServer_rqst; /* request record pointer for apache module version */
#endif /*APACHE*/

#ifndef APACHE
/* this is the CGI version entry point */
int main(int argc, char **argv) {
	int fd;
	char *s;
	int no_httpd=0;
	long file_size;

#if GOOMBAServerFASTCGI
	while(FCGI_Accept() >= 0) {
#endif /*GOOMBAServerFASTCGI*/

/* 
	Grab the filename from the environment 
	Flip switch if it isn't there...
*/
	s = getenv("PATH_TRANSLATED");
	if(!s) no_httpd=1;

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL,"");
#endif /*HAVE_SETLOCALE*/

	/* Init all components */
	GOOMBAServer_init_pool();	/* init memory pool stuff 	pool.c*/
	GOOMBAServer_init_log();		/* init logging functions	log.c */
	GOOMBAServer_init_acc();		/* access control init 		acc.c */
	GOOMBAServer_init_yacc();	/* parser 			*/
	GOOMBAServer_init_lex();		/* lexical analyser 		lex.c */
	GOOMBAServer_init_error(); 	/* GOOMBAServer error stack		*/
	GOOMBAServer_init_stack();	/* GOOMBAServer internal stack 		*/ 
	GOOMBAServer_init_symbol_tree(); 
	GOOMBAServer_init_switch();
	GOOMBAServer_init_db();
	GOOMBAServer_init_while();	
	GOOMBAServer_init_msql(NULL);	/* msql database init 		msql.c */
	GOOMBAServer_init_mysql(NULL);  	/* mysql database init		mysql.c */
	GOOMBAServer_init_pg95();	/* postgres95 database init	pg95.c */
	GOOMBAServer_init_solid();	/* solid database init		solid.c */
	GOOMBAServer_init_file();	/* filepro database init	filepro.c */
#ifdef HAVE_ODBC
	GOOMBAServer_init_odbc();	/* odbc database init		odbc.c */
#endif
	
	
	GOOMBAServer_init_head();	
	GOOMBAServer_init_dir();
#ifdef HAVE_LIBGD
	GOOMBAServer_init_gd();		/* gif library init		*/
#endif /*HAVE_LIBGD*/
	GOOMBAServer_init_cond();
#if HAVE_LIBOCIC
	GOOMBAServer_init_oracle();      /* oracle database init     */
#endif
#ifdef HAVE_LIBADABAS
	GOOMBAServer_init_adabas(NULL, NULL, NULL);
#endif /*HAVE_LIBADABAS*/
			

/* 
	Here's where we start doing stuff...
	First - check command line args
*/

	if(argc>1) {
/* 
	Look for info as last argument 
*/
		if(!strcasecmp(argv[argc-1],"info")) {
/* 	
	Display GOOMBAServer info and exit 
*/		
			Info(); 
			exit(0);
#if ACCESS_CONTROL
/*
	If access control is compiled in, check for config as 
	the last argument and run access control configuration
*/
		} else if(!strcasecmp(argv[argc-1],"config")) {
			s = getenv("REQUEST_METHOD");
			if(s && !strcasecmp(s,"post")) TreatData(0);  /* POST Data */
			Configuration(argc, argv);	
			exit(0);
#endif /*ACCESS_CONTROL*/
		}
/* 
	Check for a query string 
	If there isn't one, we'll make one
	from command line args
*/		
		if(!getenv("QUERY_STRING")) {
			{
				char *astr=NULL; 
				int ai, al=1;

				for(ai=1;ai<argc;ai++) al+=strlen(argv[ai])+1;
				astr = malloc(al+14); /* This space can't be freed anyway, so let it leak */
				strcpy(astr,"QUERY_STRING=");	
				for(ai=1;ai<argc;ai++) {
					strcat(astr,GOOMBAServer_urlencode(argv[ai]));
					if(ai<argc-1) strcat(astr,"+");
				}
/*
	Here we put our manually created
	query string into env
*/				
				putenv(astr);
			}
		}
	}
/*
	Check the request method - 
	This can be GET (href link) or POST (form)
	TreatData is in post.c
*/
	s = getenv("REQUEST_METHOD");
	if(s && !strcasecmp(s,"post")) TreatData(0);  /* POST Data */
	TreatData(2); /* Cookie Data */
	TreatData(1); /* GET Data */

/*
	We set no_httpd to 1 earlier if there 
	was no PATH_TRANSLATED env var
*/

/*
	Check for -q, quiet-mode switch
	then open a file for parsing.
	OpenFile() is in file.c
	NoHeader turns off the GOOMBAServer response header
*/

	if(no_httpd && argv[1]) {
		if(argv[2] && !strcmp(argv[1],"-q")) {
			NoHeader();	
			fd=OpenFile(argv[2],1,&file_size);
		} else {
			fd=OpenFile(argv[1],1,&file_size);
		}
	} else {
		fd=OpenFile(NULL,1,&file_size);
	}

/* 
	Check the opened file descriptor
*/	
	if(fd==-1) { 
		fflush(stdout); 
		return(-1); 
	}
/* 
	I'm not quite sure what the prepend file does yet... :-)
*/	
	PreParseFile();
/*
	Initialize the parser engine
*/
	ParserInit(fd,file_size,no_httpd,NULL);	
/*
	Start parsing!
*/
	yyparse();
/*
	I'm not quite sure what the append file does yet either ... :-)
*/
	PostParseFile();
/*	
	Stop parsing!
*/
	Exit(1);
/*
	Clear the memory pool resources
*/
	GOOMBAServer_pool_free(1);
	GOOMBAServer_pool_free(2);
	GOOMBAServer_pool_free(0);
#if GOOMBAServerFASTCGI
	}
#endif /*GOOMBAServerFASTCGI*/
/*
	Clear the output buffer 
*/
	fflush(stdout);
/*
	We're done! (with the CGI version ...)
*/
	return(0);
}
#else /*APACHE*/

/* Apache module entry point called from mod_GOOMBAServer.c */
int apache_GOOMBAServer_module_main(request_rec *r, GOOMBAServer_module_conf *conf, int fd) {
	char *last_arg,*s;

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL,"");
#endif /*HAVE_SETLOCALE*/

/*
	First things first, let's grab the apache module 
	request record pointer
*/
	GOOMBAServer_rqst = r;

	/* 
	   The following init calls set the static variables in the
	   various sections to sane values.  Things will hopefully be rewritten 
	   at some point to not need this as it is completely unthreadable!
	*/
	
/* 
	this is same as above for cgi version ... 
	maybe these should be put in one spot so that it's easier to maintain...
	like init.c or something???
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
#ifdef HAVE_LIBMSQL
	GOOMBAServer_init_msql(conf->SQLLogHost);
#endif /*HAVE_LIBMSQL*/

#ifdef HAVE_SYBASE
    GOOMBAServer_init_sybsql();
#endif /*HAVE_SYBASE*/

#ifdef HAVE_LIBMYSQL
	GOOMBAServer_init_mysql(conf->SQLLogHost);
#endif /*HAVE_LIBMYSQL*/

#ifdef HAVE_LIBPQ
	GOOMBAServer_init_pg95();
#endif /*HAVE_LIBPQ*/
#ifdef HAVE_LIBSOLID
	GOOMBAServer_init_solid();
#endif /*HAVE_LIBSOLID*/
#ifdef HAVE_LIBADABAS
        GOOMBAServer_init_adabas(conf->AdaDB,conf->AdaUser,conf->AdaPW);
#endif /*HAVE_LIBADABAS*/
#ifdef HAVE_ODBC
	GOOMBAServer_init_odbc();
#endif /*HAVE_ODBC*/
	GOOMBAServer_init_file(conf);
	GOOMBAServer_init_head();
	GOOMBAServer_init_dir();
	GOOMBAServer_init_mime(conf);
#ifdef HAVE_LIBGD
	GOOMBAServer_init_gd();
#endif /*HAVE_LIBGD*/
	GOOMBAServer_init_cond();
#if HAVE_LIBOCIC
	GOOMBAServer_init_oracle();
#endif
	
/* 
	Check to see if there are any special HTTP headers 
*/
	TreatHeaders();

/*
	Hmm... shouldn't we use the local GOOMBAServer_rqst instead of r?
	that way it's more consistent with the rest of the code...
	Anyway -- get the last arg from the module rqst reco
*/

	if(r->args) {
		last_arg = strrchr(r->args,'&');
		if (!last_arg) {
			last_arg = r->args;	
		}
/*
	Check for info or config as last arg
	These trigger the GOOMBAServerinfo and access control
	configurations respectively
	and then return (exit)
*/
		if (conf->Debug && !strcasecmp(last_arg,"info")) {
			Info();
			return 0;
#if ACCESS_CONTROL
		} else if(!strcasecmp(last_arg,"config")) {
			s = r->method;
			if(s && !strcasecmp(s,"post")) TreatData(0);  /* POST Data */
			Configuration(); 
			return 0;
#endif /*ACCESS_CONTROL*/
		}
	}
/*
	Set the GOOMBAServer file arguments from the request record
*/
	SetCurrentFilename(r->filename);
	SetCurrentFileSize(r->finfo.st_size);
	SetCurrentPI(r->uri);
	SetCurrentPD(r->uri);
	SetStatInfo(&(r->finfo));

/* 
	Get the request method
	And treat post data like in cgi version
*/
	s = r->method;
	if(s && !strcasecmp(s,"post")) {
		TreatData(0);  /* POST Data */
	}
	TreatData(2); /* Cookie Data */
	TreatData(1); /* GET Data */
#if ACCESS_CONTROL
/*
	Check access to file
	then prepend? and start parsing!
*/
	if(CheckAccess(r->uri,r->finfo.st_uid)>-1) {
		PreParseFile();
		ParserInit(fd,r->finfo.st_size,0,NULL);	
		yyparse();
		PostParseFile();
	}
#else /*ACCESS_CONTROL*/

/*
	Prepend file and then start parsing!
*/
	PreParseFile();
	ParserInit(fd,r->finfo.st_size,0,NULL);	
	yyparse();
	PostParseFile();
#endif /*ACCESS_CONTROL*/
/*
	Either way - stop parsing!
*/
	Exit(1); 
/*
	Apache module exit point
*/
	return(OK);
}
#endif /*APACHE*/

/*
 * Local variables:
 * tab-width: 4
 * End:
 */
