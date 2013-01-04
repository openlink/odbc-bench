/*
 *  LoginDialog.cpp
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

#include "odbcbench.h"

#include <sys/stat.h>
#include <dirent.h>

#include "LoginDialog.h"
#include "util.h"

ControlID kLoginDSN = { 'LOGN', 0 };
ControlID kLoginUid = { 'LOGN', 1 };
ControlID kLoginPwd = { 'LOGN', 2 };

ControlID kLoginDSNL = { 'LOGN', 3 };
ControlID kLoginFDSNL = { 'LOGN', 4 };

ControlID kLoginTABS = { 'LOGN', 5 };
ControlID kLoginDSN_TAB = { 'LOGN', 6 };
ControlID kLoginFDSN_TAB = { 'LOGN', 7 };

ControlID kLoginDIR = { 'LOGN', 8 };


// DSN List view column ids
const DataBrowserPropertyID kItemViewDSNL = 'DSN ';
// FDSN List view column ids
const DataBrowserPropertyID kItemViewFDSNL = 'FDSN';



static OSStatus
OPL_DSNItemViewItemDataCallback(ControlRef itemView, DataBrowserItemID itemID, 
	DataBrowserPropertyID property, DataBrowserItemDataRef itemData, 
	Boolean changeValue)
{  
    OSStatus err;
    CFStringRef dsn;
    static IconRef folderIcon = 0;
    static IconRef dsnIcon = 0;
    IconRef icon = 0;
	
    if (changeValue)
      return errDataBrowserPropertyNotSupported;

    // Get TestPool instance
    OPL_DSNList *dsnlist = OPL_DSNList::get(itemView);
    if (!dsnlist)
      return errDataBrowserPropertyNotSupported;

    // Get test item
    dsn = dsnlist->getItem(itemID);
    CFStringRef str;
	
    switch (property) {
      case kItemViewDSNL:
	str = dsn;
	break;

      case kItemViewFDSNL:
        if (folderIcon == 0)
          GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericFolderIcon, &folderIcon);
        if (dsnIcon == 0)
          GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericDocumentIcon, &dsnIcon);

        icon = dsnlist->getItemType(itemID) ? folderIcon : dsnIcon;
	str = dsn;
	break;

      default:
	return errDataBrowserPropertyNotSupported;
    }

    // resize columns
    SInt16 outBaseline;
    Point ioBound;
    err = GetThemeTextDimensions(str, kThemeSystemFont, kThemeStateActive,
		false, &ioBound, &outBaseline);
    require_noerr(err, error);

    UInt16 width;
    err = GetDataBrowserTableViewNamedColumnWidth(itemView, property, &width);
    require_noerr(err, error);

    if (width < ioBound.h + 20) {
	err = SetDataBrowserTableViewNamedColumnWidth(itemView,
		property, ioBound.h + 20);
	require_noerr(err, error);
    }

    // set item data text
    err = SetDataBrowserItemDataText(itemData, str);
    require_noerr(err, error);

    if (icon)
      SetDataBrowserItemDataIcon(itemData, icon);

error:
    return err;
}


static void
OPL_DSN_notification_item (ControlRef browser,
    DataBrowserItemID itemID, DataBrowserItemNotification message)
{
  OSStatus err;
  WindowRef window;

  window = GetFrontWindowOfClass(kMovableModalWindowClass, false);
  if (window == NULL)
    return;

  // get LoginDialog instance
  OPL_LoginDialog *dlg = NULL;
  err = GetWindowProperty(window, kPropertyCreator, kPropertyTag,
     sizeof(dlg), NULL, &dlg);
  require_noerr(err, error);

  if (!dlg)
    return;

  switch (message)
    {
    case kDataBrowserItemDoubleClicked:
      dlg->dir_dblclick();
      break;
    };

error:
  return;
}



OPL_LoginDialog::OPL_LoginDialog(CFStringRef resname, CFStringRef title): 
  OPL_Dialog(resname), cur_dir(NULL), tab_number(-1)
{
    OSStatus err;
    OPL_LoginDialog *dlg = this;

    SQLSetConfigMode (ODBC_BOTH_DSN);
    if (!SQLGetPrivateProfileString("ODBC", "FileDSNPath", "",
         def_path, sizeof(def_path), "odbcinst.ini"))
#if defined(DEFAULT_FILEDSNPATH)
      strcpy(def_path, DEFAULT_FILEDSNPATH);
#else
      strcpy(def_path, "/etc/ODBCDataSources");
#endif

    setTitle(title);

    getControl(kLoginTABS, &tab_panel);
    getControl(kLoginDSN_TAB, &tabs[0]);
    getControl(kLoginFDSN_TAB, &tabs[1]);

    getControl(kLoginDSNL, &dsnView);
    getControl(kLoginFDSNL, &fdsnView);

    // create instance data and associate it with the control
    dsnlist = new OPL_DSNList(dsnView);
    err = SetControlProperty(dsnView, kPropertyCreator, kPropertyTag,
  	    sizeof(dsnlist), &dsnlist);
    require_noerr(err, error);

    fdsnlist = new OPL_DSNList(fdsnView);
    err = SetControlProperty(fdsnView, kPropertyCreator, kPropertyTag,
  	    sizeof(fdsnlist), &fdsnlist);
    require_noerr(err, error);


    // set DSN browser callbacks
    DataBrowserCallbacks dsnViewCallbacks;
    dsnViewCallbacks.version = kDataBrowserLatestCallbacks;
    err = InitDataBrowserCallbacks(&dsnViewCallbacks);
	require_noerr(err, error);
	
    static DataBrowserItemDataUPP g_dsnItemDataUPP = NULL;
    if (!g_dsnItemDataUPP) {
      g_dsnItemDataUPP = NewDataBrowserItemDataUPP(
          OPL_DSNItemViewItemDataCallback);
    }
	
    dsnViewCallbacks.u.v1.itemDataCallback = g_dsnItemDataUPP;
    err = SetDataBrowserCallbacks(dsnView, &dsnViewCallbacks); 
    require_noerr(err, error);

    // set FDSN browser callbacks
    DataBrowserCallbacks fdsnViewCallbacks;
    fdsnViewCallbacks.version = kDataBrowserLatestCallbacks;
    err = InitDataBrowserCallbacks(&fdsnViewCallbacks);
	require_noerr(err, error);
	
    static DataBrowserItemDataUPP g_fdsnItemDataUPP = NULL;
    if (!g_fdsnItemDataUPP) {
      g_fdsnItemDataUPP = NewDataBrowserItemDataUPP(
          OPL_DSNItemViewItemDataCallback);
    }

    static DataBrowserItemNotificationUPP g_fdsnItemNotificationUPP = NULL;
    if (!g_fdsnItemNotificationUPP) {
      g_fdsnItemNotificationUPP = NewDataBrowserItemNotificationUPP(
          OPL_DSN_notification_item);
    }
	
    fdsnViewCallbacks.u.v1.itemNotificationCallback = g_fdsnItemNotificationUPP;
    fdsnViewCallbacks.u.v1.itemDataCallback = g_fdsnItemDataUPP;
    err = SetDataBrowserCallbacks(fdsnView, &fdsnViewCallbacks); 
    require_noerr(err, error);

    SetWindowProperty(getWindow(), kPropertyCreator, kPropertyTag,
	sizeof(dlg), &dlg);

    DisplayTabControl (0, tab_panel, 2, tabs);

    loadDSNList();
    loadFDSNList(def_path);

error:
    return;
}


void 
OPL_LoginDialog::SetConnectAttr(char *dsn, char *uid, char *pwd)
{
    setEditText(kLoginDSN, OPL_char_to_CFString(dsn));
    setEditText(kLoginUid, OPL_char_to_CFString(uid));
    setEditText(kLoginPwd, OPL_char_to_CFString(pwd));
}


char * 
OPL_LoginDialog::GetDSN()
{
    return OPL_CFString_to_char(getEditText(kLoginDSN));
}


char * 
OPL_LoginDialog::GetUID()
{
    return OPL_CFString_to_char(getEditText(kLoginUid));
}


char * 
OPL_LoginDialog::GetPWD()
{
    return OPL_CFString_to_char(getEditText(kLoginPwd));
}


OPL_LoginDialog::~OPL_LoginDialog()
{
  RemoveControlProperty(dsnView, kPropertyCreator, kPropertyTag);
  RemoveControlProperty(fdsnView, kPropertyCreator, kPropertyTag);
  RemoveWindowProperty(getWindow(), kPropertyCreator, kPropertyTag);

  if (dsnlist)
    delete dsnlist;

  if (fdsnlist)
    delete fdsnlist;
}


void
OPL_LoginDialog::loadDSNList()
{
  SQLCHAR szDSN[SQL_MAX_DSN_LENGTH + 1];
  SQLSMALLINT nDSN;
  SQLRETURN retval;
	
  // append values
  for (retval = SQLDataSources(henv, SQL_FETCH_FIRST,
			szDSN, sizeof(szDSN), &nDSN, NULL, 0, NULL);
	  retval == SQL_SUCCESS;
	  retval = SQLDataSources(henv, SQL_FETCH_NEXT,
				szDSN, sizeof(szDSN), &nDSN, NULL, 0, NULL)) 
    {
      szDSN[nDSN] = 0;
      dsnlist->addItem(OPL_char_to_CFString((char *) szDSN));
    }
}

void
OPL_LoginDialog::loadFDSNList(char *dsn_path)
{
  char *path_buf;
  DIR *dir;
  struct dirent *dir_entry;
  struct stat fstat;
	
  fdsnlist->clear();
  if ((dir = opendir(dsn_path)))
    {
      while (dir_entry = readdir(dir))
        {
	  asprintf (&path_buf, "%s/%s", dsn_path, dir_entry->d_name);

	  if (stat ((LPCSTR) path_buf, &fstat) >= 0 && S_ISDIR (fstat.st_mode))
	    {
	      if (dir_entry->d_name && dir_entry->d_name[0] != '.') 
	        {
	          fdsnlist->addItem(OPL_char_to_CFString((char *) dir_entry->d_name), 1);
	        }
	    }
	  else if (stat ((LPCSTR) path_buf, &fstat) >= 0 && !S_ISDIR (fstat.st_mode)
	           && strstr (dir_entry->d_name, ".dsn"))
	    {
	      fdsnlist->addItem(OPL_char_to_CFString((char *) dir_entry->d_name), 0);
	    }

	  if (path_buf)
	    free (path_buf);
	}
      /* Close the directory entry */
      closedir (dir);
    }

  SetDataBrowserScrollPosition(fdsnView, 0, 0);

  //fill dir menu
  fill_dir_menu(dsn_path);
}


