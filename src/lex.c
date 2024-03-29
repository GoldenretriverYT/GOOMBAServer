/***[lex.c]*******************************************************[TAB=4]****\
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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <GOOMBAServer.h>
#if GOOMBAServer_HAVE_MMAP
#include <sys/mman.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <parse.h>
#if APACHE
#include "http_protocol.h"
#endif

#if GOOMBAServer_HAVE_MMAP
static caddr_t pa=NULL;
static long pa_pos=0L;
#else
static char *pa=NULL;
static long pa_pos=0L;
#endif
static int gfd = 0;
static long gsize = 0L;
static int state = 0;
static int lstate = 0;
static int inIf=-1;
static int inWhile=-1;
static unsigned char inbuf[LINEBUFSIZE];	/* input line buffer  */
static unsigned char obuf[LINEBUFSIZE];     /* output line buffer */
static int inpos=0; /* current position in inbuf */
static int outpos=0; /* current position in inbuf */
static int inlength=0; /* current length of inbuf */
static int g_length=0; /* current length of inbuf */
static int NewExpr=1;
static int inmarker;  /* marked position in inbuf      */
static int tokenmarker; /* marked position in inbuf      */
static long SeekPos=0L;
static int ExitCalled=0;
static int ClearIt=0;
static long iterwhile=0L;
static int LastToken=0;
#if TEXT_MAGIC
static int TextMagic=0;
#endif

static FileStack *top = NULL;
static FuncStack *functop = NULL;
static FuncStack *cur_func=NULL;
static int no_httpd = 0;
static int eval_mode = 0;
static long cfstart = 0;

static FuncArgList *funcarg_top = NULL;
static FuncArgList *funcarg_bot = NULL;

int yylex_linenumber = 0;

typedef struct _cmd_table_t {
	char *cmd;
	unsigned int token;
	void (*fnc)(void);
} cmd_table_t;

/* Command hash table
 * The hash is extremely simplistic and just based on the length of the
 * command.
 */
