/*
 *  tpca_code.h
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
#ifndef __TPCA_CODE_H__
#define __TPCA_CODE_H__

#define NUMITEMS(p1) (sizeof(p1)/sizeof(p1[0]))
#define SQL_TXN_DRIVER_DEFAULT	0
#define SQL_CURSOR_MIXED	100

int fBuildBench (test_t * lpRunCfg);
int fCleanup (test_t * lpRunCfg);
void vCreateTables (test_t * lpRunCfg);
void vCreateIndices (test_t * lpRunCfg);
void vCreateProcedure (test_t * lpRunCfg);
void vLoadBranch (test_t * lpRunCfg);
void vLoadTeller (test_t * lpRunCfg);
void vLoadAccount (test_t * lpRunCfg);
void vLoadStoreOptions (test_t * lpRunCfg, int fLoad);
void vDisplayLoad (test_t * lpBenchInfo);
int fExecute (test_t * lpRunCfg, char *lpSQLString);
void vCreateVirtuosoTPCCTables (test_t * lpBench);

void vCreateTPCCTables (char *szDBMS, HSTMT hstmt);
void vCreateTPCCIndices (char *szDBMS, HSTMT hstmt);

int getDriverTypeIndex (char *szDBMSName);

BOOL fSQLBindParameter (HSTMT hstmt, UWORD ipar, SWORD fParamType,
    SWORD fCType, SWORD fSqlType, UDWORD cbColDef, SWORD ibScale,
    PTR rgbValue, SDWORD cbValueMax, SDWORD FAR * pcbValue);

int getDriverMapSize (void);
char *getDriverDBMSName (int i);

  /* execution */
int fExecuteSql (test_t * lpBench, char *pszSql);
BOOL DoRun (test_t * lpBench, char *szTitle);
BOOL DoThreadsRun (test_t * lpBench);
BOOL fRunTrans (test_t * lpBench, char *szTitle);
void CalcStats (BOOL runStatus, int nOk, test_t * lpBench, long lTranCnt, 
    long lSubSecTranCnt, long lBetween, double dDiffSum);

char *txn_isolation_name (long txn_isolation, char *def);
long txn_isolation_from_name (char *iso);
char *cursor_type_name (long txn_isolation, char *def);
long cursor_type_from_name (char *def);
#endif
