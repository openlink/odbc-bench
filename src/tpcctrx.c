/*
 *  tpcctrx.c
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
#include <memory.h>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include <time.h>
#endif

#include "odbcbench.h"
#include "tpccfun.h"


static const char new_order_text_kubl[] = " new_order (?, ?, ?, ?, ?,    "	/*  1, w_id, 2. d_id, 3. c_id, 4 ol_cnt, 5 all_local */
    "        ?, ?, ?, "		/* 1. i_id 2. 2. supply_w_id 3. qty */
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, " "        ?, ?, ?, " "        ?, ?, ?)";
static const char new_order_text_mssql[] = " exec new_order_proc ?, ?, ?, ?, ?,    "	/*  1, w_id, 2. d_id, 3. c_id, 4 ol_cnt, 5 all_local */
    "        ?, ?, ?, "		/* 1. i_id 2. 2. supply_w_id 3. qty */
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, " "        ?, ?, ?, " "        ?, ?, ? ";

static const char new_order_text_ora[] = "{call new_order_proc(?, ?, ?, ?, ?,    "	/*  1, w_id, 2. d_id, 3. c_id, 4 ol_cnt, 5 all_local */
    "        ?, ?, ?, "		/* 1. i_id 2. 2. supply_w_id 3. qty */
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, "
    "        ?, ?, ?, " "        ?, ?, ?, " "        ?, ?, ? )}";

#define NEW_ORDER_TEXT \
	((lpCfg->tpc.c.bIsVirtuoso) ? (new_order_text_kubl) : ((lpCfg->tpc.c.bIsOracle) ? (new_order_text_ora) : (new_order_text_mssql)))

static int
stmt_result_sets (HSTMT stmt)
{
  RETCODE rc;
  DECLARE_FOR_SQLERROR;
  do
    {
      do
	{
	  rc = SQLFetch (stmt);
	  IF_DEADLOCK_OR_ERR_GO (stmt, next_res, rc, deadlock_rs);
	next_res:
	  rc = rc;
	}
      while (rc != SQL_NO_DATA_FOUND && rc != SQL_ERROR);
      if (rc == SQL_ERROR)
	{
	  if (gui.warn_message)
	    gui.warn_message ("\n    Line %d, file %s\n", __LINE__, __FILE__);
	  print_error (stmt, stmt, stmt, NULL);
	  return 0;
	}
      rc = SQLMoreResults (stmt);
    }
  while (rc != SQL_NO_DATA_FOUND && rc != SQL_ERROR);
  if (rc == SQL_ERROR)
    {
      print_error (stmt, stmt, stmt, NULL);
      if (gui.warn_message)
        gui.warn_message ("\n    Line %d, file %s\n", __LINE__, __FILE__);
    }

  SQLFreeStmt (stmt, SQL_CLOSE);
  return 0;
deadlock_rs:
  SQLFreeStmt (stmt, SQL_CLOSE);
  return 1;
}


static int
make_supply_w_id (test_t * lpCfg)
{
  if (lpCfg->tpc.c.count_ware > 1
      && RandomNumber (&lpCfg->tpc.c.rnd_seed, 0, 99) < 10)
    {
      int n, n_tries = 0;
      do
	{
	  n =
	      RandomNumber (&lpCfg->tpc.c.rnd_seed, 1,
	      lpCfg->tpc.c.count_ware);
	  n_tries++;
	}
      while (n == lpCfg->tpc.c.local_w_id && n_tries < 10);
      return lpCfg->tpc.c.local_w_id;
    }
  else
    return lpCfg->tpc.c.local_w_id;
}


