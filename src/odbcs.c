/*
 *  odbcs.c
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
#include <string.h>
#include <ctype.h>
#include "odbcbench.h"
#include "thr.h"

int messages_off = 0;
int do_rollback_on_deadlock = 1;
int quiet = 0;

SQLHENV henv = SQL_NULL_HENV;

static MUTEX_T env_mutex;
MUTEX_T log_mutex;

void
do_free_env (int isFreeMutex)
{
  if (isFreeMutex)
    {
      MUTEX_FREE (log_mutex);
      MUTEX_FREE (env_mutex);
    }

  if (!henv)
    return;

  SQLFreeEnv (henv);
  henv = SQL_NULL_HENV;
}

void
do_alloc_env ()
{
  RETCODE rc;
  SQLCHAR szError[256], szState[10];

  do_free_env (0);

  MUTEX_INIT (log_mutex);
  MUTEX_INIT (env_mutex);

  rc = SQLAllocEnv (&henv);
  if (rc == SQL_ERROR)
    {
      szError[0] = szState[0] = 0;
      SQLError (henv, SQL_NULL_HDBC, SQL_NULL_HSTMT, szState, NULL, szError, sizeof (szError), NULL);
      if (gui.err_message)
        gui.err_message("do_alloc_env : STATE : %s - %s\r\n", szState, szError);
      if (gui.main_quit)
        gui.main_quit ();
      else
        exit(-1);
    }
}

int
do_login (test_t *ptest)
{
  RETCODE rc;
  char *szDSN, *szUID, *szPWD;
  SQLHSTMT hstmt;

  szDSN = ptest->szLoginDSN;
  szUID = ptest->szLoginUID;
  szPWD = ptest->szLoginPWD;

  if (!henv)
    {
      if (gui.err_message)
        gui.err_message("[Bench] do_login : Environment not allocated\r\n");
      if (gui.main_quit)
        gui.main_quit ();
      else
        exit(-1);
    }
  do_logout (ptest);

  pane_log ("\r\n\r\nConnecting to %s : DSN=<%s> UID=<%s>\r\n",
	ptest->szName, szDSN, szUID);

  rc = SQLAllocConnect (henv, &ptest->hdbc);
  if (rc == SQL_SUCCESS)
    rc =
	SQLConnect (ptest->hdbc, (SQLCHAR *) szDSN, SQL_NTS, (SQLCHAR *) szUID, SQL_NTS, (SQLCHAR *) szPWD,
	SQL_NTS);

  ptest->szSQLError[0] = 0;
  ptest->szSQLState[0] = 0;
  if (rc == SQL_ERROR)
    {
      SQLError (henv, ptest->hdbc, SQL_NULL_HSTMT, ptest->szSQLState, NULL,
	  ptest->szSQLError, sizeof (ptest->szSQLError), NULL);
      goto done;
    }

  rc = SQLAllocStmt (ptest->hdbc, &ptest->hstmt);
  if (rc == SQL_ERROR)
    {
      SQLError (henv, ptest->hdbc, SQL_NULL_HSTMT, ptest->szSQLState, NULL,
	  ptest->szSQLError, sizeof (ptest->szSQLError), NULL);
      goto done;
    }

  ptest->fBatchSupported = FALSE;
  ptest->fSQLBatchSupported = FALSE;

  SQLGetInfo (ptest->hdbc,
      SQL_DRIVER_NAME,
      ptest->szDriverName, sizeof (ptest->szDriverName), NULL);
  SQLGetInfo (ptest->hdbc,
      SQL_DRIVER_VER, ptest->szDriverVer, sizeof (ptest->szDriverVer), NULL);

  if (IS_A (*ptest))
    {
      SQLULEN irow;
      
      rc = SQLAllocStmt (ptest->hdbc, &hstmt);
      if (rc == SQL_ERROR)
        {
          SQLError (henv, ptest->hdbc, SQL_NULL_HSTMT, ptest->szSQLState, NULL,
	      ptest->szSQLError, sizeof (ptest->szSQLError), NULL);
          goto done;
        }

      rc = SQLParamOptions(hstmt, 10, &irow);
      ptest->fBatchSupported = RC_SUCCESSFUL (rc);
      SQLFreeStmt (hstmt, SQL_DROP);

      if (ptest->fBatchSupported)
        {
          char szBuff[10];
          rc = SQLGetInfo (ptest->hdbc,
                 SQL_DRIVER_ODBC_VER, szBuff, sizeof (szBuff), NULL);
          if (RC_SUCCESSFUL (rc))
            {
              SQLINTEGER param;

              szBuff[2] = 0;
              if (atoi(szBuff) >= 3);
                {
                  rc = SQLGetInfo (ptest->hdbc, SQL_PARAM_ARRAY_SELECTS,
                         &param, sizeof (param), NULL);
                  if (RC_SUCCESSFUL (rc) && param == SQL_PAS_BATCH )
                     ptest->fSQLBatchSupported = 1;
                }
            }
        }

      if (ptest->nIsolationsSupported > 0 && ptest->tpc.a.txn_isolation > 0)
	{
	  rc = SQLSetConnectOption (ptest->hdbc, SQL_TXN_ISOLATION,
	      ptest->tpc.a.txn_isolation);
	  if (!RC_SUCCESSFUL (rc))
	    ptest->tpc.a.txn_isolation = 0;
	}
    }


  /* Set title of application with DSN for user */
