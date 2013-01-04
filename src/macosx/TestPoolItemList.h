/*
 *  TestPoolItemList.h
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2013 OpenLink Software <odbc-bench@openlinksw.com>
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

#ifndef _MACOSX_TESTPOOLITEMLIST_H_
#define _MACOSX_TESTPOOLITEMLIST_H_

#include "TestPool.h"
#include "olist.h"

// List of items
class OPL_TestPoolItemList {
public:
	~OPL_TestPoolItemList()
	{
		if (m_itemList != NULL)
			DisposeHandle(m_itemList);
	}
	
	static OPL_TestPoolItemList *getSelected()
	{
		return get(NULL, kDataBrowserItemIsSelected, false);
	}
	
	static OPL_TestPoolItemList *getAll(OPL_TestPool *testPool = NULL)
	{
		return get(testPool, kDataBrowserItemAnyState, true);
	}
	
	// Get test pool
	OPL_TestPool *getTestPool()
	{
		return m_testPool;
	}
	
	// Convert to OList
	OList *getAsOList();
	
	// Get item id
	DataBrowserItemID getItemID(UInt32 idx)
	{
		DataBrowserItemID *items = getItems();
		return items[idx];
	}
	
	// Get item
	test_t *getItem(UInt32 idx)
	{
		return m_testPool->getItem(getItemID(idx));
	}
	
	// Get item count
	UInt32 getItemCount()
	{
		return m_itemCount;
	}
	
	// Update items
	void update()
	{
		m_testPool->updateItems(getItemCount(), getItems());
	}
	
	// Remove items
	void remove();

private:
	OPL_TestPoolItemList(OPL_TestPool *testPool):
		m_testPool(testPool), m_itemList(NULL), m_itemCount(0)
	{
		// nothing to do
	}

	// initialize item list instance
	bool init(DataBrowserItemState itemState, bool allowEmpty);

	// Get item list
	static OPL_TestPoolItemList *get(OPL_TestPool *testPool,
		DataBrowserItemState itemState, bool allowEmpty);
	
	// Get selected items
	DataBrowserItemID *getItems()
	{
		return (DataBrowserItemID *) *m_itemList;
	}
	
	OPL_TestPool *m_testPool;
	Handle m_itemList;
	UInt32 m_itemCount;
};

#endif /* _MACOSX_TESTPOOLITEMLIST_H_ */
