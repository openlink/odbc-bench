/*
 *  isql_interface.c
 * 
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases 
 *  Copyright (C) 2000-2003 OpenLink Software <odbc-bench@openlinksw.com>
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

#ifndef GTKBENCH_DEF_DIR
#ifdef WIN32
#define GTKBENCH_DEF_DIR "c:"
#else
#define GTKBENCH_DEF_DIR "/usr/lib/gtkbench"
#endif
#endif

static FILE *
open_file (char *szFileName, char *szMode)
{
  int nFileName = strlen (szFileName);
  char *szNewName = NULL, *szHelper;
  FILE *fi = NULL;

  if (NULL != (fi = fopen (szFileName, szMode))) /* straight */
    return fi;

  if (NULL != (szHelper = getenv ("GTKBENCH")))
    { /* from the environment */
      szNewName = malloc (nFileName + strlen (szHelper) + 2);
      sprintf (szNewName, "%s/%s", szHelper, szFileName);
      fi = fopen (szNewName, szMode);
      XFREE (szNewName);
      if (fi)
	return fi;
    }
#if 0
  { /* registry */
    HKEY GtkKey;
    DWORD Type, datasize = 0;
    if (ERROR_SUCCESS == RegOpenKey (HKEY_LOCAL_MACHINE, "SOFTWARE/OpenLink Software/GTKBench", &GtkKey))
      {
	if (ERROR_SUCCESS == RegQueryValueEx (GtkKey, "ScriptDir", NULL, &Type, NULL, &datasize) &&
	    Type == REG_SZ)
	  {
	    szNewName = g_malloc0 (nFileName + datasize + 2);
	    if (ERROR_SUCCESS == RegQueryValueEx (GtkKey, "ScriptDir", NULL, NULL, szNewName, NULL))
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
    return fi;
  }
#endif
  szNewName = calloc(1, nFileName + strlen (GTKBENCH_DEF_DIR) + 2);
  sprintf (szNewName, "%s/%s", GTKBENCH_DEF_DIR, szFileName);
  fi = fopen (szNewName, szMode);
  XFREE (szNewName);
  return fi;
}


void
pipe_trough_isql (HDBC hdbc, char *szFileName, int print_commands)
{
  char szLine[1024];
  long cmd_length = 0;
  OSList *gs = NULL;
  RETCODE rc;
  FILE *fi = open_file (szFileName, "rt");
  HSTMT hstmt;

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
	      char *szCommand = malloc (cmd_length + 1), *szPtr = szCommand;
	      OSList *iter = gs;
	      memset (szCommand, 0, cmd_length + 1);
	      while (iter)
		{
		  int line_len = strlen (iter->data);
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
	      rc = SQLExecDirect (hstmt, szCommand, SQL_NTS);
	      XFREE (szCommand);
	      if (SQL_SUCCESS != rc)
		vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);
	      SQLFreeStmt (hstmt, SQL_CLOSE);
	    }
	}
      else
	{
	  char *szStr = szLine, *szLineToPush;
	  int line_len = strlen (szLine);
	  while (*szStr && *szStr < 0x020)
	    szStr++;
	  if (szStr[0] && szStr[0] == '-' && szStr[1] && szStr[1] == '-')
	    continue;		/* on comment */
	  szLineToPush = malloc (line_len + 1);
	  memcpy (szLineToPush, szLine, line_len + 1);
	  gs = o_slist_append(gs, szLineToPush);
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
