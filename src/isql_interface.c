/*
 *  isql_interface.c
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2019 OpenLink Software <odbc-bench@openlinksw.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "odbcbench.h"

#define SCRIPT_DELIMITER ';'

#ifndef ODBCBENCH_DEF_DIR
# ifdef WIN32
#  define ODBCBENCH_DEF_DIR "C:/ODBCBench"
# else
#  define ODBCBENCH_DEF_DIR "/usr/lib/odbcbench"
# endif
#endif


static FILE *
open_file (char *szFileName, char *szMode)
{
  size_t nFileName = strlen (szFileName);
  char *szNewName = NULL, *szHelper;
  FILE *fi = NULL;

  /*
   *  File in current directory
   */
  if (NULL != (fi = fopen (szFileName, szMode)))
    return fi;


  /*
   * File in environment  $ODBCBENCH
   */
  if (NULL != (szHelper = getenv ("ODBCBENCH")))
    {
      szNewName = (char *) malloc (nFileName + strlen (szHelper) + 2);
      sprintf (szNewName, "%s/%s", szHelper, szFileName);
      fi = fopen (szNewName, szMode);
      XFREE (szNewName);
      if (fi)
	return fi;
    }


  /*
   *  Via registery
   */
#ifdef WIN32
  {
    HKEY GtkKey;
    DWORD Type, datasize = 0;
    if (ERROR_SUCCESS == RegOpenKey (HKEY_LOCAL_MACHINE,
	    "SOFTWARE/OpenLink Software/ODBCBench", &GtkKey))
      {
	if (ERROR_SUCCESS == RegQueryValueEx (GtkKey, "ScriptDir", NULL,
		&Type, NULL, &datasize) && Type == REG_SZ)
	  {
	    szNewName = g_malloc0 (nFileName + datasize + 2);
	    if (ERROR_SUCCESS == RegQueryValueEx (GtkKey, "ScriptDir", NULL,
		    NULL, szNewName, NULL))
	      {
		strcat (szNewName, "\\");
		strcat (szNewName, szFileName);
	      }
	    else
	      {
		g_free (szNewName);
		szNewName = NULL;
	      }
	  }
	RegCloseKey (GtkKey);
      }
    if (szNewName)
      {
	fi = fopen (szNewName, szMode);
	g_free (szNewName);
	szNewName = NULL;
      }
    if (fi)
      return fi;
  }
#endif

  /*
   *  Compiled in default
   */
  szNewName = (char *) malloc (nFileName + strlen (ODBCBENCH_DEF_DIR) + 2);
  sprintf (szNewName, "%s/%s", ODBCBENCH_DEF_DIR, szFileName);
  fi = fopen (szNewName, szMode);
  XFREE (szNewName);

  return fi;
}


void
pipe_trough_isql (SQLHDBC hdbc, char *szFileName, int print_commands)
{
  char szLine[1024];
  size_t cmd_length = 0;
  OSList *gs = NULL;
  RETCODE rc;
  FILE *fi = open_file (szFileName, "rt");
  SQLHSTMT hstmt;

  if (!fi)
    {
      pane_log ("Unable to open the script file %s\r\n", szFileName);
      return;
    }
  if (!hdbc)
    {
      pane_log ("Not connected\r\n");
      return;
    }
  if (SQL_SUCCESS != SQLAllocStmt (hdbc, &hstmt))
    {
      vShowErrors (NULL, SQL_NULL_HENV, hdbc, SQL_NULL_HSTMT, NULL);
      return;
    }

  while (fgets (szLine, sizeof (szLine), fi))
    {
      if (szLine[0] == SCRIPT_DELIMITER && szLine[1] < 0x020)
	{
	  if (cmd_length)
	    {
	      char *szCommand = (char *) malloc (cmd_length + 1), *szPtr =
		  szCommand;
	      OSList *iter = gs;
	      memset (szCommand, 0, cmd_length + 1);
	      while (iter)
		{
		  size_t line_len = strlen ((char *) iter->data);
		  if (line_len)
		    {
		      memcpy (szPtr, iter->data, line_len);
		      cmd_length -= line_len;
		      szPtr += line_len;
		    }
		  XFREE (iter->data);
		  iter = o_slist_next (iter);
		}
	      o_slist_free (gs);
	      gs = NULL;
	      assert (!cmd_length);
	      if (print_commands)
		{
		  if (gui.message)
		    gui.message ("%s", szCommand);
		}
	      rc = SQLExecDirect (hstmt, (SQLCHAR *) szCommand, SQL_NTS);
	      XFREE (szCommand);
	      if (SQL_SUCCESS != rc)
		vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);
	      SQLFreeStmt (hstmt, SQL_CLOSE);
	    }
	}
      else
	{
	  char *szStr = szLine, *szLineToPush;
	  size_t line_len = strlen (szLine);
	  while (*szStr && *szStr < 0x020)
	    szStr++;
	  if (szStr[0] && szStr[0] == '-' && szStr[1] && szStr[1] == '-')
	    continue;		/* on comment */
	  szLineToPush = (char *) malloc (line_len + 1);
	  memcpy (szLineToPush, szLine, line_len + 1);
	  gs = o_slist_append (gs, szLineToPush);
	  cmd_length += line_len;
	}
    }
  {
    OSList *iter = gs;
    while (iter)
      {
	XFREE (iter->data);
	iter = o_slist_next (iter);
      };
    o_slist_free (gs);
  }
  fclose (fi);
  SQLFreeStmt (hstmt, SQL_DROP);
}
