#
#  Makefile.am
#
#  odbc-bench - a TPC-A and TPC-C like benchmark program for databases 
#  Copyright (C) 2000-2003 OpenLink Software <odbc-bench@openlinksw.com>
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

if GTK

SUBDIRS		= gtk

bin_PROGRAMS	= odbc-bench-cmd odbc-bench-gtk

INCLUDES	= \
	-I$(top_srcdir)/lib \
	@ODBC_CFLAGS@ \
	@GTK_CFLAGS@ \
	@XML_CFLAGS@

else

bin_PROGRAMS	= odbc-bench-cmd

INCLUDES	= \
	-I$(top_srcdir)/lib \
	@ODBC_CFLAGS@ \
	@XML_CFLAGS@
endif


odbc_bench_gtk_LDFLAGS  = -static
odbc_bench_gtk_LDADD	= \
	../lib/libutils.a \
	gtk/libodbcbench-gtk.a \
	@ODBC_LIBS@ \
	@GTK_LIBS@ \
	@XML_LIBS@ \
	@LIBADD_DL@

odbc_bench_gtk_SOURCES = \
	command_line.c \
	isql_interface.c \
	odbcs.c \
	olist.c \
	prefs.c \
	results.c \
	tests.c \
	threads.c \
	time.c \
	tpca_code.c \
	tpcc.c \
	tpcctrx.c

odbc_bench_cmd_LDFLAGS  = -static
odbc_bench_cmd_LDADD	= \
	../lib/libutils.a \
	@ODBC_LIBS@ \
	@XML_LIBS@ \
	@LIBADD_DL@

odbc_bench_cmd_SOURCES = \
	command_line.c \
	isql_interface.c \
	main_cmd.c \
	odbcs.c \
	olist.c \
	prefs.c \
	results.c \
	tests.c \
	threads.c \
	time.c \
	tpca_code.c \
	tpcc.c \
	tpcctrx.c


noinst_HEADERS	= \
	odbcbench.h \
	odbcinc.h \
	results.h \
	thr.h \
	timeacct.h \
	tpca.h \
	tpca_code.h \
	tpcc.h \
	tpccfun.h \
	olist.h
