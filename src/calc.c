/***[calc.c]******************************************************[TAB=4]****\
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
#include <GOOMBAServer.h>
#include <parse.h>
#include <math.h>
#include <ctype.h>

int Calc(int op) {
	Stack *s, a, b;
	char temp[1024];
	static char *atemp=NULL;
	int l;
	long intval;
	double douval=0.0;

	a.strval = NULL;
	b.strval = NULL;
	s = Pop();
	if(!s) {
		Error("Stack Error");
		return(0);
	}
	b.type = s->type;
	b.intval = s->intval;
	b.douval = s->douval;
	if(s->strval) b.strval = estrdup(0,s->strval);

	s = Pop();
	if(!s) {
		Error("Stack Error");
		return(0);
	}
	a.type = s->type;
	a.intval = s->intval;
	a.douval = s->douval;
	if(s->strval) a.strval = estrdup(0,s->strval);

	switch(a.type) {
		case LNUMBER:
			switch(op) {
			case '+':
				intval = a.intval + b.intval;
				sprintf(temp,"%ld",intval);
				Push(temp,LNUMBER);
				break;
			case '-':
				intval = a.intval - b.intval;
				sprintf(temp,"%ld",intval);
				Push(temp,LNUMBER);
				break;
			case '*':
				intval = a.intval * b.intval;
				sprintf(temp,"%ld",intval);
				Push(temp,LNUMBER);
				break;
			case '/':
				if(b.intval != 0) {
					intval = a.intval / b.intval;
					sprintf(temp,"%ld",intval);
					Push(temp,LNUMBER);
				} else {
					Push("undefined",STRING);
				}
				break;
			case '%':
				intval = a.intval % b.intval;
				sprintf(temp,"%ld",intval);
				Push(temp,LNUMBER);
				break;
			case '&':
				intval = a.intval & b.intval;
				sprintf(temp,"%ld",intval);
				Push(temp,LNUMBER);
				break;
			case '|':
				intval = a.intval | b.intval;
				sprintf(temp,"%ld",intval);
				Push(temp,LNUMBER);
				break;
			case '^':
				intval = a.intval ^ b.intval;
				sprintf(temp,"%ld",intval);
				Push(temp,LNUMBER);
				break;
			}
			break;
		case DNUMBER:
			switch(op) {
			case '+':
				douval = a.douval + b.douval;
				sprintf(temp,"%.10f",douval);
				Push(temp,DNUMBER);
				break;
			case '-':
				douval = a.douval - b.douval;
				sprintf(temp,"%.10f",douval);
				Push(temp,DNUMBER);
				break;
			case '*':
				douval = a.douval * b.douval;
				sprintf(temp,"%.10f",douval);
				Push(temp,DNUMBER);
				break;
			case '/':
				if(b.douval != 0) {
					douval = a.douval / b.douval;
					sprintf(temp,"%.10f",douval);
					Push(temp,DNUMBER);
				} else {
					Push("undefined",STRING);
				}
				break;
			case '%':
				douval = (double)((int)a.douval % (int)b.douval);
				sprintf(temp,"%ld",(long)douval);
				Push(temp,DNUMBER);
				break;
			case '&':
				douval = (double)((int)a.douval & (int)b.douval);
				sprintf(temp,"%ld",(long)douval);
				Push(temp,DNUMBER);
				break;
			case '|':
				douval = (double)((int)a.douval | (int)b.douval);
				sprintf(temp,"%ld",(long)douval);
				Push(temp,DNUMBER);
				break;
			case '^':
				douval = (double)((int)a.douval ^ (int)b.douval);
				sprintf(temp,"%ld",(long)douval);
				Push(temp,DNUMBER);
				break;
			}
			break;
		case STRING:
			switch(op) {
			case '+': 
				if((l=strlen(a.strval)+strlen(b.strval)) > 1023) {
					atemp = emalloc(0,l+1);
					sprintf(atemp,"%s%s",a.strval,b.strval);
					Push(atemp,STRING);
				} else {
					sprintf(temp,"%s%s",a.strval,b.strval);
					Push(temp,STRING);
				}
				break;
			case '-':
				if(strlen(b.strval) && strlen(a.strval)) {
					Push(b.strval,STRING);
					Push("",STRING);
					Push(a.strval,STRING);
					RegReplace();
				} else if(strlen(b.strval)) Push(b.strval,STRING);
				else Push(a.strval,STRING);
				break;
			case '*':
				/* Multiplying strings? */
				break;
			case '/':
				/* Dividing strings? */
				break;
			case '%':
				/* Modding strings? */
				break;
			}
			break;
	}
	return(0);
}			

