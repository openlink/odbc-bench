/*
 *  results.c
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

#include "odbcbench.h"

static HDBC res_hdbc = SQL_NULL_HDBC;
static HSTMT res_hstmt = SQL_NULL_HSTMT;
static int res_fHaveResults = 0;

static DWORD null_val = SQL_NULL_DATA;

void
results_logout ()
{
  if (!res_hstmt)
    return;

  SQLFreeStmt (res_hstmt, SQL_DROP);
  res_hstmt = (HSTMT) 0;

  if (!res_hdbc)
    return;

  SQLFreeConnect (res_hdbc);
  res_hdbc = (HDBC) 0;
  pane_log ("results connection closed\n");
}

void
results_login (char *szDSN, char *szUID, char *szPWD)
{
  RETCODE rc;
  char szBuff;

  results_logout ();

  rc = SQLAllocConnect (henv, &res_hdbc);
  IF_CERR_GO (res_hdbc, done, rc, NULL);
  rc = SQLConnect (res_hdbc, szDSN, SQL_NTS, szUID, SQL_NTS, szPWD, SQL_NTS);
  IF_CERR_GO (res_hdbc, done, rc, NULL);

  rc = SQLAllocStmt (res_hdbc, &res_hstmt);
  IF_CERR_GO (res_hdbc, done, rc, NULL);

  szBuff = 'Y';
  rc = SQLTables (res_hstmt, NULL, 0, NULL, 0,
    "RESULTS", SQL_NTS, "TABLE", SQL_NTS);
  if (SQL_SUCCESS != rc || SQL_SUCCESS != SQLFetch (res_hstmt))
    szBuff = 'N';
  SQLFreeStmt (res_hstmt, SQL_CLOSE);
  res_fHaveResults = (szBuff == 'Y');
  pane_log ("results connection opened to %s\n", szDSN);
done:
  rc = 0;
}

static char *szCreateResultsSQL =
    "create table RESULTS ("
    "		RUNT 		timestamp, "
    "		URL 		varchar(255),"
    "		OPTIONS 	varchar(128),"
    "		TPS		float,"
    "		TOTTIME		float,"
    "		NTRANS		integer,"
    "		SUB1S		float,"
    "		SUB2S		float,"
    "		TRNTIME		float,"
    "		BTYPE		varchar(10),"
    "		DRVRNAME	varchar(128),"
    "		DRVRVER		varchar(128),"
    "		STATE		varchar(16)," 
    "		MSG		varchar(255)" ")";

void
create_results_table ()
{
  RETCODE rc;
  HSTMT stmt;

  if (!res_hstmt)
    {
      pane_log ("No results connection\n");
      return;
    }
  stmt = res_hstmt;
  if (!stmt)
    {
      pane_log ("Not Connected\n");
      return;
    }
  rc = SQLExecDirect (stmt, szCreateResultsSQL, SQL_NTS);
  IF_ERR_GO (stmt, create_error, rc, NULL);

  pane_log ("Results table created%s\n",
      (res_hstmt ? "" : " using the main connection"));
create_error:
  rc = 0;
}

void
drop_results_table ()
{
  RETCODE rc;
  HSTMT stmt;

  if (!res_hstmt)
    {
      pane_log ("No results connection\n");
      return;
    }
  stmt = res_hstmt;
  if (!stmt)
    {
      pane_log ("Not Connected\n");
      return;
    }
  rc = SQLExecDirect (stmt, "drop table RESULTS", SQL_NTS);
  IF_ERR_GO (stmt, drop_error, rc, NULL);

  pane_log ("Results table dropped successfully%s\n",
      (res_hstmt ? "" : " using the main connection"));
drop_error:
  rc = 0;
}

static char *szInsertResultsSQL =
    "insert into RESULTS (BTYPE, URL, OPTIONS, TPS, TOTTIME, NTRANS, SUB1S, SUB2S, TRNTIME, DRVRNAME, DRVRVER, STATE, MSG) "
    "	values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

void
do_add_results_record (char *test_type, char *result_test_type,
    HENV env, HDBC dbc, HSTMT stmt,
    char *szDSN, float ftps, double dDiffSum, long nTrnCnt,
    float fsub1, float fsub2, float fAvgTPTime,
    char *szDriverName, char *szDriverVer, int driver_has_results,
    char *szState, char *szMessage)
{
  RETCODE rc;
  HSTMT lstmt = res_hstmt ? res_hstmt : stmt;

  if (!res_fHaveResults && !driver_has_results)
    return;
  if (!lstmt)
    {
      pane_log ("Not Connected\n");
      return;
    }

  SQLFreeStmt (lstmt, SQL_CLOSE);

  rc = SQLPrepare (lstmt, szInsertResultsSQL, SQL_NTS);
  if (rc != SQL_SUCCESS)
    return;

  IBINDNTS (lstmt, 1, test_type);
  IBINDNTS (lstmt, 2, szDSN);
  IBINDNTS (lstmt, 3, result_test_type);
  if (ftps == -1)
    IBINDF_NULL (lstmt, 4);
  else
    IBINDF (lstmt, 4, ftps);
  IBINDD (lstmt, 5, dDiffSum);
  if (nTrnCnt == -1)
    IBINDL_NULL (lstmt, 6);
  else
    IBINDL (lstmt, 6, nTrnCnt);
  if (fsub1 == -1)
    IBINDF_NULL (lstmt, 7);
  else
    IBINDF (lstmt, 7, fsub1);
  if (fsub2 == -1)
    IBINDF_NULL (lstmt, 8);
  else
    IBINDF (lstmt, 8, fsub2);
  if (fAvgTPTime == -1)
    IBINDF_NULL (lstmt, 9);
  else
    IBINDF (lstmt, 9, fAvgTPTime);
  if (!szDriverName)
    IBINDNTS_NULL (lstmt, 10);
  else
    IBINDNTS (lstmt, 10, szDriverName);
  if (!szDriverVer)
    IBINDNTS_NULL (lstmt, 11);
  else
    IBINDNTS (lstmt, 11, szDriverVer);

  if (!szState)
    IBINDNTS (lstmt, 12, "OK");
  else
    IBINDNTS (lstmt, 12, szState);
  if (!szMessage)
    IBINDNTS (lstmt, 13, "");
  else
    IBINDNTS (lstmt, 13, szMessage);

  rc = SQLExecute (lstmt);
  SQLTransact (env, dbc, SQL_COMMIT);
  if (rc == SQL_SUCCESS)
    {
      pane_log ("Results written to the results table%s\n",
  	  res_hstmt ? "" : " using main connection");
    }	
  SQLFreeStmt (lstmt, SQL_CLOSE);
}


void
add_tpcc_result (test_t * lpCfg)
{
  char szTemp[1024];

  if (lpCfg->tpc.c.run_time <= 0 && lpCfg->tpc.c.tpcc_sum <= 0)	/* an error occurred */
    return;
  else if (lpCfg->tpc.c.run_time > 0 && lpCfg->tpc.c.tpcc_sum <= 0)	/* populating the tables */
    {
      sprintf (szTemp, "Load/%ld", lpCfg->tpc.c.count_ware);
      do_add_results_record ("TPC-C", szTemp,
	  henv, lpCfg->hdbc, lpCfg->hstmt,
	  lpCfg->szDSN,
	  lpCfg->tpc.c.count_ware, 
	  lpCfg->tpc.c.run_time, -1, -1, -1, -1,
	  lpCfg->szDriverName, lpCfg->szDriverVer, lpCfg->fHaveResults, 
	  "OK", "");
    }
  else if (lpCfg->tpc.c.run_time > 0 && lpCfg->tpc.c.tpcc_sum > 0)	/* run the benchmark */
    {
      pane_log ("\nTPC-C RUN FINISHED. TOTAL : %f\n",
	  lpCfg->tpc.c.tpcc_sum / lpCfg->tpc.c.nRounds);
      pane_log ("\nTotal TenPacks : %d\n", lpCfg->tpc.c.nRounds);
      pane_log ("\nTRANSACTION TIMINGS\n\n");
      ta_print_buffer (szTemp, &lpCfg->tpc.c.new_order_ta,
	  &lpCfg->tpc.c.ten_pack_ta);
      pane_log (szTemp);
      ta_print_buffer (szTemp, &lpCfg->tpc.c.payment_ta,
	  &lpCfg->tpc.c.ten_pack_ta);
      pane_log (szTemp);
      ta_print_buffer (szTemp, &lpCfg->tpc.c.delivery_ta,
	  &lpCfg->tpc.c.ten_pack_ta);
      pane_log (szTemp);
      ta_print_buffer (szTemp, &lpCfg->tpc.c.slevel_ta,
	  &lpCfg->tpc.c.ten_pack_ta);
      pane_log (szTemp);
      ta_print_buffer (szTemp, &lpCfg->tpc.c.ostat_ta,
	  &lpCfg->tpc.c.ten_pack_ta);
      pane_log (szTemp);
      pane_log ("\n");
      if (lpCfg->tpc._.nThreads)
	sprintf (szTemp, "Run/%d threads/%ld", lpCfg->tpc._.nThreads,
	    lpCfg->tpc.c.count_ware);
      else
	sprintf (szTemp, "Run/%ld/%ld", lpCfg->tpc.c.local_w_id,
	    lpCfg->tpc.c.count_ware);

      do_add_results_record ("TPC-C", szTemp,
	  henv, lpCfg->hdbc, lpCfg->hstmt,
	  lpCfg->szDSN,
	  lpCfg->tpc.c.tpcc_sum / lpCfg->tpc.c.nRounds,
	  lpCfg->tpc.c.run_time,
	  lpCfg->tpc.c.nRounds *
	  (lpCfg->tpc._.nThreads ? lpCfg->tpc._.nThreads : 1), -1, -1, -1,
	  lpCfg->szDriverName, lpCfg->szDriverVer, lpCfg->fHaveResults,
	  "OK", "");
    }
}
