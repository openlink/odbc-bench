/*
 *  threads.c
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
#include <sys/types.h>

#ifndef WIN32
#  include <sys/time.h>
#  include <unistd.h>
#else
#  include <time.h>
#  define pipe(phandles)	_pipe (phandles, 4096, _O_BINARY)
#endif

#include "odbcbench.h"
#include "thr.h"
#include "tpca_code.h"
#include "tpccfun.h"

static volatile BOOL do_cancel = FALSE;
static int signal_pipe[2];
int nProgressIncrement = 0;

typedef struct bench_msg_s
{
  char Type;
  int nConn;
  int nThread;
  float percent;
  int nTrnPerCall;
  long secs_remain;
  double tpca_dDiffSum;
}
bench_msg_t;


static void
threaded_SendProgress (char *pszProgress, int conn_no, int thread_no,
    float percent, int nTrnPerCall, long secs_remain, double tpca_dDiffSum)
{
  bench_msg_t msg;
  msg.Type = 'R';
  msg.nConn = conn_no;
  msg.nThread = thread_no;
  msg.percent = percent;
  msg.nTrnPerCall = nTrnPerCall;
  msg.secs_remain = secs_remain;
  msg.tpca_dDiffSum = tpca_dDiffSum;
  if (!signal_pipe[1] || sizeof (bench_msg_t) != write (signal_pipe[1], &msg, sizeof (bench_msg_t)))
    abort ();
}


static void
dummy_SetWorkingItem (char *pszItem)
{
}


static BOOL
threaded_fCancel (void)
{
  return do_cancel;
}


static void
dummy_pane_log (const char *szFormat, ...)
{
}


static void
dummy_ShowProgress (void * widget, char * text, BOOL bForceSingle, float fMax)
{
}


static void
dummy_StopProgress (void)
{
}


#if defined(PTHREADS)
static void *
worker_func (void *data)
#elif defined(WIN32)
     static DWORD WINAPI worker_func (void *data)
#endif
{
  test_t *lpBenchInfo = (test_t *) data;
  DWORD result = TRUE;
  bench_msg_t msg;

  msg.Type = 'F';
  msg.nConn = lpBenchInfo->tpc._.nConn;
  msg.nThread = lpBenchInfo->tpc._.nThreadNo;

  if (!do_cancel)
    {
      lpBenchInfo->ShowProgress = dummy_ShowProgress;
      lpBenchInfo->StopProgress = dummy_StopProgress;
      lpBenchInfo->SetWorkingItem = dummy_SetWorkingItem;
      lpBenchInfo->SetProgressText = threaded_SendProgress;
      lpBenchInfo->fCancel = threaded_fCancel;

      lpBenchInfo->hstmt = 0;
      lpBenchInfo->hdbc = 0;
      lpBenchInfo->tpc._.nThreads = 1;

      if (do_login (lpBenchInfo))
        {
          result = FALSE;
          if (lpBenchInfo->hstmt)
	    switch (lpBenchInfo->TestType)
	      {
	      case TPC_A:
	        result = DoThreadsRun (lpBenchInfo);
	        break;
	      case TPC_C:
	        result = tpcc_run_test (NULL, lpBenchInfo) ? TRUE : FALSE;
	        break;
	      }
          do_logout (lpBenchInfo);
	}
    }

  if (!signal_pipe[1] || sizeof (bench_msg_t) != write (signal_pipe[1], &msg, sizeof (bench_msg_t)))
    abort ();

#if defined(PTHREADS)
  return ((void *) result);
#elif defined(WIN32)
  return ((DWORD) result);
#endif
}

static int
ThreadedCalcStats (OList * tests, THREAD_T ** workers, 
    test_t ** data, int nConnCount, int *nThreads)
{
  OList *iter;
  int nConn;
  int rc = 1;
  int rc1;

  for (iter = tests, nConn = 0; iter && nConn < nConnCount;
      nConn++, iter = o_list_next (iter))
    {
      int nOkA = 0, nOkC = 0;
      int nA = 0, nC = 0;
      DWORD result;
      BOOL runStatus = TRUE;
      int i;
      test_t *test = (test_t *) iter->data;
      if (IS_A (*test))
	{
	  test->tpc.a.nTrnCnt = 0;
	  test->tpc.a.nTrnCnt1Sec = 0;
	  test->tpc.a.nTrnCnt2Sec = 0;
	  test->tpc.a.dDiffSum = -1;
	}
      else
	{
	  test->tpc.c.tpcc_sum = 0;
	  test->tpc.c.run_time = 0;
	  test->tpc.c.nRounds = 0;
	  reset_times (test);
	}


      for (i = 0; i < nThreads[nConn]; i++)
	{
	  GET_EXIT_STATUS (workers[nConn][i], &result);

	  if (!result)
	    runStatus = result;

	  switch (data[nConn][i].TestType)
	    {
	    case TPC_A:
	      nA++;
	      if (!test->is_unsupported && result)
	        {
	          nOkA++;
	          test->tpc.a.nTrnCnt += data[nConn][i].tpc.a.nTrnCnt;
	          test->tpc.a.nTrnCnt1Sec += data[nConn][i].tpc.a.nTrnCnt1Sec;
	          test->tpc.a.nTrnCnt2Sec += data[nConn][i].tpc.a.nTrnCnt2Sec;
	          if (data[nConn][i].tpc.a.dDiffSum > test->tpc.a.dDiffSum)
	            test->tpc.a.dDiffSum = data[nConn][i].tpc.a.dDiffSum;
	        }
	      break;

	    case TPC_C:
	      nC++;
  	      if (result)
	        {
	          nOkC++;
		  test->tpc.c.tpcc_sum += data[nConn][i].tpc.c.tpcc_sum;
		  test->tpc.c.run_time += data[nConn][i].tpc.c.run_time;
		  test->tpc.c.nRounds += data[nConn][i].tpc.c.nRounds;

		  /* individual transaction timings */
		  ta_merge (&(test->tpc.c.ten_pack_ta),
		      &(data[nConn][i].tpc.c.ten_pack_ta));
		  ta_merge (&(test->tpc.c.new_order_ta),
		      &(data[nConn][i].tpc.c.new_order_ta));
		  ta_merge (&(test->tpc.c.payment_ta),
		      &(data[nConn][i].tpc.c.payment_ta));
		  ta_merge (&(test->tpc.c.delivery_ta),
		      &(data[nConn][i].tpc.c.delivery_ta));
		  ta_merge (&(test->tpc.c.slevel_ta),
		      &(data[nConn][i].tpc.c.slevel_ta));
		  ta_merge (&(test->tpc.c.ostat_ta),
		      &(data[nConn][i].tpc.c.ostat_ta));
		}
	      break;
	    }
	}

      if (nA || nOkC)
	rc1 = do_login (test);

      if (nA)
	{
	  if (nOkA)
	    pane_log
	      ("\r\n\r\n%s - %s(%s) - %d TPC-A Threads ended with no errors.\r\n",
	      test->szName, test->szDBMS, test->szDriverName, nOkA);

          if (nOkA == 0 && !test->szSQLState[0] && !test->szSQLError[0])
            {
              strcpy(test->szSQLState, "ERROR");
              strcpy(test->szSQLError, "All Threads ended prematurely.");
            }

	  if (test->hdbc)
	    CalcStats (runStatus, nOkA, test, test->tpc.a.nTrnCnt, 
	        test->tpc.a.nTrnCnt1Sec,
		test->tpc.a.nTrnCnt2Sec, test->tpc.a.dDiffSum);
	}

      if (nOkC)
	{
	  pane_log
	      ("\r\n\r\n%s - %s(%s) - %d TPC-C Threads ended with no errors.\r\n",
	      test->szName, test->szDBMS, test->szDriverName, nOkC);
	  test->tpc.c.run_time /= nOkC;
	  if (test->hdbc)
	    add_tpcc_result (test);
	}

      if ((nA || nOkC) && rc1)
	do_logout (test);

      if (!(nOkA || nOkC))
	{
	  pane_log ("\r\n\r\n%s - %s(%s) - All Threads ended prematurely.\r\n",
	      test->szName, test->szDBMS, test->szDriverName);
	  rc = 0;
	}
    }
  return rc;
}


