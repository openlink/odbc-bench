/*
 *  TestPool.cpp
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

#include "odbcbench_macosx.h"
#include "TestPool.h"

OPL_TestPool::OPL_TestPool(ControlRef itemView, ControlRef logView, CFStringRef filename):
	m_filename(NULL), m_itemView(itemView), m_logView(logView),
	m_nextItemID(0)
{
	m_items = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, NULL);
	m_logMsgs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (filename == NULL)
		filename = CFStringCreateCopy(NULL, CFSTR("untitled.xml"));
	setFileName(filename);
}

OPL_TestPool::~OPL_TestPool()
{
	if (m_filename != NULL)
		CFRelease(m_filename);
		
	CFRelease(m_items);
	CFRelease(m_logMsgs);
}

// Get current test pool
/* static */ OPL_TestPool *
OPL_TestPool::get()
{
	OSStatus err;
	WindowRef window;

	window = GetFrontWindowOfClass(kDocumentWindowClass, false);
	if (window == NULL)
		return NULL;

	// get TestPool instance
	OPL_TestPool *testPool = NULL;
	err = GetWindowProperty(window, kPropertyCreator, kPropertyTag,
	   sizeof(testPool), NULL, &testPool);
	require_noerr(err, error);
	
error:
	return testPool;
}

// Add item
OSStatus
OPL_TestPool::addItem(test_t *test)
{
	OSStatus err;
	
	m_nextItemID++;
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &m_nextItemID);
	CFDictionarySetValue(m_items, num, test);
	
	// add new test item
	DataBrowserItemID itemID = m_nextItemID;
	err = AddDataBrowserItems(m_itemView, kDataBrowserNoItem,
		1, &itemID, kDataBrowserItemNoProperty);
	require_noerr(err, error);
	
	err = AutoSizeDataBrowserListViewColumns(m_itemView);
	require_noerr(err, error);
	
error:
	return err;
}

// Get item
test_t *
OPL_TestPool::getItem(UInt32 idx)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &idx);
		
	test_t *test = (test_t *) CFDictionaryGetValue(m_items, num);
	CFRelease(num);
	return test;
}

// Remove item
void
OPL_TestPool::removeItem(UInt32 idx)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &idx);

	test_t *test = (test_t *) CFDictionaryGetValue(m_items, num);
	if (test != NULL)
		free(test);
		
	CFDictionaryRemoveValue(m_items, num);
	CFRelease(num);
}


// Append log message
OSStatus
OPL_TestPool::addLogMsg(CFStringRef msg)
{
	OSStatus err;
	
	CFArrayAppendValue(m_logMsgs, msg);

	// add new log message
	DataBrowserItemID itemID = CFArrayGetCount(m_logMsgs);
	err = AddDataBrowserItems(m_logView, kDataBrowserNoItem,
		1, &itemID, kDataBrowserItemNoProperty);
	require_noerr(err, error);

	// scroll to the last message
	err = RevealDataBrowserItem(m_logView, itemID, kLogViewLog,
		kDataBrowserRevealWithoutSelecting);
	require_noerr(err, error);

error:
	return err;
}

// Get log message
CFStringRef
OPL_TestPool::getLogMsg(UInt32 idx)
{
	if (idx == 0)
		return NULL;
		
	return (CFStringRef) CFArrayGetValueAtIndex(m_logMsgs, idx - 1);
}

// Set test pool file name
//
// takes ownership of passed filename
void
OPL_TestPool::setFileName(CFStringRef filename)
{
	if (m_filename != NULL)
		CFRelease(m_filename);
	m_filename = filename;
	
	// set title stripping everything before the last "/"
	CFRange range = CFStringFind(m_filename, CFSTR("/"), kCFCompareBackwards);
	if (range.location == kCFNotFound) {
		SetWindowTitleWithCFString(getWindow(), m_filename);
	} else {
		range.location++;
		range.length = CFStringGetLength(filename) - range.location;

		CFStringRef title = CFStringCreateWithSubstring(NULL, filename, range);
		SetWindowTitleWithCFString(getWindow(), title);
		CFRelease(title);
	}
}

