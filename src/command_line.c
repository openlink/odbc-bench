/*
 *  command_line.c
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2018 OpenLink Software <odbc-bench@openlinksw.com>
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

#include <getopt.h>

#include <stdarg.h>

#include "odbcbench.h"
#include "odbcinc.h"
#include "thr.h"
#include "tpca_code.h"
#include "results.h"

static int verbose = 0;

extern int messages_off;
extern int do_rollback_on_deadlock;

/* progress impl */
static int n_threads, n_connections = 0;
static float spec_time_secs = -1;
static float *pTrnTimes = NULL;
static testtype test_types = TPC_A;
static float *fOldValues = NULL;
static unsigned long curr_time_msec = 0L;
static double *ptpca_dDiffSum = NULL;

#define BARS_REFRESH_INTERVAL bench_get_long_pref (DISPLAY_REFRESH_RATE)


#define SHORT_OPTIONS "d:u:p:r:m:t:P:s:c:i:S:K:T:x1avVRCD"

static void
usage (void)
{
  fprintf (stderr, "%s\n", PACKAGE_STRING);
  fprintf (stderr, "Copyright (C) 2000-2018 OpenLink Software\n");
  fprintf (stderr, "Please report all bugs to <%s>\n\n", PACKAGE_BUGREPORT);
  fprintf (stderr,
      "This utility is released under the GNU General Public License (GPL)\n\n");
  fprintf (stderr,
      "Disclaimer: The benchmarks in this application are loosely based\n"
      "on the TPC-A and TPC-C standard benchmarks, but this application\n"
      "does not claim to be a full or precise implementation, nor are\n"
      "the results obtained by this application necessarily comparable\n"
      "to the vendor's published results.\n\n");

  fputs ("\nUsage :\n\n"
      "  -d -dsn            - DSN to test\n"
      "  -u -uid            - user id for test DSN (default \"\")\n"
      "  -p -pwd            - password for test DSN (default \"\")\n"
      "  -C -create_tables  - create the tables and procedures, then exit\n"
      "                       (default 'don't create the tables and procedures')\n"
      "  -D -drop_tables    - drop the tables and procedures, then exit\n"
      "                       (default 'don't drop the tables and procedures')\n"
      "  -r -runs           - number of runs (default '1')\n"
      "  -m -time           - duration of the run (mins) (default '1')\n"
      "  -t -threads        - number of threads (default '1')\n"
      "  -P -arrayparm      - size for used array of parameters (default '1') \n"
      "  -s -test           - test type = exec | proc | prepare | runall \n"
      "                                 (default 'exec')\n"
      "  -x -txn            - use transactions (default 'do not use transactions')\n"
      "  -1 -query          - do 100 row query (default 'do not run 100 row query')\n"
      "  -a -async          - asynchronous execution (default 'synchronous execution')\n"
      "  -c -cursor_type    - cursor type = forward | static | keyset \n"
      "                                   | dynamic | mixed \n"
      "                                   (default 'forward')\n"
      "  -i -txn_isolation  - transaction isolation level = uncommitted | committed \n"
      "                                                   | repeatable | serializable\n"
      "                                                   (default 'Driver Default')\n"
      "  -S -rowset_size    - rowset size (default '1')\n"
      "  -K -keyset_size    - keyset size (default '1')\n"
      "  -T -trav_count     - traversal count (default '1')\n"
      "  -rdsn              - DSN for results\n"
      "  -ruid              - user id for results DSN (default \"\")\n"
      "  -rpwd              - password for results DSN (default \"\")\n"
      "  -rcreate_table     - create the results table, then exit\n"
      "  -rdrop_table       - drop the results table, then exit\n"
      "  -rfile             - output filename for XML results data\n"
      "  -v                 - print info on stdout (error messages, test results,\n"
      "                       messages about opening and closing connections)\n"
      "                       (default 'print nothing to stdout')\n"
      "  -V                 - same as -v, plus test progress details\n"
      "                       (default 'don't print test progress to stdout')\n"
      "  -R                 - don't do rollbacks on deadlock \n"
      "                       (default 'rollback transactions on deadlock')\n" "\n", stderr);
}


static void
stdout_pane_log (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  MUTEX_ENTER (log_mutex);
  vprintf (format, args);
  MUTEX_LEAVE (log_mutex);
  va_end (args);
}


