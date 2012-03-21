/*
 *  odbcbench_macosx.h
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

#ifndef _MACOSX_ODBCBENCH_MACOSX_H_
#define _MACOSX_ODBCBENCH_MACOSX_H_

#include "odbcbench.h"

#include <Carbon/Carbon.h>

// Main nib
extern IBNibRef g_main_nib;

// Document window property create and tag
const PropertyCreator kPropertyCreator = 'OPL ';
const PropertyTag kPropertyTag = 'INST';

// Test pool view column ids
const DataBrowserPropertyID kItemViewType = 'TYPE';
const DataBrowserPropertyID kItemViewName = 'NAME';
const DataBrowserPropertyID kItemViewDSN = 'DSN ';
const DataBrowserPropertyID kItemViewDriverName = 'DNAM';
const DataBrowserPropertyID kItemViewDriverVer = 'DVER';
const DataBrowserPropertyID kItemViewDBMS = 'DBMS';

// Log view column ids
const DataBrowserPropertyID kLogViewLog = 'LOG ';

// Create new main window
//
// Takes ownership of filename
OSStatus OPL_NewWindow(CFStringRef filename = NULL);

class OPL_TestPool;
class OPL_TestPoolItemList;

// Load saved test pool
void do_open(OPL_TestPool *testPool);

// Save item list to file
bool do_save(OPL_TestPoolItemList *itemList, bool askFileName, bool setFileName);

// Save changes to file
bool do_save_changes(OPL_TestPool *testPool, bool quitting);

// Add test to the pool
int OPL_add_test_to_the_pool(test_t *test);

// Application menu
void do_about();
void do_preferences();
OSStatus do_quit();

// File menu
void do_file_new();
void do_file_open();
void do_file_save();
void do_file_save_as();
void do_clear_log();

// Edit menu
void do_new_item();
void do_delete_items();
void do_save_items();
void do_login_details();
void do_table_details();
void do_run_details();
void do_insert_file();

// Actions menu
void do_create_tables();
void do_drop_tables();
void do_run();

// Results menu
void do_results_connect();
void do_results_disconnect();
void do_results_create_table();
void do_results_drop_table();

#endif _MACOSX_ODBCBENCH_MACOSX_H_