static int
new_order (test_t * lpCfg)
{
  RETCODE rc;
  int n;
  struct timeval tv;
  olines_t ols;
  int i;
  long d_id;
  long w_id;
  long c_id;
  char c_last[100];
  long ol_cnt = 10;
  long all_local = 1;
  DECLARE_FOR_SQLERROR;


deadlock_no:
  w_id = lpCfg->tpc.c.local_w_id;
  d_id = RandomNumber (&lpCfg->tpc.c.rnd_seed, 1, DIST_PER_WARE);
  c_id = random_c_id (&lpCfg->tpc.c.rnd_seed);

  memset (c_last, 0, sizeof (c_last));
  gettimestamp (&tv);

  for (i = 0; i < 10; i++)
    {
      ols.ol_i_id[i] = random_i_id (&lpCfg->tpc.c.rnd_seed);
      ols.ol_qty[i] = 5;
      ols.ol_supply_w_id[i] = make_supply_w_id (lpCfg);
      ols.ol_no[i] = i + 1;
      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 23, 23, ols.ol_data[i]);
    }

  if (!lpCfg->tpc.c.new_order_stmt)
    {
      INIT_STMT (lpCfg->hdbc, lpCfg->tpc.c.new_order_stmt, NEW_ORDER_TEXT, lpCfg);
      IBINDL (lpCfg->tpc.c.new_order_stmt, 1, w_id);
      IBINDL (lpCfg->tpc.c.new_order_stmt, 2, d_id);
      IBINDL (lpCfg->tpc.c.new_order_stmt, 3, c_id);
      IBINDL (lpCfg->tpc.c.new_order_stmt, 4, ol_cnt);
      IBINDL (lpCfg->tpc.c.new_order_stmt, 5, all_local);
      for (n = 0; n < 10; n++)
	{
	  IBINDL (lpCfg->tpc.c.new_order_stmt, NO_PARS + 1 + (n * OL_PARS),
	      ols.ol_i_id[n]);
	  IBINDL (lpCfg->tpc.c.new_order_stmt, NO_PARS + 2 + (n * OL_PARS),
	      ols.ol_supply_w_id[n]);
	  IBINDL (lpCfg->tpc.c.new_order_stmt, NO_PARS + 3 + (n * OL_PARS),
	      ols.ol_qty[n]);
	}
    }
  SQLSetConnectOption (lpCfg->hdbc, SQL_AUTOCOMMIT, 1);
  ta_enter (&lpCfg->tpc.c.new_order_ta);
  rc = SQLExecute (lpCfg->tpc.c.new_order_stmt);
  IF_DEADLOCK_OR_ERR_GO (lpCfg->tpc.c.new_order_stmt, err, rc, deadlock_no);
  if (rc != SQL_NO_DATA_FOUND)
    if (stmt_result_sets (lpCfg->tpc.c.new_order_stmt))
      goto deadlock_no;

  ta_leave (&lpCfg->tpc.c.new_order_ta);
  return 1;

err:
  SAVE_SQL_ERROR (lpCfg->szSQLState, lpCfg->szSQLError);
  ta_leave (&lpCfg->tpc.c.new_order_ta);

  return 0;
}


static const char payment_text_kubl[] = "payment (?, ?, ?, ?, ?, ?, ?)";
static const char payment_text_mssql[] = "exec payment ?, ?, ?, ?, ?, ?, ?";
static const char payment_text_ora[] = "{call payment(?, ?, ?, ?, ?, ?, ?)}";

#define PAYMENT_TEXT \
	((lpCfg->tpc.c.bIsVirtuoso) ? (payment_text_kubl) : ((lpCfg->tpc.c.bIsOracle) ? (payment_text_ora) : (payment_text_mssql)))

static int
payment (test_t * lpCfg)
{
  RETCODE rc;
  long w_id = lpCfg->tpc.c.local_w_id;
  long d_id = RandomNumber (&lpCfg->tpc.c.rnd_seed, 1, DIST_PER_WARE);
  long c_id = random_c_id (&lpCfg->tpc.c.rnd_seed);
  char c_last[50];
  float amount = 100.00;
  DECLARE_FOR_SQLERROR;

deadlock_pay:
  if (!lpCfg->tpc.c.payment_stmt)
    {
      INIT_STMT (lpCfg->hdbc, lpCfg->tpc.c.payment_stmt, PAYMENT_TEXT, lpCfg);
    }
  if (RandomNumber (&lpCfg->tpc.c.rnd_seed, 0, 100) < 60)
    {
      c_id = 0;
      Lastname (RandomNumber (&lpCfg->tpc.c.rnd_seed, 0, 999), c_last);
    }
  IBINDL (lpCfg->tpc.c.payment_stmt, 1, w_id);
  IBINDL (lpCfg->tpc.c.payment_stmt, 2, w_id);
  IBINDF (lpCfg->tpc.c.payment_stmt, 3, amount);
  IBINDL (lpCfg->tpc.c.payment_stmt, 4, d_id);
  IBINDL (lpCfg->tpc.c.payment_stmt, 5, d_id);
  IBINDL (lpCfg->tpc.c.payment_stmt, 6, c_id);
  IBINDNTS (lpCfg->tpc.c.payment_stmt, 7, c_last);

  ta_enter (&lpCfg->tpc.c.payment_ta);
  rc = SQLExecute (lpCfg->tpc.c.payment_stmt);
  IF_DEADLOCK_OR_ERR_GO (lpCfg->tpc.c.payment_stmt, err, rc, deadlock_pay);
  if (rc != SQL_NO_DATA_FOUND)
    if (stmt_result_sets (lpCfg->tpc.c.payment_stmt))
      goto deadlock_pay;

  ta_leave (&lpCfg->tpc.c.payment_ta);
  return 1;

err:
  SAVE_SQL_ERROR (lpCfg->szSQLState, lpCfg->szSQLError);
  ta_leave (&lpCfg->tpc.c.payment_ta);
  return 0;
}


