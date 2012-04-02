/*
 *  table_details.cpp
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

#include <memory>

#include "odbcbench_macosx.h"
#include "tpca_code.h"

#include "Dialog.h"
#include "TestPoolItemList.h"
#include "util.h"

static ControlID kControlTableCreateBranch = { 'TABL', 2 };
static ControlID kControlTableCreateTeller = { 'TABL', 4 };
static ControlID kControlTableCreateAccount = { 'TABL', 6 };
static ControlID kControlTableCreateHistory = { 'TABL', 8 };

static ControlID kControlTableLoadBranch = { 'TABL', 10 };
static ControlID kControlTableLoadTeller = { 'TABL', 11 };
static ControlID kControlTableLoadAccount = { 'TABL', 12 };

static ControlID kControlTableCreateIndices = { 'TABL', 13 };
static ControlID kControlTableCreateProcedures = { 'TABL', 14 };

static ControlID kControlTableNumberOfBranches = { 'TABL', 15 };
static ControlID kControlTableNumberOfTellers = { 'TABL', 16 };
static ControlID kControlTableNumberOfAccounts = { 'TABL', 17 };

static ControlID kControlTableDBMSSchema = { 'TABL', 18 };

class OPL_TPCATableDetailsDialog: public OPL_Dialog {
public:
	OPL_TPCATableDetailsDialog(CFStringRef name):
		OPL_Dialog(name)
	{
		setComboBoxTags(kControlTableDBMSSchema, getDBMSList());
	}
	
	void load(test_t *test)
	{
		// set database type
		do_login(test);
		if (test->tpc.a.uwDrvIdx == -1)
			test->tpc.a.uwDrvIdx = getDriverTypeIndex(test->szDBMS);
		
		// create tables
		set32BitValue(kControlTableCreateBranch, test->tpc.a.fCreateBranch);
		set32BitValue(kControlTableCreateTeller, test->tpc.a.fCreateTeller);
		set32BitValue(kControlTableCreateAccount, test->tpc.a.fCreateAccount);
		set32BitValue(kControlTableCreateHistory, test->tpc.a.fCreateHistory);
		
		// load tables
		set32BitValue(kControlTableLoadBranch, test->tpc.a.fLoadBranch);
		set32BitValue(kControlTableLoadTeller, test->tpc.a.fLoadTeller);
		set32BitValue(kControlTableLoadAccount, test->tpc.a.fLoadAccount);

		// create indexes
		set32BitValue(kControlTableCreateIndices, test->tpc.a.fCreateIndex);
		set32BitValue(kControlTableCreateProcedures, test->tpc.a.fCreateProcedure);

		// number of records to insert
		setEditText(kControlTableNumberOfBranches,
			OPL_CFString_asprintf("%ld", test->tpc.a.udwMaxBranch));
		setEditText(kControlTableNumberOfTellers,
			OPL_CFString_asprintf("%ld", test->tpc.a.udwMaxTeller));
		setEditText(kControlTableNumberOfAccounts,
			OPL_CFString_asprintf("%ld", test->tpc.a.udwMaxAccount));
			
		// DBMS schema
		int drvIdx = test->tpc.a.uwDrvIdx;
		if (drvIdx >= 0 && drvIdx < getDriverMapSize()) {
			setEditText(kControlTableDBMSSchema,
				OPL_char_to_CFString(getDriverDBMSName(drvIdx)));
		}
	}
	
	void save(test_t *test)
	{
		// create tables
		test->tpc.a.fCreateBranch = get32BitValue(kControlTableCreateBranch);
		test->tpc.a.fCreateTeller = get32BitValue(kControlTableCreateTeller);
		test->tpc.a.fCreateAccount = get32BitValue(kControlTableCreateAccount);
		test->tpc.a.fCreateHistory = get32BitValue(kControlTableCreateHistory);
		
		// load tables
		test->tpc.a.fLoadBranch = get32BitValue(kControlTableLoadBranch);
		test->tpc.a.fLoadTeller = get32BitValue(kControlTableLoadTeller);
		test->tpc.a.fLoadAccount = get32BitValue(kControlTableLoadAccount);
		
		// create indexes
		test->tpc.a.fCreateIndex = get32BitValue(kControlTableCreateIndices);
		test->tpc.a.fCreateProcedure = get32BitValue(kControlTableCreateProcedures);
		
		// number of records to insert
		test->tpc.a.udwMaxBranch = getEditTextIntValue(kControlTableNumberOfBranches);
		test->tpc.a.udwMaxTeller = getEditTextIntValue(kControlTableNumberOfTellers);
		test->tpc.a.udwMaxAccount = getEditTextIntValue(kControlTableNumberOfAccounts);
		
		// DBMS schema
		test->tpc.a.uwDrvIdx = getDriverMapSize() - 1;
		char *dbms_name = OPL_CFString_to_char(
			getEditText(kControlTableDBMSSchema));
		for (int i = 0; i < getDriverMapSize (); i++)
			if (!strcasecmp(dbms_name, getDriverDBMSName(i))) {
				test->tpc.a.uwDrvIdx = i;
				break;
		}
		free(dbms_name);
	}
	
private:
	CFArrayRef
	getDBMSList()
	{
		CFMutableArrayRef dbmsList;
		
		// create mutable array
		dbmsList = CFArrayCreateMutable(NULL,
			getDriverMapSize(), &kCFTypeArrayCallBacks);

		// append values
		for (int i = 0; i < getDriverMapSize(); i++) {
			CFArrayAppendValue(dbmsList,
				OPL_char_to_CFString(getDriverDBMSName(i)));
		}
		
		return dbmsList;
	}
};

static bool
do_table_details_tpc_a(test_t *test)
{
	// XXX should use EditTPCATableDetailsVirtDialog for Virtuoso
	OPL_TPCATableDetailsDialog dialog(CFSTR("EditTPCATableDetailsDialog"));

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
do_table_details_tpc_c(test_t *test)
{
	// XXX Virtuoso-only (not implemented yet)
	return true;
	
	OPL_Dialog dialog(CFSTR("EditTPCCTableDetailsVirtDialog"));

	// set initial values
	
	// run dialog
	if (!dialog.run())
		return false;
		
	// fetch dialog data
	return true;
}

void
do_table_details()
{
	bool isDirty = false;
	
	std::auto_ptr<OPL_TestPoolItemList> selectedItems(
		OPL_TestPoolItemList::getSelected());
	if (selectedItems.get() == NULL)
		return;

	for (UInt32 i = 0; i < selectedItems->getItemCount(); i++) {
		test_t *test = selectedItems->getItem(i);
		bool rc = false;
		
		switch (test->TestType) {
		case TPC_A:
			rc = do_table_details_tpc_a(test);
			break;
				
        case TPC_C:
			rc = do_table_details_tpc_c(test);
			break;
		}

		if (!rc)
			break;
		if (!isDirty)
			isDirty = true;
	}

	if (isDirty)
		selectedItems->getTestPool()->setDirty(true);
}
