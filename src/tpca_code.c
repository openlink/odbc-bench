/*
 *  tpca_code.c
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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "odbcbench.h"
#include "results.h"
#include "tpca_code.h"
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif

typedef struct _DRIVERMAP
{
  char *szDbName;		/* DMBS Name */
  UWORD uwDTM;			/* DATATYPEMAP index */
  UWORD uwBTM;			/* BINDTYPEMAP index */
  SWORD swITM;			/* INDEXTYPEMAP index or -1 for no index */
  int fIndex;			/* Index is required by this driver */
  SWORD swProcIndex;		/* PROCEDURETEXT Index */
}
DRIVERMAP;

typedef struct _DATATYPEMAP
{
  UWORD uwIndex;
  char *lpChar;
  char *lpFloat;
  char *lpInt;
  char *lpTimestamp;
}
DATATYPEMAP;

typedef struct _BINDTYPEMAP
{
  SWORD swChar;
  SWORD swFloat;		/* SQL_TYPE for FLOAT */
  UWORD uwFloat;		/* cbColDef for FLOAT */
  SWORD swInt;			/* SQL_TYPE for int */
  UWORD uwInt;			/* cbColDef for int */
}
BINDTYPEMAP;

typedef struct _INDEXTYPEMAP
{
  char *szIndexType;
}
INDEXTYPEMAP;

/*-------------------------------------------------------------------------- */
/*| Globals */
/*-------------------------------------------------------------------------- */
/* Constants for Table Names */
/* */
static char szBranch[] = "BRANCH";
static char szTeller[] = "TELLER";
static char szAccount[] = "ACCOUNT";
static char szHistory[] = "HISTORY";

/* Constants for Field Names */
/* */
static char szFldBranch[] = "BRANCH";
static char szFldTeller[] = "TELLER";
static char szFldAcct[] = "ACCOUNT";
/*static char szFldHist[] = "HISTID"; */

/* SQL statements */
/* */
static char szDropTable[] = "Drop table %s";
static char szCreateBranch[] =
    "create table %s (BRANCH %s, FILLERINT %s, BALANCE %s, FILLER %s(84))";
static char szCreateTeller[] =
    "create table %s (TELLER %s, BRANCH %s, BALANCE %s, FILLER %s(84))";
static char szCreateAccount[] =
    "create table %s (ACCOUNT %s, BRANCH %s, BALANCE %s, FILLER %s(84))";
static char szCreateHistory[] =
    "create table %s (HISTID %s, ACCOUNT %s, TELLER %s, BRANCH %s, AMOUNT %s, TIMEOFTXN %s, FILLER %s(22))";

/* remote tables statements */
static char szRemoteDropTable[] = "rexecute ('%s', 'Drop table %s')";
static char szAttachTable[] = "attach table %s from '%s'";
static char szRemoteCreateIndex[] =
    "rexecute ('%s', 'create %s index %six on %s(%s)')";
static char szRemoteCreateBranch[] =
    "rexecute ('%s', 'create table %s (BRANCH %s, FILLERINT %s, BALANCE %s, FILLER %s(84))')";
static char szRemoteCreateTeller[] =
    "rexecute ('%s', 'create table %s (TELLER %s, BRANCH %s, BALANCE %s, FILLER %s(84))')";
static char szRemoteCreateAccount[] =
    "rexecute ('%s', 'create table %s (ACCOUNT %s, BRANCH %s, BALANCE %s, FILLER %s(84))')";
static char szRemoteCreateHistory[] =
    "rexecute ('%s', 'create table %s (HISTID %s, ACCOUNT %s, TELLER %s, BRANCH %s, AMOUNT %s, TIMEOFTXN %s, FILLER %s(22))')";

static char szInsertBranch[] =
    "insert into %s (BRANCH, FILLERINT, BALANCE, FILLER) values (?, ?, ?, ?)";
static char szInsertTeller[] =
    "insert into %s (TELLER, BRANCH, BALANCE, FILLER) values (?, ?, ?, ?)";
static char szInsertAccount[] =
    "insert into %s (ACCOUNT, BRANCH, BALANCE, FILLER) values (?, ?, ?, ?)";

static char szCreateIndex[] = "create %s index %six on %s(%s)";
static char szCreateIndex1[] = "create %s index %s on %s(%s)";
static char szCreateIndex2[] = "create %s index %s on %s(%s, %s)";
static char szCreateIndex3[] = "create %s index %s on %s(%s, %s, %s)";
static char szCreateIndex4[] = "create %s index %s on %s(%s, %s, %s, %s)";
static char szCreateIndex5[] = "create %s index %s on %s(%s, %s, %s, %s, %s)";

/* Display Messages */
/* */
static char szLoadStatus[] = "Benchmark Loader Status";
static char szPopulateTable[] = "Populate '%s' table with records";
static char szRowsInserted[] = "%lu of %lu Records Inserted...";
static char szBldDescrip[] = "\r\nBuilding Table Definition for '%s'.\r\n";
static char szFailedIndex[] = "\t\tCreating index on %s table failed\r\n";
static char szLoadRecords[] =
    "Attempting to load %lu records into %s table\r\n";
static char szInsertFailure[] =
    "Insert of row %lu failed. Insertion will be terminated.\r\n";
static char szCopied[] = "%lu rows actually copied.\r\n\r\n";

static char szCreateWarehouse[] =
    "create table WAREHOUSE ("
    "   W_ID       %s, "
    "   W_NAME     %s(10), "
    "   W_STREET_1 %s(20), "
    "   W_STREET_2 %s(20), "
    "   W_CITY     %s(20), "
    "   W_STATE    %s(2), "
    "   W_ZIP      %s(9), " "   W_TAX      %s, " "   W_YTD      %s) ";

static char szCreateDistrict[] =
    "create table DISTRICT ( "
    "    D_ID		%s, "
    "    D_W_ID		%s, "
    "    D_NAME		%s (10), "
    "    D_STREET_1		%s (20), "
    "    D_STREET_2		%s (20), "
    "    D_CITY		%s (20), "
    "    D_STATE		%s (2), "
    "    D_ZIP		%s (9), "
    "    D_TAX		%s, "
    "    D_YTD		%s, " "    D_NEXT_O_ID	%s)";

static char szCreateCustomer[] =
    "create table CUSTOMER ( "
    "    C_ID		%s, "
    "    C_D_ID		%s, "
    "    C_W_ID		%s, "
    "    C_FIRST		%s (16), "
    "    C_MIDDLE		%s (2), "
    "    C_LAST		%s (16), "
    "    C_STREET_1		%s (20), "
    "    C_STREET_2		%s (20), "
    "    C_CITY		%s (20), "
    "    C_STATE		%s (2), "
    "    C_ZIP		%s (9), "
    "    C_PHONE		%s (16), "
    "    C_SINCE		%s, "
    "    C_CREDIT		%s (2), "
    "    C_CREDIT_LIM	%s, "
    "    C_DISCOUNT		%s, "
    "    C_BALANCE		%s, "
    "    C_YTD_PAYMENT	%s, "
    "    C_CNT_PAYMENT	%s, "
    "    C_CNT_DELIVERY	%s, "
    "    C_DATA_1		%s (250), "
    "    C_DATA_2		%s (250))";


static char szCreateTHistory[] =
    " create table THISTORY ( "
    "     H_C_ID       %s, "
    "     H_C_D_ID     %s, "
    "     H_C_W_ID     %s, "
    "     H_D_ID       %s, "
    "     H_W_ID       %s, "
    "     H_DATE       %s, "
    "     H_AMOUNT     %s, " "     H_DATA       %s(24)) ";


static char szCreateNewOrder[] =
    "create table NEW_ORDER ( "
    "     NO_O_ID      %s, "
    "     NO_D_ID      %s, " "     NO_W_ID      %s) ";


static char szCreateOrders[] =
    "create table ORDERS ( "
    "     O_ID         %s, "
    "     O_D_ID       %s, "
    "     O_W_ID       %s, "
    "     O_C_ID       %s, "
    "     O_ENTRY_D    %s, "
    "     O_CARRIER_ID %s, "
    "     O_OL_CNT     %s, " "     O_ALL_LOCAL  %s )";


static char szCreateOrderLine[] =
    "create table ORDER_LINE ( "
    "     OL_O_ID         %s, "
    "     OL_D_ID         %s, "
    "     OL_W_ID         %s, "
    "     OL_NUMBER       %s, "
    "     OL_I_ID         %s, "
    "     OL_SUPPLY_W_ID  %s, "
    "     OL_DELIVERY_D   %s, "
    "     OL_QUANTITY     %s, "
    "     OL_AMOUNT       %s, " "     OL_DIST_INFO    %s(24))";


static char szCreateItem[] =
    "create table ITEM ( "
    "     I_ID            %s, "
    "     I_IM_ID         %s, "
    "     I_NAME          %s(24), "
    "     I_PRICE         %s, " "     I_DATA          %s(50))";


static char szCreateStock[] =
    "create table STOCK ( "
    "      S_I_ID          %s, "
    "      S_W_ID          %s, "
    "      S_QUANTITY      %s, "
    "      S_DIST_01       %s(24), "
    "      S_DIST_02       %s(24), "
    "      S_DIST_03       %s(24), "
    "      S_DIST_04       %s(24), "
    "      S_DIST_05       %s(24), "
    "      S_DIST_06       %s(24), "
    "      S_DIST_07       %s(24), "
    "      S_DIST_08       %s(24), "
    "      S_DIST_09       %s(24), "
    "      S_DIST_10       %s(24), "
    "      S_YTD           %s, "
    "      S_CNT_ORDER     %s, "
    "      S_CNT_REMOTE    %s, " "      S_DATA          %s(50) )";

/* Miscellaneous */
/* */

/* Constants for DATATYPE mapping structure */
/* */
static char szChar[] = "char";
static char szText[] = "text";
static char szAlphaNum[] = "alphanumeric";
static char szInt[] = "int";
static char szInteger[] = "integer";
static char szDateTime[] = "datetime";
static char szTimeStamp[] = "timestamp";
static char szDate[] = "date";
static char szNumeric[] = "numeric";
static char szNumber[] = "number";
static char szORALong[] = "number(9,0)";
static char szQENumeric[] = "numeric(19,6)";
static char szLong[] = "long";
static char szDouble[] = "double";
static char szString[] = "string";
static char szInt4[] = "Integer4";
static char szFloat8[] = "Float8";
static char szFloat[] = "float";
static char szIntNotNull[] = "integer not null";

DRIVERMAP DriverMap[] = {
  /* szDBMSName           DTM Index       BTM Index       ITM Index       fIndex  uwProcIndex */
  {"SQL Server", 5, 0, 0, FALSE, 1},
  {"Oracle7", 3, 1, 0, FALSE, 2},
  {"Oracle", 3, 1, 0, FALSE, 2},
  {"ACCESS", 4, 2, 0, FALSE, -1},
  {"DBASE", 2, 2, 0, FALSE, -1},
  {"FOXPRO", 2, 2, 0, FALSE, -1},
  {"EXCEL", 6, 2, -1, FALSE, -1},
  {"PARADOX", 0, 2, 0, TRUE, -1},
  {"TEXT", 1, 2, -1, FALSE, -1},
  {"BTRIEVE", 7, 2, 0, FALSE, -1},
  {"ANSI", 8, 0, 0, FALSE, -1},
  {"Sybase", 5, 0, 0, FALSE, 1},
  {"Intersolv dBASE", 9, 2, 0, FALSE, -1},
  {"Informix", 10, 0, 0, FALSE, 4},
  {"Ingres", 10, 0, 0, FALSE, 3},
  {"IBM DB2", 8, 0, 0, FALSE, -1},
  {"Progress 9", 8, 11, 0, FALSE, 5},
  {"Progress", 8, 0, 11, FALSE, -1},
  {"Virtuoso", 1, 0, 0, FALSE, 0}
};


static DATATYPEMAP DataTypeMap[] = {
/*Index lpChar          lpFloat         lpInt           lpDateTime */
  {0, szAlphaNum, szNumber, szNumber, szDate},
  {1, szChar, szFloat, szInteger, szDateTime},
  {2, szChar, szNumeric, szNumeric, szDate},
  {3, szChar, szNumber, szORALong, szDate},
  {4, szChar, szDouble, szLong, szDateTime},
  {5, szChar, szFloat, szInt, szDateTime},
  {6, szText, szNumber, szNumber, szDateTime},
  {7, szString, szFloat8, szInt4, szDate},
  {8, szChar, szFloat, szInteger, szTimeStamp},
  {9, szChar, szQENumeric, szQENumeric, szDate},
  {10, szChar, szFloat, szInteger, szDate},
  {11, szChar, szFloat, szIntNotNull, szDate}
};


static BINDTYPEMAP BindTypeMap[] = {
/* Char       Float,            cbFloat,    Int,            cbInt,   */
  {SQL_CHAR, SQL_FLOAT, 8, SQL_INTEGER, 4},
  {SQL_VARCHAR, SQL_NUMERIC, 15, SQL_NUMERIC, 10},
  {SQL_CHAR, SQL_NUMERIC, 15, SQL_NUMERIC, 10}
};


static INDEXTYPEMAP IndexTypeMap[] = {
/*Index Type */
  {"unique"},
  {"unique clustered"}
};

