/*
 *  odbcs.c
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
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>

#include "odbcbench.h"
#include "LoginBox.h"
#include "thr.h"

SDWORD sql_nts = SQL_NTS, long_len;
int messages_off = 0;
int do_rollback_on_deadlock = 1;
int quiet = 0;

HENV henv = SQL_NULL_HENV;

static MUTEX_T env_mutex;

void
do_free_env (GtkWidget * widget, gpointer data)
{
  if (widget)
    {
      MUTEX_FREE (log_mutex);
      MUTEX_FREE (env_mutex);
    }

  if (!henv)
    return;

  SQLFreeEnv (henv);
  henv = (HENV) 0;
}

void
do_alloc_env (GtkWidget * widget, gpointer data)
{
  RETCODE rc;
  char szError[256], szState[10];

  do_free_env (NULL, NULL);

  MUTEX_INIT (log_mutex);
  MUTEX_INIT (env_mutex);

  rc = SQLAllocEnv (&henv);
  if (rc == SQL_ERROR)
    {
      szError[0] = szState[0] = 0;
      SQLError (henv, 0, 0, szState, NULL, szError, sizeof (szError), NULL);
      g_error ("do_alloc_env : STATE : %s - %s", szState, szError);
      gtk_main_quit ();
    }
}

void
do_login (GtkWidget * widget, gpointer data)
{
  LoginBox *login_box = NULL;
  RETCODE rc;
  char szBuff[256];
  test_t *ptest = (test_t *) data;
  char *szDSN, *szUID, *szPWD;
  HSTMT hstmt;

  if (widget)
    {
      login_box = LOGINBOX (widget);
      strcpy (ptest->szLoginDSN, (szDSN = login_box->szDSN));
      strcpy (ptest->szLoginUID, (szUID = login_box->szUID));
      strcpy (ptest->szLoginPWD, (szPWD = login_box->szPWD));
    }
  else
    {
      szDSN = ptest->szLoginDSN;
      szUID = ptest->szLoginUID;
      szPWD = ptest->szLoginPWD;
    }

  if (!henv)
    {
      g_error ("[Bench] do_login : Environment not allocated");
      gtk_main_quit ();
    }
  do_logout (widget, ptest);

  MUTEX_ENTER (env_mutex);
  rc = SQLAllocConnect (henv, &ptest->hdbc);
  if (rc == SQL_SUCCESS)
    rc =
	SQLConnect (ptest->hdbc, szDSN, SQL_NTS, szUID, SQL_NTS, szPWD,
	SQL_NTS);
  MUTEX_LEAVE (env_mutex);

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

  if (IS_A (*ptest))
    {
      UDWORD  irow;
      
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
              UDWORD param;

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
      else
	ptest->tpc.a.txn_isolation = 0;
    }


  /* Set title of application with DSN for user */
  if (login_box)
    {
      sprintf (szBuff, "%s - %s", PACKAGE_STRING, ptest->szDSN);
    }

  /* Determine whether asynchronous processing is supported */
  /* by setting the value to its default, which has no */
  /* effect other than to tell us whether it is supported */

  rc = SQLSetStmtOption (ptest->hstmt, SQL_ASYNC_ENABLE,
      SQL_ASYNC_ENABLE_OFF);
  ptest->fAsyncSupported = RC_SUCCESSFUL (rc);


  /* Set defaults for execution--use all features */
  /* supported */
/*  ptest->fClearHistory = FALSE;
  ptest->fExecAsync = ptest->fAsyncSupported;
  ptest->fUseCommit = ptest->fCommitSupported;
  ptest->fSQLOption =
      ptest->fProcsSupported ? IDX_SPROCS : IDX_PLAINSQL;
  ptest->fDoQuery = FALSE;
*/
  if (login_box)
    do_pane_log ("Connected to %s : DSN=<%s> UID=<%s> PWD=<%s>\n",
	ptest->szName, szDSN, szUID, szPWD);
/*  g_message ("[Bench] do_login : connected to : DSN=<%s> UID=<%s> PWD=<%s>", szDSN, szUID, szPWD); */

  return;
done:
  if (ptest->hdbc)
    {
      SQLFreeConnect (ptest->hdbc);
      ptest->hdbc = 0;
    }
}


