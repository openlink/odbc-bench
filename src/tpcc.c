/*
 *  tpcc.c
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

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "odbcbench.h"
#include "tpccfun.h"
#include "tpca_code.h"

static int LoadItems (test_t * lpCfg);
static int LoadWare (test_t * lpCfg);
static int LoadCust (test_t * lpCfg);
static int LoadOrd (test_t * lpCfg);
static int Stock (test_t * lpCfg, long w_id_from, long w_id_to);
static int District (test_t * lpCfg, long w_id);
static int Customer (test_t * lpCfg, long, long);
static int Orders (test_t * lpCfg, long d_id, long w_id);
static void MakeAddress (long *rnd_seed, char *str1, char *str2, char *city,
    char *state, char *zip);

#define IF_CANCEL_RETURN(rc) \
	  if (lpCfg->fCancel()) \
		  return rc;

#define IF_CANCEL_GO(tag) \
	  if (lpCfg->fCancel()) \
		  goto tag;
long
RandomNumber (long *rnd_seed, long x, long y)
{
  return (random_1 (rnd_seed, (y - x) + 1) + x);
}


static long
NURand (long *rnd_seed, int a, int x, int y)
{
  return ((((RandomNumber (rnd_seed, 0, a) |
		  RandomNumber (rnd_seed, x, y)) + 1234567)
	  % (y - x + 1)) + x);
}


int
MakeAlphaString (long *rnd_seed, int sz1, int sz2, char *str)
{
  int sz = RandomNumber (rnd_seed, sz1, sz2);
  int inx;
  for (inx = 0; inx < sz; inx++)
    str[inx] = 'a' + (inx % 24);
  str[sz - 1] = 0;
  return sz - 1;
}


static void
MakeNumberString (int sz, int sz2, char *str)
{
  int inx;
  for (inx = 0; inx < sz; inx++)
    str[inx] = '0' + (inx % 10);
  str[sz - 1] = 0;
}


long
random_c_id (long *rnd_seed)
{
  return (NURand (rnd_seed, 1023, 1, 3000));
}


long
random_i_id (long *rnd_seed)
{
  return (NURand (rnd_seed, 8191, 1, 100000));
}


void
scrap_log (test_t * lpCfg, HSTMT stmt)
{
  if (_stristr (lpCfg->szDBMS, "SQL Server"))
    {
      IS_ERR (stmt, SQLExecDirect (stmt,
	      (UCHAR *)
	      "dump transaction tpcc to disk='null.dat' with no_log",
	      SQL_NTS), lpCfg);
    }
  else if (_stristr (lpCfg->szDBMS, "Virtuoso"))
    {
      IS_ERR (stmt, SQLExecDirect (stmt, (UCHAR *) "checkpoint", SQL_NTS), lpCfg);
    }
  else if (_stristr (lpCfg->szDBMS, "Oracle"))
    {
      IS_ERR (stmt, SQLExecDirect (stmt,
	      (UCHAR *) "alter system checkpoint", SQL_NTS), lpCfg);
    }
}

static void
gettimestamp_2 (char *ts)
{
  struct timeval tv;
  gettimestamp (&tv);
  memcpy (ts, &tv, sizeof (tv));
}


void
tpcc_create_db (void * widget, test_t * lpCfg)
{

  int i;
  long start_time = -1, end_time = -1;

  do_login (lpCfg);

  lpCfg->tpc.c.bIsOracle = FALSE;
  lpCfg->tpc.c.bIsSybase = FALSE;
  lpCfg->tpc.c.bIsVirtuoso = FALSE;
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
      lpCfg->tpc.c.bIsVirtuoso = FALSE;
      lpCfg->tpc.c.bIsOracle = FALSE;
      lpCfg->tpc.c.bIsSybase = TRUE;
    }
  lpCfg->tpc.c.run_time = lpCfg->tpc.c.tpcc_sum = -1;

  if (!lpCfg->hdbc)
    {
      pane_log ("Not Connected\n");
      return;
    }

  for (i = 0; i < BATCH_SIZE; i++)
    gettimestamp_2 (lpCfg->tpc.c.timestamp_array[i]);

  pane_log ("\nTPCC Data Load Started...\n");
  if (lpCfg->ShowProgress)
    lpCfg->ShowProgress (NULL, "TPCC Data Load", TRUE, 1);

  IF_CANCEL_GO (done);

  start_time = get_msec_count ();

  if (LoadWare (lpCfg))
    scrap_log (lpCfg, lpCfg->tpc.c.misc_stmt);
  else
    goto done;

  IF_CANCEL_GO (done);

  if (LoadItems (lpCfg))
    scrap_log (lpCfg, lpCfg->tpc.c.misc_stmt);
  else
    goto done;

  IF_CANCEL_GO (done);

  if (LoadCust (lpCfg))
    scrap_log (lpCfg, lpCfg->tpc.c.misc_stmt);
  else
    goto done;

  IF_CANCEL_GO (done);

  if (LoadOrd (lpCfg))
    scrap_log (lpCfg, lpCfg->tpc.c.misc_stmt);
  else
    goto done;

  IF_CANCEL_GO (done);

  end_time = get_msec_count ();
  pane_log ("DATA LOADING COMPLETED SUCCESSFULLY.\n");
  if (start_time != -1 && end_time != -1)
    {
      lpCfg->tpc.c.run_time = (end_time - start_time) / 1000;
      add_tpcc_result (lpCfg);
    }
done:
  tpcc_close_stmts (widget, lpCfg);
  do_logout (lpCfg);
  lpCfg->StopProgress ();
}

static int
LoadItems (test_t * lpCfg)
{
  long i;
  int fill = 0;
  long i_id_1;
  static long i_id[BATCH_SIZE];
  static char i_name[BATCH_SIZE][24];
  static float i_price[BATCH_SIZE];
  static char i_data[BATCH_SIZE][50];
  char szTemp[128];

  int idatasiz;
  static short orig[MAXITEMS];
  long pos;

  LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.item_stmt,
      "insert into ITEM (I_ID, I_NAME, I_PRICE, I_DATA) values (?, ?, ?, ?)", lpCfg);
  IBINDL (lpCfg->tpc.c.item_stmt, 1, i_id);
  IBINDNTS (lpCfg->tpc.c.item_stmt, 2, i_name);
  IBINDF (lpCfg->tpc.c.item_stmt, 3, i_price);
  IBINDNTS (lpCfg->tpc.c.item_stmt, 4, i_data);

  lpCfg->SetWorkingItem ("Loading ITEM");
  lpCfg->SetProgressText ("", 0, 0, 0, 1, 0, 0);
  pane_log ("Loading ITEM\n");

  for (i = 0; i < MAXITEMS / 10; i++)
    orig[i] = 0;

  for (i = 0; i < MAXITEMS / 10; i++)
    {
      do
	{
	  pos = RandomNumber (&lpCfg->tpc.c.rnd_seed, 0L, MAXITEMS);
	}
      while (orig[pos]);
      orig[pos] = 1;
    }


  for (i_id_1 = 1; i_id_1 <= MAXITEMS; i_id_1++)
    {
      IF_CANCEL_RETURN (0);
      /* Generate Item Data */
      i_id[fill] = i_id_1;
      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 14, 24, i_name[fill]);
      i_price[fill] =  ((float) RandomNumber (&lpCfg->tpc.c.rnd_seed, 100L,
	10000L)) / 100.0;
      idatasiz =
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 26, 50, i_data[fill]);
      if (orig[i_id_1])
	{
	  pos = RandomNumber (&lpCfg->tpc.c.rnd_seed, 0L, idatasiz - 8);
	  i_data[fill][pos] = 'o';
	  i_data[fill][pos + 1] = 'r';
	  i_data[fill][pos + 2] = 'i';
	  i_data[fill][pos + 3] = 'g';
	  i_data[fill][pos + 4] = 'i';
	  i_data[fill][pos + 5] = 'n';
	  i_data[fill][pos + 6] = 'a';
	  i_data[fill][pos + 7] = 'l';
	}

      CHECK_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.item_stmt, fill, lpCfg);

      if (!(i_id_1 % 100))
	{
	  sprintf (szTemp, "%ld items loaded", i_id_1);
	  lpCfg->SetProgressText (szTemp, 0, 0, ((float) i_id_1) / MAXITEMS, 1, 0, 0);
	}
    }

  FLUSH_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.item_stmt, fill, lpCfg);

  return (1);
}