static char *procedures[] = {
  /* 0: Virtuoso */
  "create procedure ODBC_BENCHMARK(\n"
      "	IN vhistid   integer,\n"
      "	IN  acct     integer,\n"
      "	IN  vteller  integer,\n"
      "	IN  vbranch  integer,\n"
      "	IN  delta    float, \n"
      "	OUT balanc   float,\n"
      "	IN  vfiller  varchar(22))\n"
      "{\n"
      "	declare cr cursor for select BALANCE from ACCOUNT where ACCOUNT = acct;\n"
      "	\n"
      "	update ACCOUNT set BALANCE = BALANCE + delta where ACCOUNT = acct;\n"
      "\n"
      "	open cr;\n"
      "	fetch cr into balanc;\n"
      "	close cr;\n"
      "	\n"
      "	update TELLER set BALANCE = BALANCE + delta where TELLER = vteller;\n"
      "	update BRANCH set BALANCE = BALANCE + delta where BRANCH = vbranch;\n"
      "	insert INTO HISTORY\n"
      "		(HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, TIMEOFTXN, FILLER)\n"
      "	values\n"
      "		(vhistid, acct, vteller, vbranch, delta, now(), vfiller);\n"
      "}\n",

  /* 1: SQLServer */
  "CREATE PROCEDURE ODBC_BENCHMARK\n"
      "        @histid  int, \n"
      "        @acct    int, \n"
      "        @teller  int, \n"
      "        @branch  int, \n"
      "        @delta   float,\n"
      "        @balance float output,\n"
      "        @filler  char(22)\n"
      "AS\n"
      "BEGIN TRANSACTION\n"
      "UPDATE    ACCOUNT\n"
      "    SET   BALANCE = BALANCE + @delta\n"
      "    WHERE ACCOUNT = @acct\n"
      "SELECT    @balance = BALANCE\n"
      "    FROM  ACCOUNT \n"
      "    WHERE ACCOUNT = @acct\n"
      "UPDATE    TELLER\n"
      "    SET   BALANCE = BALANCE + @delta\n"
      "    WHERE TELLER  = @teller\n"
      "UPDATE    BRANCH\n"
      "    SET   BALANCE = BALANCE + @delta\n"
      "    WHERE BRANCH  = @branch\n"
      "INSERT HISTORY\n"
      "	(HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, TIMEOFTXN, FILLER)\n"
      "VALUES\n"
      "	(@histid, @acct, @teller, @branch, @delta, getdate(), @filler)\n"
      "COMMIT TRANSACTION\n",

  /* 2: Oracle */
  "create or replace procedure ODBC_BENCHMARK(\n"
      "	vhistid IN  number,\n"
      "	acct    IN  number,\n"
      "	vteller IN  number,\n"
      "	vbranch IN  number,\n"
      "	delta   IN  float, \n"
      "	balance OUT float,\n"
      "	vfiller IN  char)\n"
      "is\n"
      "BEGIN\n"
      "update ACCOUNT set BALANCE = BALANCE + delta where ACCOUNT = acct;\n"
      "update TELLER set BALANCE = BALANCE + delta where TELLER = vteller;\n"
      "update BRANCH set BALANCE = BALANCE + delta where BRANCH = vbranch;\n"
      "insert INTO HISTORY\n"
      "	(HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, TIMEOFTXN, FILLER)\n"
      "values\n"
      "	(vhistid, acct, vteller, vbranch, delta, SYSDATE, vfiller);\n"
      "COMMIT WORK;\n" "END  ODBC_BENCHMARK;\n",

  /* 3: Ingres II */
  "CREATE PROCEDURE ODBC_BENCHMARK (\n"
  "   histid  integer, \n"
  "   acct    integer, \n"
  "   telid   integer, \n"
  "   branch  integer, \n"
  "   delta   float,\n"
  "   bal     float,\n"
  "   filler  char(22))\n"
  " AS\n"
  " BEGIN\n"
  "   UPDATE ACCOUNT SET BALANCE = BALANCE + :delta WHERE ACCOUNT = :acct;\n"
  "   SELECT BALANCE INTO :bal from ACCOUNT WHERE ACCOUNT = :acct;\n"
  "   UPDATE TELLER  SET BALANCE = BALANCE + :delta WHERE TELLER  = :telid; \n"
  "   UPDATE BRANCH  SET BALANCE = BALANCE + :delta WHERE BRANCH  = :branch;\n"
  "   INSERT INTO HISTORY\n"
  "   (HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, TIMEOFTXN, FILLER)\n"
  "   VALUES\n"
  "   (:histid, :acct, :telid, :branch, :delta, date('now'), :filler);\n"
  " END\n",

  /* 4: Informix */
  "create procedure odbc_benchmark (\n"
  " vhistid  integer,\n"
  " acct     integer,\n"
  " vteller  integer,\n"
  " vbranch  integer,\n"
  " delta    float,\n"
  " balanc   float,\n"
  " vfiller  char(22))\n"
  "    update account set balance = balance + delta where account = acct;\n"
  "    update teller set balance = balance + delta where teller = vteller;\n"
  "    update branch set balance = balance + delta where branch = vbranch;\n"
  "    insert into history \n"
  "      (histid, account, teller, branch, amount, timeoftxn, filler)\n"
  "    values\n"
  "      (vhistid, acct, vteller, vbranch, delta, current, vfiller);\n"
  "end procedure;\n",

  /* 5: Progress 9.1 SQL92 */
  "create procedure ODBC_BENCHMARK (IN vhistid integer, IN acct  integer, "
  "  IN vteller integer, IN vbranch integer, IN delta  float, " 
  "  INOUT balance float, IN vfiller varchar(23))\n"
  "BEGIN\n"
  "  Double oldBalance = new Double (0);\n"
  "  SQLCursor Select_Balance = new SQLCursor "
  "    (\"select balance from account where account = ?\");\n"
  "      Select_Balance.setParam (1, acct);\n"
  "  Select_Balance.open();\n"
  "  Select_Balance.fetch();\n"
  "  if ((Select_Balance.wasNULL(1)) == true)\n"
  "    oldBalance = null;"
  "  else\n"
  "    oldBalance = (Double) Select_Balance.getValue (1, DOUBLE);\n"
  "  Select_Balance.close();\n"
  "  Double newBalance = new Double "
  "    (oldBalance.doubleValue () + delta.doubleValue ());\n"
  "  balance = newBalance;\n"
  "  SQLIStatement Update_Account = new SQLIStatement "
  "    (\"update ACCOUNT set BALANCE = ? where ACCOUNT = ?\");\n"
  "  Update_Account.setParam (1, newBalance);\n"
  "  Update_Account.setParam (2, acct);\n"
  "  SQLIStatement Update_Teller = new SQLIStatement "
  "    (\"update TELLER set BALANCE = ? where TELLER = ?\");\n"
  "  Update_Teller.setParam (1, newBalance);\n"
  "  Update_Teller.setParam (2, vteller);\n"
  "  SQLIStatement Update_Branch = new SQLIStatement "
  "    (\"update BRANCH set BALANCE = ? where BRANCH = ?\");\n"
  "  Update_Branch.setParam (1, newBalance);\n"
  "  Update_Branch.setParam (2, vbranch);\n"
  "  SQLIStatement Insert_History = new SQLIStatement "
  "    (\"insert INTO HISTORY (HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, "
          "TIMEOFTXN, FILLER) values (?, ?, ?, ?, ?, NOW(), ?)\");\n"
  "  Insert_History.setParam (1, vhistid);\n"
  "  Insert_History.setParam (2, acct);\n"
  "  Insert_History.setParam (3, vteller);\n"
  "  Insert_History.setParam (4, vbranch);\n"
  "  Insert_History.setParam (5, delta);\n"
  "  Insert_History.setParam (6, vfiller);\n"
  "  Update_Account.execute();\n"
  "  Update_Teller.execute();\n"
  "  Update_Branch.execute();\n"
  "  Insert_History.execute();\n"
  "END"
};

int
getDriverMapSize (void)
{
  return NUMITEMS (DriverMap);
}


char *
getDriverDBMSName (int i)
{
  return DriverMap[i].szDbName;
}


/*-------------------------------------------------------------------------*/
/* fBuildBench - Control Procedure for Build the Benchmark tables. */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
int
fBuildBench (test_t * ptest	/* Run Configuration Parameters */
    )
{
  time_t start_time, end_time;

  time (&start_time);

  /* Initialize character fields with filler pattern */
  memset (ptest->szTemp, 'f', sizeof (ptest->szTemp) - 1);
  ptest->szTemp[sizeof (ptest->szTemp) - 1] = '\0';

  if (-1 == ptest->tpc.a.uwDrvIdx)
    {
      ptest->tpc.a.uwDrvIdx = getDriverTypeIndex (ptest->szDBMS);
      ptest->is_dirty = 1;
    }

  /* Create table Definitions */
  if (gui.vBusy)
    gui.vBusy ();			/* Change to Hourglass Cursor */
  vCreateTables (ptest);

  if (-1 != DriverMap[ptest->tpc.a.uwDrvIdx].swITM
      && ptest->tpc.a.fCreateIndex)
    vCreateIndices (ptest);
  if (-1 != DriverMap[ptest->tpc.a.uwDrvIdx].swProcIndex
      && ptest->tpc.a.fCreateProcedure)
    vCreateProcedure (ptest);
  if (gui.vBusy)
    gui.vBusy ();			/* Change to Normal Cursor */

  /* Insert Records into Tables */
  if (ptest->tpc.a.fLoadBranch)
    {
      if (ptest->ShowProgress)
	ptest->ShowProgress (NULL,
	    (char *) szLoadStatus, TRUE, ptest->tpc.a.udwMaxBranch);
      vLoadBranch (ptest);
      if (ptest->StopProgress)
	ptest->StopProgress ();
    }

  if (ptest->tpc.a.fLoadTeller)
    {
      if (ptest->ShowProgress)
	ptest->ShowProgress (NULL,
	    (char *) szLoadStatus, TRUE, ptest->tpc.a.udwMaxTeller);
      vLoadTeller (ptest);
      if (ptest->StopProgress)
	ptest->StopProgress ();
    }

  if (ptest->tpc.a.fLoadAccount)
    {
      if (ptest->ShowProgress)
	ptest->ShowProgress (NULL,
	    (char *) szLoadStatus, TRUE, ptest->tpc.a.udwMaxAccount);
      vLoadAccount (ptest);
      if (ptest->StopProgress)
	ptest->StopProgress ();
    }

  time (&end_time);
  sprintf (ptest->szTemp, "Load tables/%ld/%ld/%ld",
      ptest->tpc.a.udwMaxBranch,
      ptest->tpc.a.udwMaxTeller, ptest->tpc.a.udwMaxAccount);

  do_add_results_record ("TPC-A", ptest->szTemp,
      henv, ptest->hdbc, ptest->hstmt,
      ptest->szDSN, -1, end_time - start_time, -1,
      -1, -1, -1, ptest->szDriverName, ptest->szDriverVer, ptest->fHaveResults);
  return TRUE;
}


/*-------------------------------------------------------------------------*/
/* fExecute - Executes the SQLStatement and displays error when not */
/*              successful. */
/* */
/* Returns:  TRUE if SQL_SUCCESS or SQL_SUCCESS_WITH_INFO */
/*-------------------------------------------------------------------------*/
int
fExecute (test_t * ptest,	/* Run Configuration Parameters */
    char *lpSQLString		/* Pointer to String to Execute */
    )
{
  RETCODE rc;

  rc = SQLExecDirect (ptest->hstmt, lpSQLString, SQL_NTS);

  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
    {
      vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, ptest->hstmt, ptest);
      return FALSE;
    }

  return TRUE;
}


/*-------------------------------------------------------------------------*/
/* vCreateTables - Creates the tables required for the Benchmarks */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/

void
vCreateTables (test_t * ptest	/* Run Configuration Parameters  */
    )
{
  RETCODE rc;
  UWORD uwTypeIdx;
  char szSQLBuffer[200];
  int uwIdx, fReqIndex, inx, TypeIdx;
  char szQualifier[100];

  strcpy (szQualifier, "\"");
  uwTypeIdx = DriverMap[ptest->tpc.a.uwDrvIdx].uwDTM;

  /* Branch table. */
  /* */
  if (ptest->tpc.a.fCreateBranch)
    {
      pane_log ((char *) szBldDescrip, szBranch);

      /* Remove old table definition */
      sprintf (szSQLBuffer, szDropTable, szBranch);
      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

      if (ptest->tpc.a.szBranchDSN[0])
	{			/* attached table case */
	  inx = getDriverTypeIndex (ptest->tpc.a.szBranchDBMS);
	  TypeIdx = DriverMap[inx].uwDTM;
	  uwIdx = DriverMap[inx].swITM;
	  fReqIndex = DriverMap[inx].fIndex;

	  /* rexecute (.. 'drop table ...') */
	  sprintf (szSQLBuffer, szRemoteDropTable, ptest->tpc.a.szBranchDSN,
	      szBranch);
	  rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

	  /* rexecute (.. 'create table ...') */
	  sprintf (szSQLBuffer, szRemoteCreateBranch,
	      ptest->tpc.a.szBranchDSN,
	      szBranch,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpChar);
	  fExecute (ptest, szSQLBuffer);

	  /* rexecute (.. create index */
	  if (ptest->tpc.a.fCreateIndex || fReqIndex)
	    {
	      sprintf (szSQLBuffer, szRemoteCreateIndex,
		  ptest->tpc.a.szBranchDSN,
		  (char *) IndexTypeMap[uwIdx].szIndexType,
		  szFldBranch, szBranch, szFldBranch);
	      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);
	      if (RC_NOTSUCCESSFUL (rc))
		pane_log ((char *) szFailedIndex, szBranch);
	    }

	  /* rexecute (.. attach table */
	  sprintf (szSQLBuffer, szAttachTable, szBranch,
	      ptest->tpc.a.szBranchDSN);
	  fExecute (ptest, szSQLBuffer);
	}
      else
	{
	  /* Create new table definition */
	  sprintf (szSQLBuffer, szCreateBranch,
	      szBranch,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);
	  fExecute (ptest, szSQLBuffer);
	}
    }

  /* Teller table. */
  /* */
  if (ptest->tpc.a.fCreateTeller)
    {
      pane_log ((char *) szBldDescrip, szTeller);

      /* Remove old table definition */
      sprintf (szSQLBuffer, szDropTable, szTeller);
      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

      if (ptest->tpc.a.szTellerDSN[0])
	{			/* attached table case */
	  inx = getDriverTypeIndex (ptest->tpc.a.szTellerDBMS);
	  uwIdx = DriverMap[inx].swITM;
	  fReqIndex = DriverMap[inx].fIndex;
	  TypeIdx = DriverMap[inx].uwDTM;

	  /* rexecute (.. 'drop table ...') */
	  sprintf (szSQLBuffer, szRemoteDropTable, ptest->tpc.a.szTellerDSN,
	      szTeller);
	  rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

	  /* rexecute (.. 'create table ...') */
	  sprintf (szSQLBuffer, szRemoteCreateTeller,
	      ptest->tpc.a.szTellerDSN,
	      szTeller,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpChar);
	  fExecute (ptest, szSQLBuffer);

	  /* rexecute (.. create index */
	  if (ptest->tpc.a.fCreateIndex || fReqIndex)
	    {
	      sprintf (szSQLBuffer, szRemoteCreateIndex,
		  ptest->tpc.a.szTellerDSN,
		  (char *) IndexTypeMap[uwIdx].szIndexType,
		  szFldTeller, szTeller, szFldTeller);
	      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);
	      if (RC_NOTSUCCESSFUL (rc))
		pane_log ((char *) szFailedIndex, szTeller);
	    }

	  /* attach table */
	  sprintf (szSQLBuffer, szAttachTable, szTeller,
	      ptest->tpc.a.szTellerDSN);
	  fExecute (ptest, szSQLBuffer);
	}
      else
	{
	  /* Create new table definitions */

	  sprintf (szSQLBuffer, szCreateTeller,
	      szTeller,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);
	  fExecute (ptest, szSQLBuffer);
	}
    }

  /* Account table. */
  /* */
  if (ptest->tpc.a.fCreateAccount)
    {
      pane_log ((char *) szBldDescrip, szAccount);

      /* Remove old table definition */
      sprintf (szSQLBuffer, szDropTable, szAccount);
      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

      if (ptest->tpc.a.szAccountDSN[0])
	{			/* attached table case */
	  inx = getDriverTypeIndex (ptest->tpc.a.szAccountDBMS);
	  TypeIdx = DriverMap[inx].uwDTM;
	  uwIdx = DriverMap[inx].swITM;
	  fReqIndex = DriverMap[inx].fIndex;

	  /* rexecute (.. 'drop table ...') */
	  sprintf (szSQLBuffer, szRemoteDropTable, ptest->tpc.a.szAccountDSN,
	      szAccount);
	  rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

	  /* rexecute (.. 'create table ...') */
	  sprintf (szSQLBuffer, szRemoteCreateAccount,
	      ptest->tpc.a.szAccountDSN,
	      szAccount,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpChar);
	  fExecute (ptest, szSQLBuffer);

	  /* rexecute (.. create index */
	  if (ptest->tpc.a.fCreateIndex || fReqIndex)
	    {
	      sprintf (szSQLBuffer, szRemoteCreateIndex,
		  ptest->tpc.a.szAccountDSN,
		  (char *) IndexTypeMap[uwIdx].szIndexType,
		  szFldAcct, szAccount, szFldAcct);
	      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);
	      if (RC_NOTSUCCESSFUL (rc))
		pane_log ((char *) szFailedIndex, szAccount);
	    }

	  /* attach table */
	  sprintf (szSQLBuffer, szAttachTable, szAccount,
	      ptest->tpc.a.szAccountDSN);
	  fExecute (ptest, szSQLBuffer);
	}
      else
	{
	  /* Create new table definition */
	  sprintf (szSQLBuffer, szCreateAccount,
	      szAccount,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);

	  fExecute (ptest, szSQLBuffer);
	}
    }

  /* History table. */
  /* */
  if (ptest->tpc.a.fCreateHistory)
    {
      pane_log ((char *) szBldDescrip, szHistory);

      /* Remove old table definition */
      sprintf (szSQLBuffer, szDropTable, szHistory);
      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

      if (ptest->tpc.a.szHistoryDSN[0])
	{			/* attached table case */
	  inx = getDriverTypeIndex (ptest->tpc.a.szHistoryDBMS);
	  TypeIdx = DriverMap[inx].uwDTM;
	  uwIdx = DriverMap[inx].swITM;
	  fReqIndex = DriverMap[inx].fIndex;

	  /* rexecute (.. 'drop table ...') */
	  sprintf (szSQLBuffer, szRemoteDropTable, ptest->tpc.a.szHistoryDSN,
	      szHistory);
	  rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);

	  /* rexecute (.. 'create table ...') */
	  sprintf (szSQLBuffer, szRemoteCreateHistory,
	      ptest->tpc.a.szHistoryDSN,
	      szHistory,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpInt,
	      DataTypeMap[TypeIdx].lpFloat,
	      DataTypeMap[TypeIdx].lpTimestamp, DataTypeMap[TypeIdx].lpChar);
	  fExecute (ptest, szSQLBuffer);
/*
	   rexecute (.. create index 
	  if (ptest->fCreateIndex || fReqIndex)
	    {
	      sprintf (szSQLBuffer, szRemoteCreateIndex,
		  ptest->szHistoryDSN,
		  (char *) IndexTypeMap[uwIdx].szIndexType,
		  szFldHist,
		  szHistory,
		  szFldHist);
	      rc = SQLExecDirect (ptest->lpBenchInfo->hstmt, szSQLBuffer, SQL_NTS);
	      if (RC_NOTSUCCESSFUL (rc))
		pane_log ((char *) szFailedIndex, szBranch);
	    }
*/
	  /* attach table */
	  sprintf (szSQLBuffer, szAttachTable, szHistory,
	      ptest->tpc.a.szHistoryDSN);
	  fExecute (ptest, szSQLBuffer);
	}
      else
	{
	  /* Create new table definition */
	  sprintf (szSQLBuffer, szCreateHistory,
	      szHistory,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpInt,
	      DataTypeMap[uwTypeIdx].lpFloat,
	      DataTypeMap[uwTypeIdx].lpTimestamp,
	      DataTypeMap[uwTypeIdx].lpChar);
	  fExecute (ptest, szSQLBuffer);
	}
    }

  pane_log ("\r\n\r\n");

  return;
}


