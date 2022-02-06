/***[MySQL.c]*****************************************************[TAB=4]****\
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
#include <GOOMBAServer.h>
#include <stdlib.h>
int main(int argc, char *argv[]){
	if(argc < 6){
		printf("MySQL: Not enough arguments");
		return 0;
	}
	system("mysql --user %s --password %s --host %s --database %s --execute \"%s\"", argv[1], argv[2], argv[3], argv[4], argv[5]);
	return 0;
}
