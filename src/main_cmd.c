/*
 *  main_cmd.c
 * 
 *  $Id$
 *
 *  odbc-bench - a TPCA and TPCC benchmark program for databases 
 *  Copyright (C) 2000-2002 OpenLink Software <odbc-bench@openlinksw.com>
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
#include <string.h>

#include <stdarg.h>

#include "odbcbench.h"
#include "results.h"
#include "tpca_code.h"
#include "tpccfun.h"
#include "thr.h"

GUI_t gui;

static void 
err_message(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

static void 
warn_message(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  vprintf(format, args);
  va_end(args);
}

static void 
message(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  vprintf(format, args);
  va_end(args);
}

static void
do_pane_log (const char *format, ...)
{
}

void (*pane_log) (const char *format, ...) = do_pane_log;

BOOL
isCancelled(void)
{
  return FALSE;
}


extern int do_command_line (int argc, char *argv[]);
extern void stdout_markfinished (int nConn, int nThread);


int
main (int argc, char *argv[])
{

  do_alloc_env ();

  memset(&gui, 0, sizeof(gui));
  gui.main_quit = NULL;
  gui.err_message = err_message; 
  gui.warn_message = warn_message; 
  gui.message = message; 
  gui.add_test_to_the_pool = NULL;//add_test_to_the_pool;
  gui.for_all_in_pool = NULL;//for_all_in_pool;
  gui.do_MarkFinished = stdout_markfinished;

  gui.isCancelled = isCancelled;
  gui.ShowProgress = NULL;//do_ShowProgress;
  gui.SetWorkingItem = NULL;//do_SetWorkingItem;
  gui.SetProgressText = NULL;//do_SetProgressText;
  gui.StopProgress = NULL;//stdout_StopProgress;
  gui.fCancel = NULL;//do_fCancel;
  gui.vBusy = NULL;//vBusy;  

  return do_command_line (argc, argv);
}