/*-------------------------------------------------------------------------*/
/* vCreateIndices - Creates the indices for the TP1 database tables. */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
void
vCreateIndices (test_t * ptest	/* Run Configuration Parameters */
    )
{
  RETCODE rc;
  BOOL fReqIndex;
  UWORD uwIdx;
  char szSQLBuffer[128];

  uwIdx = DriverMap[ptest->tpc.a.uwDrvIdx].swITM;
  fReqIndex = DriverMap[ptest->tpc.a.uwDrvIdx].fIndex;

  /* Build Index on the Branch Table */
  if (!ptest->tpc.a.szBranchDSN[0] && ptest->tpc.a.fCreateBranch &&
      (ptest->tpc.a.fCreateIndex || fReqIndex))
    {
      sprintf (szSQLBuffer, szCreateIndex,
	  (char *) IndexTypeMap[uwIdx].szIndexType,
	  szFldBranch, szBranch, szFldBranch);


      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);
      if (RC_NOTSUCCESSFUL (rc))
	pane_log ((char *) szFailedIndex, szBranch);

    }

  /* Build Index on Teller Table */
  if (!ptest->tpc.a.szTellerDSN[0] && ptest->tpc.a.fCreateTeller &&
      (ptest->tpc.a.fCreateIndex || fReqIndex))
    {
      sprintf (szSQLBuffer, szCreateIndex,
	  (char *) IndexTypeMap[uwIdx].szIndexType,
	  szFldTeller, szTeller, szFldTeller);

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);
      if (RC_NOTSUCCESSFUL (rc))
	pane_log ((char *) szFailedIndex, szTeller);
    }

  /* Build Index on Account Table */
  if (!ptest->tpc.a.szAccountDSN[0] && ptest->tpc.a.fCreateAccount &&
      (ptest->tpc.a.fCreateIndex || fReqIndex))
    {
      sprintf (szSQLBuffer, szCreateIndex,
	  (char *) IndexTypeMap[uwIdx].szIndexType,
	  szFldAcct, szAccount, szFldAcct);

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      rc = SQLExecDirect (ptest->hstmt, szSQLBuffer, SQL_NTS);
      if (RC_NOTSUCCESSFUL (rc))
	pane_log ((char *) szFailedIndex, szAccount);
    }


  /* Build Index on History Table (Paradox Needs this to Insert) */
/*  if (!ptest->szHistoryDSN[0] && ptest->fCreateHistory &&
      (ptest->fCreateIndex || fReqIndex) && uwIdx == 7)
    {
      sprintf (szSQLBuffer, szCreateIndex,
	  (char *) IndexTypeMap[uwIdx].szIndexType,
	  szFldHist,
	  szHistory,
	  szFldHist);

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
       Execute Index Create Statement 
      rc = SQLExecDirect (ptest->lpBenchInfo->hstmt, szSQLBuffer, SQL_NTS);
      if (RC_NOTSUCCESSFUL (rc))
	pane_log ((char *) szFailedIndex, szHistory);
    }
*/
  pane_log ("\r\n\r\n");

  return;
}

/*-------------------------------------------------------------------------*/
/* vCreateProcedures - Creates the procedures for the TP1 database tables. */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
void
vCreateProcedure (test_t * ptest	/* Run Configuration Parameters */
    )
{
  RETCODE rc;
  UWORD uwIdx;

  uwIdx = DriverMap[ptest->tpc.a.uwDrvIdx].swProcIndex;

  /* Build Index on the Branch Table */
  if (ptest->tpc.a.fCreateProcedure)
    {
      pane_log ("Adding a procedure for %s\r\n",
	  DriverMap[ptest->tpc.a.uwDrvIdx].szDbName);
      /* Execute Index Create Statement */
      rc = SQLExecDirect (ptest->hstmt, procedures[uwIdx], SQL_NTS);
      IF_ERR (ptest->hstmt, rc, ptest);
    }

  pane_log ("\r\n\r\n");
  return;
}