static const char delivery_text_kubl[] = "delivery_1 (?, ?, ?)";
static const char delivery_text_mssql[] = "exec delivery ?, ?";
static const char delivery_text_ora[] = "{call delivery(?, ?)}";

#define DELIVERY_TEXT \
	((lpCfg->tpc.c.bIsVirtuoso) ? (delivery_text_kubl) : ((lpCfg->tpc.c.bIsOracle) ? (delivery_text_ora) : (delivery_text_mssql)))

static int
delivery_1 (test_t * lpCfg, long w_id, long d_id)
{
  long carrier_id = 13;
  RETCODE rc;
  DECLARE_FOR_SQLERROR;

deadlock_del1:
  if (!lpCfg->tpc.c.delivery_stmt)
    {
      INIT_STMT (lpCfg->hdbc, lpCfg->tpc.c.delivery_stmt, DELIVERY_TEXT, lpCfg);
    }

  IBINDL (lpCfg->tpc.c.delivery_stmt, 1, w_id);
  IBINDL (lpCfg->tpc.c.delivery_stmt, 2, carrier_id);
  if (d_id)
    IBINDL (lpCfg->tpc.c.delivery_stmt, 3, d_id);
  rc = SQLExecute (lpCfg->tpc.c.delivery_stmt);
  IF_DEADLOCK_OR_ERR_GO (lpCfg->tpc.c.delivery_stmt, err, rc, deadlock_del1);
  if (rc != SQL_NO_DATA_FOUND)
    if (stmt_result_sets (lpCfg->tpc.c.delivery_stmt))
      goto deadlock_del1;

  return 1;

err:
  SAVE_SQL_ERROR (lpCfg->szSQLState, lpCfg->szSQLError);
  return 0;
}


static const char slevel_text_kubl[] = "slevel (?, ?, ?)";
static const char slevel_text_mssql[] = "exec slevel ?, ?, ?";
static const char slevel_text_ora[] = "{call slevel(?, ?, ?)}";

#define SLEVEL_TEXT \
	((lpCfg->tpc.c.bIsVirtuoso) ? (slevel_text_kubl) : ((lpCfg->tpc.c.bIsOracle) ? (slevel_text_ora) : (slevel_text_mssql)))

static int
slevel (test_t * lpCfg)
{
  RETCODE rc;
  long w_id = lpCfg->tpc.c.local_w_id;
  long d_id = RandomNumber (&lpCfg->tpc.c.rnd_seed, 1, DIST_PER_WARE);
  long threshold = 20;
  long count;
  SDWORD count_len = sizeof (long);
  DECLARE_FOR_SQLERROR;

deadlock_sl:
  if (!lpCfg->tpc.c.slevel_stmt)
    {
      INIT_STMT (lpCfg->hdbc, lpCfg->tpc.c.slevel_stmt, SLEVEL_TEXT, lpCfg);
    }
  IBINDL (lpCfg->tpc.c.slevel_stmt, 1, w_id);
  IBINDL (lpCfg->tpc.c.slevel_stmt, 2, d_id);
  IBINDL (lpCfg->tpc.c.slevel_stmt, 3, threshold);
  SQLBindParameter (lpCfg->tpc.c.slevel_stmt, 4, SQL_PARAM_OUTPUT, SQL_C_LONG,
      SQL_INTEGER, 0, 0, &count, sizeof (SDWORD), &count_len);

  SQLSetStmtOption (lpCfg->tpc.c.slevel_stmt, SQL_CONCURRENCY,
      SQL_CONCUR_ROWVER);
  ta_enter (&lpCfg->tpc.c.slevel_ta);
  rc = SQLExecute (lpCfg->tpc.c.slevel_stmt);
  IF_DEADLOCK_OR_ERR_GO (lpCfg->tpc.c.slevel_stmt, err, rc, deadlock_sl);
  if (rc != SQL_NO_DATA_FOUND)
    if (stmt_result_sets (lpCfg->tpc.c.slevel_stmt))
      goto deadlock_sl;

  ta_leave (&lpCfg->tpc.c.slevel_ta);
  return 1;

err:
  SAVE_SQL_ERROR (lpCfg->szSQLState, lpCfg->szSQLError);
  ta_leave (&lpCfg->tpc.c.slevel_ta);
  return 0;
}


