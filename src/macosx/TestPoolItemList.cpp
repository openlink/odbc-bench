/*
 *  TestPoolItemList.cpp
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2012 OpenLink Software <odbc-bench@openlinksw.com>
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

#include "TestPoolItemList.h"

// Convert to OList
OList *
OPL_TestPoolItemList::getAsOList()
{
	OList *olist = NULL;
	
	for (UInt32 i = 0; i < getItemCount(); i++)
		olist = o_list_append(olist, getItem(i));
		
	return olist;
}

// Remove items
void
OPL_TestPoolItemList::remove()
{	
	// remove from test pool
	for (UInt32 i = 0; i < getItemCount(); i++)
		m_testPool->removeItem(getItemID(i));
	
	// remove from browser
	m_testPool->removeItems(getItemCount(), getItems());
}

// initialize selected items instance
bool
OPL_TestPoolItemList::init(DataBrowserItemState itemState, bool allowEmpty)
{
	OSStatus err;
	
	m_itemList = NewHandle(0);
	if (m_itemList == NULL)
		return false;
	
	err = m_testPool->getItemList(m_itemList, itemState);
	require_noerr(err, error);

	m_itemCount = GetHandleSize(m_itemList) / sizeof(DataBrowserItemID);
	if (m_itemCount == 0 && !allowEmpty)
		return false;
		
	HLock(m_itemList);
	return true;
	
error:
	return false;
}

// Get item list
/* static */ OPL_TestPoolItemList *
OPL_TestPoolItemList::get(OPL_TestPool *testPool,
	DataBrowserItemState itemState, bool allowEmpty)
{
	OPL_TestPoolItemList *itemList;
	
	// get current test pool (if any)
	if (testPool == NULL) {
		testPool = OPL_TestPool::get();
		if (testPool == NULL)
			goto error;
	}
	
	// create and initialize new instance
	itemList = new OPL_TestPoolItemList(testPool);
	if (itemList == NULL)
		goto error;
	if (!itemList->init(itemState, allowEmpty))
		goto error;
	
	// we're done
	return itemList;
	
error:
	if (itemList != NULL)
		delete itemList;
	return NULL;
}
