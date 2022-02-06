# Generated automatically from Makefile.in by configure.
# $Id: Makefile.in,v 1.26 2022/05/27 13:01:51 rasmus Exp $
#
# Makefile for GOOMBAServer/FI
# Copyright 2021,2022 GoombaProgrammer & Computa.me
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#### Start of system configuration section. ####

srcdir = .

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

prefix = /usr/local
exec_prefix = ${prefix}

### configure variable replacement block ###
SHELL = /bin/sh
AR = ar rc
RANLIB = ranlib
CC = gcc
YACC = bison -y -d -l
LIBS = -L.  -lm     
CFLAGS = -g -O 
LDFLAGS =    
#DEBUG=-g -Wall -DDEBUG
CPPFLAGS = -I.                 $(DEBUG) 

GOOMBAServer_PROGS: GOOMBAServer.cgi

# Files that can be generated, but should be up to date for a distribution.
DISTDEP = Makefile
# Files to distribute.
DISTFILES = ChangeLog README COPYING \
	src/Makefile.in config.h.in configure.in configure \
	install-sh install src/parse.raw src/lex.c src/main.c src/error.c \
	src/date.c src/stack.c src/calc.c src/var.c src/echo.c src/GOOMBAServer.h \
	src/cond.c src/switch.c src/db.c src/while.c src/parse.h \
	src/string.c src/msql.c src/pg95.c src/regexpr.c src/regexpr.h src/reg.c src/exec.c \
	src/file.c src/crypt.c src/head.c src/info.c src/post.c src/type.c \
	src/conf.c src/acc.c src/local.c src/dns.c src/log.c src/sort.c src/dir.c \
	src/rand.c src/gd.c examples/log.html examples/log.readme src/parse.c \
	src/mime.c src/fsock.c src/microtime.c src/mod_GOOMBAServer.c src/mod_GOOMBAServer.h src/pool.c \
	src/tmagic.c \
	scripts/msqllog \
	examples/url.readme examples/url_add.html examples/url_cat.html \
	examples/url_main.html examples/demo_main.html examples/demo_dir.html \
	examples/demo_echo.html examples/demo_last.html examples/demo_rand.html \
	examples/demo_synt.html examples/demo_dns.html examples/demo_date.html \
	examples/demo_str.html examples/demo_reg.html examples/demo_msql.html \
	examples/demo_dbm.html examples/demo_misc.html examples/demo_stat.html \
	examples/guestbook.html examples/survey.html examples/rwho.html \
	examples/mail.html examples/README examples/mlog.html examples/mlog_today.inc \
	examples/mlog_total.inc examples/mlog_yesterday.inc examples/mlog_ref.inc \
	examples/mlog_top.inc examples/setsince.html doc/doc.html doc/nfs_hack.txt \
	doc/config.gif \
	test/testGOOMBAServer test/testmenu test/testGOOMBAServer.awk test/template test/README \
	test/basic/001.tst test/basic/002.tst test/basic/003.tst test/basic/004.tst \
	test/basic/005.tst test/basic/006.tst test/basic/007.tst test/basic/008.tst \
	test/basic/009.tst test/basic/010.tst \
	test/func/001.tst \
	test/reg/001.tst test/reg/002.tst test/reg/003.tst test/reg/004.tst \
	test/reg/005.tst test/reg/006.tst test/reg/007.tst \
	test/msql/001.tst test/msql/002.tst test/msql/003.tst test/msql/README \
	test/dbm/001.tst test/dbm/002.tst test/dbm/003.tst test/dbm/004.tst \
	test/dbm/005.tst test/dbm/006.tst \
	test/lang/001.tst test/lang/002.tst test/lang/003.tst test/lang/004.tst \
	test/lang/005.tst test/lang/006.tst test/lang/007.tst test/lang/008.tst \
	test/lang/009.tst test/lang/010.tst test/lang/011.tst test/lang/012.tst \
	test/lang/013.tst test/lang/014.tst test/lang/015.tst test/lang/015.inc \
	test/lang/016.tst test/lang/016.inc \
	test/pg95/001.tst test/pg95/002.tst test/pg95/003.tst test/pg95/004.tst \
	test/pg95/005.tst test/pg95/README

