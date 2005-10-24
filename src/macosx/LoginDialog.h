/*
 *  LoginDialog.h
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
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