void
OPL_LoginDialog::fill_dir_menu(char *path)
{
  char *tmp_dir, *prov, *dir;
  MenuRef items_m;
  int i = -1;
  ControlRef f_select;
  
  getControl(kLoginDIR, &f_select);

  if (!path || !(prov = strdup (path)))
    return;

  if (prov[strlen(prov) - 1] == '/' && strlen(prov) > 1)
    prov[strlen(prov) - 1] = 0;

  items_m = GetControlPopupMenuHandle (f_select);
  DeleteMenuItems (items_m, 1, CountMenuItems (items_m));

  /* Add the root directory */
  AppendMenuItemTextWithCFString(items_m, CFSTR("/"), 0, 0, NULL);

  for (tmp_dir = prov, dir = NULL; tmp_dir;
      tmp_dir = strchr (tmp_dir + 1, '/'))
    {
      if (strchr (tmp_dir + 1, '/'))
	{
	  dir = strchr (tmp_dir + 1, '/');
	  *dir = 0;
	}

      AppendMenuItemTextWithCFString(items_m, OPL_char_to_CFString(prov), 0, 0, NULL);

      if (dir)
	*dir = '/';
    }
  i = CountMenuItems (items_m);
  SetControlMaximum (f_select, i);
  SetControlValue (f_select, i);
  if (cur_dir)
    free(cur_dir);
  cur_dir = prov;
}


