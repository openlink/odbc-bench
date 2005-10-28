/*
 *  StatusDialog.cpp
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

#include <memory>
#include "StatusDialog.h"
#include "TestPoolItemList.h"
#include "util.h"

static ControlID kStatusWorkItem = { 'STAT', 0 };
static ControlID kStatusText = { 'STAT', 1 };
static ControlID kStatusTestName = { 'STAT', 2 };
static ControlID kStatusProgressBar = { 'STAT', 3 };
static ControlID kStatusProgressText = { 'STAT', 4 };
static ControlID kStatusProgressGroup = { 'STAT', 5 };
static ControlID kStatusCancelButton = { 'STAT', 6 };

//======================== Base ============================

OPL_StatusDialog::OPL_StatusDialog(CFStringRef name):
	OPL_Dialog(name), m_progressText(NULL),  m_maxValue(0),
	m_lastUpdated(get_msec_count())
{
	// nothing to do
}

// Begin dialog execution
bool
OPL_StatusDialog::begin(CFStringRef title, float nMax)
{
	OSStatus err;
	
	setTitle(title);
	setStaticText(kStatusWorkItem, CFStringCreateCopy(NULL, CFSTR("")));
	setStaticText(kStatusText, CFStringCreateCopy(NULL, CFSTR("")));
	m_maxValue = nMax;

	// show window and enter modal state
	ShowWindow(getWindow());
	err = BeginAppModalStateForWindow(getWindow());
	require_noerr(err, error);

	return true;
	
error:
	return false;
}

// Set work item
//
// takes ownership of text
void
OPL_StatusDialog::setWorkItem(CFStringRef text)
{
	setStaticText(kStatusWorkItem, text);
}

// Set progress text
//
// takes ownership of progressText
void
OPL_StatusDialog::setProgressText(CFStringRef progressText,
	int nConn, int threadNo,  float nValue, int nTrnPerCall)
{
	if (m_progressText != NULL)
		CFRelease(m_progressText);
	m_progressText = progressText;
	
	long time_now = get_msec_count();
	if (time_now - m_lastUpdated > bench_get_long_pref(DISPLAY_REFRESH_RATE)) {
		update();
		m_lastUpdated = time_now;
	}
}

// Update
void
OPL_StatusDialog::update()
{
	setStaticText(kStatusText, m_progressText);
	m_progressText = NULL;
}

// Run Carbon event loop
void
OPL_StatusDialog::runEventLoop()
{
	EventRef event;
	EventTargetRef target;

	target = GetEventDispatcherTarget();
	while (ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &event) == noErr) {
		SendEventToEventTarget(event, target);
		ReleaseEvent(event);
	}
}

//======================== Simple ============================

void
OPL_SimpleStatusDialog::setProgressText(CFStringRef progressText,
	int nConn, int threadNo,  float nValue, int nTrnPerCall)
{
	m_value = nValue;

	OPL_StatusDialog::setProgressText(
	    progressText, nConn, threadNo, nValue, nTrnPerCall);
}

void
OPL_SimpleStatusDialog::update()
{
	OPL_StatusDialog::update();
	
	set32BitValue(kStatusProgressBar, (SInt32) (m_value / getMaxValue() * 100));
}

//======================== Test Run ============================

class OPL_TestThreadStatus {
public:
	OPL_TestThreadStatus():
		m_thread_no(0), m_progressControl(NULL), m_textControl(NULL),
		m_trnTime(0), m_value(0)
	{
		// nothing to do
	}
	
	OSStatus init(UInt32 *thread_no,
		OPL_TestRunStatusDialog *statusDialog)
	{
		OSStatus err = noErr;
		m_thread_no = (*thread_no)++;
		
		statusDialog->getControl(kStatusProgressBar, &m_progressControl);
		statusDialog->getControl(kStatusProgressText, &m_textControl);
		if (m_thread_no != 0) {
			// create thread controls
			
			// get progress group control frame
			HIRect rGroup;
			ControlRef groupControl;
			statusDialog->getControl(kStatusProgressGroup, &groupControl);
			HIViewGetBounds(groupControl, &rGroup);

			// get original progress bar bounds
			HIRect rBar;
			HIViewGetFrame(m_progressControl, &rBar);
		
			// get original progress text bounds
			HIRect rText;
			HIViewGetFrame(m_textControl, &rText);

			// create progress bar control
			float deltaY = rGroup.size.height - (rBar.origin.y + rBar.size.height);
			rBar.origin.y += deltaY;

			HIViewConvertRect(&rBar, groupControl, NULL);
			Rect r;
			r.left = (short) rBar.origin.x;
			r.top = (short) rBar.origin.y;
			r.right = (short) (rBar.origin.x + rBar.size.width);
			r.bottom = (short) (rBar.origin.y + rBar.size.height);

			err = CreateProgressBarControl(statusDialog->getWindow(),
				&r, 0, 0, 100, FALSE, &m_progressControl);
			require_noerr(err, error);

			// we need large progress bar
			ControlSize controlSize = kControlSizeLarge;
			SetControlData(m_progressControl, kControlEntireControl,
				kControlSizeTag, sizeof(controlSize), &controlSize);

			// create static text control
			rText.origin.y += deltaY;
			
			HIViewConvertRect(&rText, groupControl, NULL);
			r.left = (short) rText.origin.x;
			r.top = (short) rText.origin.y;
			r.right = (short) (rText.origin.x + rText.size.width);
			r.bottom = (short) (rText.origin.y + rText.size.height);

			ControlFontStyleRec fontStyle;
			fontStyle.flags = 0;
			err = CreateStaticTextControl(statusDialog->getWindow(),
				&r, CFSTR(""), &fontStyle, &m_textControl);

			// we need mini static text
			controlSize = kControlSizeMini;
			SetControlData(m_textControl, kControlEntireControl,
				kControlSizeTag, sizeof(controlSize), &controlSize);
				
			statusDialog->resizeY(rBar.size.height * 2);
		}
		
	error:
		return err;
	}
	
	void setProgress(float nValue, int nTrnPerCall)
	{
		m_trnTime = nValue;
		m_value += nTrnPerCall;
	}
	
	void update(bool isTPCA, float maxValue)
	{
		SetControl32BitValue(m_progressControl,
		    (SInt32) (m_value / maxValue * 100));
		
		float val = m_trnTime != 0 ? m_value / m_trnTime : 0;
		setText(OPL_CFString_asprintf("%d %s, %10.2f tps",
			(int) m_value, isTPCA ? "txns" : "packs", val));
	}
	
	void markFinished()
	{
		float val = m_trnTime != 0 ? m_value / m_trnTime : 0;

		setText(OPL_CFString_asprintf("Finished, %10.2f tps", val));
	}
	
	void getMaxTPC(float *maxValue)
	{
		if (m_trnTime == 0)
			return;
			
		float curTPC = m_value / m_trnTime;
		if (curTPC > *maxValue)
			*maxValue = curTPC;
	}
	
private:
	void setText(CFStringRef str)
	{
		SetControlData(m_textControl, kControlEntireControl,
			kControlStaticTextCFStringTag, sizeof(str), &str);
		CFRelease(str);
	}
	
	UInt32 m_thread_no;
	
	ControlRef m_progressControl;
	ControlRef m_textControl;

	float m_trnTime;
	float m_value;
};

class OPL_TestStatus {
public:
	OPL_TestStatus():
		m_testNum(0), m_nThreads(0), m_testThreads(NULL), m_isTPCA(false)
	{
		// nothing to do
	}
	~OPL_TestStatus()
	{
		if (m_testThreads != NULL)
			delete[] m_testThreads;
	}
	
	OSStatus init(UInt32 testNum, UInt32 *thread_no,
		OPL_TestRunStatusDialog *statusDialog, test_t *test)
	{
		OSStatus err = noErr;
		m_testNum = testNum;
		m_isTPCA = (test->TestType == TPC_A);

		// set test name
		ControlRef testNameControl;
		statusDialog->getControl(kStatusTestName, &testNameControl);
		if (m_testNum != 0) {
			// insert new test name control

			// get progress group control frame
			HIRect rGroup;
			ControlRef groupControl;
			statusDialog->getControl(kStatusProgressGroup, &groupControl);
			HIViewGetBounds(groupControl, &rGroup);

			// get original static control bounds
			HIRect rText;
			HIViewGetFrame(testNameControl, &rText);
		
			// create static text control
			float deltaY = rGroup.size.height - (rText.origin.y + rText.size.height);
			rText.origin.y += deltaY;
			
			HIViewConvertRect(&rText, groupControl, NULL);
			Rect r;
			r.left = (short) rText.origin.x;
			r.top = (short) rText.origin.y;
			r.right = (short) (rText.origin.x + rText.size.width);
			r.bottom = (short) (rText.origin.y + rText.size.height);

			ControlFontStyleRec fontStyle;
			fontStyle.flags = 0;
			err = CreateStaticTextControl(statusDialog->getWindow(),
				&r, CFSTR(""), &fontStyle, &testNameControl);

			// we need mini static text
			ControlSize controlSize = kControlSizeMini;
			SetControlData(testNameControl, kControlEntireControl,
				kControlSizeTag, sizeof(controlSize), &controlSize);

			statusDialog->resizeY(rText.size.height * 2);
		}
		CFStringRef testName = OPL_char_to_CFString(test->szName);
		SetControlData(testNameControl, kControlEntireControl,
			kControlStaticTextCFStringTag, sizeof(testName), &testName);
		CFRelease(testName);

		// create controls for threads
		m_nThreads = test->tpc._.nThreads ? test->tpc._.nThreads : 1;
		m_testThreads = new OPL_TestThreadStatus[m_nThreads];
		for (UInt32 i = 0; i < m_nThreads; i++) {
			err = m_testThreads[i].init(thread_no, statusDialog);
			require_noerr(err, error);
		}
		
	error:
		return noErr;
	}
	
	void setProgress(int threadNo, float nValue, int nTrnPerCall)
	{
		if (threadNo < 0 || threadNo > m_nThreads)
			return;
			
		m_testThreads[threadNo].setProgress(nValue,
			m_isTPCA ? bench_get_long_pref(A_REFRESH_RATE) * nTrnPerCall : 1);
	}

	virtual void update(OPL_TestRunStatusDialog *statusDialog,
	    float fMaxTPCA, float fMaxTPCC)
	{
		float maxValue = m_isTPCA ? fMaxTPCA : fMaxTPCC;
		if (maxValue == 0)
			maxValue = statusDialog->getMaxValue();
			
		for (UInt32 i = 0; i < m_nThreads; i++)
			m_testThreads[i].update(m_isTPCA, maxValue);
	}
	
	virtual void markFinished(int threadNo)
	{
		if (threadNo < 0 || threadNo > m_nThreads)
			return;
			
		m_testThreads[threadNo].markFinished();
	}
	
	void getMaxTPC(float *maxTPCA, float *maxTPCC)
	{
		float *maxValue = m_isTPCA ? maxTPCA : maxTPCC;

		for (UInt32 i = 0; i < m_nThreads; i++)
			m_testThreads[i].getMaxTPC(maxValue);
	}
	
private:
	UInt32 m_testNum;
	UInt32 m_nThreads;
	OPL_TestThreadStatus *m_testThreads;
	
	bool m_isTPCA;
};

OPL_TestRunStatusDialog::~OPL_TestRunStatusDialog()
{
	if (m_testStates != NULL)
		delete[] m_testStates;
}

bool
OPL_TestRunStatusDialog::begin(CFStringRef title, float nMax)
{
	OSStatus err;
	
	setStaticText(kStatusTestName, CFStringCreateCopy(NULL, CFSTR("")));

	std::auto_ptr<OPL_TestPoolItemList> selectedItems(
		OPL_TestPoolItemList::getSelected());
	if (selectedItems.get() == NULL)
		return false;

	m_nTests = selectedItems->getItemCount();
	m_testStates = new OPL_TestStatus[m_nTests];
	UInt32 thread_no = 0;
	for (UInt32 i = 0; i < m_nTests; i++) {
		test_t *test = selectedItems->getItem(i);

		err = m_testStates[i].init(i, &thread_no, this, test);
		require_noerr(err, error);
	}

	return OPL_StatusDialog::begin(title, nMax);

error:
	return false;
}

void
OPL_TestRunStatusDialog::setProgressText(CFStringRef progressText,
	int nConn, int threadNo, float nValue, int nTrnPerCall)
{
	if (nConn >= 0 && nConn < m_nTests)
		m_testStates[nConn].setProgress(threadNo, nValue, nTrnPerCall);

	OPL_StatusDialog::setProgressText(
	    progressText, nConn, threadNo, nValue, nTrnPerCall);
}

void
OPL_TestRunStatusDialog::update()
{
	OPL_StatusDialog::update();
	
	float maxTPCA = 0;
	float maxTPCC = 0;
	for (UInt32 i = 0; i < m_nTests; i++)
		m_testStates[i].getMaxTPC(&maxTPCA, &maxTPCC);
	maxTPCA *= getMaxValue();
	maxTPCC *= getMaxValue();
	
	for (UInt32 i = 0; i < m_nTests; i++) 
		m_testStates[i].update(this, maxTPCA, maxTPCC);
}

// Mark work item as finished
void
OPL_TestRunStatusDialog::markFinished(int nConn, int threadNo)
{
	if (nConn < 0 || nConn >= m_nTests)
		return;
		
	m_testStates[nConn].markFinished(threadNo);
}

void
OPL_TestRunStatusDialog::resizeY(float deltaY)
{
	OSStatus err;
	
	// resize Group Control
	ControlRef groupControl;
	getControl(kStatusProgressGroup, &groupControl);
	
	HIRect r;
	err = HIViewGetFrame(groupControl, &r);
	require_noerr(err, error);
	r.size.height += deltaY;
	err = HIViewSetFrame(groupControl, &r);
	require_noerr(err, error);
	
	// move Cancel button
	ControlRef cancelButton;
	getControl(kStatusCancelButton, &cancelButton);
	err = HIViewMoveBy(cancelButton, 0, deltaY);
	require_noerr(err, error);

	// resize window
	Rect rect;
	err = GetWindowBounds(getWindow(), kWindowContentRgn, &rect);
	require_noerr(err, error);
	rect.bottom += (short) deltaY;
	err = SetWindowBounds(getWindow(), kWindowContentRgn, &rect);
	require_noerr(err, error);
	
error:
	/* nothing to do */;
}