static int
LoadWare (test_t * lpCfg)
{
  long w_id;
  char w_name[10];
  char w_street_1[20];
  char w_street_2[20];
  char w_city[20];
  char w_state[2];
  char w_zip[9];
  float w_tax;
  float w_ytd;

  LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.ware_stmt,
      "insert into WAREHOUSE (W_ID, W_NAME,"
      "    W_STREET_1, W_STREET_2, W_CITY, W_STATE, W_ZIP, W_TAX, W_YTD)"
      "  values (?, ?, ?, ?, ?, ?, ?, ?, ?)", lpCfg);
  IBINDL (lpCfg->tpc.c.ware_stmt, 1, w_id);
  IBINDNTS (lpCfg->tpc.c.ware_stmt, 2, w_name);
  IBINDNTS (lpCfg->tpc.c.ware_stmt, 3, w_street_1);
  IBINDNTS (lpCfg->tpc.c.ware_stmt, 4, w_street_2);
  IBINDNTS (lpCfg->tpc.c.ware_stmt, 5, w_city);
  IBINDNTS (lpCfg->tpc.c.ware_stmt, 6, w_state);
  IBINDNTS (lpCfg->tpc.c.ware_stmt, 7, w_zip);
  IBINDF (lpCfg->tpc.c.ware_stmt, 8, w_tax);
  IBINDF (lpCfg->tpc.c.ware_stmt, 9, w_ytd);

  lpCfg->SetWorkingItem ("Loading WAREHOUSE");
  lpCfg->SetProgressText ("", 0, 0, 0, 1, 0, 0);
  pane_log ("Loading WAREHOUSE\n");
  for (w_id = 1; w_id <= lpCfg->tpc.c.count_ware; w_id++)
    {
      IF_CANCEL_RETURN (0);
      /* Generate Warehouse Data */
      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 6, 10, w_name);
      MakeAddress (&lpCfg->tpc.c.rnd_seed, w_street_1, w_street_2, w_city,
	  w_state, w_zip);
      w_tax =
	  ((float) RandomNumber (&lpCfg->tpc.c.rnd_seed, 10L, 20L)) / 100.0;
      w_ytd = 3000000.00;

      if (lpCfg->tpc.c.option_debug)
	pane_log ("WID = %ld, Name= %16s, Tax = %5.2f", w_id, w_name, w_tax);

      IF_ERR_RETURN (lpCfg->tpc.c.ware_stmt,
	  SQLExecute (lpCfg->tpc.c.ware_stmt), 0, lpCfg);

