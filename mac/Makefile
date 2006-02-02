#
#  Makefile
#
#  $Id$
#
#  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
#  Copyright (C) 2000-2005 OpenLink Software <odbc-bench@openlinksw.com>
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
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  Makefile.mac
#


#
#  Determine build model (Deployment/Development)
#
ifeq (,$(MODEL))
MODEL	:= Deployment
endif

#
#  Determine which version of Mac OS X we are running
#
macosx_version	:= $(shell sw_vers -productVersion)

#
#  Mac OS X 10.2
#
ifeq (10.2, $(findstring 10.2, $(macosx_version)))
    PROJBUILD := /usr/bin/pbxbuild -buildstyle $(MODEL)
    BUILDDIR  := build
endif

#
#  Mac OS X 10.3
#
ifeq (10.3, $(findstring 10.3, $(macosx_version)))
    PROJBUILD := /usr/bin/xcodebuild -buildstyle $(MODEL)
    BUILDDIR  := build
endif

#
#  Mac OS X 10.4
#
ifeq (10.4, $(findstring 10.4, $(macosx_version)))
    PROJBUILD := /usr/bin/xcodebuild -configuration $(MODEL)
    BUILDDIR  := build/$(MODEL)
endif



all: compile

compile:
	$(PROJBUILD) build

install: compile
	sudo $(PROJBUILD) install DSTROOT=/

clean:
	$(PROJBUILD) clean

realclean: clean
	rm -rf build

installer: clean compile
	sudo ./gen-installer.sh