void
vCreateTPCCTables (char *szDBMS, HSTMT hstmt)
{
  RETCODE rc;
  UWORD uwTypeIdx;
  char szSQLBuffer[2048];
  int nDriverMapIndex = getDriverTypeIndex (szDBMS);

  uwTypeIdx = DriverMap[nDriverMapIndex].uwDTM;

  /* Warehouse table. */
  /* */
  pane_log ("Creating the warehouse table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "warehouse");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definition */
  sprintf (szSQLBuffer, szCreateWarehouse,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpFloat);
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* district table. */
  /* */
  pane_log ("Creating the district table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "district");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateDistrict,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpFloat,
      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpInt);
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* customer table. */
  /* */
  pane_log ("Creating the Customer table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "customer");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateCustomer,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpTimestamp,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpFloat,
      DataTypeMap[uwTypeIdx].lpFloat,
      DataTypeMap[uwTypeIdx].lpFloat,
      DataTypeMap[uwTypeIdx].lpFloat,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpChar, DataTypeMap[uwTypeIdx].lpChar);
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* History table. */
  /* */
  pane_log ("Creating the History table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "THISTORY");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateTHistory,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpTimestamp,
      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);

  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* New_order table. */
  /* */
  pane_log ("Creating the new_order table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "NEW_ORDER");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateNewOrder,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt, DataTypeMap[uwTypeIdx].lpInt);

  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* Orders table. */
  /* */
  pane_log ("Creating the orders table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "ORDERS");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateOrders,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpTimestamp,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt, DataTypeMap[uwTypeIdx].lpInt);

  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* Order_line table. */
  /* */
  pane_log ("Creating the order_line table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "ORDER_LINE");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateOrderLine,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpTimestamp,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);

  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* item table. */
  /* */
  pane_log ("Creating the item table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "ITEM");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateItem,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);

  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  /* stock table. */
  /* */
  pane_log ("Creating the stock table\n");

  /* Remove old table definition */
  sprintf (szSQLBuffer, szDropTable, "STOCK");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  /* Create new table definitions */

  sprintf (szSQLBuffer, szCreateStock,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpChar,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt,
      DataTypeMap[uwTypeIdx].lpInt, DataTypeMap[uwTypeIdx].lpChar);

  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  pane_log ("\r\n\r\n");

  return;
}


void
vCreateTPCCIndices (char *szDBMS, HSTMT hstmt)
{
  char szSQLBuffer[2048];

  int indexType = DriverMap[getDriverTypeIndex (szDBMS)].swITM;
  if (indexType == -1)
    return;

  sprintf (szSQLBuffer, szCreateIndex1,
      (char *) IndexTypeMap[indexType].szIndexType,
      "warehouse_prime", "warehouse", "w_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex2,
      (char *) IndexTypeMap[indexType].szIndexType,
      "district_prime", "district", "d_w_id", "d_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex3,
      (char *) IndexTypeMap[indexType].szIndexType,
      "customer_prime", "customer", "c_w_id", "c_d_id", "c_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex5,
      "unique",
      "customer_ncl",
      "customer", "c_w_id", "c_d_id", "c_last", "c_first", "c_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex3,
      (char *) IndexTypeMap[indexType].szIndexType,
      "new_order_prime", "new_order", "no_w_id", "no_d_id", "no_o_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex3,
      (char *) IndexTypeMap[indexType].szIndexType,
      "orders_prime", "orders", "o_w_id", "o_d_id", "o_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex4,
      (char *) IndexTypeMap[indexType].szIndexType,
      "order_line_prime",
      "order_line", "ol_w_id", "ol_d_id", "ol_o_id", "ol_number");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex1,
      (char *) IndexTypeMap[indexType].szIndexType,
      "item_prime", "item", "i_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  sprintf (szSQLBuffer, szCreateIndex2,
      (char *) IndexTypeMap[indexType].szIndexType,
      "stock_prime", "stock", "s_i_id", "s_w_id");

  pane_log (szSQLBuffer);
  pane_log ("\r\n");
  /* Execute Index Create Statement */
  if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);
}


/*-------------------------------------------------------------------------*/
/* vLoadBranch - Loads records into the BRANch table. */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
void
vLoadBranch (test_t * ptest	/* Run Configuration Parameters */
    )
{
  RETCODE rc;
  UDWORD udwBranch;
  double dBalance = 1000.0;
  SDWORD cbBalance;
  UDWORD udwMod;
  char szSQLBuffer[128];
  SDWORD cbFiller = 84;
  UWORD uwBindIdx;
  SDWORD cbBranch; 

  uwBindIdx = DriverMap[ptest->tpc.a.uwDrvIdx].uwBTM;

  pane_log ((char *) szLoadRecords, ptest->tpc.a.udwMaxBranch, szBranch);
  sprintf (szSQLBuffer, szPopulateTable, szBranch);
  if (ptest->SetWorkingItem)
    ptest->SetWorkingItem (szSQLBuffer);


  /* Prepare the insert statement */
  sprintf (szSQLBuffer, szInsertBranch, szBranch);
  rc = SQLPrepare (ptest->hstmt, szSQLBuffer, SQL_NTS);

  /* Bind each variable to its parameter marker */
  /* Branch ID */
  rc = fSQLBindParameter (ptest->hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
      BindTypeMap[uwBindIdx].swInt,
      BindTypeMap[uwBindIdx].uwInt, 0, (PTR) & udwBranch, 0, &cbBranch);

  /* Filler ID */
  rc = fSQLBindParameter (ptest->hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
      BindTypeMap[uwBindIdx].swInt,
      BindTypeMap[uwBindIdx].uwInt, 0, (PTR) & udwBranch, 0, &cbBranch);

  /* Balance */
  rc = fSQLBindParameter (ptest->hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE,
      BindTypeMap[uwBindIdx].swFloat,
      BindTypeMap[uwBindIdx].uwFloat, 0, (PTR) & dBalance, 0, &cbBalance);

  /* Filler char */
  rc = fSQLBindParameter (ptest->hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR,
      BindTypeMap[uwBindIdx].swChar, 84, 0, ptest->szTemp, 84, &cbFiller);

  /* Set the Progress Dialog */
  sprintf (szSQLBuffer, szRowsInserted, (UDWORD) 0,
      (UDWORD) ptest->tpc.a.udwMaxBranch);
  if (ptest->SetProgressText)
    ptest->SetProgressText (szSQLBuffer, 0, 0, 0, 1, 0, 0);
  udwMod = (UDWORD) (ptest->tpc.a.udwMaxBranch / 50);
  udwMod = udwMod ? udwMod : 1;

  /* Insert records */
  for (udwBranch = 1; udwBranch <= ptest->tpc.a.udwMaxBranch; udwBranch++)
    {

      rc = SQLExecute (ptest->hstmt);
      if (RC_NOTSUCCESSFUL (rc))
	{
	  vShowErrors (0, SQL_NULL_HENV, SQL_NULL_HDBC, ptest->hstmt, ptest);
	  pane_log ((char *) szInsertFailure, udwBranch);
	  break;
	}

      /* Update the Progress Dialog */
      if (0 == (udwBranch % udwMod))
	{
	  sprintf (szSQLBuffer, szRowsInserted, udwBranch,
	      ptest->tpc.a.udwMaxBranch);
	  if (ptest->SetProgressText)
	    ptest->SetProgressText (szSQLBuffer, 0, 0, udwBranch, 1, 0, 0);
	  if (ptest->fCancel && ptest->fCancel ())
	    goto DONE;
	}
    }
DONE:
  pane_log ((char *) szCopied, --udwBranch);

  /* Clean up Statement Handle */
  rc = SQLFreeStmt (ptest->hstmt, SQL_CLOSE);
  rc = SQLFreeStmt (ptest->hstmt, SQL_RESET_PARAMS);

  return;
}


/*-------------------------------------------------------------------------*/
/* vLoadTeller - Loads records into the Teller table. */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
void
vLoadTeller (test_t * ptest	/* Run Configuration Parameters */
    )
{
  RETCODE rc;
  double dBalance = 100000;
  UDWORD udwMod;
  UDWORD udwTeller, udwBranch;
  char szSQLBuffer[128];
  SDWORD cbFiller = 84;
  UWORD uwBindIdx;
  SDWORD cbTeller, cbBranch, cbBalance;

  uwBindIdx = DriverMap[ptest->tpc.a.uwDrvIdx].uwBTM;


  pane_log ((char *) szLoadRecords, ptest->tpc.a.udwMaxTeller, szTeller);
  sprintf (szSQLBuffer, szPopulateTable, szTeller);
  if (ptest->SetWorkingItem)
    ptest->SetWorkingItem (szSQLBuffer);


  /* Prepare the insert statement */
  sprintf (szSQLBuffer, szInsertTeller, szTeller);
  rc = SQLPrepare (ptest->hstmt, szSQLBuffer, SQL_NTS);

  /* Bind each variable to its parameter marker */
  /* Teller ID */
  rc = fSQLBindParameter (ptest->hstmt, 1, SQL_PARAM_INPUT,
      SQL_C_LONG, BindTypeMap[uwBindIdx].swInt,
      BindTypeMap[uwBindIdx].uwInt, 0, (PTR) & udwTeller, 0, &cbTeller);

  /* Branch ID */
  rc = fSQLBindParameter (ptest->hstmt, 2, SQL_PARAM_INPUT,
      SQL_C_LONG, BindTypeMap[uwBindIdx].swInt,
      BindTypeMap[uwBindIdx].uwInt, 0, (PTR) & udwBranch, 0, &cbBranch);

  /* Balance */
  rc = fSQLBindParameter (ptest->hstmt, 3, SQL_PARAM_INPUT,
      SQL_C_DOUBLE, BindTypeMap[uwBindIdx].swFloat,
      BindTypeMap[uwBindIdx].uwFloat, 0, (PTR) & dBalance, 0, &cbBalance);

  /* Filler char */
  rc = fSQLBindParameter (ptest->hstmt, 4, SQL_PARAM_INPUT,
      SQL_C_CHAR, BindTypeMap[uwBindIdx].swChar,
      84, 0, ptest->szTemp, 84, &cbFiller);

  /* Set the Progress Dialog */
  sprintf (szSQLBuffer, szRowsInserted, (UDWORD) 0,
      (UDWORD) ptest->tpc.a.udwMaxTeller);
  if (ptest->SetProgressText)
    ptest->SetProgressText (szSQLBuffer, 0, 0, 0, 1, 0, 0);
  udwMod = (UDWORD) (ptest->tpc.a.udwMaxTeller / 50);
  udwMod = udwMod ? udwMod : 1;

  srand ((unsigned) time (NULL));

  /* Insert records */
  for (udwTeller = 1; udwTeller <= ptest->tpc.a.udwMaxTeller; udwTeller++)
    {
      udwBranch = (UDWORD) ((rand () % ptest->tpc.a.udwMaxBranch) + 1);

      rc = SQLExecute (ptest->hstmt);
      if (RC_NOTSUCCESSFUL (rc))
	{
	  pane_log ((char *) szInsertFailure, udwTeller);
	  break;
	}

      /* Update the Progress Dialog */
      if (0 == (udwTeller % udwMod))
	{
	  sprintf (szSQLBuffer, szRowsInserted, udwTeller,
	      ptest->tpc.a.udwMaxTeller);
	  if (ptest->SetProgressText)
	    ptest->SetProgressText (szSQLBuffer, 0, 0, ((float) udwTeller), 1, 0, 0);
	  if (ptest->fCancel && ptest->fCancel ())
	    goto DONE;
	}
    }
DONE:
  pane_log ((char *) szCopied, --udwTeller);

  /* Clean up Statement Handle */
  rc = SQLFreeStmt (ptest->hstmt, SQL_CLOSE);
  rc = SQLFreeStmt (ptest->hstmt, SQL_RESET_PARAMS);

  return;
}


/*-------------------------------------------------------------------------*/
/* vLoadAccount - Loads records into the Account table. */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
void
vLoadAccount (test_t * ptest	/* Run Configuration Parameters */
    )
{
  RETCODE rc;
  char szSQLBuffer[128];
  UDWORD udwAcct;
  UDWORD udwBranch;
  double dBalance = 1000.;
  UDWORD udwMod;
  SDWORD cbFiller = 84;
  UWORD uwBindIdx;
  SDWORD cbAcct, cbBranch, cbBalance;

  uwBindIdx = DriverMap[ptest->tpc.a.uwDrvIdx].uwBTM;


  pane_log ((char *) szLoadRecords, ptest->tpc.a.udwMaxAccount, szAccount);
  sprintf (szSQLBuffer, szPopulateTable, szAccount);
  if (ptest->SetWorkingItem)
    ptest->SetWorkingItem (szSQLBuffer);


  /* Prepare the insert statement */
  sprintf (szSQLBuffer, szInsertAccount, szAccount);
  rc = SQLPrepare (ptest->hstmt, szSQLBuffer, SQL_NTS);

  /* Bind variables  */
  /* Account ID */
  rc = fSQLBindParameter (ptest->hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
      BindTypeMap[uwBindIdx].swInt,
      BindTypeMap[uwBindIdx].uwInt, 0, (PTR) & udwAcct, 0, &cbAcct);

  /* Branch ID */
  rc = fSQLBindParameter (ptest->hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
      BindTypeMap[uwBindIdx].swInt,
      BindTypeMap[uwBindIdx].uwInt, 0, (PTR) & udwBranch, 0, &cbBranch);

  /* Balance */
  rc = fSQLBindParameter (ptest->hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE,
      BindTypeMap[uwBindIdx].swFloat,
      BindTypeMap[uwBindIdx].uwFloat, 0, (PTR) & dBalance, 0, &cbBalance);

  /* Filler char */
  rc = fSQLBindParameter (ptest->hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR,
      BindTypeMap[uwBindIdx].swChar, 84, 0, ptest->szTemp, 84, &cbFiller);


  /* Set the Progress Dialog */
  sprintf (szSQLBuffer, szRowsInserted, (UDWORD) 0,
      (UDWORD) ptest->tpc.a.udwMaxAccount);
  if (ptest->SetProgressText)
    ptest->SetProgressText (szSQLBuffer, 0, 0, 0, 1, 0, 0);
  udwMod = (UDWORD) (ptest->tpc.a.udwMaxAccount / 50);
  udwMod = udwMod ? udwMod : 1;


  srand ((unsigned) time (NULL));

  /* Insert Records */
  for (udwAcct = 1; udwAcct <= ptest->tpc.a.udwMaxAccount; udwAcct++)
    {
      udwBranch = (UWORD) ((rand () % ptest->tpc.a.udwMaxBranch) + 1);

      rc = SQLExecute (ptest->hstmt);
      if (RC_NOTSUCCESSFUL (rc))
	{
	  pane_log ((char *) szInsertFailure, udwAcct);
	  break;
	}

      /* Update the Progress Dialog */
      if (0 == (udwAcct % udwMod))
	{
	  sprintf (szSQLBuffer, szRowsInserted, udwAcct,
	      ptest->tpc.a.udwMaxAccount);
	  if (ptest->SetProgressText)
	    ptest->SetProgressText (szSQLBuffer, 0, 0, ((float) udwAcct), 1, 0, 0);
	  if (ptest->fCancel && ptest->fCancel ())
	    goto DONE;
	}

    }

DONE:
  pane_log ((char *) szCopied, --udwAcct);

  /* Clean up Statement Handle */
  rc = SQLFreeStmt (ptest->hstmt, SQL_CLOSE);
  rc = SQLFreeStmt (ptest->hstmt, SQL_RESET_PARAMS);

  return;
}


/*-------------------------------------------------------------------------*/
/* vLoadStoreOptions - Controls the INI file writing and reading */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
void
vLoadStoreOptions (test_t * ptest,	/* Run Configuration Parameters  */
    int fLoad			/* TRUE if Loading from Ini File */
    )
{
}


/*-------------------------------------------------------------------------*/
/* vSetGetTableDialog - Controls the transfer of flags between the  */
/*                      structure and the Dialog */
/* */
/* Returns:  Nothing */
/*-------------------------------------------------------------------------*/
int
getDriverTypeIndex (char *szDBMSName)
{
  int i;
  for (i = 0; i < sizeof (DriverMap) / sizeof (DRIVERMAP); i++)
    if (_stristr (szDBMSName, DriverMap[i].szDbName))
      return i;
  return 10;			/* ANSI */
}


void
vCreateVirtuosoTPCCTables (test_t * lpBench)
{
  RETCODE rc;
  UWORD uwTypeIdx;
  char szSQLBuffer[2048], szCommandBuffer[2048];
  int nDriverMapIndex = getDriverTypeIndex (lpBench->szDBMS);
  HSTMT hstmt = lpBench->hstmt;
  int indexType = DriverMap[nDriverMapIndex].swITM;

  uwTypeIdx = DriverMap[nDriverMapIndex].uwDTM;

  /* Warehouse table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local warehouse table\n");
  sprintf (szSQLBuffer, szDropTable, "warehouse");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[0][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[0]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote warehouse table in %s\n",
	  lpBench->tpc.c.tableDSNS[0]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[0],
	  "warehouse");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the warehouse table in %s\n",
	  lpBench->tpc.c.tableDSNS[0]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateWarehouse);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[0],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpFloat);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log
	  ("Creating the warehouse_prime index on warehouse table in %s\n",
	  lpBench->tpc.c.tableDSNS[0]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex1);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[0],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "warehouse_prime", "warehouse", "w_id");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the warehouse table from %s\n",
	  lpBench->tpc.c.tableDSNS[0]);
      sprintf (szSQLBuffer, szAttachTable, "warehouse",
	  lpBench->tpc.c.tableDSNS[0]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      pane_log ("Creating the local warehouse table\n");
      /* Create new table definition */
      sprintf (szSQLBuffer, szCreateWarehouse,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpFloat);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex1,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "warehouse_prime", "warehouse", "w_id");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }

  /* district table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local district table\n");
  sprintf (szSQLBuffer, szDropTable, "district");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[1][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[1]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote district table in %s\n",
	  lpBench->tpc.c.tableDSNS[1]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[1],
	  "district");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the district table in %s\n",
	  lpBench->tpc.c.tableDSNS[1]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateDistrict);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[1],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpFloat,
	  DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpInt);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log
	  ("Creating the district_prime index on warehouse table in %s\n",
	  lpBench->tpc.c.tableDSNS[1]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex2);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[1],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "district_prime", "district", "d_w_id", "d_id");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the district table from %s\n",
	  lpBench->tpc.c.tableDSNS[1]);
      sprintf (szSQLBuffer, szAttachTable, "district",
	  lpBench->tpc.c.tableDSNS[1]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local district table\n");
      sprintf (szSQLBuffer, szCreateDistrict,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpFloat,
	  DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpInt);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex2,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "district_prime", "district", "d_w_id", "d_id");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }

  /* customer table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local customer table\n");
  sprintf (szSQLBuffer, szDropTable, "customer");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[2][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[2]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote customer table in %s\n",
	  lpBench->tpc.c.tableDSNS[2]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[2],
	  "customer");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the customer table in %s\n",
	  lpBench->tpc.c.tableDSNS[2]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateCustomer);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[2],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpTimestamp,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpFloat,
	  DataTypeMap[TypeIdx].lpFloat,
	  DataTypeMap[TypeIdx].lpFloat,
	  DataTypeMap[TypeIdx].lpFloat,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpChar, DataTypeMap[TypeIdx].lpChar);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Creating the customer_prime index on customer table in %s\n",
	  lpBench->tpc.c.tableDSNS[2]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex3);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[2],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "customer_prime", "customer", "c_w_id", "c_d_id", "c_id");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Creating the customer_ncl index on customer table in %s\n",
	  lpBench->tpc.c.tableDSNS[2]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex5);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[2],
	  "unique",
	  "customer_ncl",
	  "customer", "c_w_id", "c_d_id", "c_last", "c_first", "c_id");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the customer table from %s\n",
	  lpBench->tpc.c.tableDSNS[2]);
      sprintf (szSQLBuffer, szAttachTable, "customer",
	  lpBench->tpc.c.tableDSNS[2]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local customer table\n");
      sprintf (szSQLBuffer, szCreateCustomer,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpTimestamp,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpFloat,
	  DataTypeMap[uwTypeIdx].lpFloat,
	  DataTypeMap[uwTypeIdx].lpFloat,
	  DataTypeMap[uwTypeIdx].lpFloat,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpChar, DataTypeMap[uwTypeIdx].lpChar);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex3,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "customer_prime", "customer", "c_w_id", "c_d_id", "c_id");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex5,
	  "unique",
	  "customer_ncl",
	  "customer", "c_w_id", "c_d_id", "c_last", "c_first", "c_id");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

    }

  /* History table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local thistory table\n");
  sprintf (szSQLBuffer, szDropTable, "THISTORY");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[3][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[3]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;

      pane_log ("Dropping the remote thistory table in %s\n",
	  lpBench->tpc.c.tableDSNS[3]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[3],
	  "THISTORY");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the thistory table in %s\n",
	  lpBench->tpc.c.tableDSNS[3]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateTHistory);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[3],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpTimestamp,
	  DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpChar);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the thistory table from %s\n",
	  lpBench->tpc.c.tableDSNS[3]);
      sprintf (szSQLBuffer, szAttachTable, "THISTORY",
	  lpBench->tpc.c.tableDSNS[3]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local thistory table\n");
      sprintf (szSQLBuffer, szCreateTHistory,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpTimestamp,
	  DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);

      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }

  /* New_order table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local new_order table\n");
  sprintf (szSQLBuffer, szDropTable, "NEW_ORDER");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[4][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[4]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote new_order table in %s\n",
	  lpBench->tpc.c.tableDSNS[4]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[4],
	  "NEW_ORDER");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the new_order table in %s\n",
	  lpBench->tpc.c.tableDSNS[4]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateNewOrder);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[4],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt, DataTypeMap[TypeIdx].lpInt);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log
	  ("Creating the new_order_prime index on new_order table in %s\n",
	  lpBench->tpc.c.tableDSNS[4]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex3);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[4],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "NEW_ORDER_PRIME", "NEW_ORDER", "NO_W_ID", "NO_D_ID", "NO_O_ID");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the new_order table from %s\n",
	  lpBench->tpc.c.tableDSNS[4]);
      sprintf (szSQLBuffer, szAttachTable, "NEW_ORDER",
	  lpBench->tpc.c.tableDSNS[4]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local new_order table\n");
      sprintf (szSQLBuffer, szCreateNewOrder,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt, DataTypeMap[uwTypeIdx].lpInt);

      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex3,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "NEW_ORDER_PRIME", "NEW_ORDER", "NO_W_ID", "NO_D_ID", "NO_O_ID");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }

  /* Orders table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local orders table\n");
  sprintf (szSQLBuffer, szDropTable, "ORDERS");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[5][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[5]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote orders table in %s\n",
	  lpBench->tpc.c.tableDSNS[5]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[5],
	  "ORDERS");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the orders table in %s\n",
	  lpBench->tpc.c.tableDSNS[5]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateOrders);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[5],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpTimestamp,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt, DataTypeMap[TypeIdx].lpInt);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Creating the orders_prime index on orders table in %s\n",
	  lpBench->tpc.c.tableDSNS[5]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex3);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[5],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "ORDERS_PRIME", "ORDERS", "O_W_ID", "O_D_ID", "O_ID");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the orders table from %s\n",
	  lpBench->tpc.c.tableDSNS[5]);
      sprintf (szSQLBuffer, szAttachTable, "ORDERS",
	  lpBench->tpc.c.tableDSNS[5]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local orders table\n");
      sprintf (szSQLBuffer, szCreateOrders,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpTimestamp,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt, DataTypeMap[uwTypeIdx].lpInt);

      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex3,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "ORDERS_PRIME", "ORDERS", "O_W_ID", "O_D_ID", "O_ID");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }

  /* Order_line table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local order_line table\n");
  sprintf (szSQLBuffer, szDropTable, "ORDER_LINE");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[6][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[6]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote order_line table in %s\n",
	  lpBench->tpc.c.tableDSNS[6]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[6],
	  "ORDER_LINE");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the order_line table in %s\n",
	  lpBench->tpc.c.tableDSNS[6]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateOrderLine);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[6],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpTimestamp,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpChar);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log
	  ("Creating the order_line_prime index on order_line table in %s\n",
	  lpBench->tpc.c.tableDSNS[6]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex4);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[6],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "ORDER_LINE_PRIME",
	  "ORDER_LINE", "OL_W_ID", "OL_D_ID", "OL_O_ID", "OL_NUMBER");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the order_line table from %s\n",
	  lpBench->tpc.c.tableDSNS[6]);
      sprintf (szSQLBuffer, szAttachTable, "ORDER_LINE",
	  lpBench->tpc.c.tableDSNS[6]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local order_line table\n");
      sprintf (szSQLBuffer, szCreateOrderLine,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpTimestamp,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);

      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex4,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "ORDER_LINE_PRIME",
	  "ORDER_LINE", "OL_W_ID", "OL_D_ID", "OL_O_ID", "OL_NUMBER");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

    }

  /* item table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local item table\n");
  sprintf (szSQLBuffer, szDropTable, "ITEM");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[7][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[7]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote item table in %s\n",
	  lpBench->tpc.c.tableDSNS[7]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[7],
	  "ITEM");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the item table in %s\n",
	  lpBench->tpc.c.tableDSNS[7]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateItem);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[7],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpFloat, DataTypeMap[TypeIdx].lpChar);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Creating the item_prime index on item table in %s\n",
	  lpBench->tpc.c.tableDSNS[7]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex1);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[7],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "ITEM_PRIME", "ITEM", "I_ID");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the item table from %s\n",
	  lpBench->tpc.c.tableDSNS[7]);
      sprintf (szSQLBuffer, szAttachTable, "ITEM",
	  lpBench->tpc.c.tableDSNS[7]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local item table\n");
      sprintf (szSQLBuffer, szCreateItem,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpFloat, DataTypeMap[uwTypeIdx].lpChar);

      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex1,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "ITEM_PRIME", "ITEM", "I_ID");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

    }

  /* stock table. */
  /* */

  /* Remove old table definition */
  pane_log ("Dropping the local stock table\n");
  sprintf (szSQLBuffer, szDropTable, "STOCK");
  rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

  if (lpBench->tpc.c.tableDSNS[8][0])
    {				/* a remote table */
      int DriverMapIndex = getDriverTypeIndex (lpBench->tpc.c.tableDBMSes[8]);
      int TypeIdx = DriverMap[DriverMapIndex].uwDTM;
      int indexType = DriverMap[DriverMapIndex].swITM;

      pane_log ("Dropping the remote sock table in %s\n",
	  lpBench->tpc.c.tableDSNS[8]);
      sprintf (szSQLBuffer, szRemoteDropTable, lpBench->tpc.c.tableDSNS[8],
	  "STOCK");
      rc = SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS);

      pane_log ("Creating the stock table in %s\n",
	  lpBench->tpc.c.tableDSNS[8]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateStock);
      strcat (szCommandBuffer, "')");

      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[8],
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpChar,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt,
	  DataTypeMap[TypeIdx].lpInt, DataTypeMap[TypeIdx].lpChar);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Creating the stock_prime index on stock table in %s\n",
	  lpBench->tpc.c.tableDSNS[8]);
      strcpy (szCommandBuffer, "rexecute('%s', '");
      strcat (szCommandBuffer, szCreateIndex2);
      strcat (szCommandBuffer, "')");
      sprintf (szSQLBuffer, szCommandBuffer,
	  lpBench->tpc.c.tableDSNS[8],
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "STOCK_PRIME", "STOCK", "S_I_ID", "S_W_ID");
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      pane_log ("Attaching the stock table from %s\n",
	  lpBench->tpc.c.tableDSNS[8]);
      sprintf (szSQLBuffer, szAttachTable, "STOCK",
	  lpBench->tpc.c.tableDSNS[8]);
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }
  else
    {
      /* Create new table definitions */
      pane_log ("Creating the local stock table\n");
      sprintf (szSQLBuffer, szCreateStock,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpChar,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt,
	  DataTypeMap[uwTypeIdx].lpInt, DataTypeMap[uwTypeIdx].lpChar);

      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);

      sprintf (szSQLBuffer, szCreateIndex2,
	  (char *) IndexTypeMap[indexType].szIndexType,
	  "STOCK_PRIME", "STOCK", "S_I_ID", "S_W_ID");

      pane_log (szSQLBuffer);
      pane_log ("\r\n");
      /* Execute Index Create Statement */
      if (SQL_SUCCESS != SQLExecDirect (hstmt, szSQLBuffer, SQL_NTS))
	vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, lpBench);
    }

  pane_log ("\r\nTABLE CREATION FINISHED\r\n");

  return;
}


extern int do_rollback_on_deadlock;
/* EXECUTION*/

#define ROLLBACK_OR_EXIT(rc, start_over, error) \
  switch(rc) \
    { \
      case -1: \
	  goto error; \
	  break; \
      case 1: \
	  if (do_rollback_on_deadlock && lpBench->tpc.a.fUseCommit && strstr(lpBench->szDBMS, "Virtuoso")) \
	    SQLTransact(SQL_NULL_HENV, lpBench->hdbc, SQL_ROLLBACK); \
	  goto start_over; \
    }

#define GO_RETCODE(ret_code, deadlock, error) \
  switch (ret_code) \
    { \
      case 1: goto deadlock; \
      case -1: goto error; \
    }

#define _IF_DEADLOCK_OR_ERR_GO(stmt, tag, foo, deadlocktag) \
if (SQL_ERROR == (foo)) \
{ \
	RETCODE _rc = SQLError (SQL_NULL_HENV, SQL_NULL_HDBC, stmt, (UCHAR *) state, NULL, \
		(UCHAR *) & message, sizeof (message), (SWORD *) & len); \
	if (_rc == SQL_SUCCESS && 0 == strncmp(state, "40001", 5)) \
	  { \
	    goto deadlocktag; \
	  } \
	else \
	  { \
	    strncpy(lpBench->szSQLError, message, sizeof(lpBench->szSQLError)); \
	    strncpy(lpBench->szSQLState, state, sizeof(lpBench->szSQLState)); \
	    goto tag; \
	  } \
}