/** Make Rows associated with Warehouse **/
      if (!District (lpCfg, w_id))
	return (0);
    }
  return (Stock (lpCfg, 1, lpCfg->tpc.c.count_ware));
}


static int
LoadCust (test_t * lpCfg)
{
  long w_id;
  long d_id;

  for (w_id = 1L; w_id <= lpCfg->tpc.c.count_ware; w_id++)
    for (d_id = 1L; d_id <= DIST_PER_WARE; d_id++)
      if (!Customer (lpCfg, d_id, w_id))
	return (0);
  SQLTransact (SQL_NULL_HENV, lpCfg->hdbc, SQL_COMMIT);

  return 1;
}


static int
LoadOrd (test_t * lpCfg)
{
  long w_id;
  /* float w_tax; */
  long d_id;
  /* float d_tax; */

  for (w_id = 1L; w_id <= lpCfg->tpc.c.count_ware; w_id++)
    for (d_id = 1L; d_id <= DIST_PER_WARE; d_id++)
      if (!Orders (lpCfg, d_id, w_id))
	return 0;

  SQLTransact (SQL_NULL_HENV, lpCfg->hdbc, SQL_COMMIT);
  return 1;
}


static int
Stock (test_t * lpCfg, long w_id_from, long w_id_to)
{
  long w_id;
  long s_i_id_1;
  static long s_i_id[BATCH_SIZE];
  static long s_w_id[BATCH_SIZE];
  static long s_quantity[BATCH_SIZE];
  static char s_dist_01[BATCH_SIZE][24];
  static char s_dist_02[BATCH_SIZE][24];
  static char s_dist_03[BATCH_SIZE][24];
  static char s_dist_04[BATCH_SIZE][24];
  static char s_dist_05[BATCH_SIZE][24];
  static char s_dist_06[BATCH_SIZE][24];
  static char s_dist_07[BATCH_SIZE][24];
  static char s_dist_08[BATCH_SIZE][24];
  static char s_dist_09[BATCH_SIZE][24];
  static char s_dist_10[BATCH_SIZE][24];
  static char s_data[BATCH_SIZE][50];

  int fill = 0;
  int sdatasiz;
  long orig[MAXITEMS];
  long pos;
  int i;
  char szTemp[128];

  LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.stock_stmt,
      "insert into STOCK"
      "   (S_I_ID, S_W_ID, S_QUANTITY,"
      "S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,"
      "S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10,"
      "S_DATA, S_YTD, S_CNT_ORDER, S_CNT_REMOTE)"
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0,0,0)", lpCfg);

  IBINDL (lpCfg->tpc.c.stock_stmt, 1, s_i_id);
  IBINDL (lpCfg->tpc.c.stock_stmt, 2, s_w_id);
  IBINDL (lpCfg->tpc.c.stock_stmt, 3, s_quantity);

  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 4, s_dist_01);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 5, s_dist_02);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 6, s_dist_03);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 7, s_dist_04);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 8, s_dist_05);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 9, s_dist_06);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 10, s_dist_07);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 11, s_dist_08);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 12, s_dist_09);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 13, s_dist_10);
  IBINDNTS_ARRAY (lpCfg->tpc.c.stock_stmt, 14, s_data);

  sprintf (szTemp, "Loading STOCK for Wid=%ld-%ld", w_id_from, w_id_to);
  lpCfg->SetWorkingItem (szTemp);
  lpCfg->SetProgressText ("", 0, 0, 0, 1, 0, 0);
  pane_log (szTemp);
  pane_log ("\n");

  for (i = 0; i < MAXITEMS / 10; i++)
    orig[i] = 0;
  for (i = 0; i < MAXITEMS / 10; i++)
    {
      do
	{
	  pos = RandomNumber (&lpCfg->tpc.c.rnd_seed, 0L, MAXITEMS);
	}
      while (orig[pos]);
      orig[pos] = 1;
    }

  for (s_i_id_1 = 1; s_i_id_1 <= MAXITEMS; s_i_id_1++)
    {
      if (s_i_id_1 % 100 == 0)
	{
	  IF_CANCEL_RETURN (0);
	  sprintf (szTemp, "%ld Stock Items done", s_i_id_1);
	  lpCfg->SetProgressText (szTemp, 0, 0,
	      ((float) s_i_id_1) / (MAXITEMS), 1, 0, 0);
	}
      for (w_id = w_id_from; w_id <= w_id_to; w_id++)
	{
	  /* Generate Stock Data */
	  s_i_id[fill] = s_i_id_1;
	  s_w_id[fill] = w_id;
	  s_quantity[fill] = RandomNumber (&lpCfg->tpc.c.rnd_seed, 10L, 100L);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_01[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_02[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_03[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_04[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_05[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_06[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_07[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_08[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_09[fill]);
	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24, s_dist_10[fill]);

	  sdatasiz =
	      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 26, 50, s_data[fill]);

	  if (orig[s_i_id_1])
	    {
	      pos = RandomNumber (&lpCfg->tpc.c.rnd_seed, 0L, sdatasiz - 8);
	      s_data[fill][pos] = 'o';
	      s_data[fill][pos + 1] = 'r';
	      s_data[fill][pos + 2] = 'i';
	      s_data[fill][pos + 3] = 'g';
	      s_data[fill][pos + 4] = 'i';
	      s_data[fill][pos + 5] = 'n';
	      s_data[fill][pos + 6] = 'a';
	      s_data[fill][pos + 7] = 'l';
	    }

	  CHECK_BATCH (SQL_NULL_HENV, lpCfg->hdbc, lpCfg->tpc.c.stock_stmt,
	      fill, lpCfg);
	}
    }
  FLUSH_BATCH (SQL_NULL_HENV, lpCfg->hdbc, lpCfg->tpc.c.stock_stmt, fill, lpCfg);

  return 1;
}


static int
District (test_t * lpCfg, long w_id)
{
  long d_id;
  long d_w_id;
  char d_name[10];
  char d_street_1[20];
  char d_street_2[20];
  char d_city[20];
  char d_state[2];
  char d_zip[9];
  float d_tax;
  float d_ytd;
  long d_next_o_id;

  LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.dist_stmt,
      "insert into DISTRICT"
      " (D_ID, D_W_ID, D_NAME, "
      "D_STREET_1, D_STREET_2, D_CITY, D_STATE, D_ZIP,"
      "D_TAX, D_YTD, D_NEXT_O_ID)" "values (?,?,?,?,?,  ?,?,?,?,?,  ?)", lpCfg);

  IBINDL (lpCfg->tpc.c.dist_stmt, 1, d_id);
  IBINDL (lpCfg->tpc.c.dist_stmt, 2, d_w_id);
  IBINDNTS (lpCfg->tpc.c.dist_stmt, 3, d_name);
  IBINDNTS (lpCfg->tpc.c.dist_stmt, 4, d_street_1);
  IBINDNTS (lpCfg->tpc.c.dist_stmt, 5, d_street_2);
  IBINDNTS (lpCfg->tpc.c.dist_stmt, 6, d_city);
  IBINDNTS (lpCfg->tpc.c.dist_stmt, 7, d_state);
  IBINDNTS (lpCfg->tpc.c.dist_stmt, 8, d_zip);
  IBINDF (lpCfg->tpc.c.dist_stmt, 9, d_tax);
  IBINDF (lpCfg->tpc.c.dist_stmt, 10, d_ytd);
  IBINDL (lpCfg->tpc.c.dist_stmt, 11, d_next_o_id);

  lpCfg->SetWorkingItem ("Loading DISTRICT");
  lpCfg->SetProgressText ("", 0, 0, 0, 1, 0, 0);
  pane_log ("Loading DISTRICT\n");

  d_w_id = w_id;
  d_ytd = 30000.0;
  d_next_o_id = 3001L;
  for (d_id = 1; d_id <= DIST_PER_WARE; d_id++)
    {
      IF_CANCEL_RETURN (0);
      /* Generate District Data */
      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 6, 10, d_name);
      MakeAddress (&lpCfg->tpc.c.rnd_seed, d_street_1, d_street_2, d_city,
	  d_state, d_zip);
      d_tax =
	  ((float) RandomNumber (&lpCfg->tpc.c.rnd_seed, 10L, 20L)) / 100.0;

      IF_ERR_RETURN (lpCfg->tpc.c.dist_stmt,
	  SQLExecute (lpCfg->tpc.c.dist_stmt), 0, lpCfg);

      if (lpCfg->tpc.c.option_debug)
	pane_log ("DID = %ld, WID = %ld, Name = %10s, Tax = %5.2f",
	    d_id, d_w_id, d_name, d_tax);
    }
  SQLTransact (SQL_NULL_HENV, lpCfg->hdbc, SQL_COMMIT);

  return 1;
}


static int
Customer (test_t * lpCfg, long d_id_1, long w_id_1)
{
  long c_id_1;
  static long w_id[BATCH_SIZE];
  static long c_id[BATCH_SIZE];
  static long c_d_id[BATCH_SIZE];
  static long c_w_id[BATCH_SIZE];
  static char c_first[BATCH_SIZE][16];
  static char c_middle[BATCH_SIZE][2];
  static char c_last[BATCH_SIZE][16];
  static char c_street_1[BATCH_SIZE][20];
  static char c_street_2[BATCH_SIZE][20];
  static char c_city[BATCH_SIZE][20];
  static char c_state[BATCH_SIZE][2];
  static char c_zip[BATCH_SIZE][9];
  static char c_phone[BATCH_SIZE][16];
  static char c_credit[BATCH_SIZE][2];	/*initial 0's */
  static float c_credit_lim[BATCH_SIZE];
  static float c_discount[BATCH_SIZE];
  static float c_balance[BATCH_SIZE];
  static char c_data_1[BATCH_SIZE][250];
  static char c_data_2[BATCH_SIZE][250];
  static float h_amount[BATCH_SIZE];
  static char h_data[BATCH_SIZE][24];

  int fill = 0, h_fill = 0;
  char szTemp[128];

  if (lpCfg->tpc.c.bIsOracle)
    {
      LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.cs_stmt,
	  "insert into CUSTOMER (C_ID, C_D_ID, C_W_ID,"
	  "C_FIRST, C_MIDDLE, C_LAST, "
	  "C_STREET_1, C_STREET_2, C_CITY, C_STATE, C_ZIP,"
	  "C_PHONE, C_SINCE, C_CREDIT, "
	  "C_CREDIT_LIM, C_DISCOUNT, C_BALANCE, C_DATA_1, C_DATA_2,"
	  "C_YTD_PAYMENT, C_CNT_PAYMENT, C_CNT_DELIVERY) "
	  "values (?,?,?,?,?,   ?,?,?,?,?,   ?,?,sysdate,?, ?,   ?,?,?,?,"
	  "10.0, 1, 0)", lpCfg);
    }
  else
    {
      LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.cs_stmt,
	  "insert into CUSTOMER (C_ID, C_D_ID, C_W_ID,"
	  "C_FIRST, C_MIDDLE, C_LAST, "
	  "C_STREET_1, C_STREET_2, C_CITY, C_STATE, C_ZIP,"
	  "C_PHONE, C_SINCE, C_CREDIT, "
	  "C_CREDIT_LIM, C_DISCOUNT, C_BALANCE, C_DATA_1, C_DATA_2,"
	  "C_YTD_PAYMENT, C_CNT_PAYMENT, C_CNT_DELIVERY) "
	  "values (?,?,?,?,?,   ?,?,?,?,?,   ?,?,getdate(),?, ?,   ?,?,?,?,"
	  "10.0, 1, 0)", lpCfg);
    }

  IBINDL (lpCfg->tpc.c.cs_stmt, 1, c_id);
  IBINDL (lpCfg->tpc.c.cs_stmt, 2, c_d_id);
  IBINDL (lpCfg->tpc.c.cs_stmt, 3, c_w_id);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 4, c_first);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 5, c_middle);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 6, c_last);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 7, c_street_1);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 8, c_street_2);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 9, c_city);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 10, c_state);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 11, c_zip);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 12, c_phone);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 13, c_credit);
  IBINDF (lpCfg->tpc.c.cs_stmt, 14, c_credit_lim);
  IBINDF (lpCfg->tpc.c.cs_stmt, 15, c_discount);
  IBINDF (lpCfg->tpc.c.cs_stmt, 16, c_balance);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 17, c_data_1);
  IBINDNTS_ARRAY (lpCfg->tpc.c.cs_stmt, 18, c_data_2);

  if (lpCfg->tpc.c.bIsOracle)
    {
      LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.h_stmt,
	  "insert into THISTORY ("
	  "  H_C_ID, H_C_D_ID, H_C_W_ID, H_W_ID, H_D_ID, H_DATE, H_AMOUNT, H_DATA)"
	  "values (?,?,?,?,  ?,sysdate,?,?)", lpCfg);
    }
  else
    {
      LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.h_stmt,
	  "insert into THISTORY ("
	  "  H_C_ID, H_C_D_ID, H_C_W_ID, H_W_ID, H_D_ID, H_DATE, H_AMOUNT, H_DATA)"
	  "values (?,?,?,?,  ?,getdate(),?,?)", lpCfg);
    }

  IBINDL (lpCfg->tpc.c.h_stmt, 1, c_id);
  IBINDL (lpCfg->tpc.c.h_stmt, 2, c_d_id);
  IBINDL (lpCfg->tpc.c.h_stmt, 3, c_w_id);
  IBINDL (lpCfg->tpc.c.h_stmt, 4, c_w_id);
  IBINDL (lpCfg->tpc.c.h_stmt, 5, c_d_id);
  IBINDF (lpCfg->tpc.c.h_stmt, 6, h_amount);
  IBINDNTS_ARRAY (lpCfg->tpc.c.h_stmt, 7, h_data);

  sprintf (szTemp, "Loading CUSTOMER for DID=%ld, WID=%ld", d_id_1, w_id_1);
  lpCfg->SetWorkingItem (szTemp);
  lpCfg->SetProgressText ("", 0, 0, 0, 1, 0, 0);
  pane_log (szTemp);
  pane_log ("\n");

  for (c_id_1 = 1; c_id_1 <= CUST_PER_DIST; c_id_1++)
    {
      IF_CANCEL_RETURN (0);
      /* Generate Customer Data */
      w_id[fill] = w_id_1;
      c_id[fill] = c_id_1;
      c_d_id[fill] = d_id_1;
      c_w_id[fill] = w_id_1;

      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 8, 15, c_first[fill]);
      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 240, 240, c_data_1[fill]);
      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 240, 240, c_data_2[fill]);
      c_middle[fill][0] = 'J';
      c_middle[fill][1] = 0;
      if (c_id_1 <= 1000)
	Lastname (c_id_1 - 1, c_last[fill]);
      else
	Lastname (NURand (&lpCfg->tpc.c.rnd_seed, 255, 0, 999), c_last[fill]);
      MakeAddress (&lpCfg->tpc.c.rnd_seed, c_street_1[fill], c_street_2[fill],
	  c_city[fill], c_state[fill], c_zip[fill]);
      MakeNumberString (16, 16, c_phone[fill]);
      if (RandomNumber (&lpCfg->tpc.c.rnd_seed, 0L, 1L))
	c_credit[fill][0] = 'G';
      else
	c_credit[fill][0] = 'B';
      c_credit[fill][1] = 'C';
      c_credit_lim[fill] = 500;
      c_discount[fill] =
	  ((float) RandomNumber (&lpCfg->tpc.c.rnd_seed, 0L, 50L)) / 100.0;
      c_balance[fill] = 10.0;

      CHECK_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.cs_stmt, fill, lpCfg);

      gettimestamp (lpCfg->tpc.c.timestamp_array[h_fill]);
      h_amount[h_fill] = 10.0;
      MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 12, 24, h_data[h_fill]);

      CHECK_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.h_stmt, h_fill, lpCfg);
      if (!(c_id_1 % 100))
	{
	  sprintf (szTemp, "%ld customers done", c_id_1);
	  lpCfg->SetProgressText (szTemp, 0, 0,
	      ((float) c_id_1) / CUST_PER_DIST, 1, 0, 0);
	}
    }
  FLUSH_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.cs_stmt, fill, lpCfg);
  FLUSH_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.h_stmt, h_fill, lpCfg);

  return 1;
}


