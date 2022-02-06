/***[mod_GOOMBAServer.c]***************************************************[TAB=4]****\
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
#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_main.h"
#include "http_protocol.h"
#include "util_script.h"
#include "mod_GOOMBAServer.h"

#if WINNT|WIN32
module MODULE_VAR_EXPORT GOOMBAServer_module;
#else
module GOOMBAServer_module;
#endif
int saved_umask;
   
#ifdef GOOMBAServer_XBITHACK
#define DEFAULT_GOOMBAServer_XBITHACK 1
#else
#define DEFAULT_GOOMBAServer_XBITHACK 0
#endif

extern int apache_GOOMBAServer_module_main(request_rec *r, GOOMBAServer_module_conf *, int);

void save_umask() {
    saved_umask = umask(077);
	umask(saved_umask);
}

void restore_umask() {
	umask(saved_umask);
}

int send_parsed_GOOMBAServer(request_rec *r) {
	int fd, retval;
	GOOMBAServer_module_conf *conf;
    
	/* Make sure file exists */
	if (r->finfo.st_mode == 0) return NOT_FOUND;

	/* grab configuration settings */
	conf = (GOOMBAServer_module_conf *)get_module_config(r->per_dir_config,&GOOMBAServer_module);

	/* 
	 * If GOOMBAServer parser engine has been turned off with the GOOMBAServerEngine off directive,
	 * then decline to handle this request
	 */
	if(!conf->engine) {
		r->content_type = "text/html";		
		return DECLINED;
	}

	/* Open the file */	
	if((fd=popenf(r->pool, r->filename, O_RDONLY, 0))==-1) {
		log_reason("file permissions deny server access", r->filename, r);
		return FORBIDDEN;
	}

	/* Apache 1.2 has a more complex mechanism for reading POST data */
#if MODULE_MAGIC_NUMBER > 20221007
	if ((retval = setup_client_block(r,REQUEST_CHUNKED_ERROR))) return retval;
#endif

	if (conf->LastModified) {
		if(retval = set_last_modified (r, r->finfo.st_mtime)) {
		  return retval;
		}
	}

	/* Assume output will be HTML.  Individual scripts may change this 
	   further down the line */
	r->content_type = "text/html";		

	/* Init timeout */
	hard_timeout ("send", r);

	save_umask();
	chdir_file(r->filename);
	add_common_vars(r);
	add_cgi_vars(r);
	apache_GOOMBAServer_module_main(r,conf,fd);

	/* Done, restore umask, turn off timeout, close file and return */
	restore_umask();
	kill_timeout (r);
	pclosef(r->pool, fd);
	return OK;
}

/* Initialize a per-server module configuation structure */
void *GOOMBAServer_create_conf(pool *p, char *dummy) {
	GOOMBAServer_module_conf *new;

	new = (GOOMBAServer_module_conf *)palloc(p,sizeof(GOOMBAServer_module_conf));	
	new->ShowInfo = 1;
	new->Logging = 1;
	new->UploadTmpDir=NULL;
	new->dbmLogDir=NULL;
	new->SQLLogDB=NULL;
	new->SQLLogHost=NULL;
	new->AccessDir=NULL;
	new->MaxDataSpace=8192;
	new->XBitHack=DEFAULT_GOOMBAServer_XBITHACK;
	new->IncludePath=NULL;
	new->AutoPrependFile=NULL;
	new->AutoAppendFile=NULL;
	new->Debug = 0;
	new->engine = 1;
	new->LastModified = 0;
	new->AdaUser = NULL;
	new->AdaPW = NULL;
	new->AdaDB = NULL;
	return new;
}