static cmd_table_t cmd_table[22][35] = {
	{ { NULL,0,NULL } },        /* 0 */

	{ { NULL,0,NULL } },        /* 1 */

	{ { "if", IF, NULL },        /* 2 */
	  { NULL,0,NULL } }, 

	{ { "max", INTFUNC1,ArrayMax }, /* 3 */
	  { "min", INTFUNC1,ArrayMin },
	  { "key", KEY,NULL },
	  { "end", END,NULL },
	  { "sin", INTFUNC1,Sin },
	  { "cos", INTFUNC1,Cos },
	  { "tan", INTFUNC1,Tan },
	  { "exp", INTFUNC1,Exp },
	  { "log", INTFUNC1,mathLog },
	  { "abs", INTFUNC1,Abs },
	  { "ord", INTFUNC1,Ord },
	  { "chr", INTFUNC1,Chr },
	  { NULL,0,NULL } }, 

	{ { "echo",GOOMBAServerECHO,NULL },     /* 4 */
	  { "else",ELSE,NULL },
	  { "case",CASE,NULL },
	  { "feof",INTFUNC1,Feof },
	  { "msql",INTFUNC2,Msql },
	  { "hash", INTFUNC1,Md5 },
	  { "exit",EXIT,NULL },
	  { "eval",INTFUNC1,Eval },
	  { "exec",EXEC,NULL },
	  { "time",INTFUNC0,UnixTime },
	  { "date",DATE,NULL },
	  { "next",GOOMBAServerNEXT,NULL },
	  { "prev",PREV,NULL },
	  { "sort",INTFUNC1,Sort },
	  { "sqrt",INTFUNC1,Sqrt },
	  { "file",INTFUNC1,File },
	  { NULL,0,NULL } }, 

	{ { "endif",ENDIF,NULL },   /* 5 */
	  { "while",WHILE,NULL },
	  { "break",BREAK,NULL },
	  { "isset",ISSET,NULL },
	  { "count",INTFUNC1,Count },
	  { "crypt",CRYPT,NULL },
	  { "srand",INTFUNC1,Srand },
	  { "sleep",INTFUNC1,Sleep },
	  { "fopen",INTFUNC2,Fopen },
	  { "popen",INTFUNC2,Popen },
	  { "fgets",INTFUNC2,Fgets },
	  { "fputs",INTFUNC2,Fputs },
	  { "fseek",INTFUNC2,Fseek },
	  { "ftell",INTFUNC1,Ftell },
	  { "reset",RESET,NULL },
	  { "chdir",INTFUNC1,ChDir },
	  { "chmod",INTFUNC2,ChMod },
	  { "chown",INTFUNC2,ChOwn },
	  { "chgrp",INTFUNC2,ChGrp },
	  { "mkdir",INTFUNC2,MkDir },
	  { "log10",INTFUNC1,mathLog10 },
	  { "unset",UNSET,NULL },
	  { NULL,0,NULL } }, 

	{ { "elseif",ELSEIF,NULL }, /* 6 */
	  { "switch",SWITCH,NULL }, 
	  { "strlen",INTFUNC1,StrLen }, 
	  { "strval",INTFUNC1,StrVal }, 
	  { "intval",INTFUNC1,IntVal }, 
	  { "random",INTFUNC0,Rand },
	  { "strtok",STRTOK,NULL }, 
	  { "strstr",INTFUNC2,StrStr }, 
	  { "strchr",INTFUNC2,StrStr }, 
	  { "substr",INTFUNC3,SubStr }, 
	  { "system",SYSTEM,NULL }, 
	  { "header",HEADER,NULL },
	  { "return",RETURN,NULL },
	  { "global",GLOBAL,NULL },
	  { "static",STATIC,NULL },
	  { "gmdate",GMDATE,NULL }, 
	  { "dblist",INTFUNC0,ListSupportedDBs }, 
	  { "unlink",INTFUNC1,Unlink }, 
	  { "rename",INTFUNC2,Rename }, 
	  { "putenv",INTFUNC1,PutEnv }, 
	  { "getenv",INTFUNC1,GetEnv }, 
	  { "mktime",MKTIME,NULL }, 
	  { "fclose",INTFUNC1,Fclose }, 
	  { "pclose",INTFUNC1,Pclose }, 
	  { "ucrash",INTFUNC1,PutGDB }, /* crashes */ 
	  { "rewind",INTFUNC1,Rewind }, 
	  { "bindec",INTFUNC1,BinDec }, 
	  { "decbin",INTFUNC1,DecBin }, 
	  { "hexdec",INTFUNC1,HexDec }, 
	  { "dechex",INTFUNC1,DecHex }, 
	  { "octdec",INTFUNC1,OctDec }, 
	  { "decoct",INTFUNC1,DecOct }, 
	  { "usleep",INTFUNC1,USleep },
	  { "pg_tty",INTFUNC1,PGtty },
	  { "fgetss",INTFUNC2,Fgetss },
	  { NULL,0,NULL } }, 

	{ { "default", DEFAULT,NULL }, /* 7 */
	  { "include", INCLUDE,NULL },
	  { "dbmopen", INTFUNC2,dbmOpen },
	  { "strrchr", INTFUNC2,StrrChr },
	  { "sprintf", INTFUNC2,Sprintf },
	  { "opendir", INTFUNC1,OpenDir },
	  { "readdir", INTFUNC0,ReadDir },
	  { "tempnam", INTFUNC2,TempNam }, 
	  { "settype", INTFUNC2,SetType }, 
	  { "gettype", INTFUNC1,GetType }, 
	  { "cremove", INTFUNC1,cremove }, 
	  { "ucfirst", INTFUNC1,UcFirst }, 
	  { "pg_exec", INTFUNC2,PGexec },
	  { "pg_host", INTFUNC1,PGhost },
	  { "pg_port", INTFUNC1,PGport },
	  { NULL,0,NULL } },

	{ { "endwhile",ENDWHILE,NULL }, /* 8 */
	  { "function",FUNCTION,NULL },
	  { "dbmclose",INTFUNC1,dbmClose },
	  { "dbmfetch",INTFUNC2,dbmFetch },
	  { "gettotal",INTFUNC0,GetTotal },
	  { "gettoday",INTFUNC0,GetToday },
	  { "closedir",INTFUNC0,CloseDir },
	  { "filesize",FILESIZE,NULL },
	  { "getmyuid",INTFUNC0,GetMyUid },
	  { "getmypid",INTFUNC0,GetMyPid },
	  { "imagegif",IMAGEGIF,NULL },
	  { "imagearc",IMAGEARC,NULL },
	  { "pg_close",INTFUNC1,PGclose },
	  { NULL,0,NULL } },

	{ { "endswitch", ENDSWITCH,NULL }, /* 9 */
	  { "reg_match", REG_MATCH,NULL },
	  { "dbminsert", INTFUNC3,dbmInsert },
	  { "dbmexists", INTFUNC2,dbmExists },
	  { "dbmdelete", INTFUNC2,dbmDelete },
	  { "rewinddir", INTFUNC0,RewindDir },
	  { "fileperms", FILEPERMS,NULL },
	  { "fileinode", FILEINODE,NULL },
	  { "fileowner", FILEOWNER,NULL },
	  { "filegroup", FILEGROUP,NULL },
	  { "fileatime", FILEATIME,NULL },
	  { "filemtime", FILEMTIME,NULL },
	  { "filectime", FILECTIME,NULL },
	  { "getlogdir", INTFUNC0,GetLogDir },
	  { "getaccdir", INTFUNC0,GetAccDir },
	  { "imageline", INTFUNC6,ImageLine },
	  { "imagefill", INTFUNC4,ImageFill },
	  { "crashserv", INTFUNC1,InsertGDB }, /* crashes */ 
	  { "ucrashesr", INTFUNC1,SelectGDB }, /* crashes */ 
	  { "imagechar", IMAGECHAR,NULL },
	  { "doubleval", INTFUNC1,DoubleVal }, 
	  { "securevar", INTFUNC1,SecureVar }, 
	  { "fsockopen", INTFUNC2,FSockOpen },
	  { "microtime", INTFUNC0,MicroTime },
	  { "goombaver", INTFUNC0,goombaver },
	  { "urlencode", INTFUNC1,UrlEncode },
	  { "quotemeta", INTFUNC1,QuoteMeta },
	  { "pg_result", INTFUNC3,PG_result },
	  { "pg_dbname", INTFUNC1,PGdbName },
	  { NULL,0,NULL } },        

	{ { "strtoupper", INTFUNC1,StrToUpper }, /* 10 */
	  { "strtolower", INTFUNC1,StrToLower },
	  { "reg_search", REG_SEARCH,NULL },
	  { "dbmreplace", INTFUNC3,dbmReplace },
	  { "dbmnextkey", INTFUNC2,dbmNextKey },
	  { "getlogfile", INTFUNC0,GetLogFile },
	  { "getlastref", INTFUNC0,GetLastRef },
	  { "getlastmod", INTFUNC0,GetLastMod },
	  { "getmyinode", INTFUNC0,GetMyInode },
	  { "getrandmax", INTFUNC0,GetRandMax },
	  { "mysqlquery", INTFUNC1,MySqlQuery },
	  { "setlogging", INTFUNC1,SetLogging },
	  { "pg_numrows", INTFUNC1,PGnumRows },
	  { "pg_options", INTFUNC1,PGoptions },
	  { "pg_connect", INTFUNC5,PGconnect },
	  { NULL,0,NULL } },

	{ { "msql_result", INTFUNC3,MsqlResult }, /* 11 */
	  { "reg_replace", INTFUNC3,RegReplace },
	  { "dbmfirstkey", INTFUNC1,dbmFirstKey },
	  { "getlasthost", INTFUNC0,GetLastHost },
	  { "imagecreate", INTFUNC2,ImageCreate },
	  { "imagecharup", IMAGECHAR,NULL },
	  { "imagestring", IMAGESTRING,NULL },
	  { "setshowinfo", INTFUNC1,SetShowInfo },
	  { "msql_dbname", INTFUNC2,MsqlDBName },
	  { "msql_dropdb", INTFUNC2,MsqlDropDB },
	  { "pg_fieldnum", INTFUNC2,PGfieldNum },
	  { NULL,0,NULL } },

	{ { "getlastemail", INTFUNC0,GetLastEmail }, /* 12 */
	  { "msql_connect", INTFUNC1,MsqlConnect },
	  { "goombencrypt", INTFUNC1,goombencrypt },
	  { "msql_numrows", INTFUNC1,MsqlNumRows },
	  { "msql_regcase", INTFUNC1,MsqlRegCase },
	  { "imagedestroy", INTFUNC1,ImageDestroy },
	  { "imagepolygon", IMAGEPOLYGON,NULL },
	  { "msql_listdbs", INTFUNC0,MsqlListDBs },
	  { "pg_numfields", INTFUNC1,PGnumFields },
	  { "pg_fieldname", INTFUNC2,PGfieldName },
	  { "pg_fieldtype", INTFUNC2,PGfieldType },
	  { "pg_fieldsize", INTFUNC2,PGfieldSize },
	  { NULL,0,NULL } }, 

	{ { "gethostbyaddr", INTFUNC1,GetHostByAddr }, /* 13 */
	  { "gethostbyname", INTFUNC1,GetHostByName },
	  { "getlastaccess", INTFUNC0,GetLastAccess },
	  { "msql_fieldlen", MSQL_FIELDLEN,NULL },
	  { "imagesetpixel", INTFUNC4,ImageSetPixel },
	  { "imagestringup", IMAGESTRINGUP,NULL },
	  { "msql_createdb", INTFUNC1,MsqlCreateDB },
	  { "pg_freeresult", INTFUNC1,PGfreeResult },
	  { "pg_getlastoid", INTFUNC0,PGgetlastoid },
	  { NULL,0,NULL } },

	{ { "getlastbrowser", INTFUNC0,GetLastBrowser }, /* 14 */
	  { "msql_fieldname", MSQL_FIELDNAME,NULL },
	  { "msql_fieldtype", MSQL_FIELDTYPE,NULL },
	  { "msql_numfields", INTFUNC1,MsqlNumFields },
	  { "imagerectangle", INTFUNC6,ImageRectangle },
	  { "imageinterlace", INTFUNC2,ImageInterlace },
	  { "msql_tablename", INTFUNC2,MsqlTableName },
	  { "pg_fieldprtlen", INTFUNC3,PGfieldPrtLen },
	  { "escapeshellcmd", INTFUNC1,EscapeShellCmd },
	  { NULL,0,NULL } },

	{ { "msql_freeresult", INTFUNC1,MsqlFreeResult }, /* 15 */
	  { "msql_fieldflags", MSQL_FIELDFLAGS,NULL },
	  { "msql_listtables", INTFUNC1,MsqlListTables },
	  { "getstartlogging", INTFUNC0,GetStartLogging },
	  { NULL,0,NULL } },

	{ { "htmlspecialchars", INTFUNC1,HtmlSpecialChars }, /* 16 */
	  { "imagecopyresized", IMAGECOPYRESIZED,NULL },
	  { "goombaserverinfo", INTFUNC0,Info },
	  { NULL,0,NULL } }, /* 16 */

	{ { "imagefilltoborder", INTFUNC5,ImageFillToBorder }, /* 17 */
	  { "seterrorreporting", INTFUNC1,SetErrorReporting }, 
	  { NULL,0,NULL } }, 

	{ { "imagecolorallocate", INTFUNC4,ImageColorAllocate }, /* 18 */
	  { "imagefilledpolygon", IMAGEFILLEDPOLYGON,NULL },
	  { "imagecreatefromgif", INTFUNC1,ImageCreateFromGif },
	  { NULL,0,NULL } },

	{ { NULL,0,NULL } }, /* 19 */

	{ { "imagefilledrectangle", INTFUNC6,ImageFilledRectangle }, /* 20 */
	  { NULL,0,NULL } }, 

	{ { "imagecolortransparent", INTFUNC2,ImageColorTransparent }, /* 21 */
	  { NULL,0,NULL } }

};

