/*
 *  tpca.h
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
#ifndef __TPC_A_H__
#define __TPC_A_H__

typedef struct tpca_s
{
  test_common_t common;

  int fAuto;			/* TRUE to run all combinations */

  int nArrayParSize;
  int fExecAsync;		/* TRUE to execute in async mode */
  int fDoQuery;			/* TRUE if 100 row query should be run */
  int fClearHistory;		/* TRUE if delete rows from history table first */
  int fUseCommit;		/* TRUE if transactions should be used */
  short fSQLOption;		/* value of radio button group one of: */
  /*              IDX_PLAINSQL */
  /*              IDX_PARAMS */
  /*              IDX_SPROCS */
  UDWORD udwMaxBranch;		/* Count of branches */
  UDWORD udwMaxTeller;		/* Count of tellers */
  UDWORD udwMaxAccount;		/* Count of accounts */
  SDWORD nTrnCnt;		/* Count of transaction to date */
  SDWORD nTrnCnt1Sec;		/* Count of trans < 1 */
  SDWORD nTrnCnt2Sec;		/* Count of trans 1 < n < 2 */
  double dDiffSum;		/* Sum of transaction execute time */

  /* Statistics done after the run */
  double dOverhead;		/* Environmental overhead */
  float ftps;			/* Transactions per second */
  float fsub1;			/* % trans done in < 1 second */
  float fsub2;			/* % trans done in < 2 seconds, > 1 */
  float fAvgTPTime;		/* Average transaction time */

  long txn_isolation;		/* txn_isolation_mode */
  long nCursorType;		/* cursor type */
  short nRowsetSize;		/* the rowset size for scrollable cursor types */
  short nKeysetSize;		/* the rowset size for scrollable cursor types */
  short nTraversalCount;	/* the traversal count for scrollable cursor types */

  int fCreateBranch;
  int fCreateTeller;
  int fCreateAccount;
  int fCreateHistory;
  int fLoadBranch;
  int fLoadTeller;
  int fLoadAccount;

  /* Operations Flags */
  int fCreateIndex;
  int fCreateProcedure;
  int fExecute;

  /* Common Values */
  int uwDrvIdx;
  char szTemp[128];

  char szBranchDSN[50];
  char szTellerDSN[50];
  char szAccountDSN[50];
  char szHistoryDSN[50];

  char szBranchDBMS[50];
  char szTellerDBMS[50];
  char szAccountDBMS[50];
  char szHistoryDBMS[50];
}
tpca_t;

#endif
