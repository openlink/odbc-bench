/*
 *  LoginDialog.cpp
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
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