static const char ostat_text_kubl[] = "ostat (?, ?, ?, ?)";
static const char ostat_text_mssql[] = "exec ostat ?, ?, ?, ?";
static const char ostat_text_ora[] = "{call ostat(?, ?, ?, ?)}";

#define OSTAT_TEXT \
	((lpCfg->tpc.c.bIsVirtuoso) ? (ostat_text_kubl) : ((lpCfg->tpc.c.bIsOracle) ? (ostat_text_ora) : (ostat_text_mssql)))

static int
ostat (test_t * lpCfg)
{
  RETCODE rc;
  long w_id = lpCfg->tpc.c.local_w_id;
  long d_id = RandomNumber (&lpCfg->tpc.c.rnd_seed, 1, DIST_PER_WARE);
  long c_id = random_c_id (&lpCfg->tpc.c.rnd_seed);
  char c_last[50];
  DECLARE_FOR_SQLERROR;

deadlock_os:
  if (!lpCfg->tpc.c.ostat_stmt)
    {
      INIT_STMT (lpCfg->hdbc, lpCfg->tpc.c.ostat_stmt, OSTAT_TEXT, lpCfg);
    }
  if (RandomNumber (&lpCfg->tpc.c.rnd_seed, 0, 100) < 60)
    {
      c_id = 0;
      Lastname (RandomNumber (&lpCfg->tpc.c.rnd_seed, 0, 999), c_last);
    }
  IBINDL (lpCfg->tpc.c.ostat_stmt, 1, w_id);
  IBINDL (lpCfg->tpc.c.ostat_stmt, 2, d_id);
  IBINDL (lpCfg->tpc.c.ostat_stmt, 3, c_id);
  IBINDNTS (lpCfg->tpc.c.ostat_stmt, 4, c_last);

  ta_enter (&lpCfg->tpc.c.ostat_ta);
  rc = SQLExecute (lpCfg->tpc.c.ostat_stmt);
  IF_DEADLOCK_OR_ERR_GO (lpCfg->tpc.c.ostat_stmt, err, rc, deadlock_os);
  if (rc != SQL_NO_DATA_FOUND)
    if (stmt_result_sets (lpCfg->tpc.c.ostat_stmt))
      goto deadlock_os;

  ta_leave (&lpCfg->tpc.c.ostat_ta);
  return 1;

err:
  SAVE_SQL_ERROR (lpCfg->szSQLState, lpCfg->szSQLError);
  ta_leave (&lpCfg->tpc.c.ostat_ta);
  return 0;
}


/* #define NO_ONLY */

static int
do_10_pack (test_t * lpCfg)
{
  int n;
  long start = get_msec_count (), duration;

  ta_enter (&lpCfg->tpc.c.ten_pack_ta);
  for (n = 0; n < 10; n++)
    {
      if (!new_order (lpCfg))
	return 0;
#ifndef NO_ONLY
      if (!payment (lpCfg))
	return 0;
#endif
    }

#ifndef NO_ONLY
  ta_enter (&lpCfg->tpc.c.delivery_ta);
  if (_stristr (lpCfg->szDBMS, "Virtuoso"))
    {
      for (n = 1; n <= 10; n++)
	if (!delivery_1 (lpCfg, lpCfg->tpc.c.local_w_id, n))
	  return 0;
    }
  else
    {
      if (!delivery_1 (lpCfg, lpCfg->tpc.c.local_w_id, 0))
	return 0;
    }
  ta_leave (&lpCfg->tpc.c.delivery_ta);
  if (!slevel (lpCfg))
    return 0;
  if (!ostat (lpCfg))
    return 0;
#endif

  ta_leave (&lpCfg->tpc.c.ten_pack_ta);
  duration = get_msec_count () - start;
  lpCfg->tpc.c.tpcc_sum += 600000 / duration;
  pane_log ("-- %ld tpmC\n", 600000 / duration);
  return (600000 / duration);
}