#if MODULE_MAGIC_NUMBER > 20221007
const char *GOOMBAServerflaghandler(cmd_parms *cmd, GOOMBAServer_module_conf *conf, int val) {
#else
char *GOOMBAServerflaghandler(cmd_parms *cmd, GOOMBAServer_module_conf *conf, int val) {
#endif
	int c = (int)cmd->info;

	switch(c) {
	case 0:
		conf->ShowInfo = val;
		break;
	case 1:
		conf->Logging = val;
		break;
	case 2:
		conf->XBitHack = val;
		break;
	case 3:
		conf->Debug = val;
		break;
	case 4:
		conf->engine = val;
		break;
	case 5:
		conf->LastModified = val;
		break;
	}			
	return NULL;
}

#if MODULE_MAGIC_NUMBER > 20221007
const char *GOOMBAServertake1handler(cmd_parms *cmd, GOOMBAServer_module_conf *conf, char *arg) {
#else
char *GOOMBAServertake1handler(cmd_parms *cmd, GOOMBAServer_module_conf *conf, char *arg) {
#endif
	int c = (int)cmd->info;

	switch(c) {
	case 0:
		conf->UploadTmpDir = pstrdup(cmd->pool,arg);
		break;
	case 1:
		conf->dbmLogDir = pstrdup(cmd->pool,arg);
		break;
	case 2:
		conf->SQLLogDB = pstrdup(cmd->pool,arg);
		break;
	case 3:
		conf->AccessDir = pstrdup(cmd->pool,arg);
		break;
	case 4:
		conf->MaxDataSpace = atoi(arg);
		break;
	case 5:
		conf->SQLLogHost = pstrdup(cmd->pool,arg);
		break;
	case 6:
		conf->IncludePath = pstrdup(cmd->pool,arg);
		break;
	case 7:
		conf->AutoPrependFile = pstrdup(cmd->pool,arg);
		break;
	case 8:
		conf->AutoAppendFile = pstrdup(cmd->pool,arg);
		break;
	case 9:
		conf->AdaDB = pstrdup(cmd->pool,arg);
		break;
	case 10:
		conf->AdaUser = pstrdup(cmd->pool,arg);
		break;
	case 11:
		conf->AdaPW = pstrdup(cmd->pool,arg);
		break;
	}
	return NULL;
}

int GOOMBAServer_xbithack_handler (request_rec *r) {   
	GOOMBAServer_module_conf *conf;

	conf = (GOOMBAServer_module_conf *)get_module_config(r->per_dir_config,&GOOMBAServer_module);
	if (!(r->finfo.st_mode & S_IXUSR)) return DECLINED;
	if (conf->XBitHack == 0) return DECLINED;
	return send_parsed_GOOMBAServer (r);
}  

handler_rec GOOMBAServer_handlers[] = {
	{ "application/x-httpd-GOOMBAServer", send_parsed_GOOMBAServer },
	{ "text/html", GOOMBAServer_xbithack_handler },
	{ NULL }
}; 


command_rec GOOMBAServer_commands[] = {
	{ "GOOMBAServerShowInfo",GOOMBAServerflaghandler,(void *)0,OR_OPTIONS,FLAG,"on|off" },
	{ "GOOMBAServerLogging",GOOMBAServerflaghandler,(void *)1,OR_OPTIONS,FLAG,"on|off" },
	{ "GOOMBAServerDebug",GOOMBAServerflaghandler,(void *)3,OR_OPTIONS,FLAG,"on|off" },
	{ "GOOMBAServerUploadTmpDir",GOOMBAServertake1handler,(void *)0,OR_OPTIONS,TAKE1,"directory" },
	{ "GOOMBAServerDbmLogDir",GOOMBAServertake1handler,(void *)1,OR_OPTIONS,TAKE1,"directory" },
	{ "GOOMBAServerMsqlLogDB",GOOMBAServertake1handler,(void *)2,OR_OPTIONS,TAKE1,"database" },
	{ "GOOMBAServerSQLLogDB",GOOMBAServertake1handler,(void *)2,OR_OPTIONS,TAKE1,"database" },
	{ "GOOMBAServerAccessDir",GOOMBAServertake1handler,(void *)3,OR_OPTIONS,TAKE1,"directory" },
	{ "GOOMBAServerMaxDataSpace",GOOMBAServertake1handler,(void *)4,OR_OPTIONS,TAKE1,"number of kilobytes" },
	{ "GOOMBAServerMsqlLogHost",GOOMBAServertake1handler,(void *)5,OR_OPTIONS,TAKE1,"hostname" },
	{ "GOOMBAServerSQLLogHost",GOOMBAServertake1handler,(void *)5,OR_OPTIONS,TAKE1,"hostname" },
	{ "GOOMBAServerXBitHack", GOOMBAServerflaghandler, (void *)2, OR_OPTIONS, FLAG, "on|off" },
	{ "GOOMBAServerIncludePath",GOOMBAServertake1handler,(void *)6,OR_OPTIONS,TAKE1,"colon-separated path" },
	{ "GOOMBAServerEngine",GOOMBAServerflaghandler,(void *)4,RSRC_CONF,FLAG,"on|off" },
	{ "GOOMBAServerLastModified", GOOMBAServerflaghandler, (void *)5, OR_OPTIONS, FLAG, "on|off" },
	{ "GOOMBAServerAutoPrependFile", GOOMBAServertake1handler,(void *)7,OR_OPTIONS,TAKE1,"file name" },
	{ "GOOMBAServerAutoAppendFile", GOOMBAServertake1handler,(void *)8,OR_OPTIONS,TAKE1,"file name" },
	{ "GOOMBAServerAdaDefDB",GOOMBAServertake1handler,(void *)9,OR_OPTIONS,TAKE1,"database" },
	{ "GOOMBAServerAdaDefUser",GOOMBAServertake1handler,(void *)10,OR_OPTIONS,TAKE1,"user" },
	{ "GOOMBAServerAdaDefPW",GOOMBAServertake1handler,(void *)11,OR_OPTIONS,TAKE1,"password" },
	{ NULL }
};



module GOOMBAServer_module = {
	STANDARD_MODULE_STUFF,
	NULL,				/* initializer */
	GOOMBAServer_create_conf,	/* dir config creater */
	NULL,				/* dir merger --- default is to override */
	NULL,				/* server config */
	NULL,				/* merge server config */
	GOOMBAServer_commands,		/* command table */
	GOOMBAServer_handlers,		/* handlers */
	NULL,				/* filename translation */
	NULL,				/* check_user_id */
	NULL,				/* check auth */
	NULL,				/* check access */
	NULL,				/* type_checker */
	NULL,				/* fixups */
	NULL				/* logger */
};
