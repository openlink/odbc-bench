/*
 *  TestPool.h
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

		// update control
		DrawOneControl(m_itemView);
	}
	
	// Remove items
	void removeItems(UInt32 numItems, DataBrowserItemID *items)
	{
		RemoveDataBrowserItems(m_itemView, kDataBrowserNoItem,
			numItems, items, kDataBrowserItemNoProperty);
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

	// Get dirty state
	bool isDirty()
	{
		return m_isDirty;
	}
		
	// Set dirty state
	void setDirty(bool isDirty)
	{
		m_isDirty = isDirty;
	}
		
private:
	CFStringRef m_filename;
	ControlRef m_itemView;
	ControlRef m_logView;
    UInt32 m_nextItemID;
	CFMutableDictionaryRef m_items;
	CFMutableArrayRef m_logMsgs;
	bool m_isDirty;
};

#endif /* _MACOSX_TESTPOOL_H_ */