void
reset_times (test_t * lpCfg)
{
  ta_init (&lpCfg->tpc.c.new_order_ta, "NEW ORDER");
  ta_init (&lpCfg->tpc.c.payment_ta, "PAYMENT");
  ta_init (&lpCfg->tpc.c.delivery_ta, "DELIVERY");
  ta_init (&lpCfg->tpc.c.slevel_ta, "STOCK LEVEL");
  ta_init (&lpCfg->tpc.c.ostat_ta, "ORDER STATUS");
  ta_init (&lpCfg->tpc.c.ten_pack_ta, "10 Pack");
}


static int
get_warehouse_count (test_t * lpCfg)
{
  LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.misc_stmt,
      "select count(*) from WAREHOUSE", lpCfg);

  IBINDL (lpCfg->tpc.c.misc_stmt, 1, lpCfg->tpc.c.count_ware);

  IF_ERR_GO (lpCfg->tpc.c.misc_stmt, err,
      SQLExecute (lpCfg->tpc.c.misc_stmt), lpCfg);
  IF_ERR_GO (lpCfg->tpc.c.misc_stmt, err, SQLFetch (lpCfg->tpc.c.misc_stmt), lpCfg);

  lpCfg->tpc.c.local_w_id =
      RandomNumber (&lpCfg->tpc.c.rnd_seed, 0,
      lpCfg->tpc.c.count_ware - 1) + 1;

  SQLFreeStmt (lpCfg->tpc.c.misc_stmt, SQL_DROP);
  lpCfg->tpc.c.misc_stmt = (HSTMT) 0;
  return lpCfg->tpc.c.count_ware;

err:
  SQLFreeStmt (lpCfg->tpc.c.misc_stmt, SQL_DROP);
  lpCfg->tpc.c.misc_stmt = (HSTMT) 0;

  lpCfg->tpc.c.count_ware = 0;
  lpCfg->tpc.c.local_w_id = 0;
  return 0;
}

int
tpcc_run_test (void * widget, test_t * lpCfg)
{
  char szTemp[128];
  time_t start_time, curr_time;
  double dDiff = 0;


  lpCfg->tpc.c.bIsOracle = FALSE;
  lpCfg->tpc.c.bIsVirtuoso = FALSE;
  lpCfg->tpc.c.bIsSybase = FALSE;
  if (_stristr (lpCfg->szDBMS, "Virtuoso"))
    {
      lpCfg->tpc.c.bIsVirtuoso = TRUE;
    }
  else if (_stristr (lpCfg->szDBMS, "SQL Server"))
    {
      lpCfg->tpc.c.bIsVirtuoso = FALSE;
    }
  else if (_stristr (lpCfg->szDBMS, "Oracle"))
    {
      lpCfg->tpc.c.bIsVirtuoso = FALSE;
      lpCfg->tpc.c.bIsOracle = TRUE;
    }
  else if (_stristr (lpCfg->szDBMS, "Sybase"))
    {
      lpCfg->tpc.c.bIsSybase = TRUE;
      lpCfg->tpc.c.bIsVirtuoso = FALSE;
      lpCfg->tpc.c.bIsOracle = FALSE;
    }
  else
    {
      return 0;
    }

  if (!get_warehouse_count (lpCfg))
    return 0;

  reset_times (lpCfg);
  lpCfg->tpc.c.nRounds = 0;

  lpCfg->ShowProgress (NULL, "Running TPCC", FALSE,
      lpCfg->tpc._.nMinutes * 60);
  lpCfg->SetWorkingItem ("Running TPCC");
  time (&start_time);
  while (1)
    {
      time (&curr_time);
      dDiff = difftime (curr_time, start_time);
      if (dDiff >= lpCfg->tpc._.nMinutes * 60)
	break;
      sprintf (szTemp, "%ld tenpacks, %10ld secs remaining",
	  lpCfg->tpc.c.nRounds,
	  (long int) (lpCfg->tpc._.nMinutes * 60 - dDiff));
      lpCfg->SetProgressText (szTemp, lpCfg->tpc._.nConn,
	  lpCfg->tpc._.nThreadNo, curr_time - start_time, 1);
      if (lpCfg->fCancel ())
	break;
      if (!do_10_pack (lpCfg))
	break;
      lpCfg->tpc.c.nRounds += 1;
    }
  lpCfg->StopProgress ();
  lpCfg->tpc.c.run_time = lpCfg->tpc.c.ten_pack_ta.ta_total / 1000;
  return 1;
}
