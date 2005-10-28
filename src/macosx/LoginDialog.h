/*
 *  LoginDialog.h
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

#ifndef _MACOSX_LOGINDIALOG_H_
#define _MACOSX_LOGINDIALOG_H_

#include "Dialog.h"

extern ControlID kLoginDSN;
extern ControlID kLoginUid;
extern ControlID kLoginPwd;

class OPL_LoginDialog: public OPL_Dialog {
public:
	OPL_LoginDialog(CFStringRef name, CFStringRef title);
	
private:
	CFArrayRef getDSNList();
};

#endif /* _MACOSX_LOGINDIALOG_H_ */