static int
Orders (test_t * lpCfg, long d_id, long w_id)
{
  long ol_1;
  long o_id_1;
  static long o_id[BATCH_SIZE];
  static long o_c_id[BATCH_SIZE];
  static long o_d_id[BATCH_SIZE];
  static long o_w_id[BATCH_SIZE];
  static long o_carrier_id[BATCH_SIZE];
  static long o_ol_cnt[BATCH_SIZE];
  static long ol[BATCH_SIZE];
  static long ol_i_id[BATCH_SIZE];
  static long ol_supply_w_id[BATCH_SIZE];
  static long ol_quantity[BATCH_SIZE];
  static long ol_amount[BATCH_SIZE];
  static char ol_dist_info[BATCH_SIZE][24];
  static long ol_o_id[BATCH_SIZE];
  static long ol_o_d_id[BATCH_SIZE];
  static long ol_o_w_id[BATCH_SIZE];
  int fill = 0, ol_fill = 0;
  char szTemp[128];

  if (lpCfg->tpc.c.bIsOracle)
    {
      LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.o_stmt,
	  "insert into "
	  " ORDERS (O_ID, O_C_ID, O_D_ID, O_W_ID, "
	  "O_ENTRY_D, O_CARRIER_ID, O_OL_CNT, O_ALL_LOCAL)"
	  "values (?,?,?,?,  sysdate,?,?, 1)", lpCfg);
    }
  else
    {
      LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.o_stmt,
	  "insert into "
	  " ORDERS (O_ID, O_C_ID, O_D_ID, O_W_ID, "
	  "O_ENTRY_D, O_CARRIER_ID, O_OL_CNT, O_ALL_LOCAL)"
	  "values (?,?,?,?,  getdate (),?,?, 1)", lpCfg);
    }

  IBINDL (lpCfg->tpc.c.o_stmt, 1, o_id);
  IBINDL (lpCfg->tpc.c.o_stmt, 2, o_c_id);
  IBINDL (lpCfg->tpc.c.o_stmt, 3, o_d_id);
  IBINDL (lpCfg->tpc.c.o_stmt, 4, o_w_id);
  IBINDL (lpCfg->tpc.c.o_stmt, 5, o_carrier_id);
  IBINDL (lpCfg->tpc.c.o_stmt, 6, o_ol_cnt);

  LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.ol_stmt,
      "insert into "
      " ORDER_LINE (OL_O_ID, OL_D_ID, OL_W_ID, OL_NUMBER,"
      "OL_I_ID, OL_SUPPLY_W_ID, OL_QUANTITY, OL_AMOUNT,"
      "OL_DIST_INFO, OL_DELIVERY_D)" "values (?,?,?,?,?,  ?,?,?,?,  NULL)", lpCfg);

  IBINDL (lpCfg->tpc.c.ol_stmt, 1, ol_o_id);
  IBINDL (lpCfg->tpc.c.ol_stmt, 2, ol_o_d_id);
  IBINDL (lpCfg->tpc.c.ol_stmt, 3, ol_o_w_id);
  IBINDL (lpCfg->tpc.c.ol_stmt, 4, ol);
  IBINDL (lpCfg->tpc.c.ol_stmt, 5, ol_i_id);
  IBINDL (lpCfg->tpc.c.ol_stmt, 6, ol_supply_w_id);
  IBINDL (lpCfg->tpc.c.ol_stmt, 7, ol_quantity);
  IBINDL (lpCfg->tpc.c.ol_stmt, 8, ol_amount);
  IBINDNTS_ARRAY (lpCfg->tpc.c.ol_stmt, 9, ol_dist_info);

  LOCAL_STMT (lpCfg->hdbc, lpCfg->tpc.c.no_stmt,
      "insert into NEW_ORDER (NO_O_ID, NO_D_ID, NO_W_ID) values (?,?,?)", lpCfg);

  sprintf (szTemp, "Loading ORDERS for D=%ld, W= %ld", d_id, w_id);
  lpCfg->SetWorkingItem (szTemp);
  lpCfg->SetProgressText ("", 0, 0, 0, 1, 0, 0);
  pane_log (szTemp);
  pane_log ("\n");

  for (o_id_1 = 1; o_id_1 <= ORD_PER_DIST; o_id_1++)
    {
      IF_CANCEL_RETURN (0);
      /* Generate Order Data */
      o_id[fill] = o_id_1;
      o_d_id[fill] = d_id;
      o_w_id[fill] = w_id;
      o_c_id[fill] = RandomNumber (&lpCfg->tpc.c.rnd_seed, 1, CUST_PER_DIST);	/* GetPermutation(); */
      o_carrier_id[fill] = RandomNumber (&lpCfg->tpc.c.rnd_seed, 1L, 10L);
      o_ol_cnt[fill] = RandomNumber (&lpCfg->tpc.c.rnd_seed, 5L, 15L);

      /* the last 900 orders have not been delivered */
      if (o_id_1 > ORD_PER_DIST - 900)
	{
	  IBINDL (lpCfg->tpc.c.no_stmt, 1, o_id[fill]);
	  IBINDL (lpCfg->tpc.c.no_stmt, 2, o_d_id[fill]);
	  IBINDL (lpCfg->tpc.c.no_stmt, 3, o_w_id);

	  IF_ERR_RETURN (lpCfg->tpc.c.no_stmt,
	      SQLExecute (lpCfg->tpc.c.no_stmt), 0, lpCfg);
	}

      /* Generate Order Line Data */
      for (ol_1 = 1; ol_1 <= o_ol_cnt[fill]; ol_1++)
	{
	  ol[ol_fill] = ol_1;
	  ol[ol_fill] = ol_1;
	  ol_o_id[ol_fill] = o_id[fill];
	  ol_o_d_id[ol_fill] = o_d_id[fill];
	  ol_o_w_id[ol_fill] = o_w_id[fill];
	  ol_i_id[ol_fill] =
	      RandomNumber (&lpCfg->tpc.c.rnd_seed, 1L, MAXITEMS);
	  ol_supply_w_id[ol_fill] = o_w_id[fill];
	  ol_quantity[ol_fill] = 5;
	  ol_amount[ol_fill] = 0.0;

	  MakeAlphaString (&lpCfg->tpc.c.rnd_seed, 24, 24,
	      ol_dist_info[ol_fill]);

	  CHECK_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.ol_stmt, ol_fill, lpCfg);
	}
      CHECK_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.o_stmt, fill, lpCfg);

      if (!(o_id_1 % 100))
	{
	  sprintf (szTemp, "%ld orders done", o_id_1);
	  lpCfg->SetProgressText (szTemp, 0, 0,
	      ((float) o_id_1) / ORD_PER_DIST, 1, 0, 0);
	}
    }

  FLUSH_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.o_stmt, fill, lpCfg);
  FLUSH_BATCH (henv, lpCfg->hdbc, lpCfg->tpc.c.ol_stmt, ol_fill, lpCfg);

  return 1;
}


