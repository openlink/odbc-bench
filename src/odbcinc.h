/*
 *  odbcinc.h
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
#ifdef WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int messages_off;
extern int quiet;
void print_error (SQLHENV e1, SQLHDBC e2, SQLHSTMT e3, void *test);

#define vShowErrors(dlg, henv, hdbc, hstmt, _test_) \
	print_error(henv, hdbc, hstmt, _test_)

#define ERRORS_OFF messages_off = 1
#define ERRORS_ON messages_off = 0
#define QUIET quiet = 1
#define QUIET_OFF quiet = 0

#define RC_SUCCESSFUL(rc) (!((rc)>>1))
#define RC_NOTSUCCESSFUL(rc) ((rc)>>1)


#define IS_ERR(stmt, foo, _test_) \
if (SQL_ERROR == foo) \
{ \
	print_error (stmt, stmt, stmt, _test_); \
}

#define IF_ERR(stmt, foo, _test_) \
if (SQL_ERROR == foo) \
{ \
	print_error (stmt, stmt, stmt, _test_); \
}

#define IF_ERR_GO(stmt, tag, foo, _test_) \
if (SQL_ERROR == (foo)) \
{ \
	print_error (stmt, stmt, stmt, _test_); \
	goto tag; \
}

#define SLEEP_COUNT	10000
#define DECLARE_FOR_SQLERROR \
SQLSMALLINT len; \
SQLCHAR state[10] = {""}; \
SQLCHAR message[256] = {""}

#define IF_DEADLOCK_OR_ERR_GO(stmt, tag, foo, deadlocktag) \
if (SQL_ERROR == (foo)) \
{ \
	RETCODE _rc = SQLError (SQL_NULL_HENV, SQL_NULL_HDBC, stmt, (SQLCHAR *) state, NULL, (SQLCHAR *) message, sizeof (message), &len); \
	if (_rc == SQL_SUCCESS && 0 == strncmp((char *) state, "40001", 5)) \
	  { \
	    goto deadlocktag; \
	  } \
	else \
	  { \
	    goto tag; \
	  } \
}

#define SAVE_SQL_ERROR(_stat, _msg) \
strncpy ((char *) (_stat), (char *) state, sizeof (_stat)); \
strncpy ((char *) (_msg), (char *) message, sizeof (_msg)); \
pane_log ("SQL Error [%s] : %s\n", _stat, _msg)


#define IF_DEADLOCK_OR_ERR_GO_WITH_ROLLBACK(stmt, tag, foo, deadlocktag) \
if (SQL_ERROR == (foo)) \
{ \
	RETCODE _rc = SQLError (stmt, stmt, stmt, (SQLCHAR *) state, NULL, (SQLCHAR *) message, sizeof (message), &len); \
	if (_rc == SQL_SUCCESS && 0 == strncmp((char *) state, "40001", 5)) \
	  { \
            if (do_rollback_on_deadlock && \
		(IS_C (*lpBench) || (IS_A (*lpBench) && lpBench->tpc.a.fUseCommit)) \
		&& strstr(lpBench->szDBMS, "Virtuoso")) \
	      SQLTransact(SQL_NULL_HENV, lpBench->hdbc, SQL_ROLLBACK); \
	    goto deadlocktag; \
	  } \
	else \
	  { \
	    strncpy((char *) lpBench->szSQLError, (char *) message, sizeof (lpBench->szSQLError) - 1); \
	    strncpy((char *) lpBench->szSQLState, (char *) state, 5); \
	    lpBench->szSQLError[255] = 0; \
	    lpBench->szSQLError[255] = 0; \
	    goto tag; \
	  } \
}


#define IF_CERR_GO(con, tag, foo, _test_) \
if (SQL_ERROR == (foo)) \
  { \
    print_error (0, con, 0, _test_); \
    goto tag; \
  }


#define IF_ERR_EXIT(stmt, foo, _test_) \
if (SQL_ERROR == foo) \
{ \
	print_error (stmt, stmt, stmt, _test_); \
	gtk_main_quit (); \
}

#define IF_ERR_RETURN(stmt, foo, rc, _test_) \
if (SQL_ERROR == foo) \
{ \
	print_error (stmt, stmt, stmt, _test_); \
	return (rc); \
}


#define BINDN(stmt, n, v) \
SQLSetParam (stmt, n, SQL_C_LONG, SQL_INTEGER, 0,0, &v, NULL);

#define IBINDNTS(stmt, n, nts) \
SQLSetParam (stmt, n, SQL_C_CHAR, SQL_CHAR, 0,0, nts, NULL)

#define IBINDNTS_NULL(stmt, n) \
SQLBindParameter (stmt, n, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0,0, NULL, 0, &null_val)

#define IBINDNTS_ARRAY(stmt, n, nts) \
SQLSetParam (stmt, n, SQL_C_CHAR, SQL_CHAR, sizeof (nts [0]), 0, nts, NULL)


#define IBINDSTR_ARRAY(stmt, n, nts, len_arr) \
SQLSetParam (stmt, n, SQL_C_CHAR, SQL_CHAR, sizeof (nts [0]),0, &nts, &len_arr)


#define IBINDOID(stmt, n, str, len_arr) \
SQLSetParam (stmt, n, SQL_C_OID, SQL_OID, sizeof (str [0]), 0, str, &len_arr)


#define IBINDL(stmt, n, l) \
SQLSetParam (stmt, n, SQL_C_LONG, SQL_INTEGER, 0,0, &l, NULL)

#define IBINDL_NULL(stmt, n) \
SQLBindParameter (stmt, n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0,0, NULL, 0, &null_val)

#define IBINDF(stmt, n, l) \
SQLSetParam (stmt, n, SQL_C_FLOAT, SQL_DOUBLE, 0, 0, &l, NULL)

#define IBINDF_NULL(stmt, n) \
SQLBindParameter (stmt, n, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_DOUBLE, 0,0, NULL, 0, &null_val)

#define IBINDD(stmt, n, l) \
SQLSetParam (stmt, n, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &l, NULL)

#define IBINDD_NULL(stmt, n) \
SQLBindParameter (stmt, n, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0,0, NULL, 0, &null_val)

#define OBINDFIX(stmt, n, buf, len) \
SQLBindCol (stmt, n, SQL_C_CHAR, buf, sizeof (buf), &len)

#define OBINDL(stmt, n, buf, len) \
SQLBindCol (stmt, n, SQL_C_LONG, &buf, sizeof (buf) , &len)


#define INIT_STMT(hdbc, st, text, _test_) \
	SQLAllocStmt (hdbc, &st); \
	IF_ERR_RETURN (st, SQLPrepare (st, (SQLCHAR *) text, SQL_NTS), 0, _test_);


#define HIST_READ(st) \
SQLSetStmtOption (st, SQL_CONCURRENCY, SQL_CONCUR_ROWVER)

#ifdef WIN32
#define SQLSetParam(stmt, ipar, ct, sqlt, prec, sc, ptr, len) \
SQLBindParameter (stmt, ipar, SQL_PARAM_INPUT, ct, sqlt, prec, sc, ptr, prec, len)
#endif

void do_logout (test_t * data);
int do_login (test_t * data);
void do_alloc_env (void);
void do_free_env (int isFreeMutex);

#ifdef __cplusplus
}
#endif
