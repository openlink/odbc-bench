/*
 *  command_line.c
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
#ifndef WIN32
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#endif
#if defined (linux)
#include <getopt.h>
#elif defined (WIN32)
#include "win32/getopt.h"
#endif

#include <stdarg.h>

#include "odbcbench.h"
#include "odbcinc.h"
#include "thr.h"
#include "tpca_code.h"

static int verbose = 0;

extern int messages_off;
extern int do_rollback_on_deadlock;

#define SHORT_OPTIONS "d:u:p:r:m:t:s:n1avVRc:i:S:K:T:C"

static void
usage (void)
{
  fputs ("\nUsage :\n\n", stderr);
  fputs ("\t-d\t- login dsn\n", stderr);
  fputs ("\t-u\t- user id\n", stderr);
  fputs ("\t-p\t- password\n", stderr);
  fputs ("\t-r\t- number of runs (default 1)\n", stderr);
  fputs ("\t-m\t- duration of the run (mins) (default 5)\n", stderr);
  fputs ("\t-t\t- number of threads (default 1)\n", stderr);
  fputs ("\t-s\t- exec | proc | prepare\n", stderr);
  fputs ("\t-n\t- don't try to use transactions\n", stderr);
  fputs ("\t-1\t- do 100 row query\n", stderr);
  fputs ("\t-a\t- asynchronous execution\n", stderr);
  fputs ("\t-v\t- print additional info on stdout\n", stderr);
  fputs ("\t-V\t- print debug info on stdout\n", stderr);
  fputs ("\t-R\t- doesn't do rollbacks on deadlock\n", stderr);
  fputs
      ("\t-c\t- cursor type = forward | static | keyset | dynamic | mixed\n",
      stderr);
  fputs
      ("\t-i\t- transaction isolation level = uncommitted | committed | repeatable | serializable\n",
      stderr);
  fputs ("\t-S\t- rowset size\n", stderr);
  fputs ("\t-K\t- keyset size\n", stderr);
  fputs ("\t-T\t- traversal count\n", stderr);
  fputs ("\t-C\t- create the tables\n", stderr);
  fputs ("\n", stderr);
}

static void
stdout_pane_log (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  vprintf (format, args);
  va_end (args);
}

static void
stdout_showprogress (void * parent_win, char * title, int nThreads,
    float nMax)
{
  printf ("\n%s\n", title);
}

static void
stdout_setprogresstext (char *pszProgress, int n_conn, int thread_no,
    float nValue, int nTrnPerCall)
{
  static BOOL bLEFT = TRUE;
  fputc (bLEFT ? '(' : ')', stderr);
  bLEFT = bLEFT ? FALSE : TRUE;
  fflush (stderr);
  fputc ('\b', stderr);
}

static void
dummy_pane_log (const char *format, ...)
{
}

static void
dummy_showprogress (void * parent_win, char * title, int nThreads,
    float nMax)
{
}

static void
dummy_setworkingitem (char *pszWorking)
{
}

static void
dummy_setprogresstext (char *pszProgress, int n_conn, int thread_no,
    float nValue, int nTrnPerCall)
{
}

static void
dummy_stopprogress (void)
{
}

static int
dummy_fcancel (void)
{
  return FALSE;
}


int
do_command_line (int argc, char *argv[])
{
  int curr_opt;
  int Load = 0;

  if (argc > 1)
    {
      test_t test;
      OList *tests = NULL;

      memset (&test, 0, sizeof (test));
      test.TestType = TPC_A;
      strcpy (test.szName, "CommandLine");
      init_test (&test);
      gui.for_all_in_pool ();
      tests = o_list_append (tests, &test);
      test.ShowProgress = dummy_showprogress;
      test.SetWorkingItem = dummy_setworkingitem;
      test.SetProgressText = dummy_setprogresstext;
      test.StopProgress = dummy_stopprogress;
      test.fCancel = dummy_fcancel;
      test.tpc.a.fUseCommit = TRUE;
      test.tpc.a.fExecAsync = FALSE;
      test.tpc.a.fDoQuery = FALSE;
      test.tpc.a.fSQLOption = -1;

      test.tpc.a.nTraversalCount = 1;
      pane_log = dummy_pane_log;
      memset (&test.szSQLError, 0, sizeof (test.szSQLError));

      while ((curr_opt = getopt (argc, argv, SHORT_OPTIONS)) != EOF)
	switch (curr_opt)
	  {
	  case 'd':
	    strncpy (test.szLoginDSN, optarg, 49);
	    test.szLoginDSN[49] = 0;
	    break;

	  case 'u':
	    strncpy (test.szLoginUID, optarg, 49);
	    test.szLoginUID[49] = 0;
	    break;

	  case 'p':
	    strncpy (test.szLoginPWD, optarg, 49);
	    test.szLoginPWD[49] = 0;
	    break;

	  case 'r':
	    test.tpc._.nRuns = atoi (optarg);
	    if (test.tpc._.nRuns < 1)
	      test.tpc._.nRuns = 1;
	    break;

	  case 'm':
	    test.tpc._.nMinutes = atoi (optarg);
	    if (test.tpc._.nMinutes < 1)
	      test.tpc._.nMinutes = 5;
	    break;

	  case 't':
	    test.tpc._.nThreads = atoi (optarg);
	    if (test.tpc._.nThreads < 0)
	      test.tpc._.nThreads = 0;
	    break;

	  case 's':
	    if (!strncmp (optarg, "proc", 4))
	      test.tpc.a.fSQLOption = IDX_SPROCS;
	    else if (!strncmp (optarg, "prepare", 7))
	      test.tpc.a.fSQLOption = IDX_PARAMS;
	    else if (!strncmp (optarg, "exec", 4))
	      test.tpc.a.fSQLOption = IDX_PLAINSQL;
	    else
	      {
		fprintf (stdout, "Unknown test type : %s\n", optarg);
		usage ();
		return -1;
	      }
	    break;

	  case 'n':
	    test.tpc.a.fUseCommit = FALSE;
	    break;
	  case '1':
	    test.tpc.a.fDoQuery = TRUE;
	    break;
	  case 'a':
	    test.tpc.a.fExecAsync = TRUE;
	    break;

	  case 'R':
	    do_rollback_on_deadlock = 0;
	    break;
	  case 'V':
	    messages_off = 0;
	    test.SetProgressText = stdout_setprogresstext;
	  case 'v':
	    verbose = 1;
	    pane_log = stdout_pane_log;
	    test.ShowProgress = stdout_showprogress;
	    break;

	  case 'c':
	    if (!strcmp (optarg, "forward"))
	      test.tpc.a.nCursorType = SQL_CURSOR_FORWARD_ONLY;
	    else if (!strcmp (optarg, "static"))
	      test.tpc.a.nCursorType = SQL_CURSOR_STATIC;
	    else if (!strcmp (optarg, "keyset"))
	      test.tpc.a.nCursorType = SQL_CURSOR_KEYSET_DRIVEN;
	    else if (!strcmp (optarg, "dynamic"))
	      test.tpc.a.nCursorType = SQL_CURSOR_DYNAMIC;
	    else if (!strcmp (optarg, "mixed"))
	      test.tpc.a.nCursorType = SQL_CURSOR_MIXED;
	    else
	      {
		fprintf (stdout, "Unknown cursor type : %s\n", optarg);
		usage ();
		return -1;
	      }
	    break;

	  case 'i':
	    if (!strcmp (optarg, "uncommitted"))
	      test.tpc.a.txn_isolation = SQL_TXN_READ_UNCOMMITTED;
	    else if (!strcmp (optarg, "committed"))
	      test.tpc.a.txn_isolation = SQL_TXN_READ_COMMITTED;
	    else if (!strcmp (optarg, "repeatable"))
	      test.tpc.a.txn_isolation = SQL_TXN_REPEATABLE_READ;
	    else if (!strcmp (optarg, "serializable"))
	      test.tpc.a.txn_isolation = SQL_TXN_SERIALIZABLE;
	    else
	      {
		fprintf (stdout, "Unknown isolation level : %s\n", optarg);
		usage ();
		return -1;
	      }
	    break;

	  case 'S':
	    test.tpc.a.nRowsetSize = atoi (optarg);
	    break;

	  case 'K':
	    test.tpc.a.nKeysetSize = atoi (optarg);
	    break;

	  case 'T':
	    test.tpc.a.nTraversalCount = atoi (optarg);
	    break;

	  case 'C':
	    Load = 1;
	    break;
	  default:
	    usage ();
	    return -2;
	  };
      if (Load)
	{
	  do_login (&test);
	  get_dsn_data (&test);
	  if (!test.hdbc)
	    {
	      fprintf (stdout, "%s\n", test.szSQLError);
	      fprintf (stderr, "%s\n", test.szSQLError);
	      return -5;
	    }
	  fBuildBench (&test);
	  do_logout (&test);
	  return 0;
	}

#if defined(PTHREADS) || defined(WIN32)
      if (test.tpc._.nThreads > 1)
	{
	  do_login (&test);
	  get_dsn_data (&test);
	  if (!test.hdbc)
	    {
	      fprintf (stderr, "%s\n", test.szSQLError);
	      fprintf (stdout, "%s\n", test.szSQLError);
	      return -5;
	    }
	  do_logout (&test);
	  if (test.tpc.a.fSQLOption != -1)
	    {
	      int rc = do_threads_run (1, tests, test.tpc._.nMinutes,
		  "Command Line");
	      do_save_run_results ("results.xml", tests, test.tpc._.nMinutes);
	      if (rc)
		return 0;
	      else
		return -4;
	    }
	  else
	    {
	      if (do_threads_run_all (1, tests, test.tpc._.nMinutes,
		      "results.xml"))
		return 0;
	      else
		return -4;
	    }

	}
      else
#endif
	{
	  do_login (&test);
	  get_dsn_data (&test);
	  if (!test.hdbc)
	    {
	      fprintf (stderr, "%s\n", test.szSQLError);
	      fprintf (stdout, "%s\n", test.szSQLError);
	      return -5;
	    }
	  else
	    DoRun (&test, NULL);
	  do_save_run_results ("results.xml", tests, test.tpc._.nMinutes);
	  do_logout (&test);
	}
      return 0;
    }
  else
    return 0;
}
