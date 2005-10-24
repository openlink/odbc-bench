/*
 *  odbcbench_macosx.h
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
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
void do_save(OPL_TestPoolItemList *itemList, bool askFileName, bool setFileName);

// Add test to the pool
int OPL_add_test_to_the_pool(test_t *test);

// Application menu
void do_preferences();

// File menu
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