void
get_dsn_data (test_t * ptest)
{
  long ulDTFunc;
  SWORD sTxnCapable;
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
	  "ODBC_BENCHMARK", SQL_NTS);
      if (SQL_SUCCESS != rc || SQL_SUCCESS != SQLFetch (ptest->hstmt))
	*szBuff = 'N';
      SQLFreeStmt (ptest->hstmt, SQL_CLOSE);
    }
  ptest->fProcsSupported = (*szBuff == 'Y');


  *szBuff = 'Y';
  if ('Y' == *szBuff)
    {
      rc = SQLTables (ptest->hstmt, NULL, 0, NULL, 0,
	  "RESULTS", SQL_NTS, "TABLE", SQL_NTS);
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
do_logout (GtkWidget * widget, gpointer data)
{
  test_t *ptest = (test_t *) data;

  if (!ptest->hstmt)
    return;

  SQLFreeStmt (ptest->hstmt, SQL_DROP);
  ptest->hstmt = (HSTMT) 0;

  if (!ptest->hdbc)
    return;

  MUTEX_ENTER (env_mutex);
  SQLDisconnect (ptest->hdbc);
  SQLFreeConnect (ptest->hdbc);
  ptest->hdbc = (HDBC) 0;
  MUTEX_LEAVE (env_mutex);

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
  if (widget)
    do_pane_log ("Connection to %s closed\n", ptest->szName);
}

GList *
get_dsn_list (void)
{
  char *szDSN = (char *) g_malloc (256);
  GList *list = NULL;
  SWORD nDSN;

  if (SQL_SUCCESS == SQLDataSources (henv, SQL_FETCH_FIRST, (UCHAR *) szDSN,
	  255, &nDSN, NULL, 0, NULL))
    do
      {
	if (nDSN > 0 && nDSN < 255)
	  szDSN[nDSN] = 0;
	list = g_list_insert_sorted (list, szDSN, (GCompareFunc) strcmp);
	szDSN = (char *) g_malloc (256);
      }
    while (SQL_SUCCESS == SQLDataSources (henv, SQL_FETCH_NEXT,
	    (UCHAR *) szDSN, 255, &nDSN, NULL, 0, NULL));
  return (list);
}

void
print_error (HENV env, HDBC dbc, HSTMT stmt, void *_test)
{
  test_t *test = (test_t *)_test;
  char szMessage[256], szState[10];
  RETCODE rc;

  szMessage[0] = szState[0] = 0;

  while (1)
    {
      rc =
	  SQLError (env, dbc, stmt, szState, NULL, szMessage,
	  sizeof (szMessage), NULL);
      if (rc != SQL_SUCCESS)
	break;
      pane_log ("SQL Error [%s] : %s\n", szState, szMessage);
      if (!messages_off)
	g_warning ("SQL Error [%s] : %s", szState, szMessage);
      if (test)
	{
	  strncpy (test->szSQLError, szMessage, sizeof (test->szSQLError));
	  strncpy (test->szSQLState, szState, sizeof (test->szSQLState));
	}
    }
}

/*-------------------------------------------------------------------------*/
/* fSQLParamOption 
/*                                                                                                                                                 */
/* Returns:  TRUE if function was successful, FALSE if there was an error  */
/*-------------------------------------------------------------------------*/
BOOL
fSQLParamOptions (HSTMT hstmt,
UDWORD crow,
UDWORD *pirow)
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
fSQLBindParameter (HSTMT hstmt,
UWORD ipar,
SWORD fParamType,
SWORD fCType,
SWORD fSqlType,
UDWORD cbColDef, SWORD ibScale, PTR rgbValue, SDWORD cbValueMax, SDWORD FAR * pcbValue)
{
  RETCODE rc;

  rc = SQLBindParameter (hstmt, ipar, fParamType,
      fCType, fSqlType, cbColDef, ibScale, rgbValue, cbValueMax, pcbValue);
  if (SQL_ERROR == rc && fSqlType >= SQL_INTEGER && fSqlType <= SQL_DOUBLE)
    {
      char szSQLSTATE[6];
      SDWORD lErr;
      char msg[20];
      SWORD cbmsg;

      *szSQLSTATE = '\0';
      SQLError (0, 0, hstmt, szSQLSTATE, &lErr, msg, sizeof (msg), &cbmsg);
      if (!strcmp (szSQLSTATE, "S1C00"))
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
fSQLGetData (HSTMT hstmt,	/* Statement handle */
    UWORD icol,			/* Column number */
    SWORD fCType,		/* C data type */
    PTR rgbValue,		/* Buffer to bind to */
    SDWORD cbValueMax,		/* Maximum size of rgbValue */
    SDWORD FAR * pcbValue	/* Available bytes to return */
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
fSQLExecDirect (HSTMT hstmt,	/* Statement handle */
    char *pszSqlStr,		/* SQL String to execute */
    test_t * lpBench)
{
  DECLARE_FOR_SQLERROR;
  RETCODE rc = SQL_ERROR;
deadlock_execdir:
  if (lpBench->fCancel && lpBench->fCancel ())
    goto error_execdir;
  rc = SQLExecDirect (hstmt, (UCHAR FAR *) pszSqlStr, SQL_NTS);
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
fSQLBindCol (HSTMT hstmt,	/* Statement handle */
    UWORD icol,			/* Column number */
    SWORD fCType,		/* C data type */
    PTR rgbValue,		/* Buffer to bind to */
    SDWORD cbValueMax,		/* Maximum size of rgbValue */
    SDWORD FAR * pcbValue	/* Available bytes to return */
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
  int len;
  const char *cp;
  const char *ep;

  len = strlen (find);
  ep = str + strlen (str) - len;
  for (cp = str; cp <= ep; cp++)
    if (toupper (*cp) == toupper (*find) && !_strnicmp (cp, find, len))
      return (char *) cp;

  return NULL;
}
