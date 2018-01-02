/*
 *  DSNList.cpp
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

#include "odbcbench_macosx.h"
#include "DSNList.h"

OPL_DSNList::OPL_DSNList(ControlRef itemView):
	m_itemView(itemView), m_nextItemID(0)
{
	m_items = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, 
		&kCFTypeDictionaryValueCallBacks);
	m_itemsType = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, 
		&kCFTypeDictionaryValueCallBacks);
}

OPL_DSNList::~OPL_DSNList()
{
	CFRelease(m_items);
	CFRelease(m_itemsType);
}

// Get current test pool
/* static */ OPL_DSNList *
OPL_DSNList::get(ControlRef cntlView)
{
	OSStatus err;

	if (cntlView == NULL)
		return NULL;

	// get instance
	OPL_DSNList *dsnlist = NULL;
	err = GetControlProperty(cntlView, kPropertyCreator, kPropertyTag,
	   sizeof(dsnlist), NULL, &dsnlist);
	require_noerr(err, error);
	
error:
	return dsnlist;
}

// Add item
OSStatus
OPL_DSNList::addItem(CFStringRef dsn, int item_type)
{
	OSStatus err;
	
	m_nextItemID++;
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &m_nextItemID);
	CFDictionarySetValue(m_items, num, dsn);
	CFDictionarySetValue(m_itemsType, num, CFNumberCreate(NULL, kCFNumberSInt32Type, &item_type));

	// add new test item
	DataBrowserItemID itemID = m_nextItemID;
	err = AddDataBrowserItems(m_itemView, kDataBrowserNoItem,
		1, &itemID, kDataBrowserItemNoProperty);
	require_noerr(err, error);

	// update control
	DrawOneControl(m_itemView);
	
error:
	return err;
}

// Get item
CFStringRef
OPL_DSNList::getItem(UInt32 idx)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &idx);
		
	CFStringRef dsn = (CFStringRef) CFDictionaryGetValue(m_items, num);
	CFRelease(num);
	return dsn;
}

// Get item
int
OPL_DSNList::getItemType(UInt32 idx)
{
	int itemType;
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &idx);
		
	CFNumberRef val = (CFNumberRef) CFDictionaryGetValue(m_itemsType, num);
	CFRelease(num);
	CFNumberGetValue(val, kCFNumberSInt32Type, &itemType); 
	return itemType;
}

// Remove item
void
OPL_DSNList::removeItem(UInt32 idx)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &idx);

	CFDictionaryRemoveValue(m_items, num);
	CFRelease(num);
}

UInt32 
OPL_DSNList::getSelectedItem()
{
    OSStatus err;
    UInt32 retID = 0;
    UInt32 itemCount;
    Handle itemList = NewHandle(0);

    if (itemList == NULL)
      return 0;

    err = GetDataBrowserItems(m_itemView, kDataBrowserNoItem, false,
	kDataBrowserItemIsSelected, itemList);
    require_noerr(err, error);

    itemCount = GetHandleSize(itemList) / sizeof(DataBrowserItemID);
    if (itemCount != 0)
      {
        DataBrowserItemID *list = (DataBrowserItemID *) *itemList;
        retID = list[0];
      }

    DisposeHandle(itemList);
    return retID;

error:
    return 0;
}