int CalcInc(int op) {
	Stack *s;
	char temp[1024];

	s = Pop();
	if(!s) {
		Error("Stack error in expression");
		return(0);
	}
	switch(s->type) {
		case LNUMBER:
		case STRING:
			switch(op) {
				case INC:
					s->intval++;
					sprintf(temp,"%ld",s->intval);		
					if(!s->var) Push(temp,LNUMBER);
					else {
						if(s->var->count > 1) {
							Push(s->var->iname,STRING);
							Push(temp,LNUMBER);
							SetVar(s->var->name,2,0);
						} else {	
							Push(temp,LNUMBER);
							SetVar(s->var->name,0,0);
						}
					}
					break;
				case DEC:
					s->intval--;
					sprintf(temp,"%ld",s->intval);
					if(!s->var) Push(temp,LNUMBER);
					else {
						if(s->var->count > 1) {
							Push(s->var->iname,STRING);
							Push(temp,LNUMBER);
							SetVar(s->var->name,2,0);
						} else {	
							Push(temp,LNUMBER);
							SetVar(s->var->name,0,0);
						}
					}
					break;
			}
			break;
		case DNUMBER:
			switch(op) {
				case INC:
					s->douval++;
					sprintf(temp,"%.10f",s->douval);
					if(!s->var) Push(temp,DNUMBER);
					else {
						if(s->var->count > 1) {
							Push(s->var->iname,STRING);
							Push(temp,DNUMBER);
							SetVar(s->var->name,2,0);
						} else {	
							Push(temp,DNUMBER);
							SetVar(s->var->name,0,0);
						}
					}
					break;	
				case DEC:
					s->douval--;
					sprintf(temp,"%.10f",s->douval);
					if(!s->var) Push(temp,DNUMBER);
					else {
						if(s->var->count > 1) {
							Push(s->var->iname,STRING);
							Push(temp,DNUMBER);
							SetVar(s->var->name,2,0);
						} else {	
							Push(temp,DNUMBER);
							SetVar(s->var->name,0,0);
						}
					}
					break;	
			}
			break;
	}
	return(0);
}

void Neg(void) {
	Stack *s;
	char temp[128];
 
	s = Pop();
	if(!s) {
		Error("Stack Error");
		return;
	}
	switch(s->type) {
		case LNUMBER:
			sprintf(temp,"%ld",-s->intval);
			break;
		case DNUMBER:
			sprintf(temp,"%.10f",-s->douval);
			break;
		case STRING:
			sprintf(temp,"%d",-(int)strlen(s->strval));
			break;
	}
	if(!s->var) Push(temp,LNUMBER);
	else {
		if(s->var->count > 1) {
			Push(s->var->iname,STRING);
			Push(temp,LNUMBER);
			SetVar(s->var->name,2,0);
		} else {	
			Push(temp,LNUMBER);
			SetVar(s->var->name,0,0);
		}
	}
}

/* Binary to Decimal conversion */
void BinDec(void) {
	Stack *s;
	char temp[32];
	unsigned long num=0, val=1;
	int i;

	s = Pop();
	if(!s) {
		Error("Stack error in bindec");
		return;
	}
	i = strlen(s->strval)-1;
	while(i>-1) {
		if((s->strval)[i--]=='1') {
			num+=val;
		}
		val *= 2;
	}
	sprintf(temp,"%ld",num);
	Push(temp,LNUMBER);
}