/*==================================================================+
 | ROUTINE NAME
 |      MakeAddress()
 | DESCRIPTION
 |      Build an Address
 | ARGUMENTS
 +==================================================================*/
void
MakeAddress (long *rnd_seed, char *str1, char *str2, char *city, char *state,
    char *zip)
{
  MakeAlphaString (rnd_seed, 10, 18, str1);	/* Street 1 */
  MakeAlphaString (rnd_seed, 10, 18, str2);	/* Street 2 */
  MakeAlphaString (rnd_seed, 10, 18, city);	/* City */
  MakeAlphaString (rnd_seed, 2, 2, state);	/* State */
  MakeNumberString (9, 9, zip);	/* Zip */
}


/*==================================================================+
 | ROUTINE NAME
 |      Lastname
 | DESCRIPTION
 |      TPC-C Lastname Function.
 | ARGUMENTS
 |      num  - non-uniform random number
 |      name - last name string
 +==================================================================*/
void
Lastname (int num, char *name)
{
  static char *n[] = {
    "BAR", "OUGHT", "ABLE", "PRI", "PRES",
    "ESE", "ANTI", "CALLY", "ATION", "EING"
  };

  strcpy (name, n[num / 100]);
  strcat (name, n[(num / 10) % 10]);
  strcat (name, n[num % 10]);

  return;
}