static void
stdout_showprogress (void *parent_win, char *title, int nThreads, float fMax)
{
  int i;

  spec_time_secs = -1;
  spec_time_secs = (n_threads == 1 && n_connections == 1) ? -1 : fMax;

  pTrnTimes = (float *) calloc (n_threads, sizeof (float));
  fOldValues = (float *) calloc (n_threads, sizeof (float));
  ptpca_dDiffSum = (double *) calloc (n_threads, sizeof (double));

  MUTEX_ENTER (log_mutex);
  printf ("\n%s\n", title);
  printf ("\nREMAIN");
  for (i = 1; i <= n_threads; i++)
    printf ("   THR %-2d", i);
  printf ("\n======");
  for (i = 1; i <= n_threads; i++)
    printf ("=========");
  printf ("\n");
  fflush (stdout);
  MUTEX_LEAVE (log_mutex);
}


#if 0
static void
stdout_setprogresstext0 (char *pszProgress, int n_conn, int thread_no,
    float nValue, int nTrnPerCall, long secs_remain, double tpca_dDiffSum)
{
  static BOOL bLEFT = TRUE;

  MUTEX_ENTER (log_mutex);
  fputc (bLEFT ? '(' : ')', stderr);
  bLEFT = bLEFT ? FALSE : TRUE;
  fflush (stderr);
  fputc ('\b', stderr);
  MUTEX_LEAVE (log_mutex);
}
#endif



static void
stdout_setprogresstext (char *pszProgress, int nConn, int thread_no,
    float percent, int nTrnPerCall, long secs_remain, double tpca_dDiffSum)
{
  unsigned long time_now = get_msec_count ();
  int i;

  if (nConn >= 0 && thread_no >= 0 && thread_no < n_threads)
    {
      if (spec_time_secs < 0)
	{
	  if (test_types == TPC_A)
	    fOldValues[thread_no] +=
		(bench_get_long_pref (A_REFRESH_RATE) * nTrnPerCall);
	  else
	    fOldValues[thread_no] += 1;
	}
      else
	{
	  fOldValues[thread_no] +=
	      test_types == TPC_A ? nProgressIncrement * nTrnPerCall : 1;
	}

      ptpca_dDiffSum[thread_no] = tpca_dDiffSum;
      pTrnTimes[thread_no] = percent;

      if (time_now - curr_time_msec > (unsigned long) BARS_REFRESH_INTERVAL)
	{
	  long total_txns = 0;
	  double txn_time = 0;

	  MUTEX_ENTER (log_mutex);
	  printf ("%6ld", secs_remain);
	  for (i = 0; i < n_threads; i++)
	    {
	      printf (" %8.0f", fOldValues[i]);
	      total_txns += (long) fOldValues[i];
	      if (ptpca_dDiffSum[i] > txn_time)
		txn_time = ptpca_dDiffSum[i];
	    }
	  printf (" TPS=%f\r",
	      txn_time > 0 ? ((double) total_txns / (double) txn_time) : 0.0);
	  fflush (stdout);
	  MUTEX_LEAVE (log_mutex);
	}
    }

  if (time_now - curr_time_msec > (unsigned long) BARS_REFRESH_INTERVAL)
    curr_time_msec = get_msec_count ();
}


void
stdout_stopprogress (void)
{
  if (pTrnTimes)
    {
      free (pTrnTimes);
      pTrnTimes = NULL;
    }
  if (fOldValues)
    {
      free (fOldValues);
      fOldValues = NULL;
    }
  if (ptpca_dDiffSum)
    {
      free (ptpca_dDiffSum);
      ptpca_dDiffSum = NULL;
    }
}


void
stdout_markfinished (int nConn, int thread_no)
{
  spec_time_secs = -1;
}


static void
dummy_pane_log (const char *format, ...)
{
}


static void
dummy_showprogress (void *parent_win, char *title, int nThreads, float nMax)
{
}


static void
dummy_setworkingitem (char *pszWorking)
{
}


static void
dummy_setprogresstext (char *pszProgress, int n_conn, int thread_no,
    float nValue, int nTrnPerCall, long secs_remain, double tpca_dDiffSum)
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


typedef enum
{
  OP_RDSN = 127,
  OP_RUID,
  OP_RPWD,
  OP_RFILE,
  OP_RCREATE_TABLE,
  OP_RDROP_TABLE
} opt_type;