#define _IF_ERR_GO(stmt, tag, foo) \
if (SQL_ERROR == (foo)) \
{ \
  SQLError(SQL_NULL_HENV, SQL_NULL_HDBC, stmt, NULL, NULL, lpBench->szSQLError, sizeof(lpBench->szSQLError), NULL); \
  goto tag; \
}

#define MAX_MIN_SIZE		2

BOOL
DoThreadsRun (test_t * lpBench)
{
  int nRun;			/* Counter for each run */

  for (nRun = 0; nRun < lpBench->tpc._.nRuns; nRun++)
    {
      if (!fRunTrans (lpBench, NULL))
	return FALSE;
    }

  /* User didn't break, so return success */
  return TRUE;
}

/***************************************************************************
 DoRun - Run a combination based on the current settings.

 Returns:  FALSE if the runs should stop
***************************************************************************/


BOOL
DoRun (test_t * lpBench,	/* Benchmark settings */
    char *szTitle)
{
  int nRun;			/* Counter for each run */

  /* Do every requested run for the time allotted  */
  for (nRun = 0; nRun < lpBench->tpc._.nRuns; nRun++)
    {
      pane_log ("Starting benchmark run number: %d\r\n", nRun + 1);
      if (fRunTrans (lpBench, szTitle))
	CalcStats (lpBench,
	    lpBench->tpc.a.nTrnCnt,
	    lpBench->tpc.a.nTrnCnt1Sec,
	    lpBench->tpc.a.nTrnCnt2Sec, lpBench->tpc.a.dDiffSum);
      else
	{
	  if (lpBench->szSQLError[0])
	    pane_log ("Error running the benchmark : %s\n",
		lpBench->szSQLError);
	  return FALSE;
	}
    }

  /* User didn't break, so return success */
  return TRUE;
}


/***************************************************************************
 fGetParams - Prompts user for run-time options and puts them in
                      the lpBench struct.  Errors may be issued.

 Returns:  TRUE if successful, FALSE on an error
***************************************************************************/
#if 0
BOOL
fGetParams (GtkWidget * executer,	/* Parent dialog */
    test_t * lpBench		/* Benchmark info to init */
    )
{
  save_config (EXECUTER (executer));
  return TRUE;
}
#endif

/***************************************************************************
 GetMaxVals - Find the max records in each transaction table, in order
                      to generate data in line with the data.

 Returns:  Nothing
***************************************************************************/
BOOL
GetMaxVals (test_t * lpBench	/* Benchmark info */
    )
{
  HSTMT hstmt;
  SDWORD cbData;

  SQLAllocStmt (lpBench->hdbc, &hstmt);
  SQLSetStmtOption (hstmt, SQL_ASYNC_ENABLE, SQL_ASYNC_ENABLE_OFF);

  if (!fSQLExecDirect (hstmt, "select max(BRANCH) from BRANCH", lpBench))
    return FALSE;
  SQLFetch (hstmt);
  fSQLGetData (hstmt, 1, SQL_C_LONG, &lpBench->tpc.a.udwMaxBranch, 0,
      &cbData);
  SQLFreeStmt (hstmt, SQL_CLOSE);
  if (cbData == SQL_NULL_DATA)
    {
      pane_log ("No data in BRANCH table");
      return FALSE;
    }

  if (!fSQLExecDirect (hstmt, "select max(TELLER) from TELLER", lpBench))
    return FALSE;
  SQLFetch (hstmt);
  fSQLGetData (hstmt, 1, SQL_C_LONG, &lpBench->tpc.a.udwMaxTeller, 0,
      &cbData);
  SQLFreeStmt (hstmt, SQL_CLOSE);
  if (cbData == SQL_NULL_DATA)
    {
      pane_log ("No data in TELLER table");
      return FALSE;
    }

  if (!fSQLExecDirect (hstmt, "select max(ACCOUNT) from ACCOUNT", lpBench))
    return FALSE;
  SQLFetch (hstmt);
  fSQLGetData (hstmt, 1, SQL_C_LONG, &lpBench->tpc.a.udwMaxAccount, 0,
      &cbData);
  SQLFreeStmt (hstmt, SQL_DROP);
  if (cbData == SQL_NULL_DATA)
    {
      pane_log ("No data in ACCOUNT table");
      return FALSE;
    }
  pane_log ("Max branch = %ld, Max teller = %ld, Max account = %ld\r\n",
      lpBench->tpc.a.udwMaxBranch,
      lpBench->tpc.a.udwMaxTeller, lpBench->tpc.a.udwMaxAccount);

  return (lpBench->tpc.a.udwMaxBranch && lpBench->tpc.a.udwMaxTeller
      && lpBench->tpc.a.udwMaxAccount);
}


/***************************************************************************
 fExecuteSql - Execute a SQL statement with error checking.

 Returns:  TRUE if successful, FALSE on error
***************************************************************************/
int
fExecuteSql (test_t * lpBench,	/* Bench info */
    char *pszSql		/* SQL Statement to execute */
    )
{
  RETCODE rc;
  DECLARE_FOR_SQLERROR;

  if (!lpBench->hstmt)
    {
      if (!lpBench->hdbc)
	return -1;

      rc = SQLAllocStmt (lpBench->hdbc, &lpBench->hstmt);
      if (rc != SQL_SUCCESS)
	{
	  lpBench->hstmt = 0;
	  return -1;
	}
    }
  /* Execute the statement until finished */
  do
    {
      rc = SQLExecDirect (lpBench->hstmt, (UCHAR FAR *) pszSql, SQL_NTS);
    }
  while (SQL_STILL_EXECUTING == rc);
  _IF_DEADLOCK_OR_ERR_GO (lpBench->hstmt, error_fexecute, rc,
      deadlock_fexecute);
  return (0);

deadlock_fexecute:
  return (1);
error_fexecute:
  return (-1);
}
/***************************************************************************
 fExecuteQuery - This function executes a simple query that returns
                      ~100 rows to add more read locks into the mix.

 Returns:  TRUE if successful, FALSE on error
***************************************************************************/
int
fExecuteQuery (test_t * lpBench,	/* Bench info */
    BOOL fExecute		/* TRUE if we should execute as well */
    )
{
  HSTMT hstmt = SQL_NULL_HSTMT;			/* Statement handle for executes */

  RETCODE rc = SQL_SUCCESS;	/* ODBC return code */

  BOOL fRtn = TRUE;		/* Bind col return */

  /* Dummy data buffers, we don't actually care about the results */
  struct result_s
  {
    long account;
    long branch;
    double balance;
    char filler[85];
    SDWORD account_len, branch_len, balance_len, filler_len;
  }
  buffer[100];

  UDWORD row_count;
  UWORD row_status[100];
  DECLARE_FOR_SQLERROR;

  if (!lpBench->tpc.a.fDoQuery)
    return 0;

  if (lpBench->tpc.a.nRowsetSize > 100)
    {
      if (gui.err_message)
        gui.err_message ("Rowset size can't be greater then 100\n");
      exit (-1);
    }
  /* Give user some feed-back */
  if (lpBench->SetWorkingItem)
    lpBench->SetWorkingItem ("Execute 100 row query");

  /* 
     Execute the statement with error processing 
     This will stress not only raw fetch speed but conversions from integer
     and float to character
   */
  if (lpBench->tpc.a.fUseCommit)
    {
      rc = SQLTransact (SQL_NULL_HENV, lpBench->hdbc, SQL_COMMIT);
      /*_IF_DEADLOCK_OR_ERR_GO(hstmt, error_query, rc, deadlock_query);*/
      if (rc == SQL_ERROR)
	{
	  if (SQL_SUCCESS == SQLError (SQL_NULL_HENV, lpBench->hdbc,
		  SQL_NULL_HSTMT, state, NULL, (UCHAR *) & message,
		  sizeof (message), (SWORD *) & len))
	    {
	      if (0 == strncmp (state, "40001", 5))
		goto deadlock_query;
	      strncpy (lpBench->szSQLError, message,
		  sizeof (lpBench->szSQLError));
	      strncpy (lpBench->szSQLState, state,
		  sizeof (lpBench->szSQLState));
	    }
	  goto error_query;
	}
    }
  SQLAllocStmt (lpBench->hdbc, &hstmt);
  SQLSetStmtOption (hstmt, SQL_CONCURRENCY, SQL_CONCUR_READ_ONLY);
  if (lpBench->tpc.a.nCursorType > 0)
    {
      SQLSetStmtOption (hstmt, SQL_CURSOR_TYPE,
	  lpBench->tpc.a.nCursorType ==
	  SQL_CURSOR_MIXED ? SQL_CURSOR_KEYSET_DRIVEN : lpBench->tpc.
	  a.nCursorType);
      if (lpBench->tpc.a.nCursorType == SQL_CURSOR_MIXED)
	SQLSetStmtOption (hstmt, SQL_KEYSET_SIZE, lpBench->tpc.a.nKeysetSize);
      SQLSetStmtOption (hstmt, SQL_ROWSET_SIZE, lpBench->tpc.a.nRowsetSize);
      SQLSetStmtOption (hstmt, SQL_BIND_TYPE, sizeof (struct result_s));
    }

  if (fExecute)
    {
      do
	{
	  rc = SQLExecDirect (hstmt,
	      "select ACCOUNT, BRANCH, BALANCE, FILLER from ACCOUNT where ACCOUNT < 101",
	      SQL_NTS);
	}
      while (SQL_STILL_EXECUTING == rc);
      _IF_DEADLOCK_OR_ERR_GO (hstmt, error_query, rc, deadlock_query);
    }

/*  buffer = g_malloc0 (sizeof (struct result_s) * lpBench->tpc.a.nRowsetSize);
  row_status = g_malloc0 (sizeof (UWORD) * lpBench->tpc.a.nRowsetSize);
      row_status = g_malloc0 (lpBench->tpc.a.nRowsetSize * sizeof (UWORD));
      */
  /*Bind all of the columns for return */
  fRtn = fSQLBindCol (hstmt, 1, SQL_C_LONG,
      &buffer[0].account, 0, &buffer[0].account_len);
  fRtn &= fSQLBindCol (hstmt, 2, SQL_C_LONG,
      &buffer[0].branch, 0, &buffer[0].branch_len);
  fRtn &= fSQLBindCol (hstmt, 3, SQL_C_DOUBLE,
      &buffer[0].balance, 0, &buffer[0].balance_len);
  fRtn &= fSQLBindCol (hstmt, 4, SQL_C_CHAR,
      &buffer[0].filler, sizeof (buffer[0].filler), &buffer[0].filler_len);

  if (!fRtn)
    goto error_query;

  /* If we bound successfully, then fetch all results */
  if (lpBench->tpc.a.nCursorType == SQL_CURSOR_FORWARD_ONLY)
    {
      do
	{
	  rc = SQLFetch (hstmt);
	}
      while (RC_SUCCESSFUL (rc) || (SQL_STILL_EXECUTING == rc));
      _IF_DEADLOCK_OR_ERR_GO (hstmt, error_query, rc, deadlock_query);
    }
  else
    {
      int nRep;
      for (nRep = 0; nRep < lpBench->tpc.a.nTraversalCount; nRep++)
	{
	  do
	    {
	      rc =
		  SQLExtendedFetch (hstmt, SQL_FETCH_NEXT, 0, &row_count,
		  row_status);
	    }
	  while (RC_SUCCESSFUL (rc) || SQL_STILL_EXECUTING == rc);
	  _IF_DEADLOCK_OR_ERR_GO (hstmt, error_query, rc, deadlock_query);
	  do
	    {
	      rc =
		  SQLExtendedFetch (hstmt, SQL_FETCH_PRIOR, 0, &row_count,
		  row_status);
	    }
	  while (RC_SUCCESSFUL (rc) || SQL_STILL_EXECUTING == rc);
	  _IF_DEADLOCK_OR_ERR_GO (hstmt, error_query, rc, deadlock_query);
	}
    }
  if (hstmt)
    SQLFreeStmt (hstmt, SQL_DROP);
/*  if (row_status)
    g_free (row_status);
  if (buffer)
    g_free (buffer);*/
  return (0);

error_query:
  if (hstmt)
    SQLFreeStmt (hstmt, SQL_DROP);
/*  if (row_status)
    g_free (row_status);
  if (buffer)
    g_free (buffer);*/
  return (-1);
deadlock_query:
  if (hstmt)
    SQLFreeStmt (hstmt, SQL_DROP);
/*  if (row_status)
    g_free (row_status);
  if (buffer)
    g_free (buffer);*/
  return (1);
}


/***************************************************************************
 fExecuteTrans - Execute the statements that make up the transaction.
                      Perform error processing.

 Returns:  TRUE if successful, FALSE on error
***************************************************************************/
int
fExecuteTrans (test_t * lpBench,	/* Bench info */
    SDWORD nTrnCnt,		/* Transaction number */
    SDWORD nAcctNum,		/* Account number */
    SDWORD nTellerNum,		/* Teller number */
    SDWORD nBranchNum,		/* Branch number */
    double *dBalance,		/* Balance for the transaction */
    double dDelta		/* Change for each transaction */
    )
{
  RETCODE rc;
  DECLARE_FOR_SQLERROR;

  char szStmt[1024];		/* The statement */

  char *filler = "1234567890123456789012";
  SDWORD cbBal;

/* SQL Statements in the transactions */
  static const char gszUPD_ACCOUNTS[] =
      "UPDATE ACCOUNT SET BALANCE = BALANCE + (%f) WHERE ACCOUNT = %ld";

  static const char gszSEL_BALANCE[] =
      "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT = %ld";

  static const char gszUPD_TELLERS[] =
      "UPDATE TELLER SET BALANCE = BALANCE + (%f) WHERE TELLER = %ld";

  static const char gszUPD_BRANCHES[] =
      "UPDATE BRANCH SET BALANCE = BALANCE + (%f) WHERE BRANCH = %ld";

  static const char gszINSERT_HIST[] =
      "INSERT INTO HISTORY (HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, TIMEOFTXN, FILLER) "
      "VALUES (%ld, %ld, %ld, %ld, %f, {fn %s()}, '%s')";



  /* Build SQL string for each statement and execute with SQLExecDirect */
  if (lpBench->fCancel && lpBench->fCancel ())
    {
      return FALSE;
    }
  sprintf (szStmt, gszUPD_ACCOUNTS, dDelta, nAcctNum);
  GO_RETCODE (fExecuteSql (lpBench, szStmt), start_over_text, error_text);

  sprintf (szStmt, gszSEL_BALANCE, nAcctNum);
  GO_RETCODE (fExecuteSql (lpBench, szStmt), start_over_text, error_text);

  /* Retrieve new balance for the account */
  rc = SQLFetch (lpBench->hstmt);
  _IF_DEADLOCK_OR_ERR_GO (lpBench->hstmt, error_text, rc, start_over_text);

  fSQLGetData (lpBench->hstmt, 1, SQL_C_DOUBLE, dBalance, 0, &cbBal);
  SQLFreeStmt (lpBench->hstmt, SQL_CLOSE);

  sprintf (szStmt, gszUPD_TELLERS, dDelta, nTellerNum);
  GO_RETCODE (fExecuteSql (lpBench, szStmt), start_over_text, error_text);

  sprintf (szStmt, gszUPD_BRANCHES, dDelta, nBranchNum);
  GO_RETCODE (fExecuteSql (lpBench, szStmt), start_over_text, error_text);

  sprintf (szStmt, gszINSERT_HIST, nTrnCnt, nAcctNum, nTellerNum,
      nBranchNum, dDelta, lpBench->pszDateTimeSQLFunc, filler);
  GO_RETCODE (fExecuteSql (lpBench, szStmt), start_over_text, error_text);

  /* Execute the query */
  GO_RETCODE (fExecuteQuery (lpBench, TRUE), start_over_text, error_text);
  return 0;

start_over_text:
  return 1;

error_text:
  return -1;
}


/***************************************************************************
 fExecuteSprocCall - Execute the prepared sproc.

 Returns:  TRUE if successful, FALSE on error
***************************************************************************/
int
fExecuteSprocCall (test_t * lpBench	/* Bench info */
    )
{
  RETCODE rc;
  DECLARE_FOR_SQLERROR;

  /* Execute the prepared statement as many times as required */
  if (lpBench->fCancel && lpBench->fCancel ())
    {
      return FALSE;
    }
  do
    {
      rc = SQLExecute (lpBench->hstmt);
    }
  while (SQL_STILL_EXECUTING == rc);
  _IF_DEADLOCK_OR_ERR_GO (lpBench->hstmt, error, rc, deadlock);
  SQLFreeStmt (lpBench->hstmt, SQL_CLOSE);

  /* If successful so far, then fetch the results */
  GO_RETCODE (fExecuteQuery (lpBench, TRUE), deadlock, error);
  return 0;
deadlock:
  return (1);
error:
  return (-1);
}