int
do_threads_run (int nConnCount, OList * tests, int nMinutes, char *szTitle)
{
  test_t *lpBenchInfo;
  test_t **data;
  int *n_threads, nThreads = 0, nThreadsWork;
  char szTemp[2048];
  long nRuns = 0;
  FILE *fi;
  THREAD_T **workers;
  bench_msg_t msg;
  time_t start_time, now_time;
  long time_remaining;
#ifdef WIN32
  DWORD thrid;
#endif
  int thr, conn;
  int rc = 0;
  void (*old_pane_log) (const char *format, ...) = pane_log;
  OList *iter;
  BOOL wasError = FALSE;

  nProgressIncrement = bench_get_long_pref (A_REFRESH_RATE);
  if (!nConnCount)
    return 0;
  if (pipe (signal_pipe))
    {
      old_pane_log ("Can't open the pipe");

      return rc;
    }
  fi = fdopen (signal_pipe[0], "rb");

  lpBenchInfo = (test_t *) tests->data;
  do_cancel = FALSE;
  time (&start_time);

  sprintf (szTemp, "Running for %d minutes", nMinutes);
  lpBenchInfo->ShowProgress (NULL, szTitle, FALSE, nMinutes * 60);
  lpBenchInfo->SetWorkingItem (szTemp);

  data = (test_t **) malloc (nConnCount * sizeof (test_t *));
  workers = (THREAD_T **) malloc (nConnCount * sizeof (THREAD_T *));
  n_threads = (int *) malloc (nConnCount * sizeof (int));
  pane_log = dummy_pane_log;

  for (iter = tests, conn = 0; iter; iter = o_list_next (iter), conn++)
    {
      test_t *test = (test_t *) iter->data;
      test->tpc._.nMinutes = nMinutes;
      if (do_login (test))
        {
          get_dsn_data (test);
          if (test->hdbc && IS_A (*test))
	    {
	      fExecuteSql (test, "delete from HISTORY");
	      SQLTransact (SQL_NULL_HENV, test->hdbc, SQL_COMMIT);
	    }
          do_logout (test);

          n_threads[conn] = test->tpc._.nThreads ? test->tpc._.nThreads : 1;
          data[conn] = (test_t *) malloc (n_threads[conn] * sizeof (test_t));
          workers[conn] =
	      (THREAD_T *) malloc (n_threads[conn] * sizeof (THREAD_T));
          nThreads += n_threads[conn];
          memset (test->szSQLError, 0, sizeof (test->szSQLError));
          memset (test->szSQLState, 0, sizeof (test->szSQLState));
          for (thr = 0; thr < n_threads[conn]; thr++)
	    {
	      memcpy (&(data[conn][thr]), test, sizeof (test_t));
              data[conn][thr].test = test;
	      data[conn][thr].tpc._.nThreadNo = thr;
	      data[conn][thr].tpc._.nConn = conn;
	      START_THREAD (workers[conn][thr], worker_func, data[conn][thr]);
	    }
	}
    }

  nThreadsWork = nThreads;
  while (nThreadsWork && fread (&msg, sizeof (bench_msg_t), 1, fi))
    {
      if (lpBenchInfo->fCancel ())
	do_cancel = TRUE;
      switch (msg.Type)
	{
	case 'F':
	  if (gui.do_MarkFinished)
	    gui.do_MarkFinished (msg.nConn, msg.nThread);
	  sprintf (szTemp, "%s %3d threads running", szTitle, --nThreadsWork);
	  lpBenchInfo->SetWorkingItem (szTemp);
	  if (data[msg.nConn][msg.nThread].szSQLError[0])
	    {
	      old_pane_log ("*** Error : Thread %3d in dsn %s : [%s] %s\r\n",
		  msg.nThread + 1,
		  data[msg.nConn][msg.nThread].szLoginDSN,
		  data[msg.nConn][msg.nThread].szSQLState,
		  data[msg.nConn][msg.nThread].szSQLError);
	      strcpy(data[msg.nConn][msg.nThread].test->szSQLState, data[msg.nConn][msg.nThread].szSQLState);
	      strcpy(data[msg.nConn][msg.nThread].test->szSQLError, data[msg.nConn][msg.nThread].szSQLError);
              wasError = TRUE;
	    }
	  break;
	case 'R':
	  time (&now_time);
	  time_remaining = nMinutes * 60 - (now_time - start_time);
	  time_remaining = (time_remaining > 0 ? time_remaining : 0);
	  sprintf (szTemp, "%10ld secs remaining", time_remaining);
	  nRuns += 1;
	  lpBenchInfo->SetProgressText (szTemp, msg.nConn, msg.nThread,
	      msg.percent, msg.nTrnPerCall, time_remaining, msg.tpca_dDiffSum);
	  break;
	}
    }
  fclose (fi);
  close (signal_pipe[1]);
  close (signal_pipe[0]);
  memset (signal_pipe, 0, sizeof (signal_pipe));
  pane_log = old_pane_log;
  lpBenchInfo->StopProgress ();

  rc = ThreadedCalcStats (tests, workers, data, nConnCount, n_threads);

  for (iter = tests, conn = 0; iter; iter = o_list_next (iter), conn++)
    {
      XFREE (workers[conn]);
      XFREE (data[conn]);
    }

  XFREE (workers);
  XFREE (data);
  XFREE (n_threads);

  if (do_cancel || wasError)
    rc = 0;
  return rc;
}


void
pthread_yield (void)
{
}
