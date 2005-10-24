/*
 *  StatusDialog.h
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/12/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
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
