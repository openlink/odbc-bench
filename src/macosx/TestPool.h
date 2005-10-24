/*
 *  TestPool.h
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MACOSX_TESTPOOL_H_
#define _MACOSX_TESTPOOL_H_

#include "odbcbench_macosx.h"

// Test pool
class OPL_TestPool {
public:
	OPL_TestPool(ControlRef itemView, ControlRef logView, CFStringRef filename);
	~OPL_TestPool();

	// Get current test pool
	static OPL_TestPool *get();

	// Add item
	OSStatus addItem(test_t *test);
	
	// Get item
	test_t *getItem(UInt32 idx);

	// Remove item
	void removeItem(UInt32 idx);

	// Get list of items of specified state
	OSStatus getItemList(Handle itemList, DataBrowserItemState itemState)
	{
		return GetDataBrowserItems(m_itemView, kDataBrowserNoItem, false,
			itemState, itemList);
	}
	
	// Update items
	void updateItems(UInt32 numItems, DataBrowserItemID *items)
	{
		UpdateDataBrowserItems(m_itemView, kDataBrowserNoItem,
			numItems, items, kDataBrowserItemNoProperty, kDataBrowserNoItem);

		// auto-size test item browser columns
		AutoSizeDataBrowserListViewColumns(m_itemView);
	}
	
	// Remove items
	void removeItems(UInt32 numItems, DataBrowserItemID *items)
	{
		RemoveDataBrowserItems(m_itemView, kDataBrowserNoItem,
			numItems, items, kDataBrowserItemNoProperty);

		// auto-size test item browser columns
		AutoSizeDataBrowserListViewColumns(m_itemView);
	}

	// Append log message
	OSStatus addLogMsg(CFStringRef msg);

	// Get log message
	CFStringRef getLogMsg(UInt32 idx);

	// Clear log view
	void clearLog()
	{
		// remove all items
		RemoveDataBrowserItems(m_logView, kDataBrowserNoItem, 0, NULL,
			kDataBrowserItemNoProperty);
	}

	// Get document window
	WindowRef getWindow()
	{
		return GetControlOwner(m_itemView);
	}

	// Get test pool file name
	CFStringRef getFileName()
	{
		return m_filename;
	}
		
	// Set test pool file name
	//
	// takes ownership of passed filename
	void setFileName(CFStringRef filename);

private:
	CFStringRef m_filename;
	ControlRef m_itemView;
	ControlRef m_logView;
    UInt32 m_nextItemID;
	CFMutableDictionaryRef m_items;
	CFMutableArrayRef m_logMsgs;
};

#endif /* _MACOSX_TESTPOOL_H_ */