static BOOL
fBindParameters (test_t * lpBench,
    SDWORD * pnAcctNum,
    SDWORD * pnTellerNum,
    SDWORD * pnBranchNum,
    double *pdDelta,
    double *pdBalance,
    HSTMT hstmtUpdAcct,
    HSTMT hstmtSelBal,
    HSTMT hstmtUpdTeller, HSTMT hstmtUpdBranch, HSTMT hstmtInsHist)
{
  BOOL fSuccess;
  RETCODE rc;
  static char filler[] = "1234567890123456789012";

  if (IDX_SPROCS == lpBench->tpc.a.fSQLOption)
    {
      /* If we are using procedures, then the sproc gets all the arguments */
      /* in the call and performs all statements in the transaction inside the */
      /* sproc.  Here we bind all the sproc parameters so that in the main */
      /* loop we just call the procedure and the driver will send the current */
      /* value of the parameters. */
      fSuccess = TRUE;
      fSuccess &= fSQLBindParameter (lpBench->hstmt, 1, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  &lpBench->tpc.a.nTrnCnt, sizeof (lpBench->tpc.a.nTrnCnt), NULL);
      fSuccess &= fSQLBindParameter (lpBench->hstmt, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnAcctNum, sizeof (*pnAcctNum), NULL);
      fSuccess &= fSQLBindParameter (lpBench->hstmt, 3, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnTellerNum, sizeof (*pnTellerNum), NULL);
      fSuccess &= fSQLBindParameter (lpBench->hstmt, 4, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnBranchNum, sizeof (*pnBranchNum), NULL);
      fSuccess &= fSQLBindParameter (lpBench->hstmt, 5, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, sizeof (*pdDelta), NULL);
      fSuccess &=
	  fSQLBindParameter (lpBench->hstmt, 6, SQL_PARAM_INPUT_OUTPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0, pdBalance,
	  sizeof (*pdBalance), NULL);
      fSuccess &=
	  fSQLBindParameter (lpBench->hstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR,
	  SQL_CHAR, sizeof (filler), 0, filler, sizeof (filler), NULL);
      if (fSuccess)
	{
	  do
	    {
	      rc = SQLPrepare (lpBench->hstmt,
		  "{CALL ODBC_BENCHMARK(?,?,?,?,?,?,?)}", SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (lpBench->hwndMain, SQL_NULL_HENV, SQL_NULL_HDBC,
		  lpBench->hstmt, lpBench);
	      fSuccess = FALSE;
	    }
	}
    }
  else if (IDX_PARAMS == lpBench->tpc.a.fSQLOption)
    {
      /* If we are using Prepare/Execute of each statement, then the parameters */
      /* for each statement must be bound separately on a separate hstmt. */
      /* Like the sproc method we bind parameters so that in the main */
      /* loop we just call SQLExecute for each statement and the driver will use */
      /* the current values of the parameters. */
      fSuccess = TRUE;
      /* Update Account */
      fSuccess &= fSQLBindParameter (hstmtUpdAcct, 1, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, sizeof (*pdDelta), NULL);
      fSuccess &= fSQLBindParameter (hstmtUpdAcct, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnAcctNum, sizeof (*pnAcctNum), NULL);
      if (fSuccess)
	{
	  do
	    {
	      rc = SQLPrepare (hstmtUpdAcct,
		  "UPDATE ACCOUNT SET BALANCE = BALANCE + ? WHERE ACCOUNT = ?",
		  SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (lpBench->hwndMain, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtUpdAcct, lpBench);
	      fSuccess = FALSE;
	    }
	}


      /* Select balance */
      fSuccess &= fSQLBindParameter (hstmtSelBal, 1, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnAcctNum, sizeof (*pnAcctNum), NULL);
      if (fSuccess)
	{
	  do
	    {
	      rc = SQLPrepare (hstmtSelBal,
		  "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT = ?", SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (lpBench->hwndMain, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtSelBal, lpBench);
	      fSuccess = FALSE;
	    }
	}

      /* Update teller */
      fSuccess &= fSQLBindParameter (hstmtUpdTeller, 1, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, sizeof (*pdDelta), NULL);
      fSuccess &= fSQLBindParameter (hstmtUpdTeller, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnTellerNum, sizeof (*pnTellerNum), NULL);
      if (fSuccess)
	{

	  do
	    {
	      rc = SQLPrepare (hstmtUpdTeller,
		  "UPDATE TELLER SET BALANCE = BALANCE + ? WHERE TELLER = ?",
		  SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtUpdTeller, lpBench);
	      fSuccess = FALSE;
	    }
	}

      /* Update Branch */
      fSuccess &= fSQLBindParameter (hstmtUpdBranch, 1, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, sizeof (*pdDelta), NULL);
      fSuccess &= fSQLBindParameter (hstmtUpdBranch, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnBranchNum, sizeof (*pnBranchNum), NULL);
      if (fSuccess)
	{

	  do
	    {
	      rc = SQLPrepare (hstmtUpdBranch,
		  "UPDATE BRANCH SET BALANCE = BALANCE + ? WHERE BRANCH = ?",
		  SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtUpdBranch, lpBench);
	      fSuccess = FALSE;
	    }
	}

      /* Insert History record */

      fSuccess &= fSQLBindParameter (hstmtInsHist, 1, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  &lpBench->tpc.a.nTrnCnt, sizeof (lpBench->tpc.a.nTrnCnt), NULL);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnAcctNum, sizeof (*pnAcctNum), NULL);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 3, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnTellerNum, sizeof (*pnTellerNum), NULL);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 4, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnBranchNum, sizeof (*pnBranchNum), NULL);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 5, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, sizeof (*pdDelta), NULL);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 6, SQL_PARAM_INPUT,
	  SQL_C_CHAR, SQL_CHAR, sizeof (filler), 0,
	  filler, sizeof (filler), NULL);
      if (fSuccess)
	{
	  char szInsStmt[220];

	  sprintf (szInsStmt,
	      "INSERT INTO HISTORY (HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, TIMEOFTXN, FILLER) "
	      "VALUES (?, ?, ?, ?, ?, {fn %s()}, ?)",
	      lpBench->pszDateTimeSQLFunc);

	  do
	    {
	      rc = SQLPrepare (hstmtInsHist, szInsStmt, SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmtInsHist, lpBench);
	      fSuccess = FALSE;
	    }
	}
    }
  else
    fSuccess = TRUE;

  return fSuccess;
}


static BOOL
fBindParametersArray (test_t * lpBench,
    SDWORD nArrayParSize,
    UDWORD * nParamsProcessed,
    SDWORD * pnAcctNum,
    SDWORD * pnTellerNum,
    SDWORD * pnBranchNum,
    double *pdDelta,
    double *pdBalance,
    SDWORD *pID,
    SDWORD * pIndAcctNum,
    SDWORD * pIndTellerNum,
    SDWORD * pIndBranchNum,
    SDWORD *pIndDelta,
    SDWORD *pIndBalance,
    SDWORD *pIndID,
    SDWORD * nAcctNum,
    HSTMT hstmtUpdAcct,
    HSTMT hstmtSelBal,
    HSTMT hstmtUpdTeller, HSTMT hstmtUpdBranch, HSTMT hstmtInsHist)
{
  BOOL fSuccess;
  RETCODE rc;
  static char filler[] = "1234567890123456789012";

  if (IDX_PARAMS == lpBench->tpc.a.fSQLOption)
    {
      /* If we are using Prepare/Execute of each statement, then the parameters */
      /* for each statement must be bound separately on a separate hstmt. */
      /* Like the sproc method we bind parameters so that in the main */
      /* loop we just call SQLExecute for each statement and the driver will use */
      /* the current values of the parameters. */
      fSuccess = TRUE;

      /* Update Account */
      fSuccess &= fSQLParamOptions(hstmtUpdAcct, nArrayParSize, nParamsProcessed);
      fSuccess &= fSQLBindParameter (hstmtUpdAcct, 1, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, 0, pIndDelta);
      fSuccess &= fSQLBindParameter (hstmtUpdAcct, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnAcctNum, 0, pIndAcctNum);
      if (fSuccess)
	{
	  do
	    {
	      rc = SQLPrepare (hstmtUpdAcct,
		  "UPDATE ACCOUNT SET BALANCE = BALANCE + ? WHERE ACCOUNT = ?",
		  SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (lpBench->hwndMain, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtUpdAcct, lpBench);
	      fSuccess = FALSE;
	    }
	}


      /* Select balance */
      if (lpBench->fSQLBatchSupported)
        {
          fSuccess &= fSQLParamOptions(hstmtSelBal, nArrayParSize,
              nParamsProcessed);
          fSuccess &= fSQLBindParameter (hstmtSelBal, 1, SQL_PARAM_INPUT,
	      SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	      pnAcctNum, 0, pIndAcctNum);
        }
      else
        { 
          fSuccess &= fSQLBindParameter (hstmtSelBal, 1, SQL_PARAM_INPUT,
 	      SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	      nAcctNum, sizeof (*nAcctNum), NULL);
        }
      if (fSuccess)
	{
	  do
	    {
	      rc = SQLPrepare (hstmtSelBal,
		  "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT = ?", SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (lpBench->hwndMain, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtSelBal, lpBench);
	      fSuccess = FALSE;
	    }
	}


      /* Update teller */
      fSuccess &= fSQLParamOptions(hstmtUpdTeller, nArrayParSize, nParamsProcessed);
      fSuccess &= fSQLBindParameter (hstmtUpdTeller, 1, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, 0, pIndDelta);
      fSuccess &= fSQLBindParameter (hstmtUpdTeller, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnTellerNum, 0, pIndTellerNum);
      if (fSuccess)
	{

	  do
	    {
	      rc = SQLPrepare (hstmtUpdTeller,
		  "UPDATE TELLER SET BALANCE = BALANCE + ? WHERE TELLER = ?",
		  SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtUpdTeller, lpBench);
	      fSuccess = FALSE;
	    }
	}


      /* Update Branch */
      fSuccess &= fSQLParamOptions(hstmtUpdBranch, nArrayParSize, nParamsProcessed);
      fSuccess &= fSQLBindParameter (hstmtUpdBranch, 1, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, 0, pIndDelta);
      fSuccess &= fSQLBindParameter (hstmtUpdBranch, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnBranchNum, 0, pIndBranchNum);
      if (fSuccess)
	{

	  do
	    {
	      rc = SQLPrepare (hstmtUpdBranch,
		  "UPDATE BRANCH SET BALANCE = BALANCE + ? WHERE BRANCH = ?",
		  SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC,
		  hstmtUpdBranch, lpBench);
	      fSuccess = FALSE;
	    }
	}


      /* Insert History record */
      fSuccess &= fSQLParamOptions(hstmtInsHist, nArrayParSize, nParamsProcessed);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 1, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pID, 0, pIndID);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 2, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnAcctNum, 0, pIndAcctNum);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 3, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnTellerNum, 0, pIndTellerNum);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 4, SQL_PARAM_INPUT,
	  SQL_C_LONG, SQL_INTEGER, sizeof (SDWORD), 0,
	  pnBranchNum, 0, pIndBranchNum);
      fSuccess &= fSQLBindParameter (hstmtInsHist, 5, SQL_PARAM_INPUT,
	  SQL_C_DOUBLE, SQL_DOUBLE, sizeof (double), 0,
	  pdDelta, 0, pIndDelta);
      if (fSuccess)
	{
	  char szInsStmt[220];

	  sprintf (szInsStmt,
	      "INSERT INTO HISTORY (HISTID, ACCOUNT, TELLER, BRANCH, AMOUNT, TIMEOFTXN, FILLER) "
	      "VALUES (?, ?, ?, ?, ?, {fn %s()}, '%s')",
	      lpBench->pszDateTimeSQLFunc, filler);

	  do
	    {
	      rc = SQLPrepare (hstmtInsHist, szInsStmt, SQL_NTS);
	    }
	  while (SQL_STILL_EXECUTING == rc);

	  if (SQL_SUCCESS != rc)
	    {
	      vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmtInsHist, lpBench);
	      fSuccess = FALSE;
	    }
	}
    }
  else
    fSuccess = TRUE;

  return fSuccess;
}