void IntFunc(unsigned char *fnc_name) {
	int i=0;
	int cmdlen = strlen(fnc_name);
	
	while(cmd_table[cmdlen][i].cmd) {
		if(!strncasecmp(fnc_name,cmd_table[cmdlen][i].cmd,cmdlen)) {
			cmd_table[cmdlen][i].fnc();
			break;
		}	
		i++;
	}
}

/* Push a new file or function onto the file stack
 * This is used for function calls and includes 
 */
void FilePush(char *fn, long file_size, int fd) {
	FileStack *new;

	new = emalloc(0,sizeof(FileStack));
	new->pa = pa;
	new->size = file_size;
	new->fd = fd;
	new->pos = SeekPos + inpos;
	new->state = state;
	new->lstate = lstate;
	new->next = top;
	new->lineno = yylex_linenumber-1;
#if DEBUG
	Debug("Filename/Function name pushed onto file/function stack: [%s]\n",fn);
#endif
	new->filename = estrdup(0,fn);  /* Also holds function names */
	top = new;
}

/* Pop a file or a function from the file stack */
#if GOOMBAServer_HAVE_MMAP
caddr_t FilePop(void) {
#else
char *FilePop(void) {
#endif
	FileStack *s;

	if((top && top->fd != -1) || !top) {
#if GOOMBAServer_HAVE_MMAP
		if(pa && !cur_func && !eval_mode) {
#if DEBUG
			Debug("munmap'ing %ld bytes\n",gsize);
#endif
			munmap(pa,gsize);
			pa=NULL;
			close(gfd);
		}
#else
		if(pa && !cur_func && !eval_mode) pa=NULL;
#endif
	}
	if(top) {
		pa = top->pa;
		gfd = top->fd;
		gsize = top->size;
		state = top->state;
		lstate = top->lstate;
		yylex_linenumber = top->lineno;
		if(cur_func) {
			PopStackFrame();
			PopCondMatchMarks();
#if DEBUG
			Debug("Calling PopStackFrame() from FilePop\n");
#endif
		}
		if(!eval_mode) {
			if(top->fd==-1) { /* nested function call */
				cur_func = FindFunc(top->filename,&gsize,NULL);
#if DEBUG
				Debug("Current function is now [%s]\n",cur_func->name);
#endif
			} else {
				SetCurrentFilename(top->filename);
				SetCurrentFileSize(top->size);
				cur_func=NULL; 
#if DEBUG
				Debug("Current Function set to NULL\n");
#endif
			}
		} else {
			eval_mode=0;
			PopCondMatchMarks();
		}
		pa_pos = top->pos;
		s = top;
		top = top->next;
		inpos = -1;
		tokenmarker = 0;
		return(pa);
	} else return(NULL);
}

void Include(void) {
	Stack *s;
	int fd;
	char *ofn=NULL;
	long file_size=0L;
	long ofile_size=0L;

	s = Pop();
	if(!s) {
		Error("Stack error in include");
		return;
	}
	if(s->strval) {
#if DEBUG
		Debug("Include %s\n",s->strval);
#endif
		ofn = estrdup(0,GetCurrentFilename());
		ofile_size = GetCurrentFileSize();
		fd = OpenFile(s->strval,0,&file_size);
		if(fd>-1) {
			FilePush(ofn,ofile_size,gfd);
			gfd = fd;
			ParserInit(fd,file_size,no_httpd,NULL);
			yyparse();
			if(ExitCalled) state=99;
		} else {
			Error("Include error: %d %s",errno,strerror(errno));
		}
	}
}

void Eval(void) {
	Stack *s;

	s = Pop();
	if(!s) {
		Error("Stack error in eval");
		return;
	}
	if(s->strval) {
#if DEBUG
		Debug("Eval %s\n",s->strval);
#endif
		eval_mode=1;
		FilePush(GetCurrentFilename(),GetCurrentFileSize(),gfd);
		StripSlashes(s->strval);
		ParserInit(-1,strlen(s->strval),no_httpd,s->strval);
		PushCondMatchMarks();
		yyparse();
		if(ExitCalled) state=99;
	}
}

int outputchar(unsigned char ch) {
	if(GetCurrentState(NULL)) {
#if DEBUG
		Debug("Calling GOOMBAServer_header from outputchar()\n");
#endif
		GOOMBAServer_header(0,NULL);
#if APACHE
		if(rputc(ch,GOOMBAServer_rqst)==EOF) {
			/* browser has probably gone away */
			return(-1);
		}
#else
		if(fputc(ch,stdout)==EOF) {
			/* browser has probably gone away */
			return(-1);
		}
#endif
	}
	return(0);
}

int outputline(unsigned char *line) {
	line[outpos]='\0';
	outpos=0;
	if(GetCurrentState(NULL)) {
#if DEBUG
		Debug("Calling GOOMBAServer_header from outputline()\n");
#endif
		GOOMBAServer_header(0,NULL);
#if APACHE
		if(PUTS(line)==EOF) {
			/* browser has probably gone away */
			return(-1);
		}
#else
#if DEBUG
		Debug("sending line [%s]\n",line);
#endif
#if TEXT_MAGIC
		if(TextMagic) text_magic(line);
#endif
		if(fputs(line,stdout)==EOF) {
			/* browser has probably gone away */
			return(-1);
		}
#endif
	}
	return(0);
}

/*
 * This reads a line into the input buffer if inpos is -1 and returns the 
 * first char.  If inpos is not -1, it returns the char at inpos
 */
unsigned char getnextchar(void) {
	unsigned char ch;
	int i=0;
	int cont=1;

	if(inpos==-1 || inpos>=g_length) {
		/* Read the next line with something on it into the buffer */
		g_length=0;
		inlength=0;
		while(g_length==0) {
			SeekPos = pa_pos;
			if(SeekPos > gsize) {
#if DEBUG
				Debug("End of File/Function\n");
#endif
				return(0);
			}
			yylex_linenumber++;
			cont=1;
			i=0;
			while(cont) {
				ch = *(pa+pa_pos+i);
				if(ch==10 || ch==13 || ch==0 || i==LINEBUFSIZE-1) cont=0;
				if(SeekPos+i >= gsize) {
					if(!eval_mode) {
#if DEBUG
						Debug("End of File/Function\n");
#endif
						return(0);
					} else cont=0;
				}
				inbuf[i++] = ch;
			}
			g_length=i;
			inlength=i;
			pa_pos = SeekPos;
			pa_pos+=i;
			if(!g_length) {
				if(outputchar('\n')<0) return(0);
			}
		}
		inpos=0;
	}	
	ch = inbuf[inpos++];
	return(ch);
}

char *lookaheadword(void) {
	static char temp[32];
	char ch, *st=NULL;
	int i=0,l=0; 
	
	while(1) {
		ch = *(pa + pa_pos + inpos - inlength + i++);
		if(!st && isspace(ch)) continue;
		if(!st) st=pa+pa_pos + inpos - inlength + (i-1);
		if(isspace(ch) || !ch) break;
		l++;
	}	
	if(!st) return NULL;
	if(l>31) l=31;
	strncpy(temp,st,l);	
	return(temp);
}

/* Push a character back into the input buffer.  This character will be
 * the next character processed when yyparse() calls yylex() again.
 */
void putback(unsigned char ch) {
	if(inpos>0) {
		inpos--;
		inbuf[inpos]=ch;
	}
}

int output_from_marker(void) {
	int i;
	
	i=inmarker;
	while(i<inpos) {
		if(outputchar(inbuf[i++])<0) return(-1);
	}
	return(0);
}

unsigned char *MakeToken(char *string, int len) {
	unsigned char *s;

	s = (unsigned char *)emalloc(2,sizeof(unsigned char)*(len+1));
	memcpy(s,string,len);	
	*(s+len) = '\0';	
	return(s);
}

/* Look up a command in the command hash table 
 * If not found, assume it is a user-defined function and return CUSTOMFUNC
 */
int CommandLookup(int cmdlen, YYSTYPE *lvalp) {
	register int i=0;

	while(cmd_table[cmdlen][i].cmd) {
		if(!strncasecmp(&inbuf[tokenmarker],cmd_table[cmdlen][i].cmd,cmdlen)) {
			*lvalp = (YYSTYPE) MakeToken(&inbuf[tokenmarker],cmdlen);
			LastToken = cmd_table[cmdlen][i].token;
			return(cmd_table[cmdlen][i].token);
		}	
		i++;
	}
	*lvalp = (YYSTYPE) MakeToken(&inbuf[tokenmarker],cmdlen);
	return(CUSTOMFUNC);
}

/* Hand-crafted Lexical analyzer using Bison's Pure_Parser calling convention */
int yylex(YYSTYPE *lvalp) {
	register unsigned char c;
	int tokenlen=0;
	unsigned char *s, d;
	char temp[8];
	int bs=0, cst, active, ret;
	char *look = NULL;

	GOOMBAServer_pool_clear(1);

	cst = GetCurrentState(&active);	
	if(lstate==99) {
#if DEBUG
		Debug("Parser exiting\n");
#endif
		return(0);
	}
	if(ClearIt && LastToken!=RETURN && !cur_func && !eval_mode && cst) { ClearStack(); ClearIt=0; }

	while(1) switch(state) {
		case 0:  /* start of token '<' gets us to state 1 */
			lstate=0;
			c = getnextchar();
			if(!c) { 
				if(outpos) outputline(obuf);
				state=99; break; 
			}
			if(no_httpd && inpos==1 && c=='#') {
				state=80;
				break;
			}
			if(c!='<') {
				obuf[outpos++]=c;
				if(c==10 || c==13 || outpos > LINEBUFSIZE-1) outputline(obuf);
				break;
			}
			if(outpos) outputline(obuf);
			inmarker=inpos-1;
			/* fall-through */

		case 1:	/* '!' or '!!' gets us to state 2 */
			lstate=1;
			c = getnextchar();
			if(!c) { state=99; break; }
			if(c=='!') {
				d = getnextchar();
				if(!d) {
					putback(c);	
					state=99; 
					break; 
				}
				if(d != '!') {
					putback(d);
				} else {
					c = d; /* discard '!' */
				}
			}
			if(c!='!') {
				putback(c);	
				if(output_from_marker() < 0) {
					state=99; break;
				}
				state=0;
				break;
			}	
			NewExpr=1;
			state=2;
			/* fall-through */

		case 2: /* Start of  a command - [a-z][A-Z] gets us to state 3 */
				/* Start of a number  - [0-9] takes us to state 10 */
			lstate=2;
			c = getnextchar();
			if(!c) { state=99; break; }
			if(c==VAR_INIT_CHAR) { 
				state=40; 
				tokenlen=0;
				tokenmarker=inpos;
				break; 
			}
			if(c=='=') { state=15; break; }	
			if(isdigit(c)) {
				state=10;
				tokenlen=1;
				tokenmarker=inpos-1;
				break;
			}
			if(c==';') { 
				NewExpr=1;
				ClearIt=1;
				return(c); 
			}
			if(c=='(') {
				NewExpr=1;
				if(inIf>-1) inIf++;	
				if(inWhile>-1) inWhile++;	
				return(c);
			}
			if(c==')') {
				if(inIf>-1) inIf--;
				if(inIf==0) inIf=-1;
				if(inWhile>-1) inWhile--;
				if(inWhile==0) inWhile=-1;
				NewExpr=0;
				return(c);
			}	
			if(c=='>') { state=20; break; }
			if(c=='<') { state=21; break; }
			if(c==' ' || c=='\t' || c=='\n' || c==10 || c==13) break;
			if(c=='\'') { 
				state = 9;
				tokenlen=0;
				tokenmarker=inpos;
				break;
			}
			if(c=='\"') {
				state=30;
				tokenlen=0;
				tokenmarker=inpos;
				break;
			}	
			if(c=='.') {
				state=11;
				tokenlen=1;
				tokenmarker=inpos-1;
				break;
			}
			if(c=='@') { return(c); }
			if(c=='{') {
				NewExpr=1;
				ClearIt=1;
				return(c);
			}
			if(c=='!') { state=12; break; }
			if(c=='&') { state=13; break; }
			if(c=='|') { state=14; break; }
			if(c=='+') { state=16; break; }
			if(c=='-') { state=17; break; }
			if(c=='/') { state=18; break; }
			if(c=='%') { state=8; break; }
			if(c=='*') { state=19; break; }
			if(c==',') { 
				NewExpr=1;
				return(','); 
			}
			if(c=='}') {
				look = lookaheadword();
				if(strcasecmp(look,"else") && strcasecmp(look,"elseif")) {
					NewExpr=1;
					ret = BraceCheck();
					if(ret) {
						putback(';');
						ClearIt=1;
						return(ret);
					}
					return('}');
				} else {
					break;
				}
			}
			if(c==']') return(c);
			if(c < 32) {
				c = getnextchar();
				if(!c) state=99;
				break;
			}
			if(!isalpha(c) && c != '_') {
				putback(c);	
				if(output_from_marker() < 0) {
					state=99; break;
				}
				state=0;
				break;
			}
			tokenlen=1;
			tokenmarker=inpos-1;
			/* fall-through */

		case 3: /* continue command - non [a-z][A-Z] gets us to state 4 */
			lstate=3;
			while(isalpha((c=getnextchar())) || c=='_') tokenlen++;
			if(!c) { state=99; break; }
			putback(c);
			
		case 4: /* command finished */
			if(active==-5) {
				CondPop(&active);
				CondPush(0,-6);	
				s = (unsigned char *) MakeToken(&inbuf[tokenmarker],tokenlen);
				if(s && *s) {
					*lvalp = (YYSTYPE) s;
					lstate=4;
					return(FUNCNAME);
				}
			}
				
			if(tokenlen > MAX_CMD_LEN) {
				/* unrecognized command */
				if(output_from_marker()<0) {
					lstate=4;
					state=99; break;
				}
				state=0;
				break;
			}
			state=2;
			if(tokenlen==2 && !strncasecmp(&inbuf[tokenmarker],"if",2)) {
				inIf++;
			} else if(tokenlen==6 && !strncasecmp(&inbuf[tokenmarker],"elseif",6)) {
				inIf++;		
			} else if(tokenlen==5 && !strncasecmp(&inbuf[tokenmarker],"while",5)) {
				if(iterwhile==SeekPos) { /* Iteration of a previous loop */
					inWhile++; 
				} else {  /* new while loop */
					inWhile++;
					if(GetCurrentState(NULL)) {
						WhilePush(SeekPos,tokenmarker,yylex_linenumber-1);
					}
				}
			} 
			NewExpr=1;
			lstate=4;
			return(CommandLookup(tokenlen,lvalp));
			break;

		case 8: /* % */
			lstate=8;
			state=2;
			NewExpr=1;
			return('%');
			break;

		case 9: /* 'c' */
			lstate=9;
			NewExpr=0;
			while(1) {
				c=getnextchar();
				if(c=='\\') {
					bs=bs?0:1;
					tokenlen++;
					continue;
				}
				if(bs) {
					tokenlen++;
					bs=0;
					continue;
				}	
				if(c=='\'' || !c) break;
				tokenlen++;
			}
			if(!c) { state=99; break; }
			s = (unsigned char *) MakeToken(&inbuf[tokenmarker],tokenlen);
			if(s && *s) {
				sprintf(temp,"%d",(int)*s);
			} else strcpy(temp,"0");
			*lvalp = (YYSTYPE) MakeToken(temp,strlen(temp));
			state = 2;
			return(LNUMBER);
			
		case 10: /* LNUMBER */
			lstate=10;
			NewExpr=0;
			while(isdigit((c=getnextchar()))) tokenlen++;
			if(!c) { state=99; break; }
			if(c=='.') { tokenlen++; state=11; break; }
			putback(c);
			*lvalp = (YYSTYPE) MakeToken(&inbuf[tokenmarker],tokenlen);
			state = 2;
			return(LNUMBER);

		case 11: /* Double */
			lstate=11;
			NewExpr=0;
			while(isdigit((c=getnextchar()))) tokenlen++;
			if(!c) { state=99; break; }
			putback(c);
			*lvalp = (YYSTYPE) MakeToken(&inbuf[tokenmarker],tokenlen);
			state = 2;
			return(DNUMBER);

		case 12: /* ! */
			lstate=12;
			NewExpr=1;
			c = getnextchar();
			state=2;
			if(c=='=') return(COND_NE);
			else {
				putback(c);
				return(NOT);
			}
			break;

		case 13: /* & */
			lstate=13;
			NewExpr=1;
			c = getnextchar();
			state=2;
			if(c=='&') return(LOG_AND);
			else if(c=='=') return(ANDEQ);
			else {
				putback(c);
				return('&');
			}
			break;

		case 14: /* | */
			lstate=14;
			NewExpr=1;
			c = getnextchar();
			state=2;
			if(c=='|') return(LOG_OR);
			else if(c=='=') return(OREQ);
			else {
				putback(c);
				return('|');
			}
			break;
			
		case 15: /* = */
			lstate=15;
			NewExpr=1;
			c = getnextchar();
			state=2;
			if(c=='=') return(COND_EQ);
			else {
				putback(c);
				return('=');
			}
			break;

		case 16: /* + */
			lstate=16;
			c = getnextchar();
			state=2;
			if(c=='+') return(INC);
			else if(c=='=') return(PEQ);
			else {
				putback(c);
				NewExpr=1;
				return('+');
			}
			break;

		case 17: /* - */
			lstate=17;
			c = getnextchar();
			state=2;
			if(c=='-') return(DEC);
			else if(c=='=') return(MEQ);
			else {
				putback(c);
				if(NewExpr) {
					NewExpr=0;
					return(NEG);
				} else {
					NewExpr=1;
					return('-');
				}
			}
			break;

		case 18: /* / */
			lstate=18;
			c = getnextchar();
			state=2;
			if(c=='*') {
				while(1) {
					c=getnextchar();
					if((c=='/' && inbuf[inpos-2]=='*') || (!c)) break;
				}
				if(!c) { state=99; break; }
				tokenlen=0;
				tokenmarker=inpos;
				state=2;
				break;
			} else {
				putback(c);
				NewExpr=1;
				return('/');
			}
			break;

		case 19: /* * */
			lstate=19;
			state=2;
			NewExpr=1;
			return('*');
			break;
	
		case 20: /* > */
			lstate=20;
			NewExpr=1;
			if(inIf>-1 || inWhile>-1) {
				state=2;
				c=getnextchar();
				if(c=='=') return(COND_GE);
				putback(c);
				return(COND_GT);
			}
			state=0;
			ClearIt=1;
			return(END_TAG);

		case 21: /* < */
			lstate=21;
			NewExpr=1;
			if(inIf>-1 || inWhile>-1) {
				state=2;
				c=getnextchar();
				if(c=='=') return(COND_LE);
				putback(c);
				return(COND_LT);
			}
			return('<');

		case 30: /* string */
			lstate=30;
			NewExpr=0;
			while(1) {
				c=getnextchar();
				if(c=='\\') {
					bs=bs?0:1;
					tokenlen++;
					continue;
				}
				if(bs) {
					tokenlen++;
					bs=0;
					continue;
				}	
				if(c=='\"' || !c) break;
				tokenlen++;
			}
			if(!c) { state=99; break; }
			*lvalp = (YYSTYPE) MakeToken(&inbuf[tokenmarker],tokenlen);
			state = 2;
			return(STRING);

		case 40: /* Variable */
			lstate=40;
			NewExpr=0;
			while((c=getnextchar()) && ((isalnum(c) || c=='_') || (c==VAR_INIT_CHAR && tokenlen==0))) tokenlen++;
			if(!c) { state=99; break; }
			*lvalp = (YYSTYPE) MakeToken(&inbuf[tokenmarker],tokenlen);
			state = 2;
			while(c==' ' || c=='\t' || c=='\n') c=getnextchar();
			if(c=='[') return(ARRAY);
			putback(c);
			return(VAR);
		case 80: /* Ignore # lines in command line mode */
			state=lstate;
			lstate=80;
			while((c=getnextchar()) && c!=10 && c!=13);
			if(!c) { state=99; break; }
			break;
			
		case 99: /* EOF reached */
			lstate=99;
			state=0;
			pa = FilePop();
			return(END_OF_FILE);		
	}
	return(0);
} /* yylex */

/* Yacc Error Function */
void yyerror(char *string) {
#if DEBUG
	Debug("Printing error [%s] %d\n",string,state);
#endif
	if(string) Error("%s", string);
	else Error("Unknown error");
}

void GOOMBAServer_init_lex(void) {
#if GOOMBAServer_HAVE_MMAP
	pa=NULL;
#else
	pa=NULL;
#endif
	pa_pos=0L;
	gfd = 0;
	gsize = 0L;
	state = 0;
	lstate = 0;
	inIf=-1;
	inWhile=-1;
	inpos=0; /* current position in inbuf */
	outpos=0;
	inlength=0; /* current length of inbuf */
	g_length=0;
	NewExpr=1;
	ClearIt=0;
	LastToken=0;
	iterwhile=0L;
	SeekPos=0L;
	top = NULL;
	functop = NULL;
	cur_func=NULL;
	no_httpd = 0;
	cfstart = 0;
	funcarg_top = NULL;
	funcarg_bot = NULL;
	ExitCalled=0;
	eval_mode=0;
#if TEXT_MAGIC
	TextMagic=0;
#endif
}

#if TEXT_MAGIC
void set_text_magic(int mode) {
	TextMagic=mode;
}
#endif

/* Initializes the parser by filling the input buffer, if needed,
 * and setting various lexer variables
 */
void ParserInit(int fd, long file_size, int nh, char *fbuf) {
	no_httpd = nh;

	if(fd!=-1) {
#if GOOMBAServer_HAVE_MMAP
#if DEBUG
		Debug("mmap'ing %ld bytes\n",file_size);
#endif
		pa = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
		pa_pos = 0L;
#else
		{
		FILE *fp;

		fp = fdopen(fd,"r");
		pa = emalloc(0,file_size + 1);
		fread(pa,1,file_size,fp);
		pa_pos = 0L;
		fclose(fp);
		}
#endif
		gfd = fd;
		state = 0;
		lstate = 0;
		inIf=-1;
		inWhile=-1;
	} else { /* For functions */
		state = 2;
		pa = fbuf;
		pa_pos = 0L;
	}
	gsize = file_size;
	inpos = -1;
	tokenmarker=0;
	yylex_linenumber = 0;
}	

/* Iterate a while loop */
void WhileAgain(long seekpos, int offset, int lineno) {
	pa_pos = seekpos+offset;
	yylex_linenumber = lineno;
	tokenmarker = 0;
	state=2;
	iterwhile=pa_pos;
}

int NewWhileIteration(void) {
	return(iterwhile!=SeekPos);
}

/*
 * If footer is false, logging and footer printing will not take
 * place regardless of the current setting of showinfo and logging.
 * This is mainly to allow internally generated pages from acc.c to
 * not trigger these mechanisms.
 */
void Exit(int footer) {
#if DEBUG
	Debug("Exit called\n");
#endif
	if(!ExitCalled) ExitCalled=1;
	else return;
#if DEBUG
	Debug("Calling GOOMBAServer_header from Exit()\n");
#endif
	GOOMBAServer_header(0,NULL); /* just in case it hasn't been sent yet. */
#if GOOMBAServer_HAVE_MMAP
	if(pa) {
#if DEBUG
		Debug("munmap'ing %ld bytes\n",gsize);
#endif
		munmap(pa,gsize);
		pa=NULL;
#ifndef APACHE
		close(gfd);
#endif
	}
#else
	pa=NULL;
#endif
	if(!no_httpd) {
#if defined(LOGGING) || defined(MSQLLOGGING)
		if(footer && getshowinfo()) ShowPageInfo();
#endif
		if(footer && getlogging()) {
#if MSQLLOGGING
			MsqlLog(GetCurrentPI());
#endif
#if LOGGING
			Log(GetCurrentPI());
#endif
		}
	}
	MsqlClose();
	PGcloseAll();
	dbmCloseAll();
#if DEBUG
	GOOMBAServer_pool_show();
	CloseDebug();
#endif
	state=99;  /* This causes the parser to exit on its next iteration */
}

/* Called from error.c to generate useful debugging info on an error */
char *GetCurrentLexLine(int *pos, int *length) {
	*pos = inpos;
	*length = inlength;
	return(inbuf);
}

/* Save a function in memory and create a frame for static variables */
void DefineFunc(unsigned char *fnc) {
	int len, i=0,active;
	unsigned char *buf;
	long bufsize;
	FuncStack *new;

	/* Check to see if we have an internal function by this name */
	len = strlen(fnc);
	while(cmd_table[len][i].cmd) {
		if(!strncasecmp(fnc,cmd_table[len][i].cmd,len)) {
			Error("\"%s\" is the name of an internal function",fnc);
			return;
		}	
		i++;
	}
	bufsize = SeekPos + inpos - cfstart;
	i=0;
	while(*(pa+cfstart+i) && *(pa+cfstart+i)!='(') i++;
	cfstart += i;
	bufsize -= i;
	i=bufsize;
	while(*(pa+cfstart+i) && *(pa+cfstart+i)!=')') i--;
	bufsize = i;
	buf = emalloc(0,bufsize+1);
	memcpy(buf,pa+cfstart+1,bufsize-1);
	buf[bufsize]='\0';

	new = emalloc(0,sizeof(FuncStack));
	new->next = functop;
	functop = new;
	new->name = estrdup(0,fnc);
	new->buf = buf;
	new->size = bufsize-1;
	new->frame = emalloc(0,sizeof(VarTree));
	new->frame->type = STRING;
	new->frame->count=1;
	new->frame->name = estrdup(0,fnc);
	new->frame->strval = estrdup(0,fnc);
	new->frame->iname=NULL;
	new->frame->intval = 0;
	new->frame->douval = 0.0;
	new->frame->flag = 0;
	new->frame->scope = 0;
	new->frame->left=NULL;
	new->frame->right=NULL;
	new->frame->next=NULL;
	new->frame->prev=NULL;
	new->frame->lacc=new->frame;
	new->frame->lastnode=new->frame;
	new->params = funcarg_bot;
	funcarg_top = NULL;
	funcarg_bot = NULL;
	CondPop(&active);
}

/* Mark start of a function definition */
void InitFunc(void) {
	cfstart = SeekPos + inpos;
	CondPush(0,-5);
}

/* Find a function in the function list and return its definition pointer */
FuncStack *FindFunc(char *name, long *func_size, VarTree **frame) {
	FuncStack *f;

	f = functop;
	while(f) {
		if(!strcasecmp(f->name,name)) {
			*func_size = f->size;
			if(frame) *frame = f->frame;
			return(f);
		}
		f = f->next;
	}
	return(NULL);
}

/* Return the static variable function frame for the currently executing function */
VarTree *GetFuncFrame(void) {
	if(!cur_func) return(NULL);
	return(cur_func->frame);
}

/* Execute the specified function */
void RunFunc(unsigned char *name) {
	char *ofn=NULL;
	long func_size=0L;
	long ofile_size=0L;
	FuncStack *f;
	FuncArgList *arg;

#if DEBUG
	Debug("Running function [%s]\n",name);
#endif
	f = FindFunc(name, &func_size, NULL);
	if(!f) {
		Error("No function named %s",name);
		return;
	}
	if(cur_func) { /* Nested function call */
		FilePush(cur_func->name,gsize,-1);
	} else {
		ofn = estrdup(0,GetCurrentFilename());
		ofile_size = GetCurrentFileSize();
		FilePush(ofn,ofile_size,gfd);
	}
	cur_func = f;
	ParserInit(-1,func_size,no_httpd,f->buf);
	PushStackFrame();
	PushCondMatchMarks();
	arg = f->params;
	while(arg) {
		SetVar(arg->arg,0,0);
		arg = arg->prev;
	}	
	yyparse();
	if(ExitCalled) state=99;
}

/* Doubly-linked argument list */
void AddToArgList(unsigned char *arg) {
	FuncArgList *new;

	new = emalloc(0,sizeof(FuncArgList));
	new->next = funcarg_top;
	if(new->next) new->next->prev = new;
	else funcarg_bot = new;
	new->prev = NULL;
	new->arg = estrdup(0,arg);
	funcarg_top = new;
}

FuncArgList *GetFuncArgList(void) {
	return funcarg_top;
}

void ClearFuncArgList(void) {
	FuncArgList *top, *next;

	if(!funcarg_top) return;
	top = funcarg_top;
	next = top->next;
	while(next) {
		top = next;
		next = top->next;
	}
	funcarg_top=NULL;
	funcarg_bot=NULL;
}

void Return(void) {
	putback(0);
}