/* Decimal to Binary Conversion */
void DecBin(void) {
	Stack *s;
	char temp[48];
	long num, exp;
	int i=0;

	s = Pop();
	if(!s) {
		Error("Stack error in decbin");
		return;
	}
	num = s->intval;
	exp = log(num)/log(2);
	temp[0] = '1';
	temp[1] = '\0';
	i = 1;
	num -= pow(2,exp--);
	while(exp>-1) {
		if(num >= pow(2,exp)) {
			temp[i]='1';
			num -= pow(2,exp);
		} else temp[i]='0';
		temp[++i] = '\0';
		exp--;
	}
	Push(temp,STRING);
}

/* Decimal to Hexadecimal */
void DecHex(void) {
	Stack *s;
	char hex[17] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',0 };
	long num,exp,val;
	char temp[32];
	int i=0;

	s = Pop();
	if(!s) {
		Error("Stack error in dechex");
		return;
	}
	num = s->intval;
	exp = log(num)/log(16);
	while(exp>-1) {
		val = num/pow(16,exp);
		temp[i++] = hex[val];
		temp[i]='\0';
		if(val) num -= val*pow(16,exp);
		exp--;
	}
	Push(temp,STRING);
}	

/* Hexadecimal to Decimal */
void HexDec(void) {
	Stack *s;
	long mult=1, num=0;
	int i,c;
	char temp[32];

	s = Pop();
	if(!s) {
		Error("Stack error in hexdec");
		return;
	}
	i = strlen(s->strval)-1;
	while(i>-1) {
		c = toupper((s->strval)[i--]);
		if(c<'0' || c > 'F') continue;
		if(c <= '9') num+=mult*(c-'0');
		else num+=mult*(c-'A'+10);
		mult *= 16;
	}
	sprintf(temp,"%ld",num);
	Push(temp,LNUMBER);
}

/* Octal to Decimal */
void OctDec(void) {
	Stack *s;
	long mult=1, num=0;
	int i,c;
	char temp[32];

	s = Pop();
	if(!s) {
		Error("Stack error in octdec");
		return;
	}
	i = strlen(s->strval)-1;
	while(i>-1) {
		c = (s->strval)[i--];
		if(c<'0' || c > '7') continue;
		num+=mult*(c-'0');
		mult *= 8;
	}
	sprintf(temp,"%ld",num);
	Push(temp,LNUMBER);
}

/* Decimal to Octal */
void DecOct(void) {
	Stack *s;
	long num,exp,val;
	char temp[32];
	int i=0;

	s = Pop();
	if(!s) {
		Error("Stack error in decoct");
		return;
	}
	num = s->intval;
	exp = log(num)/log(8);
	while(exp>-1) {
		val = num/pow(8,exp);
		temp[i++] = '0'+val;
		temp[i]='\0';
		if(val) num -= val*pow(8,exp);
		exp--;
	}
	Push(temp,STRING);
}	

void Abs(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in abs");
		return;
	}
	if(s->type==DNUMBER) sprintf(temp,"%f",fabs(s->douval));	
	else sprintf(temp,"%ld",labs(s->intval));
	Push(temp,s->type);
}

void Exp(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in exp");
		return;
	}
	sprintf(temp,"%f",exp(s->douval));	
	Push(temp,DNUMBER);
}

void mathLog(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in log");
		return;
	}
	sprintf(temp,"%f",log(s->douval));	
	Push(temp,DNUMBER);
}

void mathLog10(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in log10");
		return;
	}
	sprintf(temp,"%f",log10(s->douval));	
	Push(temp,DNUMBER);
}

void Sin(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in sin");
		return;
	}
	sprintf(temp,"%f",sin(s->douval));	
	Push(temp,DNUMBER);
}

void Cos(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in cos");
		return;
	}
	sprintf(temp,"%f",cos(s->douval));	
	Push(temp,DNUMBER);
}

void Tan(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in tan");
		return;
	}
	sprintf(temp,"%f",tan(s->douval));	
	Push(temp,DNUMBER);
}

void Sqrt(void) {
	Stack *s;
	char temp[64];

	s = Pop();
	if(!s) {
		Error("Stack error in sqrt");
		return;
	}
	/* Square root of a negative number will result in 0 */
	if(s->douval < 0) {
		Push("0",DNUMBER);
		return;
	}
	sprintf(temp,"%f",sqrt(s->douval));	
	Push(temp,DNUMBER);
}