/***************************************************************************
 fRunTransArray - Run the transaction test

 Returns:  TRUE if successful, FALSE on error
***************************************************************************/
BOOL
fRunTransArray (test_t * lpBench,	/* Benchmark info */
    char *szTitle)
{
  BOOL fRtn = FALSE;		/* Assume the worst */

  BOOL fDone = FALSE;		/* Loop control */

  BOOL fTrace = FALSE;		/* Debugging variable */

  RETCODE rc;			/* Return code for ODBC calls */

  /* Timing information */
  double dDiff;			/* Track time left for run */

  double dTransDiff;		/* Transaction total time */

  double dTimeToRun;		/* Time to run for */

  time_t tStartTime;		/* Start time */

  time_t tCurTime;		/* The current time */

  time_t tTransStartTime;	/* Transaction start time */

  time_t tTransEndTime;		/* End time for transaction */

  /* Data values */
  short nWorkStn;		/* Workstation id */

  SDWORD *pnAcctNum = NULL;		/* Account numbers  */
  SDWORD nAcctNum;			/* Account number  */

  SDWORD *pnBranchNum = NULL;		/* Branch number */

  SDWORD *pnTellerNum = NULL;		/* Teller number */

  double *pdBalance = NULL; 		/* Balance for transaction */

  double *pdDelta = NULL;		/* Delta, randomly set */

  SDWORD *pID = NULL;

  SDWORD *pIndAcctNum = NULL;		
  SDWORD *pIndBranchNum = NULL;	
  SDWORD *pIndTellerNum = NULL;
  SDWORD *pIndBalance = NULL;
  SDWORD *pIndDelta = NULL;
  SDWORD *pIndID = NULL;


  /* hstmts for bound parameters for Prepare/Execute method */
  HSTMT hstmtUpdAcct;		/* Update account table */

  HSTMT hstmtSelBal;		/* Select new balance from account */

  HSTMT hstmtUpdTeller;		/* Update teller table */

  HSTMT hstmtUpdBranch;		/* Update branch table */

  HSTMT hstmtInsHist;		/* Insert history record */

  BOOL fSuccess;

  DECLARE_FOR_SQLERROR;

  SDWORD cbBal;

  int ret_code;
  int i, nCallCnt;
  int nArrayParSize;
  UDWORD  nParamsProcessed;

  hstmtUpdAcct = hstmtSelBal = hstmtUpdTeller = hstmtUpdBranch = 
    hstmtInsHist = SQL_NULL_HSTMT;

  if (IDX_PARAMS != lpBench->tpc.a.fSQLOption)
    goto general_error;

  /* clear history table; get limits for each value  */
  /* to be randomly generated */
  if (!GetMaxVals (lpBench))
    return FALSE;

  nArrayParSize = lpBench->tpc.a.nArrayParSize;
  
  if (nArrayParSize == 0)
    nArrayParSize = 1;

  if ((pnAcctNum = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pnBranchNum = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pnTellerNum = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pdBalance = (double *)calloc(nArrayParSize, sizeof(double))) == NULL)
    goto general_error;
  if ((pdDelta = (double *)calloc(nArrayParSize, sizeof(double))) == NULL)
    goto general_error;
  if ((pID = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;

  if ((pIndAcctNum = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pIndBranchNum = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pIndTellerNum = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pIndBalance = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pIndDelta = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;
  if ((pIndID = (SDWORD *)malloc(sizeof(SDWORD) * nArrayParSize)) == NULL)
    goto general_error;

  /* Clear out any previous runs */
  lpBench->tpc.a.nTrnCnt = 0;
  lpBench->tpc.a.nTrnCnt1Sec = 0;
  lpBench->tpc.a.nTrnCnt2Sec = 0;
  lpBench->tpc.a.dDiffSum = 0;

  /* Tell the user we are starting */
  if (lpBench->ShowProgress)
    lpBench->ShowProgress (NULL, szTitle ? szTitle : "Running Benchmark...",
	FALSE, lpBench->tpc._.nMinutes * 60);

  /* Get the start time */
  time (&tStartTime);
  time (&tCurTime);
  dDiff = difftime (tCurTime, tStartTime);

  /* Set run-time options */
  /* Note setting async on the hdbc enables it for all hstmts on the hdbc */
  rc = SQLSetConnectOption (lpBench->hdbc, SQL_AUTOCOMMIT,
      (lpBench->tpc.a.fUseCommit) ? SQL_AUTOCOMMIT_OFF : SQL_AUTOCOMMIT_ON);
  rc = SQLSetConnectOption (lpBench->hdbc, SQL_ASYNC_ENABLE,
      (lpBench->tpc.a.fExecAsync) ? SQL_ASYNC_ENABLE_ON : SQL_ASYNC_ENABLE_OFF);

  /* Create a unique workstation id by using our instance handle */
#ifndef WIN32
  nWorkStn = (short) getpid ();
#else  
  nWorkStn = (short) _getpid ();
#endif

  /* allocate the statements */
  _IF_ERR_GO (hstmtUpdAcct, general_error, SQLAllocStmt (lpBench->hdbc,
      &hstmtUpdAcct));
  _IF_ERR_GO (hstmtSelBal, general_error, SQLAllocStmt (lpBench->hdbc,
      &hstmtSelBal));
  _IF_ERR_GO (hstmtUpdTeller, general_error, SQLAllocStmt (lpBench->hdbc,
      &hstmtUpdTeller));
  _IF_ERR_GO (hstmtUpdBranch, general_error, SQLAllocStmt (lpBench->hdbc,
      &hstmtUpdBranch));
  _IF_ERR_GO (hstmtInsHist, general_error, SQLAllocStmt (lpBench->hdbc,
      &hstmtInsHist));

  if (!(fSuccess = fBindParametersArray (lpBench, nArrayParSize, &nParamsProcessed,
	      pnAcctNum, pnTellerNum, pnBranchNum, pdDelta, pdBalance, pID,
	      pIndAcctNum, pIndTellerNum, pIndBranchNum, pIndDelta, pIndBalance, 
              pIndID, &nAcctNum,
	      hstmtUpdAcct, hstmtSelBal, hstmtUpdTeller, hstmtUpdBranch,
	      hstmtInsHist)))
    fDone = TRUE;

  /* Main loop executes the transactions and checks for the duration */
  /* and the possibility that the user may cancel. */
  dTimeToRun = lpBench->tpc._.nMinutes * 60;
  srand ((unsigned) time (NULL));
  nCallCnt = 0;
  while ((dDiff <= dTimeToRun) && !fDone)
    {
      SDWORD id;
      
      id = lpBench->tpc.a.nTrnCnt + 1;
      for (i = 0; i < nArrayParSize; i++)
        {
          /* Generate random data for each field and amount */
          pnAcctNum[i] = ((rand () * rand ()) % lpBench->tpc.a.udwMaxAccount) + 1;
          assert (pnAcctNum[i] > -1);

          pnBranchNum[i] = (rand () % lpBench->tpc.a.udwMaxBranch) + 1;
          assert (pnBranchNum[i] > -1);

          pnTellerNum[i] = (rand () % lpBench->tpc.a.udwMaxTeller) + 1;
          assert (pnTellerNum[i] > -1);

          /* Arbitrarily set bank transaction to a random amount */
          /* no greater than the number of tellers. The type of */
          /* transaction (deposit or withdrawal) is determined */
          /* by the C runtime function 'time' being even or odd */
          pdDelta[i] = ((rand () % lpBench->tpc.a.udwMaxTeller) + 1) *
	      (double) (((long) time (NULL) % 2) ? 1 : -1);
	  pID[i] = id;
	  id++;
	}
      /* Add to transaction counter */
      lpBench->tpc.a.nTrnCnt += nArrayParSize;

      nCallCnt++;

    txn_start_over:
      /* Obtain the start time for this transaction */
      time (&tTransStartTime);

      /* Now execute this transaction */

      /* Prepare/Execute with parameters */
	fRtn = TRUE;
	if (lpBench->SetWorkingItem)
	  lpBench->SetWorkingItem
		("Executing prepared statement with parameters");
	/* Update account */
	if (lpBench->fCancel && lpBench->fCancel ())
	  {
	    fDone = TRUE;
	    fRtn = FALSE;
	    pane_log ("*** Canceled ***\r\n");
	  }
	do
	  {
	    rc = SQLExecute (hstmtUpdAcct);
	  }
	while (SQL_STILL_EXECUTING == rc);
	IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtUpdAcct, general_error,
	      rc, deadlock_main);

	/* Select new balance for the account just updated */
        if (lpBench->fSQLBatchSupported)
          {
	    i = 0;
	    do
	      {
	        rc = SQLExecute (hstmtSelBal);
	      }
	    while (SQL_STILL_EXECUTING == rc);
	    IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error, rc,
	        deadlock_main);

            ret_code = SQL_SUCCESS;
            do 
	      {
	        rc = SQLFetch (hstmtSelBal);
	        IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error, 
                    rc, deadlock_main);
	        rc = SQLGetData (hstmtSelBal, 1, SQL_C_DOUBLE, pdBalance + i, 
                    0, &cbBal);
	        IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error,
                    rc, deadlock_main);
                do
	          {
	            rc = SQLMoreResults(hstmtSelBal);
	          }
	        while (SQL_STILL_EXECUTING == rc);
	        IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error,
                    rc, deadlock_main);
	        ret_code = rc;
	        i++;
	      }
	    while (SQL_SUCCESS == ret_code && i < nArrayParSize);

            SQLFreeStmt (hstmtSelBal, SQL_CLOSE);
          }
        else /*** if Arrays binding for Selects isn't suported ***/
          for (i = 0; i < nArrayParSize; i++)
            {
              nAcctNum = pnAcctNum[i];
	      do
	        {
	          rc = SQLExecute (hstmtSelBal);
	        }
	      while (SQL_STILL_EXECUTING == rc);
	      IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error,
                rc, deadlock_main);

	      rc = SQLFetch (hstmtSelBal);
	      IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error, 
                    rc, deadlock_main);
	      rc = SQLGetData (hstmtSelBal, 1, SQL_C_DOUBLE, pdBalance + i, 
                    0, &cbBal);
	      IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error,
                    rc, deadlock_main);

              SQLFreeStmt (hstmtSelBal, SQL_CLOSE);
            }

	/* Update teller */
	do
	  {
	    rc = SQLExecute (hstmtUpdTeller);
	  }
	while (SQL_STILL_EXECUTING == rc);
	IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtUpdTeller, general_error,
	    rc, deadlock_main);

	/* Update branch */
	do
	  {
	    rc = SQLExecute (hstmtUpdBranch);
	  }
	while (SQL_STILL_EXECUTING == rc);
	IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtUpdBranch, general_error,
	    rc, deadlock_main);
	/* Insert into history table */
	do
	  {
	    rc = SQLExecute (hstmtInsHist);
	  }
	while (SQL_STILL_EXECUTING == rc);
	IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtInsHist, general_error,
	    rc, deadlock_main);
	ret_code = fExecuteQuery (lpBench, TRUE);
	ROLLBACK_OR_EXIT (ret_code, deadlock_main, general_error);

      if (fTrace)
        for (i = 0; i < nArrayParSize; i++)
          {
	    pane_log
	        ("Account # = %ld, Teller = %ld, Branch = %ld, Amount = %f, Balance=%f\r\n",
	        pnAcctNum[i], pnTellerNum[i], pnBranchNum[i], pdDelta[i], pdBalance[i]);
           }

      /* Commit the transaction based on success */
      /* Note: For stored procedure method this has no effect; */
      /* the commit happens in the sproc */
      if (lpBench->tpc.a.fUseCommit)
	{
	  rc = SQLTransact (SQL_NULL_HENV, lpBench->hdbc,
	      (UWORD) ((fRtn) ? SQL_COMMIT : SQL_ROLLBACK));
	  fRtn &= RC_SUCCESSFUL (rc);
	}

      /* Get the end time and the elapsed time */
      time (&tTransEndTime);
      dTransDiff = difftime (tTransEndTime, tTransStartTime);
      lpBench->tpc.a.dDiffSum += dTransDiff;

      /* Track 1-second and 2-second transactions */
      if (dTransDiff <= 1)
	lpBench->tpc.a.nTrnCnt1Sec += nArrayParSize;
      if (dTransDiff > 1 && dTransDiff <= 2)
	lpBench->tpc.a.nTrnCnt2Sec += nArrayParSize;

      /* Get elapsed time now to see whether we should quit */
      time (&tCurTime);
      dDiff = difftime (tCurTime, tStartTime);
      /* Every five transactions, see whether we need to cancel */
      if (0 == (nCallCnt % bench_get_long_pref (A_REFRESH_RATE)))
	{
	  char szBuff[50];
	  long secs_remain = (long int)(dTimeToRun - dDiff);
	  sprintf (szBuff, "%10ld txns, %10ld secs remaining",
	      lpBench->tpc.a.nTrnCnt, secs_remain);
	  if (lpBench->SetProgressText)
	    lpBench->SetProgressText (szBuff, lpBench->tpc._.nConn,
		lpBench->tpc._.nThreadNo, dDiff, nArrayParSize, secs_remain, 
		lpBench->tpc.a.dDiffSum);
	  if (lpBench->fCancel && lpBench->fCancel ())
	    {
	      fDone = TRUE;
	      fRtn = FALSE;
	      pane_log ("*** Cancelled ***\n");
	    }
	}
      continue;

    deadlock_main:
      if (bench_get_long_pref (LOCK_TIMEOUT) > 0
	  && lpBench->tpc.a.txn_isolation > SQL_TXN_READ_UNCOMMITTED)
	sleep_msecs (bench_get_long_pref (LOCK_TIMEOUT));
#if 0
      if (!(fSuccess = fBindParameters (lpBench,
		  &nAcctNum, &nTellerNum, &nBranchNum, &dDelta, &dBalance,
		  hstmtUpdAcct, hstmtSelBal, hstmtUpdTeller, hstmtUpdBranch,
		  hstmtInsHist)))
	fDone = TRUE;
#endif
      time (&tCurTime);
      dDiff = difftime (tCurTime, tStartTime);
      if (dDiff <= dTimeToRun)
	goto txn_start_over;
    }				/* end main loop */
  fDone = TRUE;
  if (dDiff >= dTimeToRun)
    fRtn = TRUE;



general_error:
  /* We're done, so dismiss the dialog */
  if (lpBench->StopProgress)
    lpBench->StopProgress ();

  if (fDone)
    pane_log ("\n\nBenchmark finished.\r\n");
  else
    pane_log ("\n\nBenchmark is ended with an error.\r\n");

  SQLFreeStmt (lpBench->hstmt, SQL_CLOSE);
  SQLFreeStmt (lpBench->hstmt, SQL_UNBIND);
  SQLFreeStmt (lpBench->hstmt, SQL_RESET_PARAMS);
  if (IDX_PARAMS == lpBench->tpc.a.fSQLOption)
    {
      if (hstmtUpdAcct != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtUpdAcct, SQL_DROP);
      if (hstmtSelBal != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtSelBal, SQL_DROP);
      if (hstmtUpdTeller != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtUpdTeller, SQL_DROP);
      if (hstmtUpdBranch != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtUpdBranch, SQL_DROP);
      if (hstmtInsHist != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtInsHist, SQL_DROP);
    }
  
  XFREE(pnAcctNum);
  XFREE(pnBranchNum);
  XFREE(pnTellerNum);
  XFREE(pdBalance);
  XFREE(pdDelta);
  XFREE(pID);

  XFREE(pIndAcctNum);
  XFREE(pIndBranchNum);
  XFREE(pIndTellerNum);
  XFREE(pIndBalance);
  XFREE(pIndDelta);
  XFREE(pIndID);

  if (!fDone)
    fRtn = FALSE;

  return fRtn;
}


/***************************************************************************
 fRunTrans - Run the transaction test

 Returns:  TRUE if successful, FALSE on error
***************************************************************************/
BOOL
fRunTrans (test_t * lpBench,	/* Benchmark info */
    char *szTitle)
{
  BOOL fRtn = FALSE;		/* Assume the worst */

  BOOL fDone = FALSE;		/* Loop control */

  BOOL fTrace = FALSE;		/* Debugging variable */

  RETCODE rc;			/* Return code for ODBC calls */

  /* Timing information */
  double dDiff;			/* Track time left for run */

  double dTransDiff;		/* Transaction total time */

  double dTimeToRun;		/* Time to run for */

  time_t tStartTime;		/* Start time */

  time_t tCurTime;		/* The current time */

  time_t tTransStartTime;	/* Transaction start time */

  time_t tTransEndTime;		/* End time for transaction */

  /* Data values */
  short nWorkStn;		/* Workstation id */

  SDWORD nAcctNum;		/* Account number  */

  SDWORD nBranchNum;		/* Branch number */

  SDWORD nTellerNum;		/* Teller number */

  double dBalance = 0;		/* Balance for transaction */

  double dDelta;		/* Delta, randomly set */


  /* hstmts for bound parameters for Prepare/Execute method */
  HSTMT hstmtUpdAcct;	/* Update account table */

  HSTMT hstmtSelBal;		/* Select new balance from account */

  HSTMT hstmtUpdTeller;		/* Update teller table */

  HSTMT hstmtUpdBranch;		/* Update branch table */

  HSTMT hstmtInsHist;		/* Insert history record */

  BOOL fSuccess;

  DECLARE_FOR_SQLERROR;

  SDWORD cbBal;

  int ret_code;

  hstmtUpdAcct = hstmtSelBal = hstmtUpdTeller = hstmtUpdBranch = 
    hstmtInsHist = SQL_NULL_HSTMT;

  if (IDX_PARAMS == lpBench->tpc.a.fSQLOption 
      && lpBench->tpc.a.nArrayParSize > 0
      && lpBench->fBatchSupported)
    return fRunTransArray (lpBench, szTitle);

  /* clear history table; get limits for each value  */
  /* to be randomly generated */
  if (!GetMaxVals (lpBench))
    return FALSE;


  /* Clear out any previous runs */
  lpBench->tpc.a.nTrnCnt = 0;
  lpBench->tpc.a.nTrnCnt1Sec = 0;
  lpBench->tpc.a.nTrnCnt2Sec = 0;
  lpBench->tpc.a.dDiffSum = 0;

  /* Tell the user we are starting */
  if (lpBench->ShowProgress)
    lpBench->ShowProgress (NULL, szTitle ? szTitle : "Running Benchmark...",
	FALSE, lpBench->tpc._.nMinutes * 60);

  /* Get the start time */
  time (&tStartTime);
  time (&tCurTime);
  dDiff = difftime (tCurTime, tStartTime);

  /* Set run-time options */
  /* Note setting async on the hdbc enables it for all hstmts on the hdbc */
  rc = SQLSetConnectOption (lpBench->hdbc, SQL_AUTOCOMMIT,
      (lpBench->tpc.a.fUseCommit) ? SQL_AUTOCOMMIT_OFF : SQL_AUTOCOMMIT_ON);
  rc = SQLSetConnectOption (lpBench->hdbc, SQL_ASYNC_ENABLE,
     (lpBench->tpc.a.fExecAsync) ? SQL_ASYNC_ENABLE_ON : SQL_ASYNC_ENABLE_OFF);

  /* Create a unique workstation id by using our instance handle */
#ifndef WIN32
  nWorkStn = (short) getpid ();
#else  
  nWorkStn = (short) _getpid ();
#endif

  if (IDX_PARAMS == lpBench->tpc.a.fSQLOption)
    {
      /* allocate the statements */
      _IF_ERR_GO (hstmtUpdAcct, general_error, SQLAllocStmt (lpBench->hdbc,
	      &hstmtUpdAcct));
      _IF_ERR_GO (hstmtSelBal, general_error, SQLAllocStmt (lpBench->hdbc,
	      &hstmtSelBal));
      _IF_ERR_GO (hstmtUpdTeller, general_error, SQLAllocStmt (lpBench->hdbc,
	      &hstmtUpdTeller));
      _IF_ERR_GO (hstmtUpdBranch, general_error, SQLAllocStmt (lpBench->hdbc,
	      &hstmtUpdBranch));
      _IF_ERR_GO (hstmtInsHist, general_error, SQLAllocStmt (lpBench->hdbc,
	      &hstmtInsHist));
    }

  if (!(fSuccess = fBindParameters (lpBench,
	      &nAcctNum, &nTellerNum, &nBranchNum, &dDelta, &dBalance,
	      hstmtUpdAcct, hstmtSelBal, hstmtUpdTeller, hstmtUpdBranch,
	      hstmtInsHist)))
    fDone = TRUE;

  /* Main loop executes the transactions and checks for the duration */
  /* and the possibility that the user may cancel. */
  dTimeToRun = lpBench->tpc._.nMinutes * 60;
  srand ((unsigned) time (NULL));
  while ((dDiff <= dTimeToRun) && !fDone)
    {
      /* Generate random data for each field and amount */
      nAcctNum = ((rand () * rand ()) % lpBench->tpc.a.udwMaxAccount) + 1;
      assert (nAcctNum > -1);

      nBranchNum = (rand () % lpBench->tpc.a.udwMaxBranch) + 1;
      assert (nBranchNum > -1);

      nTellerNum = (rand () % lpBench->tpc.a.udwMaxTeller) + 1;
      assert (nTellerNum > -1);

      /* Arbitrarily set bank transaction to a random amount */
      /* no greater than the number of tellers. The type of */
      /* transaction (deposit or withdrawal) is determined */
      /* by the C runtime function 'time' being even or odd */
      dDelta = ((rand () % lpBench->tpc.a.udwMaxTeller) + 1) *
	  (double) (((long) time (NULL) % 2) ? 1 : -1);

      /* Add 1 to transaction counter */
      ++lpBench->tpc.a.nTrnCnt;

    txn_start_over:
      /* Obtain the start time for this transaction */
      time (&tTransStartTime);

      /* Now execute this transaction */

      /* Stored procedure method */
      if (IDX_SPROCS == lpBench->tpc.a.fSQLOption)
	{
	  if (lpBench->SetWorkingItem)
	    lpBench->SetWorkingItem
		("Executing stored procedure with parameters");
	  ret_code = fExecuteSprocCall (lpBench);
	  ROLLBACK_OR_EXIT (ret_code, deadlock_main, general_error);
	}

      /* Prepare/Execute with parameters */
      else if (IDX_PARAMS == lpBench->tpc.a.fSQLOption)
	{
	  fRtn = TRUE;
	  if (lpBench->SetWorkingItem)
	    lpBench->SetWorkingItem
		("Executing prepared statement with parameters");
	  /* Update account */
	  if (lpBench->fCancel && lpBench->fCancel ())
	    {
	      fDone = TRUE;
	      fRtn = FALSE;
	      pane_log ("*** Canceled ***\r\n");
	    }
	  do
	    {
	      rc = SQLExecute (hstmtUpdAcct);
	    }
	  while (SQL_STILL_EXECUTING == rc);
	  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtUpdAcct, general_error,
	      rc, deadlock_main);

	  /* Select new balance for the account just updated */
	  do
	    {
	      rc = SQLExecute (hstmtSelBal);
	    }
	  while (SQL_STILL_EXECUTING == rc);
	  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error, rc,
	      deadlock_main);

	  rc = SQLFetch (hstmtSelBal);
	  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error, rc,
	      deadlock_main);
	  rc =
	      SQLGetData (hstmtSelBal, 1, SQL_C_DOUBLE, &dBalance, 0, &cbBal);
	  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtSelBal, general_error, rc,
	      deadlock_main);
	  SQLFreeStmt (hstmtSelBal, SQL_CLOSE);

	  /* Update teller */
	  do
	    {
	      rc = SQLExecute (hstmtUpdTeller);
	    }
	  while (SQL_STILL_EXECUTING == rc);
	  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtUpdTeller, general_error,
	      rc, deadlock_main);

	  /* Update branch */
	  do
	    {
	      rc = SQLExecute (hstmtUpdBranch);
	    }
	  while (SQL_STILL_EXECUTING == rc);
	  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtUpdBranch, general_error,
	      rc, deadlock_main);

	  /* Insert into history table */
	  do
	    {
	      rc = SQLExecute (hstmtInsHist);
	    }
	  while (SQL_STILL_EXECUTING == rc);
	  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmtInsHist, general_error,
	      rc, deadlock_main);
	  ret_code = fExecuteQuery (lpBench, TRUE);
	  ROLLBACK_OR_EXIT (ret_code, deadlock_main, general_error);
	}			/* Prepare/Execute w/params */

      /* For SQLExecDirect with SQL text method, */
      /* build SQL string with all values */
      else
	{
	  fRtn = TRUE;
	  if (lpBench->SetWorkingItem)
	    lpBench->SetWorkingItem ("Executing SQL text with no parameters");

	  ret_code = fExecuteTrans (lpBench, lpBench->tpc.a.nTrnCnt,
	      nAcctNum, nTellerNum, nBranchNum, &dBalance, dDelta);
	  ROLLBACK_OR_EXIT (ret_code, deadlock_main, general_error);
	}

      if (fTrace)
	pane_log
	    ("Account # = %ld, Teller = %ld, Branch = %ld, Amount = %f, Balance=%f\r\n",
	    nAcctNum, nTellerNum, nBranchNum, dDelta, dBalance);

      /* Commit the transaction based on success */
      /* Note: For stored procedure method this has no effect; */
      /* the commit happens in the sproc */
      if (lpBench->tpc.a.fUseCommit)
	{
	  rc = SQLTransact (SQL_NULL_HENV, lpBench->hdbc,
	      (UWORD) ((fRtn) ? SQL_COMMIT : SQL_ROLLBACK));
	  fRtn &= RC_SUCCESSFUL (rc);
	}

      /* Get the end time and the elapsed time */
      time (&tTransEndTime);
      dTransDiff = difftime (tTransEndTime, tTransStartTime);
      lpBench->tpc.a.dDiffSum += dTransDiff;

      /* Track 1-second and 2-second transactions */
      if (dTransDiff <= 1)
	++lpBench->tpc.a.nTrnCnt1Sec;
      if (dTransDiff > 1 && dTransDiff <= 2)
	++lpBench->tpc.a.nTrnCnt2Sec;

      /* Get elapsed time now to see whether we should quit */
      time (&tCurTime);
      dDiff = difftime (tCurTime, tStartTime);
      /* Every five transactions, see whether we need to cancel */
      if (0 ==
	  (lpBench->tpc.a.nTrnCnt % bench_get_long_pref (A_REFRESH_RATE)))
	{
	  char szBuff[50];
	  long secs_remain = (long int)(dTimeToRun - dDiff);
	  sprintf (szBuff, "%10ld txns, %10ld secs remaining",
	      lpBench->tpc.a.nTrnCnt, secs_remain);
	  if (lpBench->SetProgressText)
	    lpBench->SetProgressText (szBuff, lpBench->tpc._.nConn,
		lpBench->tpc._.nThreadNo, dDiff, 1, secs_remain,
		lpBench->tpc.a.dDiffSum);
	  if (lpBench->fCancel && lpBench->fCancel ())
	    {
	      fDone = TRUE;
	      fRtn = FALSE;
	      pane_log ("*** Cancelled ***\n");
	    }
	}
      continue;

    deadlock_main:
      if (bench_get_long_pref (LOCK_TIMEOUT) > 0
	  && lpBench->tpc.a.txn_isolation > SQL_TXN_READ_UNCOMMITTED)
	sleep_msecs (bench_get_long_pref (LOCK_TIMEOUT));
