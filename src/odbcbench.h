/*
 *  odbcbench.h
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "odbcinc.h"

#define NUMITEMS(p1) (sizeof(p1)/sizeof(p1[0]))
#define IDX_PLAINSQL 1011
#define	IDX_PARAMS   1012
#define	IDX_SPROCS   1013

/*
 * Reasonable defaults
 */
#ifndef HAVE_CONFIG_H
#define PACKAGE			"odbc-bench"
#define PACKAGE_BUGREPORT	"odbc-bench@openlinksw.com"
#define PACKAGE_NAME 		"OpenLink ODBC Benchmark Utility"
#define PACKAGE_STRING 		"OpenLink ODBC Benchmark Utility 0.99.1"
#define PACKAGE_TARNAME 	"odbc-bench"
#define PACKAGE_VERSION 	"0.99.1"
#define VERSION			"0.99.1"
#endif

/* status log handling routines - status.c */
void clear_status_handler (GtkWidget * widget, gpointer data);	/* clears the status text */
void do_pane_log (const char *format, ...);	/* adds a text line(s) to the status log */
extern void (*pane_log) (const char *format, ...);
void vBusy (void);
void create_status_widget (void);

#ifdef _DEBUG
#define assert(exp)	((exp) ? (void)0 : g_error("'%s', File %s, line %d", #exp, __FILE__, __LINE__))
#else
#define assert(exp)
#endif

typedef enum
{ DLG_OK, DLG_CANCEL, DLG_YES, DLG_NO }
answer_code;
/* dialog box functions - dialog.c */
GtkWidget *message_box_new (GtkWidget * Parent, const gchar * Text,
    const gchar * Title);	/* shows a dialog box with the specified attributes */
answer_code ok_cancel_dialog (const gchar * Text, const gchar * Title);
answer_code yes_no_cancel_dialog (const gchar * Text, const gchar * Title);
void login_dialog (GtkWidget * Parent, gpointer data);	/* show a login combo with a dsn_list as specified */
char *fill_file_name (char *szFileName, char *caption, int add_xmls);

/* progress implementations - dialog.c */
void do_ShowProgress (GtkWidget * parent, gchar * title,
    gboolean bForceSingle, float nMax);
void do_SetWorkingItem (char *pszWorking);
void do_SetProgressText (char *pszProgress, int nConn, int thread_no,
    float value);
void do_StopProgress (void);
int do_fCancel (void);
void do_ShowCancel (int fShow);
void do_RestartProgress (void);
void do_MarkFinished (int nConn, int nThread);

#if 0
void pipe_trough_isql (char *szFileName);
#else
void pipe_trough_isql (HSTMT stmt, char *szFileName, int print_commands);
#endif

extern HENV henv;

/* odbc callbacks - odbcs.c */
typedef enum
{ TPC_A, TPC_C }
testtype;

typedef struct test_common_s
{
  int nThreads;
  int nThreadNo;
  int nConn;

  int nMinutes;			/* Minutes to execute the test */
  int nRuns;			/* How many runs to do */
}
test_common_t;

#include "timeacct.h"
#include "tpca.h"
#include "tpcc.h"

typedef struct test_s
{
  char szName[128];
  testtype TestType;
  int is_dirty;

  GtkWidget *hwndOut;		/* Output window for logging info */

  /* ODBC handles */
  HDBC hdbc;			/* Connection handle */
  HSTMT hstmt;			/* Statement handle */

  /* login data */
  char szLoginDSN[50];
  char szLoginUID[50];
  char szLoginPWD[50];

  char szSQLState[10];
  char szSQLError[256];

  /* Other info */
  char szDSN[SQL_MAX_DSN_LENGTH];
  char szDBMS[128];
  char szDBMSVer[128];
  char szDriverName[128];
  char szDriverVer[128];
  char *pszDateTimeSQLFunc;	/* ODBC scalar function for current date/time */

  int fAsyncSupported;		/* TRUE if async mode is supported */
  int fCommitSupported;		/* TRUE if commitment control is supported */
  int fProcsSupported;		/* TRUE if sprocs are supported */
  unsigned long nCursorsSupported;	/* the bitmask of supported cursor types */
  unsigned long nIsolationsSupported;	/* the bitmask of supported isolation levels */
  long default_txn_isolation;	/* txn_isolation_mode */

  /* log functions */
  void (*ShowProgress) (GtkWidget * parent, gchar * title,
      gboolean bForceSingle, float nMax);
  void (*SetWorkingItem) (char *pszWorking);
  void (*SetProgressText) (char *pszProgress, int n_conn, int thread_no,
      float nValue);
  void (*StopProgress) (void);
  int (*fCancel) (void);

  int fHaveResults;		/* TRUE if sprocs are supported */
  union
  {
    struct tpca_s a;
    struct tpcc_s c;
    struct test_common_s _;
  }
  tpc;

  char szTemp[512];
  gboolean bTablePropsShown, bRunPropsShown;
}
test_t;

#define IS_A(test) ((test).TestType == TPC_A)
#define IS_C(test) ((test).TestType == TPC_C)

void get_dsn_data (test_t * ptest);
/* from prefs.c */
typedef enum
{
  A_REFRESH_RATE,
  C_REFRESH_RATE,
  LOCK_TIMEOUT,
  DISPLAY_REFRESH_RATE
}
OdbcBenchPref;

long bench_get_long_pref (OdbcBenchPref pref);
int bench_set_long_pref (OdbcBenchPref pref, long value);
char *bench_get_string_pref (OdbcBenchPref pref);
void set_display_refresh_rate (GtkWidget * widget, gpointer data);
void set_lock_timeout (GtkWidget * widget, gpointer data);
void do_flip (GtkWidget * widget, gboolean * data);

int _strnicmp (const char *s1, const char *s2, size_t n);
char *_stristr (const char *str, const char *find);

BOOL fSQLExecDirect (HSTMT hstmt, char *pszSqlStr, test_t * lpBench);
BOOL fSQLGetData (HSTMT hstmt, UWORD icol, SWORD fCType, PTR rgbValue,
    SDWORD cbValueMax, SDWORD FAR * pcbValue);
BOOL fSQLBindCol (HSTMT hstmt, UWORD icol, SWORD fCType, PTR rgbValue,
    SDWORD cbValueMax, SDWORD FAR * pcbValue);

#if defined(WIN32)
#define sleep_msecs(x) Sleep(x)
#else
#include <sys/poll.h>
#define sleep_msecs(x) poll (NULL, 0, x)
#endif
