/***[mysql.c]*****************************************************[TAB=4]****\
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
#include <mysql.h>
#include <GOOMBAServer.h>
#include <parse.h>
#include <stdio.h>
int MySqlQuery() {
  Stack *s;
  s = Pop();
	if(!s) {
		Error("Stack error in MySQL");
		return;
	}
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	FILE* file = fopen("credentials.cgi", "r"); 
  if(!file){
    	Push("Credentials file not found!",STRING);
  }
  char line[256]; 
  int i = 0; 
  char *server = "localhost";
	char *user = "root";
	char *password = ""; /* set me first */
	char *database = "default";
  while (fgets(line, sizeof(line), file)) { 
      i++; 
      if(i == 1 ) 
      { 
          server = line;
      } 
      if(i == 2 ) 
      { 
          user = line;
      } 
      if(i == 3 ) 
      { 
          password = line;
      } 
      if(i == 4 ) 
      { 
          database = line;
      } 
  } 
 
  fclose(file); 
	
	conn = mysql_init(NULL);
	
	/* Connect to database */
	if (!mysql_real_connect(conn, server, user, password, 
                                      database, 0, NULL, 0)) {
		Push(mysql_error(conn),STRING);
    return 1;
	}
	if (mysql_query(conn, s->strval)) {
		Push(mysql_error(conn),STRING);
    return 1;
	}
   
	res = mysql_use_result(conn);
   
	while ((row = mysql_fetch_row(res)) != NULL)
		Push(row[0],STRING);
   
	/* close connection */
	mysql_free_result(res);
	mysql_close(conn);
}
