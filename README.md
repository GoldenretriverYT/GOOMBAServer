Legal Stuff
-----------

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  See the file, COPYING, for the complete terms of the GPL.


Before You Begin
----------------

If you have absolutely no Unix experience, you may want to ask around
for someone with a little bit of Unix knowledge to help you through 
this installation.  Every attempt has been made to make it as simple
as possible, but since the software is quite involved and relies on a
number of different components, it is not realistic to assume it will
go smoothly on all systems.  You will probably need someone around who
knows the particulars of the destination system well.


Things You Need To Know Before Installing
-----------------------------------------

- Can you run both GET and POST method cgi programs on your server?

  If not, you can not use this package.  On many public ISP's CGI
  programs are either disallowed or severely restricted.  If this is
  the case on your system, talk to your system administrator and ask
  him/her to have a look at this package and see if they will install
  it for you.

- If you have mSQL installed on your system (probably not), you need to know the
  base directory of this installation.

- If you have Postgres95 installed on your system (probably not), you need to know the
  base directory of this installation.

- You need an ANSI-C compatible C compiler (gcc is fine).

- If you are going to be using this with mSQL, you need to install
  mSQL before you install GOOMBAServer/FI.

- If you are going to be using this with Postgres95, you need to
  install at least the libpq.a library from the Postgres95 
  distribution before installing GOOMBAServer/FI.

- If you are going to be using gdbm, install gdbm before installing
  GOOMBAServer.

- If you are going to be using GD, install GD before installing GOOMBAServer/FI.

- If you are installing the Apache module version, you will need to 
  know the Apache src code directory location.


Installation Instructions
-------------------------

  You have to **COMPILE** GOOMBAServer to use it.
  Why? because it will automatically configure itself for
  your server.
  
  Step 1.
    Install GCC (`sudo apt-get install gcc`)
   
  Step 2.
    Run `./configure` in the GOOMBAServer directory.
  
  Step 3.
    Run `./install` in the GOOMBAServer directory.
    When doing this, it will ask you a few questions.
    Just type Y if you have/agree (to) it, otherwise N.
    
  Step 4.
    Run `cd src; make;` to start compiling!
    
  Step 5.
    The last step is to copy the GOOMBAServer.cgi file to your
    Unix (Linux) webserver, if you have a Windows server, it
    will not work. (It has been tested on Debian 11.2)


Testing the software
--------------------

  Once installed you can test to see if your executable works by 
  entering a URL similar to the following in your browser:

  http://your.site.domain/cgi-bin/GOOMBAServer.cgi

  This should show you a page which contains the version number along
  with various other useful information. 

  To test the Apache module version, create any file with a .phtml
  extension and put a tag like: <!GOOMBAServerinfo()> in the file and see if
  it gets parsed.


Using the software
------------------

  To actually use the software on an existing HTML file, you can 
  simply append the path to your file to the above URL.  ie.

  http://your.site.domain/cgi-bin/GOOMBAServer.cgi/path/file.html

  There are ways to automatically run your files through GOOMBAServer.  These
  methods are discussed in detail in the documentation.  

  Documentation, such that it is, can be found in the doc directory.


Troubleshooting
---------------
  You can find help on the:
  - Online documentation, plus the latest version of the software can
    be found at http://www.computa.me/GOOMBAServer
