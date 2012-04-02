#
#  Makefile.am
# 
#  odbc-bench - a TPC-A and TPC-C like benchmark program for databases 
#  Copyright (C) 2000-2012 OpenLink Software <odbc-bench@openlinksw.com>
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

noinst_LIBRARIES		= libodbcbench-gtk.a

INCLUDES	= \
	-I$(top_srcdir)/src \
	@ODBC_CFLAGS@ \
	@GTK_CFLAGS@ \
	@XML_CFLAGS@

libodbcbench_gtk_a_SOURCES	= \
	ArrayParams.c \
	dialog.c \
	LoginBox.c \
	main.c \
	results_gtk.c \
	ServerDSN.c \
	status.c \
	testpool.c \
	ThreadOptions.c \
	TPCARunProps.c \
	TPCATableProps.c \
	TPCCTableProps.c \
	util.c


noinst_HEADERS	= \
	ArrayParams.h \
	LoginBox.h \
	ServerDSN.h \
	TPCARunProps.h \
	TPCATableProps.h \
	TPCCTableProps.h \
	ThreadOptions.h \
	odbcbench_gtk.h \
	results_gtk.h \
	testpool.h \
	util.h
