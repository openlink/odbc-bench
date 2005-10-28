/*
 *  LoginDialog.cpp
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2005 OpenLink Software <odbc-bench@openlinksw.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "odbcbench.h"

#include "LoginDialog.h"
#include "util.h"

ControlID kLoginDSN = { 'LOGN', 0 };
ControlID kLoginUid = { 'LOGN', 1 };
ControlID kLoginPwd = { 'LOGN', 2 };

OPL_LoginDialog::OPL_LoginDialog(CFStringRef resname, CFStringRef title): 
  OPL_Dialog(resname)
{
	setTitle(title);
	setComboBoxTags(kLoginDSN, getDSNList());
}

CFArrayRef
OPL_LoginDialog::getDSNList()
{
	SQLCHAR szDSN[SQL_MAX_DSN_LENGTH + 1];
	SQLSMALLINT nDSN;
	CFMutableArrayRef dsnList;
	SQLRETURN retval;
	
	// create mutable array
	dsnList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	// append values
	for (retval = SQLDataSources(henv, SQL_FETCH_FIRST,
					szDSN, sizeof(szDSN), &nDSN, NULL, 0, NULL);
		 retval == SQL_SUCCESS;
		 retval = SQLDataSources(henv, SQL_FETCH_NEXT,
					szDSN, sizeof(szDSN), &nDSN, NULL, 0, NULL)) {
		szDSN[nDSN] = 0;
		CFArrayAppendValue(dsnList, OPL_char_to_CFString((char *) szDSN));
	}
	
	return dsnList;
}
