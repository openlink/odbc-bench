/*
 *  TestPoolItemList.h
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
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
		return get(kDataBrowserItemIsSelected, false);
	}
	
	static OPL_TestPoolItemList *getAll()
	{
		return get(kDataBrowserItemAnyState, true);
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
	static OPL_TestPoolItemList *get(DataBrowserItemState itemState, bool allowEmpty);
	
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