static struct option long_options[] = {
/* name | has_arg | *flag | val */
  {"dsn", 1, 0, 'd'},
  {"uid", 1, 0, 'u'},
  {"pwd", 1, 0, 'p'},
  {"runs", 1, 0, 'r'},
  {"time", 1, 0, 'm'},
  {"threads", 1, 0, 't'},
  {"arrayparm", 1, 0, 'P'},
  {"test", 1, 0, 's'},
  {"txn", 0, 0, 'x'},
  {"query", 0, 0, '1'},
  {"async", 0, 0, 'a'},
  {"cursor_type", 1, 0, 'c'},
  {"txn_isolation", 1, 0, 'i'},
  {"rowset_size", 1, 0, 'S'},
  {"keyset_size", 1, 0, 'K'},
  {"trav_count", 1, 0, 'T'},
  {"create_tables", 0, 0, 'C'},
  {"drop_tables", 0, 0, 'D'},
  {"rdsn", 1, 0, OP_RDSN},
  {"ruid", 1, 0, OP_RUID},
  {"rpwd", 1, 0, OP_RPWD},
  {"rfile", 1, 0, OP_RFILE},
  {"rcreate_table", 0, 0, OP_RCREATE_TABLE},
  {"rdrop_table", 0, 0, OP_RDROP_TABLE},
  { 0, 0, 0, 0}
};


