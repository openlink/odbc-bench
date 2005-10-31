/*
 *  util.c
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

#include "odbcbench.h"

#include "util.h"

/*  
 *  Max length of a UTF-8 encoded character sequence
 */
#define UTF8_MAX_CHAR_LEN 4

static char *
OPL_W2A(const wchar_t *inStr, ssize_t size)
{
	char *outStr = NULL;
	size_t len;

	if (inStr == NULL)
		return NULL;

	if (size == SQL_NTS)
		len = wcslen(inStr);
	else
		len = size;

	if (len < 0)
		return NULL;

	if ((outStr = (char *) malloc (len * UTF8_MAX_CHAR_LEN + 1)) != NULL) {
		if (len > 0)
			wcstombs(outStr, inStr, len);
		outStr[len] = '\0';
    }

	return outStr;
}

static wchar_t *
OPL_A2W(const char *inStr, ssize_t size)
{
	wchar_t *outStr = NULL;
	size_t len;
	
	if (inStr == NULL)
		return NULL;
	
	if (size == SQL_NTS)
		len = strlen ((char *) inStr);
	else
		len = size;
	
	if (len < 0)
		return NULL;
	
	if ((outStr = (wchar_t *) calloc (len + 1, sizeof(wchar_t))) != NULL) {
		if (len > 0)
			mbstowcs(outStr, inStr, len);
		outStr[len] = L'\0';
    }
	
	return outStr;
}

wchar_t *
OPL_CFString_to_wchar(const CFStringRef str)
{
	wchar_t *prov = (wchar_t *) malloc(
		sizeof(wchar_t) * (CFStringGetLength(str) + 1));
	CFIndex i;

	if (prov) {
		for(i = 0 ; i < CFStringGetLength(str) ; i++)
			prov[i] = CFStringGetCharacterAtIndex(str, i);
		prov[i] = L'\0';
    }

	return prov;
}

char *
OPL_CFString_to_char(const CFStringRef str)
{
	wchar_t *prov = OPL_CFString_to_wchar(str);
	char *buffer = NULL;

	if (prov) {
		buffer = OPL_W2A(prov, SQL_NTS);
		free(prov);
	}

	return buffer;
}

CFStringRef
OPL_wchar_to_CFString(const wchar_t *str)
{
	CFMutableStringRef prov = CFStringCreateMutable(NULL, 0);
	CFIndex i;
	UniChar c;
    
	if (prov) {
		for (i = 0 ; str[i] != L'\0' ; i++) {
			c = (UniChar) str[i];
			CFStringAppendCharacters(prov, &c, 1);
        }
    }
	
	return prov;
}

CFStringRef
OPL_char_to_CFString(const char *str)
{
	wchar_t *wstr = OPL_A2W(str, SQL_NTS);
	CFStringRef stringRef = NULL;
	
	if (wstr != NULL) {
		stringRef = OPL_wchar_to_CFString(wstr);
		free(wstr);
	}
	
	return stringRef;
}

CFStringRef
OPL_CFString_vasprintf(const char *format, va_list ap)
{
	CFStringRef fmt, str;

	fmt = OPL_char_to_CFString(format);
	str = CFStringCreateWithFormatAndArguments(NULL, NULL, fmt, ap);
	CFRelease(fmt);

	return str;
}
	
CFStringRef
OPL_CFString_asprintf(const char *format, ...)
{
	CFStringRef res;
	va_list ap;
	
	va_start(ap, format);
	res = OPL_CFString_vasprintf(format, ap);
	va_end(ap);
	
	return res;
}

// Case-insensitive CFString comparator
CFComparisonResult
OPL_CFString_cmp(const void *a, const void *b, void *ctx)
{
	return CFStringCompare((CFStringRef) a, (CFStringRef) b,
	    kCFCompareCaseInsensitive);
}

// Get FSRef from AEDesc
OSStatus
OPL_GetFSRefFromAEDesc(FSRef *fsRef, AEDesc *desc)
{
    OSStatus err = noErr;
    AEDesc coerceDesc = { 0, NULL };

    /* If the AEDesc isn't already an FSSpec,
        convert it to one... */
    if (desc->descriptorType != typeFSRef) {
        err = AECoerceDesc(desc, typeFSRef, &coerceDesc);
        if (err == noErr)
            desc = &coerceDesc;
    }
	
    /* Get the FSRef out of the AEDesc */
    if (err == noErr)
        err = AEGetDescData(desc, fsRef, sizeof(*fsRef));
    AEDisposeDesc(&coerceDesc);

    /* If we have could not get an FSRef try getting an FSSpec and
        make an FSRef out of it. */
    if (err != noErr) {
        FSSpec fsSpec;
        AEDesc coerceDesc2 = { 0, NULL };

        /* If the AEDesc isn't already an FSSpec, convert it to one... */
        if (desc->descriptorType != typeFSS) {
            err = AECoerceDesc(desc, typeFSS, &coerceDesc2);
            desc = &coerceDesc2;
        }

        /* Get the FSSpec out of the AEDesc and convert it to an FSRef... */
        if (err == noErr)
            err = AEGetDescData(desc, &fsSpec, sizeof(fsSpec));
        AEDisposeDesc(&coerceDesc2);
        if (err == noErr)
            err = FSpMakeFSRef(&fsSpec, fsRef);
    }

    return (err);
}

