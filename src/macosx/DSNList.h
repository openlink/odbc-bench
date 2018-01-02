/*
 *  DSNList.h
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2018 OpenLink Software <odbc-bench@openlinksw.com>
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

#ifndef _MACOSX_DSNLIST_H_
#define _MACOSX_DSNLIST_H_

#include "odbcbench_macosx.h"

// DSN List
class OPL_DSNList {
public:
	OPL_DSNList(ControlRef itemView);
	~OPL_DSNList();

	// Get current test pool
	static OPL_DSNList *get(ControlRef cntlView);

	// Add item
	OSStatus addItem(CFStringRef dsn, int item_type = 0);
	
	// Get item
	CFStringRef getItem(UInt32 idx);
	int getItemType(UInt32 idx);

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

	void clear()
	{
		// remove all items
		RemoveDataBrowserItems(m_itemView, kDataBrowserNoItem, 0, NULL,
			kDataBrowserItemNoProperty);
		CFDictionaryRemoveAllValues(m_items);
		CFDictionaryRemoveAllValues(m_itemsType);
	}

	// Get document window
	WindowRef getWindow()
	{
		return GetControlOwner(m_itemView);
	}

	UInt32 getSelectedItem();


private:
	ControlRef m_itemView;
        UInt32 m_nextItemID;
	CFMutableDictionaryRef m_items;
	CFMutableDictionaryRef m_itemsType;
};

#endif /* _MACOSX_DSNLIST_H_ */