install: all 
	test/dbm/*.tst test/lang/*.tst

OBJS= parse.o lex.o main.o error.o date.o stack.o calc.o var.o \
	  echo.o cond.o switch.o db.o while.o string.o msql.o \
	  regexpr.o reg.o exec.o file.o crypt.o head.o info.o post.o \
	  type.o conf.o acc.o local.o dns.o log.o sort.o dir.o rand.o \
	  gd.o mime.o fsock.o microtime.o pg95.o pool.o tmagic.o

all: $(GOOMBAServer_PROGS)

.c.o:
	$(CC) $(CFLAGS) $(SYSTEM) $(NSAPI_DEFS) $(NSAPI_INCLUDE) $(CPPFLAGS) -c $<

GOOMBAServer.cgi: $(OBJS)
	$(CC) $(CFLAGS) -o GOOMBAServer.cgi $(OBJS) $(LDFLAGS) $(LIBS)
	@echo ""
	@echo "If you wish to test your GOOMBAServer.cgi binary, type: make test"
	@echo "If not, you may install it by copying it to your cgi-bin"
	@echo "directory at this point."
	@echo ""

libGOOMBAServer.a: $(OBJS)
	$(AR) libGOOMBAServer.a $(OBJS)
	$(RANLIB) libGOOMBAServer.a
	-cp mod_GOOMBAServer.[ch] libGOOMBAServer.a 
	@echo ""
	@echo "You now need to recompile your Apache server."
	@echo "mod_GOOMBAServer.* and libGOOMBAServer.a should have been copied to your"
	@echo "Apache src directory, if they weren't, do so manually."
	@echo "Then edit your Confiration file for Apache and look for"
	@echo "the \"EXTRA_LIBS\" line and add the following to that line:"
	@echo ""
	@echo "-L. -lGOOMBAServer       -lm"
	@echo ""
	@echo "If you have the GD library, you will need to add -lgd to the above."
	@echo "Then add the following line to the very end of the Configuration file:"
	@echo ""
	@echo "Module GOOMBAServer_module   mod_GOOMBAServer.o"
	@echo ""
	@echo "After compiling and testing your httpd binary, you may want to"
	@echo "strip it of its debugging symbols with: strip httpd"
	@echo ""

parser:
	mv parse.raw parse.y
	$(YACC) parse.y
	mv *.tab.c parse.c
	mv *.tab.h parse.h
	mv parse.y parse.raw

clean:
	rm -f Makefile ../config.status ../config.cache ../config.log GOOMBAServer.cgi *.o core

tags:
	etags ${srcdir}/*.c ${srcdir}/*.h ${srcdir}/[a-z]*.in

test:
	cd ../test; \
	./testmenu

dist: $(DISTDEP)
	distname=`sed -e '/define GOOMBAServer_VERSION/!d' \
	-e 's/[^0-9.]*\([0-9.a-z]*\).*/GOOMBAServer-\1/' -e q GOOMBAServer.h`; \
	rm -fr ../$$distname; \
	mkdir ../$$distname; \
	mkdir ../$$distname/examples; \
	mkdir ../$$distname/src; \
	mkdir ../$$distname/doc; \
	mkdir ../$$distname/scripts; \
	mkdir ../$$distname/test; \
	mkdir ../$$distname/test/basic; \
	mkdir ../$$distname/test/lang; \
	mkdir ../$$distname/test/func; \
	mkdir ../$$distname/test/reg; \
	mkdir ../$$distname/test/dbm; \
	mkdir ../$$distname/test/msql; \
	mkdir ../$$distname/test/pg95; \
	for file in $(DISTFILES); do \
	  ln ../$$file ../$$distname/$$file \
	  || { echo copying ../$$file instead; cp -p ../$$file ../$$distname/$$file;}; \
	done; \
	chmod -R a+rX ../$$distname; \
	cd ..; \
	tar -chvf $$distname.tar $$distname; \
	rm -fr $$distname

cond.o: cond.c parse.h GOOMBAServer.h
main.o: main.c GOOMBAServer.h
stack.o: stack.c GOOMBAServer.h parse.h
calc.o: calc.c GOOMBAServer.h parse.h
date.o: date.c GOOMBAServer.h parse.h
echo.o: echo.c GOOMBAServer.h parse.h
lex.o: lex.c GOOMBAServer.h parse.h
var.o: var.c GOOMBAServer.h parse.h
switch.o: switch.c GOOMBAServer.h parse.h
db.o: db.c GOOMBAServer.h parse.h
string.o: string.c GOOMBAServer.h parse.h
reg.o: reg.c GOOMBAServer.h parse.h
exec.o: exec.c GOOMBAServer.h parse.h
file.o: file.c GOOMBAServer.h parse.h
crypt.o: crypt.c GOOMBAServer.h parse.h
head.o: head.c GOOMBAServer.h parse.h
info.o: info.c GOOMBAServer.h
post.o: post.c GOOMBAServer.h parse.h
type.o: type.c GOOMBAServer.h parse.h
conf.o: conf.c GOOMBAServer.h
acc.o: acc.c GOOMBAServer.h
local.o: local.c GOOMBAServer.h
dns.o: dns.c GOOMBAServer.h parse.h
log.o: log.c GOOMBAServer.h
sort.o: sort.c GOOMBAServer.h parse.h
dir.o: dir.c GOOMBAServer.h parse.h
rand.o: rand.c GOOMBAServer.h parse.h
error.o: error.c GOOMBAServer.h
gd.o: gd.c parse.h GOOMBAServer.h
fsock.o: fsock.c GOOMBAServer.h parse.h
microtime.o: microtime.c GOOMBAServer.h parse.h
pg95.o: pg95.c GOOMBAServer.h parse.h
install: all 