/**
  if (login_box)
    {
      sprintf (szBuff, "%s - %s", PACKAGE_STRING, ptest->szDSN);
    }
**/

  /* Determine whether asynchronous processing is supported */
  /* by setting the value to its default, which has no */
  /* effect other than to tell us whether it is supported */

  rc = SQLSetStmtOption (ptest->hstmt, SQL_ASYNC_ENABLE,
      SQL_ASYNC_ENABLE_OFF);
  ptest->fAsyncSupported = RC_SUCCESSFUL (rc);


  /* Set defaults for execution--use all features */
  /* supported */
  pane_log ("Driver : %s (%s)\r\n", ptest->szDriverVer, ptest->szDriverName);
  pane_log ("Connection to %s opened\r\n", ptest->szName);
  pane_log ("\r\n");

  return TRUE;
done:
  if (ptest->hdbc)
    {
      SQLFreeConnect (ptest->hdbc);
      ptest->hdbc = 0;
    }
  return FALSE;
}


void
get_dsn_data (test_t * ptest)
{
  long ulDTFunc;
  SQLSMALLINT sTxnCapable;
  char szBuff[256];
  RETCODE rc;

  SQLGetInfo (ptest->hdbc,
      SQL_DATA_SOURCE_NAME, ptest->szDSN, sizeof (ptest->szDSN), NULL);
  SQLGetInfo (ptest->hdbc,
      SQL_DBMS_NAME, ptest->szDBMS, sizeof (ptest->szDBMS), NULL);
  SQLGetInfo (ptest->hdbc,
      SQL_DBMS_VER, ptest->szDBMSVer, sizeof (ptest->szDBMS), NULL);
  SQLGetInfo (ptest->hdbc,
      SQL_DRIVER_NAME,
      ptest->szDriverName, sizeof (ptest->szDriverName), NULL);
  SQLGetInfo (ptest->hdbc,
      SQL_DRIVER_VER, ptest->szDriverVer, sizeof (ptest->szDriverVer), NULL);
  SQLGetInfo (ptest->hdbc, SQL_PROCEDURES, szBuff, sizeof (szBuff), NULL);

  if ('Y' == *szBuff)
    {
      rc = SQLProcedures (ptest->hstmt, NULL, 0, NULL, 0,
	  (SQLCHAR *) "ODBC_BENCHMARK", SQL_NTS);
      if (SQL_SUCCESS != rc || SQL_SUCCESS != SQLFetch (ptest->hstmt))
	*szBuff = 'N';
      SQLFreeStmt (ptest->hstmt, SQL_CLOSE);
    }
  ptest->fProcsSupported = (*szBuff == 'Y');


  *szBuff = 'Y';
  if ('Y' == *szBuff)
    {
      rc = SQLTables (ptest->hstmt, NULL, 0, NULL, 0,
	  (SQLCHAR *) "RESULTS", SQL_NTS, (SQLCHAR *) "TABLE", SQL_NTS);
      if (SQL_SUCCESS != rc || SQL_SUCCESS != SQLFetch (ptest->hstmt))
	*szBuff = 'N';
      SQLFreeStmt (ptest->hstmt, SQL_CLOSE);
    }
  ptest->fHaveResults = (*szBuff == 'Y');

  SQLGetInfo (ptest->hdbc,
      SQL_TXN_CAPABLE, &sTxnCapable, sizeof (sTxnCapable), NULL);
  ptest->fCommitSupported = sTxnCapable > 0;

  SQLGetInfo (ptest->hdbc,
      SQL_TIMEDATE_FUNCTIONS, &ulDTFunc, sizeof (ulDTFunc), NULL);
  if (ulDTFunc & SQL_FN_TD_NOW)
    ptest->pszDateTimeSQLFunc = "now";
  else if (ulDTFunc & SQL_FN_TD_CURDATE)
    ptest->pszDateTimeSQLFunc = "curdate";
  else
    ptest->pszDateTimeSQLFunc = NULL;

  rc = SQLGetInfo (ptest->hdbc,
      SQL_TXN_ISOLATION_OPTION,
      &ptest->nIsolationsSupported,
      sizeof (ptest->nIsolationsSupported), NULL);
  if (!RC_SUCCESSFUL (rc))
    ptest->nIsolationsSupported = 0;

  rc = SQLGetInfo (ptest->hdbc,
      SQL_SCROLL_OPTIONS,
      &ptest->nCursorsSupported, sizeof (ptest->nCursorsSupported), NULL);
  if (!RC_SUCCESSFUL (rc))
    ptest->nCursorsSupported = 0;

  rc = SQLGetInfo (ptest->hdbc,
      SQL_DEFAULT_TXN_ISOLATION,
      &ptest->default_txn_isolation,
      sizeof (ptest->default_txn_isolation), NULL);
}

