/*
 *  file_menu.cpp
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/12/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "odbcbench_macosx.h"
#include "TestPoolItemList.h"
#include "util.h"

// Load saved test pool
void
do_open(OPL_TestPool *testPool)
{
	OSStatus err;
	NavDialogCreationOptions dialogOptions;
	NavDialogRef openDialog =  NULL;
	NavUserAction userAction;
	NavReplyRecord reply;
	bool got_reply = false;
	AEKeyword keyword;
	AEDesc desc;
	FSRef fsref;
	char path[PATH_MAX];
	
	// create Open dialog
	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	require_noerr(err, error);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	dialogOptions.optionFlags |= kNavAllFilesInPopup;
	dialogOptions.modality = kWindowModalityAppModal;
	
	err = NavCreateGetFileDialog(&dialogOptions, NULL, NULL, NULL, NULL, NULL,
		&openDialog);
	require_noerr(err, error);
	
	// run Open dialog
	err = NavDialogRun(openDialog);
	require_noerr(err, error);
	
	// get user action
	userAction = NavDialogGetUserAction(openDialog);
	if (userAction == kNavUserActionNone || userAction == kNavUserActionCancel)
		goto error;

	// get dialog reply
	err = NavDialogGetReply(openDialog, &reply);
	require_noerr(err, error);
	got_reply = true;
	
	// get file name
	err = AEGetNthDesc(&reply.selection, 1, typeFSS, &keyword, &desc);
	require_noerr(err, error);

	err = OPL_GetFSRefFromAEDesc(&fsref, &desc);
	require_noerr(err, error);

	err = FSRefMakePath(&fsref, (UInt8 *) path, sizeof(path));
	require_noerr(err, error);

	if (!testPool) {
		// create new test pool
		err = OPL_NewWindow(OPL_char_to_CFString((char *) path));
		require_noerr(err, error);
	}
	
	// load tests into current test pool
	do_load_test((char *) path);

error:
	if (got_reply)
		NavDisposeReply(&reply);
	if (openDialog != NULL)
		NavDialogDispose(openDialog);
}

// Save item list to file
void
do_save(OPL_TestPoolItemList *itemList, bool askFileName, bool setFileName)
{
	OPL_TestPool *testPool = itemList->getTestPool();
	char path[PATH_MAX];
	OList *item_olist;
	
	if (!testPool->getFileName() || askFileName) {
		if (!OPL_getSaveFileName(path, sizeof(path), NULL, NULL))
			return;
			
		if (setFileName)
			testPool->setFileName(OPL_char_to_CFString(path));
	} else {
		char *p;
		
		p = OPL_CFString_to_char(testPool->getFileName());
		strlcpy(path, p, sizeof(path));
		free(p);
	}
	
	item_olist = itemList->getAsOList();
	do_save_selected(path, item_olist);
	o_list_free(item_olist);
}