void
tpcc_init_globals (void * widget, test_t * lpCfg)
{
  int i;
  for (i = 0; i < BATCH_SIZE; i++)
    {
      lpCfg->tpc.c.sql_timelen_array[i] = 8;
    }
}

#define CLOSE_STMT(stmt) \
	if (lpCfg->tpc.c.stmt != SQL_NULL_HSTMT) \
      { \
		SQLFreeStmt(lpCfg->tpc.c.stmt, SQL_DROP); \
		lpCfg->tpc.c.stmt = SQL_NULL_HSTMT; \
      }

void
tpcc_close_stmts (void * widget, test_t * lpCfg)
{
  CLOSE_STMT (misc_stmt);
  CLOSE_STMT (new_order_stmt);
  CLOSE_STMT (payment_stmt);
  CLOSE_STMT (delivery_stmt);
  CLOSE_STMT (slevel_stmt);
  CLOSE_STMT (ostat_stmt);
  CLOSE_STMT (item_stmt);
  CLOSE_STMT (ware_stmt);
  CLOSE_STMT (stock_stmt);
  CLOSE_STMT (dist_stmt);
  CLOSE_STMT (cs_stmt);
  CLOSE_STMT (h_stmt);
  CLOSE_STMT (o_stmt);
  CLOSE_STMT (no_stmt);
  CLOSE_STMT (ol_stmt);
}