void
do_logout (test_t *ptest)
{
  if (!ptest->hstmt)
    return;

  SQLFreeStmt (ptest->hstmt, SQL_DROP);
  ptest->hstmt = SQL_NULL_HSTMT;

  if (!ptest->hdbc)
    return;

  SQLDisconnect (ptest->hdbc);
  SQLFreeConnect (ptest->hdbc);
  ptest->hdbc = SQL_NULL_HDBC;

  if (IS_C (*ptest))
    {
      ptest->tpc.c.misc_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.new_order_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.payment_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.delivery_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.slevel_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.ostat_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.item_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.ware_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.stock_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.dist_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.cs_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.h_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.o_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.no_stmt = SQL_NULL_HSTMT;
      ptest->tpc.c.ol_stmt = SQL_NULL_HSTMT;
    }
  pane_log ("Connection to %s closed\r\n", ptest->szName);
}

void
print_error (SQLHENV env, SQLHDBC dbc, SQLHSTMT stmt, void *_test)
{
  test_t *test = (test_t *)_test;
  SQLCHAR szMessage[256], szState[10];
  RETCODE rc;

  szMessage[0] = szState[0] = 0;

  while (1)
    {
      rc =
	  SQLError (env, dbc, stmt, szState, NULL, szMessage,
	  sizeof (szMessage), NULL);
      if (rc != SQL_SUCCESS)
	break;
      pane_log ("SQL Error [%s] : %s\r\n", szState, szMessage);
      if (!messages_off && gui.warn_message)
	gui.warn_message ("SQL Error [%s] : %s\r\n", szState, szMessage);
      if (test)
	{
	  strncpy ((char *) test->szSQLError, (char *) szMessage, sizeof (test->szSQLError));
	  strncpy ((char *) test->szSQLState, (char *) szState, sizeof (test->szSQLState));
	}
    }
}

/*-------------------------------------------------------------------------*/
/* fSQLParamOption 							   */
/* Returns:  TRUE if function was successful, FALSE if there was an error  */
/*-------------------------------------------------------------------------*/
BOOL
fSQLParamOptions (SQLHSTMT hstmt,
SQLULEN crow,
SQLULEN *pirow)
{
  RETCODE rc;

  rc = SQLParamOptions (hstmt, crow, pirow);
  if (SQL_SUCCESS != rc)
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  return (RC_SUCCESSFUL (rc));
}