void 
OPL_LoginDialog::dir_dblclick()
{
  char *dsn;
  char tmp_path[1024];
  UInt32 idx;

  idx = fdsnlist->getSelectedItem();
  if (fdsnlist->getItemType(idx))
    {
      //change directory
      dsn = OPL_CFString_to_char(fdsnlist->getItem(idx));
      snprintf(tmp_path, sizeof(tmp_path), "%s/%s", cur_dir, dsn);
      loadFDSNList(tmp_path);
      free(dsn);
    }
}


OSStatus 
OPL_LoginDialog::handleCommandEvent(UInt32 commandID)
{
    UInt32 idx;
    CFStringRef str;
    char *dsn;
    char tmp_path[1024];

    switch (commandID) {
      case 'SDIR':
        ControlRef f_select;
        CFStringRef menuText;
        MenuRef menu;

        getControl(kLoginDIR, &f_select);
        menu = GetControlPopupMenuHandle(f_select);
        idx = GetControlValue (f_select);
        CopyMenuItemTextAsCFString(menu, idx, &menuText);
        CFStringGetCString(menuText, tmp_path, sizeof(tmp_path), kCFStringEncodingUTF8);
        CFRelease(menuText);

        loadFDSNList(tmp_path);
	return noErr;
			
      case 'SFDS':
        idx = fdsnlist->getSelectedItem();
        if (idx && !fdsnlist->getItemType(idx))
          { // set DSN
            dsn = OPL_CFString_to_char(fdsnlist->getItem(idx));
            str = OPL_CFString_asprintf("%s/%s", cur_dir, dsn);
            setEditText(kLoginDSN, str);
            free(dsn);
          }
        return noErr;

      case 'SDS ':
        idx = dsnlist->getSelectedItem();
        if (idx)
          setEditText(kLoginDSN, CFStringCreateCopy(NULL, dsnlist->getItem(idx)));
        return noErr;

      default:
        return OPL_Dialog::handleCommandEvent(commandID);
    }
}


OSStatus 
OPL_LoginDialog::handleControlHit(ControlRef cntl)
{
  OSStatus err = eventNotHandledErr;

  if (!cntl)
    return err;

  if (cntl == tab_panel)
  {
    int tab_index;

    /* Search which tab is activated */
    tab_index = GetControlValue (tab_panel) - 1;
    DisplayTabControl (tab_index, tab_panel, 2, tabs);

    /* Is the panel has been changed */
    if (tab_number == tab_index)
      return noErr;

    tab_number = tab_index;
    ClearKeyboardFocus (getWindow());
    AdvanceKeyboardFocus (getWindow());
    return noErr;
  } 

error:
  return err;
}
