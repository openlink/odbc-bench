/*
 *  run_details.cpp
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

#include "odbcbench_macosx.h"
#include "tpca_code.h"

#include "Dialog.h"
#include "TestPoolItemList.h"
#include "util.h"

#define CMD_RUN_THREAD_OPTIONS		7000
#define CMD_RUN_ARRAY_PARAMS		7001
#define CMD_RUN_DO_100_ROW_QUERY	7002
#define CMD_RUN_SCROLLABLE_CURSORS	7003

static ControlID kControlRunThreadOptions = { 'RUN ', 0 };
static ControlID kControlRunNumberOfThreadsTag = { 'RUN ', 1 };
static ControlID kControlRunNumberOfThreads = { 'RUN ', 2 };

static ControlID kControlRunArrayParams = { 'RUN ', 3 };
static ControlID kControlRunNumberOfArrayParamsTag = { 'RUN ', 4 };
static ControlID kControlRunNumberOfArrayParams = { 'RUN ', 5 };

static ControlID kControlRunSQLOptions = { 'RUN ', 6 };

static ControlID kControlRunAsynchronous = { 'RUN ', 7 };
static ControlID kControlRunUseTransactions = { 'RUN ', 8 };
static ControlID kControlRunDo100RowQuery = { 'RUN ', 9 };

static ControlID kControlRunIsolationLevel = { 'RUN ', 10 };

static ControlID kControlRunScrollableCursorsBox = { 'RUN ', 11 };
static ControlID kControlRunScrollableCursors = { 'RUN ', 12 };

static ControlID kControlRunRowsetSizeTag = { 'RUN ', 13 };
static ControlID kControlRunRowsetSize = { 'RUN ', 14 };
static ControlID kControlRunKeysetSizeTag = { 'RUN ', 15 };
static ControlID kControlRunKeysetSize = { 'RUN ', 16 };
static ControlID kControlRunTraversalCountTag = { 'RUN ', 17 };
static ControlID kControlRunTraversalCount = { 'RUN ', 18 };

class OPL_RunDetailsDialog: public OPL_Dialog {
public:
	OPL_RunDetailsDialog(CFStringRef name):
		OPL_Dialog(name)
	{
		// nothing to do
	}

	void load(test_t *test)
	{
		int nThreads = test->tpc._.nThreads;
		
		if (nThreads > 0)
			set32BitValue(kControlRunThreadOptions, 2);
		else
			set32BitValue(kControlRunThreadOptions, 1);
		setEditText(kControlRunNumberOfThreads,
			OPL_CFString_asprintf("%d", nThreads < 2 ? 2 : nThreads));
			
		updateThreadOptions();
	}
	
	void save(test_t *test)
	{
		test->tpc._.nThreads = get32BitValue(kControlRunThreadOptions) == 2 ?
			getEditTextIntValue(kControlRunNumberOfThreads) : 0;
	}

protected:
	OSStatus handleCommandEvent(UInt32 commandID)
	{
		switch (commandID) {
		case CMD_RUN_THREAD_OPTIONS:
			updateThreadOptions();
			return noErr;
			
		default:
			return OPL_Dialog::handleCommandEvent(commandID);
		}
	}
	
private:
	void updateThreadOptions()
	{
		switch (get32BitValue(kControlRunThreadOptions)) {
		case 2:
			enableControl(kControlRunNumberOfThreadsTag);
			enableControl(kControlRunNumberOfThreads);
			break;
			
		default:
			disableControl(kControlRunNumberOfThreadsTag);
			disableControl(kControlRunNumberOfThreads);
			break;
		}
	}
};

class OPL_TPCARunDetailsDialog: public OPL_RunDetailsDialog {
public:
	OPL_TPCARunDetailsDialog(CFStringRef name):
		OPL_RunDetailsDialog(name)
	{
		// nothing to do
	}
	
	void load(test_t *test)
	{
		OPL_RunDetailsDialog::load(test);
		
		// array params
		int nArrayParSize = test->tpc.a.nArrayParSize;
		set32BitValue(kControlRunArrayParams, nArrayParSize > 0);
		setEditText(kControlRunNumberOfArrayParams,
			OPL_CFString_asprintf("%d",
				nArrayParSize < 10 ? 10 : nArrayParSize));
				
		// SQL options
		SInt32 val;
		switch (test->tpc.a.fSQLOption) {
		case IDX_PLAINSQL:
			val = 1;
			break;
				
		case IDX_PARAMS:
			val = 2;
			break;
			
		case IDX_SPROCS:
			val = 3;
			break;
		}
		set32BitValue(kControlRunSQLOptions, val);
		
		// execution options
		set32BitValue(kControlRunAsynchronous, test->tpc.a.fExecAsync);
		set32BitValue(kControlRunUseTransactions, test->tpc.a.fUseCommit);
		set32BitValue(kControlRunDo100RowQuery, test->tpc.a.fDoQuery);
		
		// isolation level
		switch (test->tpc.a.txn_isolation) {
		default:
		case SQL_TXN_DRIVER_DEFAULT:
			val = 1;
			break;
			
		case SQL_TXN_READ_UNCOMMITTED:
			val = 2;
			break;
			
		case SQL_TXN_READ_COMMITTED:
			val = 3;
			break;
			
		case SQL_TXN_REPEATABLE_READ:
			val = 4;
			break;
			
		case SQL_TXN_SERIALIZABLE:
			val = 5;
			break;
		}
		set32BitValue(kControlRunIsolationLevel, val);
		
		// scrollable cursors
		switch (test->tpc.a.nCursorType) {
		default:
		case SQL_CURSOR_FORWARD_ONLY:
			val = 1;
			break;
			
		case SQL_CURSOR_STATIC:
			val = 2;
			break;
			
		case SQL_CURSOR_KEYSET_DRIVEN:
			val = 3;
			break;
			
		case SQL_CURSOR_DYNAMIC:
			val = 4;
			break;
			
		case SQL_CURSOR_MIXED:
			val = 5;
			break;
		}
		set32BitValue(kControlRunScrollableCursors, val);

		setEditText(kControlRunRowsetSize,
			OPL_CFString_asprintf("%d", test->tpc.a.nRowsetSize));
		setEditText(kControlRunKeysetSize,
			OPL_CFString_asprintf("%d", test->tpc.a.nKeysetSize));
		setEditText(kControlRunTraversalCount,
			OPL_CFString_asprintf("%d", test->tpc.a.nTraversalCount));

		updateArrayParams();
		updateDo100RowQuery();
		updateScrollableCursors();
	}
	
	void save(test_t *test)
	{
		OPL_RunDetailsDialog::save(test);
		
		// array params
		test->tpc.a.nArrayParSize = get32BitValue(kControlRunArrayParams) ?
			getEditTextIntValue(kControlRunNumberOfArrayParams) : 0;

		// SQL options
		int val;
		switch (get32BitValue(kControlRunSQLOptions)) {
		case 1:
			val = IDX_PLAINSQL;
			break;
			
		case 2:
			val = IDX_PARAMS;
			break;
			
		case 3:
			val = IDX_SPROCS;
			break;
		}
		test->tpc.a.fSQLOption = val;
		
		// execution optioons
		test->tpc.a.fExecAsync = get32BitValue(kControlRunAsynchronous);
		test->tpc.a.fUseCommit = get32BitValue(kControlRunUseTransactions);
		test->tpc.a.fDoQuery = get32BitValue(kControlRunDo100RowQuery);

		// isolation level
		switch (get32BitValue(kControlRunIsolationLevel)) {
		default:
		case 1:
			val = SQL_TXN_DRIVER_DEFAULT;
			break;
			
		case 2:
			val = SQL_TXN_READ_UNCOMMITTED;
			break;
			
		case 3:
			val = SQL_TXN_READ_COMMITTED;
			break;
			
		case 4:
			val = SQL_TXN_REPEATABLE_READ;
			break;
			
		case 5:
			val = SQL_TXN_SERIALIZABLE;
			break;
		}
		test->tpc.a.txn_isolation = val;

		// scrollable cursors
		switch (get32BitValue(kControlRunScrollableCursors)) {
		default:
		case 1:
			val = SQL_CURSOR_FORWARD_ONLY;
			break;
			
		case 2:
			val = SQL_CURSOR_STATIC;
			break;
			
		case 3:
			val = SQL_CURSOR_KEYSET_DRIVEN;
			break;
			
		case 4:
			val = SQL_CURSOR_DYNAMIC;
			break;
			
		case 5:
			val = SQL_CURSOR_MIXED;
			break;
		}
		test->tpc.a.nCursorType = val;
		
		test->tpc.a.nRowsetSize = getEditTextIntValue(kControlRunRowsetSize);
		test->tpc.a.nKeysetSize = getEditTextIntValue(kControlRunKeysetSize);
		test->tpc.a.nTraversalCount = getEditTextIntValue(kControlRunTraversalCount);
	}
	
protected:
	OSStatus handleCommandEvent(UInt32 commandID)
	{
		switch (commandID) {
		case CMD_RUN_ARRAY_PARAMS:
			updateArrayParams();
			return noErr;
		
		case CMD_RUN_DO_100_ROW_QUERY:
			updateDo100RowQuery();
			return noErr;
		
		case CMD_RUN_SCROLLABLE_CURSORS:
			updateScrollableCursors();
			return noErr;
			
		default:
			return OPL_RunDetailsDialog::handleCommandEvent(commandID);
		}
	}
	
private:
	void updateArrayParams()
	{
		switch (get32BitValue(kControlRunArrayParams)) {
		case 1:
			enableControl(kControlRunNumberOfArrayParamsTag);
			enableControl(kControlRunNumberOfArrayParams);
			break;
			
		default:
			disableControl(kControlRunNumberOfArrayParamsTag);
			disableControl(kControlRunNumberOfArrayParams);
			break;
		}
	}
	
	void updateDo100RowQuery()
	{
		switch (get32BitValue(kControlRunDo100RowQuery)) {
		case 1:
			enableControl(kControlRunScrollableCursorsBox);
			enableControl(kControlRunScrollableCursors);
			enableControl(kControlRunRowsetSizeTag);
			enableControl(kControlRunRowsetSize);
			enableControl(kControlRunKeysetSizeTag);
			enableControl(kControlRunKeysetSize);
			enableControl(kControlRunTraversalCountTag);
			enableControl(kControlRunTraversalCount);
			break;
			
		default:
			disableControl(kControlRunScrollableCursorsBox);
			disableControl(kControlRunScrollableCursors);
			disableControl(kControlRunRowsetSizeTag);
			disableControl(kControlRunRowsetSize);
			disableControl(kControlRunKeysetSizeTag);
			disableControl(kControlRunKeysetSize);
			disableControl(kControlRunTraversalCountTag);
			disableControl(kControlRunTraversalCount);
			break;
		}
	}
	
	void updateScrollableCursors()
	{
		SInt32 val = get32BitValue(kControlRunScrollableCursors);

		if (val > 1) {
			enableControl(kControlRunRowsetSizeTag);
			enableControl(kControlRunRowsetSize);
			enableControl(kControlRunTraversalCountTag);
			enableControl(kControlRunTraversalCount);
		} else {
			disableControl(kControlRunRowsetSizeTag);
			disableControl(kControlRunRowsetSize);
			disableControl(kControlRunTraversalCountTag);
			disableControl(kControlRunTraversalCount);
		}

		if (val == 5) {
			enableControl(kControlRunKeysetSizeTag);
			enableControl(kControlRunKeysetSize);
		} else {
			disableControl(kControlRunKeysetSizeTag);
			disableControl(kControlRunKeysetSize);
		}
	}
};

static bool
do_run_details_tpc_a(test_t *test)
{
	OPL_TPCARunDetailsDialog dialog(CFSTR("EditTPCARunDetailsDialog"));
	
	// set initial values
	dialog.load(test);
	
	// run dialog
	if (!dialog.run())
		return false;
		
	// fetch dialog data
	dialog.save(test);
	return true;
}

static bool
do_run_details_tpc_c(test_t *test)
{
	OPL_RunDetailsDialog dialog(CFSTR("EditTPCCRunDetailsDialog"));

	// set initial values
	dialog.load(test);
	
	// run dialog
	if (!dialog.run())
		return false;
		
	// fetch dialog data
	dialog.save(test);
	return true;
}

void
do_run_details()
{
	std::auto_ptr<OPL_TestPoolItemList> selectedItems(
		OPL_TestPoolItemList::getSelected());
	if (selectedItems.get() == NULL)
		return;

	for (UInt32 i = 0; i < selectedItems->getItemCount(); i++) {
		test_t *test = selectedItems->getItem(i);
		bool rc = false;
		
		switch (test->TestType) {
		case TPC_A:
			rc = do_run_details_tpc_a(test);
			break;
				
        case TPC_C:
			rc = do_run_details_tpc_c(test);
			break;
		}

		if (!rc)
			break;
	}
}