void
tpcc_schema_create (void * widget, test_t * lpBench)
{

  pane_log ("TPC-C SCHEMA CREATION STARTED\n\n");
  if (!strlen (lpBench->szLoginDSN))
    {
      pane_log ("No login data\n");
      return;
    }

  if (!lpBench->hstmt)
    {
      RETCODE rc = SQLAllocStmt (lpBench->hdbc, &lpBench->hstmt);
      if (rc != SQL_SUCCESS)
	{
	  lpBench->hstmt = 0;
	  return;
	}
    }

  if (_stristr (lpBench->szDBMS, "Virtuoso"))
    {
      vCreateVirtuosoTPCCTables (lpBench);
      pipe_trough_isql (lpBench->hdbc, "create.virt", 1);
    }
  else if (_stristr (lpBench->szDBMS, "SQL Server"))
    {
      vCreateTPCCTables (lpBench->szDBMS, lpBench->hstmt);
      vCreateTPCCIndices (lpBench->szDBMS, lpBench->hstmt);
      pipe_trough_isql (lpBench->hdbc, "create.mssql", 1);
    }
  else if (_stristr (lpBench->szDBMS, "Oracle"))
    {
      vCreateTPCCTables (lpBench->szDBMS, lpBench->hstmt);
      vCreateTPCCIndices (lpBench->szDBMS, lpBench->hstmt);
      pipe_trough_isql (lpBench->hdbc, "create.ora", 1);
    }
  else if (_stristr (lpBench->szDBMS, "Sybase"))
    {
      vCreateTPCCTables (lpBench->szDBMS, lpBench->hstmt);
      vCreateTPCCIndices (lpBench->szDBMS, lpBench->hstmt);
      pipe_trough_isql (lpBench->hdbc, "create.syb", 1);
    }
  else
    {
      pane_log ("No procedure definition script for the DBMS %s\n",
	  lpBench->szDBMS);
    }
  pane_log ("TPC-C SCHEMA CREATION FINISHED\n\n");
}