// make file name
static void
OPL_appendFileName(char *path, size_t path_len, CFStringRef saveFileName)
{
	char *p;

	p = OPL_CFString_to_char(saveFileName);
	strlcat(path, p, path_len);
	free(p);

	// append .xml
	p = strrchr(path, '/');
	if (p == NULL)
		p = path;
	p = strchr(p, '.');
	if (p == NULL)
		strlcat(path, ".xml", path_len);
}

// Get file name for saving
bool
OPL_getSaveFileName(char *path, size_t path_len,
					CFStringRef title, CFStringRef defaultFileName)
{
	OSStatus err;
	NavDialogRef saveDialog = NULL;
	NavDialogCreationOptions dialogOptions;
	NavReplyRecord reply;
	bool got_reply = false;
	NavUserAction userAction;
	AEKeyword keyword;
	AEDesc desc;
	FSRef fsref;
	
	// create Save dialog
	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	require_noerr(err, error);
	dialogOptions.modality = kWindowModalityAppModal;
	
	err = NavCreatePutFileDialog(&dialogOptions,
		'OPL ', kNavGenericSignature, NULL, NULL, &saveDialog);
	require_noerr(err, error);

	if (title)
		SetWindowTitleWithCFString(NavDialogGetWindow(saveDialog), title);
	if (defaultFileName)
		NavDialogSetSaveFileName(saveDialog, defaultFileName);
		
	// run Save dialog
	err = NavDialogRun(saveDialog);
	require_noerr(err, error);

	// get user action
	userAction = NavDialogGetUserAction(saveDialog);
	if (userAction == kNavUserActionNone || userAction == kNavUserActionCancel) {
		err = userCanceledErr;
		goto error;
	}

	// get dialog reply
	err = NavDialogGetReply(saveDialog, &reply);
	require_noerr(err, error);
	got_reply = true;

	// get file name
	err = AEGetNthDesc(&reply.selection, 1, typeFSS, &keyword, &desc);
	require_noerr(err, error);

	err = OPL_GetFSRefFromAEDesc(&fsref, &desc);
	require_noerr(err, error);

	err = FSRefMakePath(&fsref, (UInt8 *) path, path_len);
	require_noerr(err, error);

	strlcat(path, "/", path_len);
	OPL_appendFileName(path, path_len, reply.saveFileName);

error:
	if (got_reply)
		NavDisposeReply(&reply);
	if (saveDialog != NULL)
		NavDialogDispose(saveDialog);

	return err == noErr;
}

// Ask to save changes
bool
OPL_getSaveChangesFileName(char *path, size_t path_len,
						   CFStringRef fileName, bool quitting)
{
	OSStatus err;
	NavDialogRef saveDialog = NULL;
	NavDialogCreationOptions dialogOptions;
	NavUserAction userAction;
	
	// create Save Changes dialog
	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	require_noerr(err, error);
	dialogOptions.modality = kWindowModalityAppModal;
	
	err = NavCreateAskSaveChangesDialog(&dialogOptions,
		quitting ?
		     kNavSaveChangesQuittingApplication : kNavSaveChangesClosingDocument,
		NULL, NULL, &saveDialog);
	require_noerr(err, error);

	NavDialogSetSaveFileName(saveDialog, fileName);
	
	// run Save Changes dialog
	err = NavDialogRun(saveDialog);
	require_noerr(err, error);

	// get user action
	userAction = NavDialogGetUserAction(saveDialog);
	if (userAction == kNavUserActionNone || userAction == kNavUserActionCancel) {
		err = userCanceledErr;
		goto error;
	}

	path[0] = '\0';
	if (userAction == kNavUserActionSaveChanges)
		OPL_appendFileName(path, path_len, fileName);
	
error:
	if (saveDialog != NULL)
		NavDialogDispose(saveDialog);

	return err == noErr;
}