/*-------------------------------------------------------------------------*/
/* fSQLBindParameter - Binds a parameter for input or output                       */
/*                                                                                                                                                 */
/* Returns:  TRUE if function was successful, FALSE if there was an error  */
/*-------------------------------------------------------------------------*/
BOOL
fSQLBindParameter (
  SQLHSTMT	hstmt,
  SQLUSMALLINT	ipar,
  SQLSMALLINT	fParamType,
  SQLSMALLINT	fCType,
  SQLSMALLINT	fSqlType,
  SQLULEN	cbColDef,
  SQLSMALLINT	ibScale,
  SQLPOINTER	rgbValue,
  SQLLEN	cbValueMax,
  SQLLEN *	pcbValue)
{
  RETCODE rc;

  rc = SQLBindParameter (hstmt, ipar, fParamType,
      fCType, fSqlType, cbColDef, ibScale, rgbValue, cbValueMax, pcbValue);
  if (SQL_ERROR == rc && fSqlType >= SQL_INTEGER && fSqlType <= SQL_DOUBLE)
    {
      SQLCHAR szSQLSTATE[6];
      SQLINTEGER lErr;
      SQLCHAR msg[20];
      SQLSMALLINT cbmsg;

      *szSQLSTATE = '\0';
      SQLError (0, 0, hstmt, szSQLSTATE, &lErr, msg, sizeof (msg), &cbmsg);
      if (!strcmp ((char *) szSQLSTATE, "S1C00"))
	rc = SQLBindParameter (hstmt, ipar, fParamType,
	    fCType, SQL_NUMERIC, 15, ibScale, rgbValue, cbValueMax, pcbValue);
    }
  if (SQL_SUCCESS != rc)
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  return (RC_SUCCESSFUL (rc));
}

/***************************************************************************
 fSQLGetData - Invoke SQLGetData, check return info, show errors if required

 Returns:  TRUE if function was successful, FALSE if there was an error
***************************************************************************/
BOOL
fSQLGetData (
  SQLHSTMT	hstmt,		/* Statement handle */
  SQLUSMALLINT	icol,		/* Column number */
  SQLSMALLINT	fCType,		/* C data type */
  SQLPOINTER	rgbValue,	/* Buffer to bind to */
  SQLLEN	cbValueMax,	/* Maximum size of rgbValue */
  SQLLEN *	pcbValue	/* Available bytes to return */
  )
{
  RETCODE rc;

  rc = SQLGetData (hstmt, icol, fCType, rgbValue, cbValueMax, pcbValue);
  if (SQL_SUCCESS != rc)
    vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  return (RC_SUCCESSFUL (rc));
}

/***************************************************************************
 fSQLExecDirect - Executes a statement with error checking.

 Returns:  TRUE if function was successful, FALSE if there was an error
***************************************************************************/
BOOL
fSQLExecDirect (
  SQLHSTMT	hstmt,		/* Statement handle */
  SQLCHAR *	pszSqlStr,	/* SQL String to execute */
  test_t *	lpBench)
{
  DECLARE_FOR_SQLERROR;
  RETCODE rc = SQL_ERROR;
deadlock_execdir:
  if (lpBench->fCancel && lpBench->fCancel ())
    goto error_execdir;
  rc = SQLExecDirect (hstmt, pszSqlStr, SQL_NTS);
  IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK (hstmt, error_execdir, rc,
      deadlock_execdir);

error_execdir:
  return (RC_SUCCESSFUL (rc));
}

/***************************************************************************
 fSQLBindCol - Invoke SQLBindCol, check return info, show errors if required

 Returns:  TRUE if function was successful, FALSE if there was an error
***************************************************************************/
BOOL
fSQLBindCol (
  SQLHSTMT	hstmt,		/* Statement handle */
  SQLUSMALLINT	icol,		/* Column number */
  SQLSMALLINT	fCType,		/* C data type */
  SQLPOINTER	rgbValue,	/* Buffer to bind to */
  SQLLEN	cbValueMax,	/* Maximum size of rgbValue */
  SQLLEN *	pcbValue	/* Available bytes to return */
  )
{
  RETCODE rc;

  /* SDK 2.10 forces a non-null pcbValue for retrieving data */
  assert (NULL != pcbValue);

  /* Call the function and show errors only if user wants to */
  rc = SQLBindCol (hstmt, icol, fCType, rgbValue, cbValueMax, pcbValue);
  if (SQL_SUCCESS != rc)
    vShowErrors (hwnd, SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, NULL);

  return (RC_SUCCESSFUL (rc));
}


int
_strnicmp (const char *s1, const char *s2, size_t n)
{
  int cmp;

  while (*s1 && n)
    {
      n--;
      if ((cmp = toupper (*s1) - toupper (*s2)) != 0)
	return cmp;
      s1++;
      s2++;
    }
  if (n)
    return (*s2) ? -1 : 0;
  return 0;
}

char *
_stristr (const char *str, const char *find)
{
  size_t len;
  const char *cp;
  const char *ep;

  len = strlen (find);
  ep = str + strlen (str) - len;
  for (cp = str; cp <= ep; cp++)
    if (toupper (*cp) == toupper (*find) && !_strnicmp (cp, find, len))
      return (char *) cp;

  return NULL;
}