int
do_command_line (int argc, char *argv[])
{
  int curr_opt;
  int Load = 0;
  int UnLoad = 0;
  int fCreateResultsTable = 0;
  int fDropResultsTable = 0;
  int opt_index;
  char szRDSN[50] = { "" };
  char szRUID[50] = { "" };
  char szRPWD[50] = { "" };
  char szRFILE[128] = { "results.xml" };

  if (argc > 1)
    {
      test_t test;
      OList *tests = NULL;

      memset (&test, 0, sizeof (test));
      test.TestType = TPC_A;
      strcpy (test.szName, "CommandLine");
      init_test (&test);
      if (gui.for_all_in_pool)
	gui.for_all_in_pool ();
      tests = o_list_append (tests, &test);
      test.ShowProgress = dummy_showprogress;
      test.SetWorkingItem = dummy_setworkingitem;
      test.SetProgressText = dummy_setprogresstext;
      test.StopProgress = dummy_stopprogress;
      test.fCancel = dummy_fcancel;
      test.tpc.a.fExecAsync = FALSE;
      test.tpc.a.fDoQuery = FALSE;
      test.tpc.a.fSQLOption = -1;
      test.tpc.a.fUseCommit = FALSE;

      test.tpc._.nMinutes = 1;
      test.tpc.a.nTraversalCount = 1;
      test.tpc.a.nRowsetSize = 1;
      test.tpc.a.nKeysetSize = 1;
      test.tpc.a.fSQLOption = IDX_PLAINSQL;
      pane_log = dummy_pane_log;
      memset (&test.szSQLError, 0, sizeof (test.szSQLError));

      while ((curr_opt =
	      getopt_long_only (argc, argv, SHORT_OPTIONS, long_options,
		  &opt_index)) != EOF)
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
	      test.tpc._.nMinutes = 1;
	    break;

	  case 't':
	    test.tpc._.nThreads = atoi (optarg);
	    if (test.tpc._.nThreads < 1)
	      test.tpc._.nThreads = 1;
	    break;

	  case 'P':
	    test.tpc.a.nArrayParSize = atoi (optarg);
	    if (test.tpc.a.nArrayParSize < 10)
	      test.tpc.a.nArrayParSize = 10;
	    break;

	  case 's':
	    if (!strncmp (optarg, "proc", 4))
	      test.tpc.a.fSQLOption = IDX_SPROCS;
	    else if (!strncmp (optarg, "prepare", 7))
	      test.tpc.a.fSQLOption = IDX_PARAMS;
	    else if (!strncmp (optarg, "exec", 4))
	      test.tpc.a.fSQLOption = IDX_PLAINSQL;
	    else if (!strncmp (optarg, "runall", 6))
	      test.tpc.a.fSQLOption = -1;
	    else
	      {
		fprintf (stdout, "Unknown test type : %s\n", optarg);
		usage ();
		return -1;
	      }
	    break;

	  case 'x':
	    test.tpc.a.fUseCommit = TRUE;
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
	    test.ShowProgress = stdout_showprogress;
	    test.StopProgress = stdout_stopprogress;

	  case 'v':
	    verbose = 1;
	    pane_log = stdout_pane_log;
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
	    if (test.tpc.a.nRowsetSize < 1)
	      test.tpc.a.nRowsetSize = 1;
	    break;

	  case 'K':
	    test.tpc.a.nKeysetSize = atoi (optarg);
	    if (test.tpc.a.nKeysetSize < 1)
	      test.tpc.a.nKeysetSize = 1;
	    break;

	  case 'T':
	    test.tpc.a.nTraversalCount = atoi (optarg);
	    if (test.tpc.a.nTraversalCount < 1)
	      test.tpc.a.nTraversalCount = 1;
	    break;

	  case 'C':
	    Load = 1;
	    break;

	  case 'D':
	    UnLoad = 1;
	    break;

	  case OP_RDSN:
	    strncpy (szRDSN, optarg, 49);
	    szRDSN[49] = 0;
	    break;

	  case OP_RUID:
	    strncpy (szRUID, optarg, 49);
	    szRUID[49] = 0;
	    break;

	  case OP_RPWD:
	    strncpy (szRPWD, optarg, 49);
	    szRPWD[49] = 0;
	    break;

	  case OP_RFILE:
	    strncpy (szRFILE, optarg, 127);
	    szRFILE[127] = 0;
	    break;

	  case OP_RCREATE_TABLE:
	    fCreateResultsTable = 1;
	    break;

	  case OP_RDROP_TABLE:
	    fDropResultsTable = 1;
	    break;

	  default:
	    usage ();
	    return -2;
	  };


      if (szRDSN[0])
	{
	  if (!results_login (szRDSN, szRUID, szRPWD))
	    exit (-1);
	}

      if (fDropResultsTable)
	drop_results_table ();

      if (fCreateResultsTable)
	create_results_table ();

      if (UnLoad)
	{
	  if (!do_login (&test))
	    exit (-1);
	  get_dsn_data (&test);
	  if (!test.hdbc)
	    {
	      fprintf (stdout, "%s\n", test.szSQLError);
	      fprintf (stderr, "%s\n", test.szSQLError);
	      return -5;
	    }
	  fCleanup (&test);
	  do_logout (&test);
	}
      if (Load)
	{
	  if (!do_login (&test))
	    exit (-1);
	  get_dsn_data (&test);
	  if (!test.hdbc)
	    {
	      fprintf (stdout, "%s\n", test.szSQLError);
	      fprintf (stderr, "%s\n", test.szSQLError);
	      return -5;
	    }
	  fBuildBench (&test);
	  do_logout (&test);
	}

      if (Load || UnLoad || fDropResultsTable || fCreateResultsTable)
	return 0;

/* progress impl */
      n_connections = 1;
      n_threads = (test.tpc._.nThreads > 1 ? test.tpc._.nThreads : 1);
      test_types = test.TestType;

#if defined(PTHREADS) || defined(WIN32)
      if (test.tpc._.nThreads > 1)
	{
	  if (!do_login (&test))
	    exit (-1);
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
	      do_save_run_results (szRFILE, tests, test.tpc._.nMinutes);
	      if (rc)
		return 0;
	      else
		return -4;
	    }
	  else
	    {
	      if (do_threads_run_all (1, tests, test.tpc._.nMinutes, szRFILE))
		return 0;
	      else
		return -4;
	    }

	}
      else
#endif
	{
	  if (do_login (&test))
	    {
	      get_dsn_data (&test);
	      if (!test.hdbc)
		{
		  fprintf (stderr, "%s\n", test.szSQLError);
		  fprintf (stdout, "%s\n", test.szSQLError);
		  return -5;
		}

	      fExecuteSql (&test, (SQLCHAR *) "delete from HISTORY");
	      SQLTransact (SQL_NULL_HENV, test.hdbc, SQL_COMMIT);
	      do_logout (&test);

	      if (test.tpc.a.fSQLOption != -1)
		{
		  DoRun (&test, NULL);
		  do_save_run_results (szRFILE, tests, test.tpc._.nMinutes);
		}
	      else
		DoRunAll (&test, szRFILE);
#if 0
case TPC_C:
	      if (tpcc_run_test (NULL, ptest))
		{
		  add_tpcc_result (ptest);
		}
	      else
		pane_log ("TPC-C RUN FAILED\r\n");
	      do_save_run_results (szRFILE, tests, nMinutes);
#endif
	    }
	}
      if (szRDSN[0])
	results_logout ();
      return 0;
    }
  else
    {
      usage ();
      return 0;
    }
}