void
tpcc_schema_cleanup (void * widget, test_t * lpBench)
{

  static char *szTables[] = {
    "WAREHOUSE",
    "DISTRICT",
    "CUSTOMER",
    "THISTORY",
    "NEW_ORDER",
    "ORDERS",
    "ORDER_LINE",
    "ITEM",
    "STOCK"
  };
  int i;
  static char *szDropTable = "drop table %s\n";
  char szSQL[100];

  if (!strlen (lpBench->szLoginDSN))
    {
      pane_log ("No login data\n");
      return;
    }

  pane_log ("\n\nSCHEMA CLEANUP STARTED\n");
  do_login (lpBench);

  if (!lpBench->hstmt)
    {
      RETCODE rc = SQLAllocStmt (lpBench->hdbc, &lpBench->hstmt);
      if (rc != SQL_SUCCESS)
	{
	  lpBench->hstmt = 0;
	  return;
	}
    }
  if (lpBench->ShowProgress)
    lpBench->ShowProgress (widget, "Dropping the TPC-C tables", TRUE,
      sizeof (szTables) / sizeof (char *));
  for (i = 0; i < sizeof (szTables) / sizeof (char *); i++)
    {
      lpBench->SetProgressText (szTables[i], 0, 0, i, 1, 0, 0);
      sprintf (szSQL, szDropTable, szTables[i]);
      pane_log (szSQL);
      if (SQL_SUCCESS != SQLExecDirect (lpBench->hstmt, szSQL, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt, lpBench);
    }
  if (_stristr (lpBench->szDBMS, "Virtuoso"))
    {
      pipe_trough_isql (lpBench->hdbc, "drop.virt", 1);
    }
  else if (_stristr (lpBench->szDBMS, "SQL Server"))
    {
      pipe_trough_isql (lpBench->hdbc, "drop.mssql", 1);
    }
  else if (_stristr (lpBench->szDBMS, "Oracle"))
    {
      pipe_trough_isql (lpBench->hdbc, "drop.ora", 1);
    }
  else if (_stristr (lpBench->szDBMS, "Sybase"))
    {
      pipe_trough_isql (lpBench->hdbc, "drop.syb", 1);
    }
  else
    {
      pane_log ("No schema cleanup script for the DBMS %s\n",
	  lpBench->szDBMS);
    }
  lpBench->SetProgressText (szTables[i], 0, 0, 100, 1, 0, 0);
  lpBench->StopProgress ();
  do_logout (lpBench);
  pane_log ("\n\nSCHEMA CLEANUP FINISHED\n");
}
