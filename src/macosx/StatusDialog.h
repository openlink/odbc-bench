/*
 *  StatusDialog.h
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2019 OpenLink Software <odbc-bench@openlinksw.com>
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

#ifndef _MACOSX_STATUSDIALOG_H_
#define _MACOSX_STATUSDIALOG_H_

#include "Dialog.h"

// Base Status Dialog
class OPL_StatusDialog: public OPL_Dialog {
public:
	OPL_StatusDialog(CFStringRef name);
	virtual ~OPL_StatusDialog()
	{
		if (m_progressText != NULL)
			CFRelease(m_progressText);
	}
	
	// Begin dialog execution
	virtual bool begin(CFStringRef title, float nMax);

	// End dialog execution
	void end()
	{
		// leave modal state
		EndAppModalStateForWindow(getWindow());
	}
	
	// Set work item
	//
	// takes ownership of text
	void setWorkItem(CFStringRef text);

	// Set progress text
	//
	// takes ownership of progressText
	virtual void setProgressText(CFStringRef progressText,
		int nConn, int threadNo,  float nValue, int nTrnPerCall);

	// Update
	virtual void update();
	
	// Mark work item as finished
	virtual void markFinished(int nConn, int threadNo) {}

	// Run Carbon event loop
	void runEventLoop();

	float getMaxValue()
	{
		return m_maxValue;
	}
	
protected:
	virtual void endModal()
	{
		// status is checked in runEventLoop() -- nothing to do here
	}
	
private:
	CFStringRef m_progressText;
	float m_maxValue;
	long m_lastUpdated;
};

// Simple Status Dialog
class OPL_SimpleStatusDialog: public OPL_StatusDialog {
public:
	OPL_SimpleStatusDialog():
		OPL_StatusDialog(CFSTR("SimpleStatusDialog")),
		m_value(0)
	{
		// nothing to do
	}
	
	virtual void setProgressText(CFStringRef progressText,
		int nConn, int threadNo,  float nValue, int nTrnPerCall);

	virtual void update();
	
private:
	float m_value;
};

class OPL_TestStatus;

class OPL_TestRunStatusDialog: public OPL_StatusDialog {
public:
	OPL_TestRunStatusDialog():
		OPL_StatusDialog(CFSTR("TestRunStatusDialog")),
		m_nTests(0), m_testStates(NULL)
	{
		// nothing to do
	}

	~OPL_TestRunStatusDialog();

	virtual bool begin(CFStringRef title, float nMax);

	virtual void setProgressText(CFStringRef progressText,
		int nConn, int threadNo,  float nValue, int nTrnPerCall);

	virtual void update();
	
	virtual void markFinished(int nConn, int threadNo);
	
	void resizeY(float deltaY);
	
private:
	UInt32 m_nTests;
	OPL_TestStatus *m_testStates;
};

#endif /* _MACOSX_STATUSDIALOG_H_ */
