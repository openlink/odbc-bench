#
#  Makefile.am
# 
#  odbc-bench - a TPCA and TPCC benchmark program for databases 
#  Copyright (C) 2000-2002 OpenLink Software <odbc-bench@openlinksw.com>
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

bin_PROGRAMS	= odbc-bench

INCLUDES	= \
	-I$(top_srcdir)/odbc-bench \
	@ODBC_CFLAGS@ \
	@GTK_CFLAGS@ \
	@XML_CFLAGS@

odbc_bench_LDADD	= \
	@ODBC_LIBS@ \
	@GTK_LIBS@ \
	@XML_LIBS@

odbc_bench_SOURCES = \
	LoginBox.c \
	ServerDSN.c \
	TPCARunProps.c \
	TPCATableProps.c \
	TPCCTableProps.c \
	ThreadOptions.c \
	command_line.c \
	dialog.c \
	isql_interface.c \
	main.c \
	odbcs.c \
	prefs.c \
	results.c \
	status.c \
	testpool.c \
	tests.c \
	threads.c \
	time.c \
	tpca_code.c \
	tpcc.c \
	tpcctrx.c


noinst_HEADERS	= \
	LoginBox.h \
	ServerDSN.h \
	TPCARunProps.h \
	TPCATableProps.h \
	TPCCTableProps.h \
	ThreadOptions.h \
	odbcbench.h \
	odbcinc.h \
	results.h \
	testpool.h \
	thr.h \
	timeacct.h \
	tpca.h \
	tpca_code.h \
	tpcc.h \
	tpccfun.h
