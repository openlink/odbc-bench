/*
 *  Dialog.h
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MACOSX_DIALOG_H_
#define _MACOSX_DIALOG_H_

#include <Carbon/Carbon.h>

extern ControlID kLoginDSN;
extern ControlID kLoginUid;
extern ControlID kLoginPwd;

// Simple ODBC login dialog
class OPL_Dialog {
public:
	OPL_Dialog(CFStringRef resname);
	
	virtual ~OPL_Dialog()
	{
		if (m_window)
			ReleaseWindow(m_window);
	}
	
	void setTitle(CFStringRef title)
	{
		SetWindowTitleWithCFString(getWindow(), title);
	}

	WindowRef getWindow()
	{
		return m_window;
	}

	void getControl(const ControlID &controlID, ControlRef *control)
	{
		GetControlByID(getWindow(), &controlID, control);
	}
	
	void enableControl(const ControlID &controlID)
	{
		ControlRef control;

		getControl(controlID, &control);
		EnableControl(control);
	}

	void disableControl(const ControlID &controlID)
	{
		ControlRef control;
		
		getControl(controlID, &control);
		DisableControl(control);
	}
	
	SInt32 get32BitValue(const ControlID &controlID)
	{
		ControlRef control;
		
		getControl(controlID, &control);
		return GetControl32BitValue(control);
	}
	
	void set32BitValue(const ControlID &controlID, SInt32 v)
	{
		ControlRef control;
		
		getControl(controlID, &control);
		SetControl32BitValue(control, v);
	}
	
	CFStringRef getEditText(const ControlID &controlID)
	{
		Size len;
		CFStringRef str;
		ControlRef control;
		
		getControl(controlID, &control);
		GetControlData(control, kControlEditTextPart,
			kControlEditTextCFStringTag, sizeof(str), &str, &len);
		return str;
	}

	SInt32 getEditTextIntValue(const ControlID &controlID)
	{
		return CFStringGetIntValue(getEditText(controlID));
	}

	// Set static text
	//
	// takes ownership of str
	void setStaticText(const ControlID &controlID, CFStringRef str)
	{
		ControlRef control;
		
		getControl(controlID, &control);
		SetControlData(control, kControlEntireControl,
			kControlStaticTextCFStringTag, sizeof(str), &str);
		CFRelease(str);
	}
	
	// Set string value
	//
	// takes ownership of str
	void setEditText(const ControlID &controlID, CFStringRef str)
	{
		ControlRef control;
		
		getControl(controlID, &control);
		SetControlData(control, kControlEditTextPart,
			kControlEditTextCFStringTag, sizeof(str), &str);
		CFRelease(str);
	}
	
	CFStringRef getPasswordEditText(const ControlID &controlID)
	{
		Size len;
		CFStringRef str;
		ControlRef control;
		
		getControl(controlID, &control);
		GetControlData(control, kControlNoPart,
			kControlEditTextPasswordCFStringTag, sizeof(str), &str, &len);
		return str;
	}

	// Set password string value
	//
	// takes ownership of str
	void setPasswordEditText(const ControlID &controlID, CFStringRef str)
	{
		ControlRef control;
		
		getControl(controlID, &control);
		SetControlData(control, kControlNoPart,
			kControlEditTextPasswordCFStringTag, sizeof(str), &str);
		CFRelease(str);
	}

	// Set combo box tags
	//
	// takes ownership of array
	void setComboBoxTags(const ControlID &controlID, CFArrayRef list)
	{
		ControlRef control;

		// set tags
		getControl(controlID, &control);
		SetControlData(control,
			kHIComboBoxDisclosurePart, kHIComboBoxListTag,
			sizeof(list), &list);
		CFRelease(list);
	}

	// run dialog
	bool run()
	{
		OSStatus err;
		
		if (!getWindow())
			return false;

		// show and run dialog
		ShowWindow(getWindow());
		err = RunAppModalLoopForWindow(getWindow());
		require_noerr(err, error);

		return getStatus();
		
	error:
		return false;
	}

	virtual void endModal()
	{
		QuitAppModalLoopForWindow(getWindow());
	}
	
	bool getStatus()
	{
		return m_status;
	}
	
protected:
	bool m_status;

	// Handle command
	virtual OSStatus handleCommandEvent(UInt32 /* commandID */)
	{
		return eventNotHandledErr;
	}
	
private:
	WindowRef m_window;
	
	static pascal OSStatus
	eventHandler(EventHandlerCallRef handlerRef, EventRef eventRef, void *userData);
};

#endif /* _MACOSX_DIALOG_H_ */