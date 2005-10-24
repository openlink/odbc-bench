/*
 *  TestPoolItemList.cpp
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
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
OPL_TestPoolItemList::get(DataBrowserItemState itemState, bool allowEmpty)
{
	OPL_TestPool *testPool;
	OPL_TestPoolItemList *itemList;
	
	// get current test pool (if any)
	testPool = OPL_TestPool::get();
	if (testPool == NULL)
		goto error;
	
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