#ifndef WIN32
      if (!(fSuccess = fBindParameters (lpBench,
		  &nAcctNum, &nTellerNum, &nBranchNum, &dDelta, &dBalance,
		  hstmtUpdAcct, hstmtSelBal, hstmtUpdTeller, hstmtUpdBranch,
		  hstmtInsHist)))
	fDone = TRUE;
#endif
      time (&tCurTime);
      dDiff = difftime (tCurTime, tStartTime);
      if (dDiff <= dTimeToRun)
	goto txn_start_over;
    }				/* end main loop */
  fDone = TRUE;
  if (dDiff >= dTimeToRun)
    fRtn = TRUE;


  /* If we where not cancelled due to an error */
general_error:
  /* We're done, so dismiss the dialog */
  if (lpBench->StopProgress)
    lpBench->StopProgress ();

  if (fDone)
    pane_log ("\n\nBenchmark finished.\r\n");
  else
    pane_log ("\n\nBenchmark is ended with an error.\r\n");

  SQLFreeStmt (lpBench->hstmt, SQL_CLOSE);
  SQLFreeStmt (lpBench->hstmt, SQL_UNBIND);
  SQLFreeStmt (lpBench->hstmt, SQL_RESET_PARAMS);
  if (IDX_PARAMS == lpBench->tpc.a.fSQLOption)
    {
      if (hstmtUpdAcct != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtUpdAcct, SQL_DROP);
      if (hstmtSelBal != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtSelBal, SQL_DROP);
      if (hstmtUpdTeller != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtUpdTeller, SQL_DROP);
      if (hstmtUpdBranch != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtUpdBranch, SQL_DROP);
      if (hstmtInsHist != SQL_NULL_HSTMT)
        SQLFreeStmt (hstmtInsHist, SQL_DROP);
    }

  if (!fDone)
    fRtn = FALSE;

  return fRtn;
}


/***************************************************************************
 CalcStats - Determine the statistics for the run.

 Returns:  Nothing
***************************************************************************/

char *
txn_isolation_name (long txn_isolation, char *def)
{
  switch (txn_isolation)
    {
    case SQL_TXN_READ_UNCOMMITTED:
      return ("uncommitted");
    case SQL_TXN_READ_COMMITTED:
      return ("committed");
    case SQL_TXN_REPEATABLE_READ:
      return ("repeatable");
    case SQL_TXN_SERIALIZABLE:
      return ("serializable");
    default:
      return (def);
    }
}


long
txn_isolation_from_name (char *iso)
{
  if (!strcmp (iso, "uncommitted"))
    return (SQL_TXN_READ_UNCOMMITTED);
  else if (!strcmp (iso, "committed"))
    return (SQL_TXN_READ_COMMITTED);
  else if (!strcmp (iso, "repeatable"))
    return (SQL_TXN_REPEATABLE_READ);
  else if (!strcmp (iso, "serializable"))
    return (SQL_TXN_SERIALIZABLE);
  else
    return 0;
}


char *
cursor_type_name (long nCursorType, char *def)
{
  return
      (nCursorType == SQL_CURSOR_FORWARD_ONLY ? "ForwardOnly" :
      (nCursorType == SQL_CURSOR_STATIC ? "Static" :
	  (nCursorType == SQL_CURSOR_KEYSET_DRIVEN ? "Keyset" :
	      (nCursorType == SQL_CURSOR_DYNAMIC ? "Dynamic" :
		  (nCursorType == SQL_CURSOR_MIXED ? "Mixed" : def)))));
}


long
cursor_type_from_name (char *crsr)
{
  if (!strcmp (crsr, "ForwardOnly"))
    return (SQL_CURSOR_FORWARD_ONLY);
  else if (!strcmp (crsr, "Static"))
    return (SQL_CURSOR_STATIC);
  else if (!strcmp (crsr, "Keyset"))
    return (SQL_CURSOR_KEYSET_DRIVEN);
  else if (!strcmp (crsr, "Dynamic"))
    return (SQL_CURSOR_DYNAMIC);
  else if (!strcmp (crsr, "Mixed"))
    return (SQL_CURSOR_MIXED);
  else
    return 0;
}

void
CalcStats (test_t * lpBench,	/* Main stat information */
    long lTranCnt,		/* Transaction count */
    long lSubSecTranCnt,	/* Sub-second transaction count  */
    long lBetween,		/* Between 1 and 2 second transaction count */
    double dDiffSum		/* Processing time for all transactions */
    )
{
  char szBuf[254];
  char szOptions[100];

  /* Build a string with the execution options used */
  *szOptions = '\0';
  if (lpBench->tpc._.nThreads)
    {
      sprintf (szBuf, "%d Threads/", lpBench->tpc._.nThreads);
      strcat (szOptions, szBuf);
    }
  if (lpBench->tpc.a.nArrayParSize)
    {
      sprintf (szBuf, "%d ArrParSize/", lpBench->tpc.a.nArrayParSize);
      strcat (szOptions, szBuf);
    }
  if (lpBench->tpc.a.fExecAsync)
    strcat (szOptions, "Async/");
  if (lpBench->tpc.a.fUseCommit)
    strcat (szOptions, "Txn/");
  if (IDX_SPROCS == lpBench->tpc.a.fSQLOption)
    strcat (szOptions, "Sprocs/");
  else if (IDX_PARAMS == lpBench->tpc.a.fSQLOption)
    strcat (szOptions, "Params/");
  else
    strcat (szOptions, "SQLText/");
  if (lpBench->tpc.a.fDoQuery)
    {
      strcat (szOptions, "Query/");
      if (lpBench->tpc.a.nCursorType > 0 && lpBench->nCursorsSupported > 0 &&
	  lpBench->tpc.a.fSQLOption != IDX_SPROCS)
	{
	  sprintf (szBuf, "%s crsr (rowset %d)/",
	      cursor_type_name (lpBench->tpc.a.nCursorType, "Unknown"),
	      lpBench->tpc.a.nRowsetSize);
	  strcat (szOptions, szBuf);

	  if (lpBench->tpc.a.nTraversalCount > 1)
	    {
	      sprintf (szBuf, "%d Traversals",
		  lpBench->tpc.a.nTraversalCount);
	      strcat (szOptions, szBuf);
	    }
	}
    }

  if (lpBench->tpc.a.txn_isolation > 0)
    {
      sprintf (szBuf, "%s/", txn_isolation_name (lpBench->tpc.a.txn_isolation,
	      "Unknown"));
      strcat (szOptions, szBuf);
    }

  if (*szOptions)
    szOptions[strlen (szOptions) - 1] = '\0';

  /* Do the calculations */
  lpBench->tpc.a.dOverhead = (lpBench->tpc._.nMinutes * 60) - dDiffSum;
  lpBench->tpc.a.ftps = (float) (lTranCnt / dDiffSum);
  lpBench->tpc.a.fsub1 = ((float) lSubSecTranCnt / (float) lTranCnt) * 100;
  lpBench->tpc.a.fsub2 = ((float) lBetween / (float) lTranCnt) * 100;
  lpBench->tpc.a.fAvgTPTime = (float) (dDiffSum / lTranCnt);

  /* And print the results */
  pane_log ("Calculating statistics:\n");
  pane_log ("\tSQL options used:\t\t\t\t%s\n", szOptions);
  pane_log ("\tTransaction time:\t\t\t\t%f\n", dDiffSum);
  pane_log ("\tEnvironmental overhead:\t\t%f\n", lpBench->tpc.a.dOverhead);
  pane_log ("\tTotal transactions:\t\t\t\t%ld\n", lTranCnt);
  pane_log ("\tTransactions per second:\t\t%f\n", lpBench->tpc.a.ftps);
  pane_log ("\t%c less than 1 second:\t\t\t%f\n", '%', lpBench->tpc.a.fsub1);
  pane_log ("\t%c 1 < n < 2 seconds:\t\t\t%f\n", '%', lpBench->tpc.a.fsub2);
  pane_log ("\tAverage processing time:\t\t%f\n", lpBench->tpc.a.fAvgTPTime);
  if (lpBench->tpc.a.nArrayParSize > 1 
      && (!lpBench->fBatchSupported || !lpBench->fSQLBatchSupported))
    {
      if (!lpBench->fBatchSupported)
        {
          pane_log ("\t   The ODBC driver '%s' doesn't support the 'SQLParamOptions' call, \n",
		lpBench->szDriverName);
	  pane_log ("\t therefore the 'ArrayOptimization' was disabled\n");
	}
      else if (!lpBench->fSQLBatchSupported)
        {
          pane_log ("\t   The ODBC driver '%s' doesn't support batches for SELECT queries, \n",
		lpBench->szDriverName);
	  pane_log ("\t therefore the 'ArrayOptimization' for SELECT queries was disabled\n");
	}
    }

  do_add_results_record ("TPC-A", szOptions,
      henv, lpBench->hdbc, lpBench->hstmt,
      lpBench->szLoginDSN, lpBench->tpc.a.ftps, dDiffSum,
      lTranCnt, lpBench->tpc.a.fsub1, lpBench->tpc.a.fsub2,
      lpBench->tpc.a.fAvgTPTime, lpBench->szDriverName, lpBench->szDriverVer, lpBench->fHaveResults);
}


int
fCleanup (test_t * lpRunCfg	/* Run Configuration Parameters */
    )
{
  char szTempBuff[128];
  int i;
  static char szBranch[] = "BRANCH";
  static char szTeller[] = "TELLER";
  static char szAccount[] = "ACCOUNT";
  static char szHistory[] = "HISTORY";


  struct _TABLES
  {
    int *lpValue;
    char *szTable;
    char *szDSN;
  }
  Tables[4];

  Tables[0].lpValue = &lpRunCfg->tpc.a.fCreateBranch;
  Tables[0].szTable = szBranch;
  Tables[0].szDSN = lpRunCfg->tpc.a.szBranchDSN;

  Tables[1].lpValue = &lpRunCfg->tpc.a.fCreateTeller;
  Tables[1].szTable = szTeller;
  Tables[1].szDSN = lpRunCfg->tpc.a.szTellerDSN;

  Tables[2].lpValue = &lpRunCfg->tpc.a.fCreateAccount;
  Tables[2].szTable = szAccount;
  Tables[2].szDSN = lpRunCfg->tpc.a.szAccountDSN;

  Tables[3].lpValue = &lpRunCfg->tpc.a.fCreateHistory;
  Tables[3].szTable = szHistory;
  Tables[3].szDSN = lpRunCfg->tpc.a.szHistoryDSN;

  /* Drop the Tables */
  for (i = 0; i < NUMITEMS (Tables); i++)
    {
      if (TRUE == (*Tables[i].lpValue))
	{
	  pane_log ("Dropping table: %s\r\n", Tables[i].szTable);
	  sprintf (szTempBuff, "Drop Table %s", Tables[i].szTable);
	  fExecute (lpRunCfg, szTempBuff);

	  if (Tables[i].szDSN[0])
	    {
	      pane_log ("Dropping table: %s from the remote %s\r\n",
		  Tables[i].szTable, Tables[i].szDSN);
	      sprintf (szTempBuff, "rexecute ('%s', 'Drop Table %s')",
		  Tables[i].szDSN, Tables[i].szTable);
	      fExecute (lpRunCfg, szTempBuff);
	    }
	}
    }

  pane_log ("Dropping procedures");
  if (lpRunCfg->tpc.a.fCreateProcedure)
    fExecute (lpRunCfg, "Drop procedure ODBC_BENCHMARK");

  return TRUE;
}
