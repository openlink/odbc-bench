/*
 *  tpcc.h
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
/* #define MINI */

#ifdef MINI

#define MAXITEMS      1000
#define CUST_PER_DIST 3000
#define DIST_PER_WARE 10
#define ORD_PER_DIST  300

#else

#define MAXITEMS      100000
#define CUST_PER_DIST 3000
#define DIST_PER_WARE 10
#define ORD_PER_DIST  3000

#endif


#define LOCAL_STMT(hdbc, stmt, text, _test_) \
  if (! stmt) { \
    INIT_STMT (hdbc, stmt, text, _test_); \
  }

#ifdef WIN32
#define dk_exit exit
#endif


#define CHECK_BATCH(henv, hdbc, stmt, fill, _test_) \
  if (fill >= BATCH_SIZE - 1) \
    { \
      SQLParamOptions (stmt, fill + 1, NULL); \
      IF_ERR_RETURN (stmt, SQLExecute (stmt), 0, _test_); \
      SQLTransact (henv, hdbc, SQL_COMMIT); \
      fill = 0; \
    } \
  else \
    fill++;

#define FLUSH_BATCH(henv, hdbc, stmt, fill, _test_) \
  if (fill > 0) \
    { \
      SQLParamOptions (stmt, fill, NULL); \
      IF_ERR_RETURN (stmt, SQLExecute (stmt), 0, _test_); \
      fill = 0; \
      SQLTransact (henv, hdbc, SQL_COMMIT); \
    }


#define BATCH_SIZE 1

#define OL_MAX 20
#define OL_PARS 3
#define NO_PARS 5

#define TEN_PACK_TIME	120

typedef struct olsstruct
{
  int ol_no[OL_MAX];
  long ol_i_id[OL_MAX];
  long ol_qty[OL_MAX];
  long ol_supply_w_id[OL_MAX];
  char ol_data[OL_MAX][24];
}
olines_t;

typedef struct tpcc_s
{
  test_common_t common;

  long count_ware;
  long local_w_id;
  long nRounds;

  int option_debug;		/* 1 if generating debug output    */
  int bIsVirtuoso;


  char timestamp_array[BATCH_SIZE][20];
  long sql_timelen_array[BATCH_SIZE];

  timer_account_t ten_pack_ta;
  timer_account_t new_order_ta;
  timer_account_t payment_ta;
  timer_account_t delivery_ta;
  timer_account_t slevel_ta;
  timer_account_t ostat_ta;

  HSTMT misc_stmt;
  HSTMT new_order_stmt, payment_stmt, delivery_stmt, slevel_stmt, ostat_stmt;
  HSTMT item_stmt, ware_stmt, stock_stmt, dist_stmt, cs_stmt, h_stmt,
      o_stmt, no_stmt, ol_stmt;

  long rnd_seed;

  double run_time;
  double tpcc_sum;
  int bIsOracle;
  int bIsSybase;

  char tableDSNS[9][50], tableDBMSes[9][50];
}
tpcc_t;
