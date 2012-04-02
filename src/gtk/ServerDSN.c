/*
 *  ServerDSN.c
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2012 OpenLink Software <odbc-bench@openlinksw.com>
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
#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "odbcbench.h"
#include "odbcbench_gtk.h"
#include "ServerDSN.h"

static void ServerDSN_class_init (ServerDSNClass * sclass);
static void ServerDSN_init (ServerDSN * tableloader);


int
ServerDSN_get_type (void)
{
  static guint tld_type = 0;

  if (!tld_type)
    {
      GtkTypeInfo tld_info = {
	"ServerDSN",
	sizeof (ServerDSN),
	sizeof (ServerDSNClass),
	(GtkClassInitFunc) ServerDSN_class_init,
	(GtkObjectInitFunc) ServerDSN_init,
	NULL,
	NULL
      };

      tld_type = gtk_type_unique (gtk_dialog_get_type (), &tld_info);
    }

  return tld_type;
}


enum
{
  DO_THE_WORK_SIGNAL,
  DSNS_CHANGED,
  LAST_SIGNAL
};

static guint ServerDSN_signals[LAST_SIGNAL] = { 0 };


static void
ServerDSN_class_init (ServerDSNClass * sclass)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass *) sclass;

  ServerDSN_signals[DO_THE_WORK_SIGNAL] = gtk_signal_new ("do_the_work",
      GTK_RUN_FIRST,
      object_class->type,
      GTK_SIGNAL_OFFSET (ServerDSNClass, ServerDSN_do_the_work),
      gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);

  ServerDSN_signals[DSNS_CHANGED] = gtk_signal_new ("dsns_changed",
      GTK_RUN_FIRST,
      object_class->type,
      GTK_SIGNAL_OFFSET (ServerDSNClass, ServerDSN_do_the_work),
      gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);

  gtk_object_class_add_signals (object_class, ServerDSN_signals, LAST_SIGNAL);

  sclass->ServerDSN_do_the_work = NULL;
  sclass->ServerDSN_dsns_changed = NULL;
}


#define free_dsns_list(list, type, prefix) \
{ \
  type *iter = list; \
  while (iter) \
    { \
      g_free (iter->data); \
      iter = prefix##_next (iter); \
    } \
  list = NULL; \
}


static long
fill_dsns_list (ServerDSN * dlg)
{
  SQLCHAR szDSNName[50], szDBMS[50];
  SQLLEN len1 = SQL_NTS, len2 = SQL_NTS;
  char *szNewDSN, *szNewDBMS;
  long ret = 0;
  test_t *lpBench = dlg->lpBench;
  dsn_info_t *dsn_info = &dlg->dsn_info;

  free_dsns_list (dsn_info->dsns, GList, g_list);
  free_dsns_list (dsn_info->names, GList, g_list);

  if (_stristr (lpBench->szDBMS, "Virtuoso"))
    {
      ret = 1;
      if (SQL_SUCCESS !=
	  SQLPrepare (lpBench->hstmt,
	      (SQLCHAR *)
	      "select DS_DSN, get_keyword (17, deserialize (DS_CONN_STR), 'ANSI') from DB.DBA.SYS_DATA_SOURCE",
	      SQL_NTS))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}

      if (SQL_SUCCESS !=
	  SQLBindCol (lpBench->hstmt,
	      1, SQL_C_CHAR, szDSNName, sizeof (szDSNName), &len1))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}
      if (SQL_SUCCESS !=
	  SQLBindCol (lpBench->hstmt,
	      2, SQL_C_CHAR, szDBMS, sizeof (szDBMS), &len2))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}

      if (SQL_SUCCESS != SQLExecute (lpBench->hstmt))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}

      while (SQL_SUCCESS == SQLFetch (lpBench->hstmt))
	{
	  int nPos;
	  szNewDSN = (char *) g_malloc (len1 + 1);
	  szNewDBMS = (char *) g_malloc (len2 + 1);
	  memcpy (szNewDSN, szDSNName, len1);
	  szNewDSN[len1] = 0;
	  dsn_info->dsns =
	      g_list_insert_sorted (dsn_info->dsns, szNewDSN,
	      (GCompareFunc) strcmp);
	  nPos = g_list_index (dsn_info->dsns, szNewDSN);
	  memcpy (szNewDBMS, szDBMS, len2);
	  szNewDBMS[len2] = 0;
	  dsn_info->names = g_list_insert (dsn_info->names, szNewDBMS, nPos);
	}
    }
error:
  gtk_signal_emit_by_name (GTK_OBJECT (dlg), "dsns_changed");
  return ret;
}


static SQLCHAR dsn_proc[] =
    "create procedure gtkbench_sql_data_sources(in n integer) { \n"
    "  declare v1,v2,n1 integer; \n"
    "  declare n1 integer; \n"
    "  declare dsn_name, dsn_desc varchar; \n"
    "  \n"
    "  v1 := sql_data_sources(); \n"
    "  result_names(dsn_name, dsn_desc); \n"
    "  n1 := 0; \n"
    "  while (n1 < length(v1))  \n"
    "    { \n"
    "      v2 := aref(v1, n1); \n"
    "      dsn_name := aref(v2, 0); \n"
    "      dsn_desc := aref(v2, 1); \n"
    "      result(dsn_name, dsn_desc); \n"
    "      n1 := n1 + 1; \n"
    "    } \n"
    "}";


static long
fill_avail_dsns_list (ServerDSN * dlg)
{
  char szDSNName[50], szDBMS[50];
  SQLLEN len1 = SQL_NTS, len2 = SQL_NTS;
  char *szNewDSN, *szNewDBMS;
  long ret = 0, proc_defined = 0;
  test_t *lpBench = dlg->lpBench;
  dsn_info_t *dsn_info = &dlg->avail_dsn_info;

  free_dsns_list (dsn_info->dsns, GList, g_list);
  free_dsns_list (dsn_info->names, GList, g_list);

  if (_stristr (lpBench->szDBMS, "Virtuoso"))
    {
      ret = 1;

      if (SQL_SUCCESS != SQLExecDirect (lpBench->hstmt, dsn_proc, SQL_NTS))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}

      proc_defined = 1;

      if (SQL_SUCCESS !=
	  SQLPrepare (lpBench->hstmt,
	      (SQLCHAR *) "gtkbench_sql_data_sources (0)", SQL_NTS))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}

      if (SQL_SUCCESS !=
	  SQLBindCol (lpBench->hstmt,
	      1, SQL_C_CHAR, szDSNName, sizeof (szDSNName), &len1))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}
      if (SQL_SUCCESS !=
	  SQLBindCol (lpBench->hstmt,
	      2, SQL_C_CHAR, szDBMS, sizeof (szDBMS), &len2))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}

      if (SQL_SUCCESS != SQLExecute (lpBench->hstmt))
	{
	  vShowErrors (NULL, SQL_NULL_HENV, SQL_NULL_HDBC, lpBench->hstmt,
	      lpBench);
	  goto error;
	}

      while (SQL_SUCCESS == SQLFetch (lpBench->hstmt))
	{
	  int nPos;
	  szNewDSN = (char *) g_malloc (len1 + 1);
	  szNewDBMS = (char *) g_malloc (len2 + 1);
	  memcpy (szNewDSN, szDSNName, len1);
	  szNewDSN[len1] = 0;
	  dsn_info->dsns =
	      g_list_insert_sorted (dsn_info->dsns, szNewDSN,
	      (GCompareFunc) strcmp);
	  nPos = g_list_index (dsn_info->dsns, szNewDSN);
	  memcpy (szNewDBMS, szDBMS, len2);
	  szNewDBMS[len2] = 0;
	  dsn_info->names = g_list_insert (dsn_info->names, szNewDBMS, nPos);
	}
    error:
      if (proc_defined)
	SQLExecDirect (lpBench->hstmt,
	    (SQLCHAR *) "drop procedure gtkbench_sql_data_sources", SQL_NTS);

    }
  return ret;
}


GtkWidget *
ServerDSN_new (test_t * lpBench)
{
  GList *dsns, *names;
  ServerDSN *newdlg = SERVER_DSN (gtk_type_new (ServerDSN_get_type ()));
  newdlg->lpBench = lpBench;
  if (0 != (newdlg->isVirtuoso = fill_dsns_list (newdlg)))
    fill_avail_dsns_list (newdlg);

  dsns = newdlg->avail_dsn_info.dsns;
  names = newdlg->avail_dsn_info.names;
  while (dsns && names)
    {
      static gchar *cols[2];
      cols[0] = (gchar *) dsns->data;
      cols[1] = (gchar *) names->data;
      gtk_clist_append (GTK_CLIST (newdlg->available_list), cols);
      dsns = g_list_next (dsns);
      names = g_list_next (names);
    }
  gtk_clist_columns_autosize (GTK_CLIST (newdlg->available_list));

  dsns = newdlg->dsn_info.dsns;
  names = newdlg->dsn_info.names;
  while (dsns && names)
    {
      static gchar *cols[2];
      cols[0] = (gchar *) dsns->data;
      cols[1] = (gchar *) names->data;
      gtk_clist_append (GTK_CLIST (newdlg->defined_list), cols);
      dsns = g_list_next (dsns);
      names = g_list_next (names);
    }
  gtk_clist_columns_autosize (GTK_CLIST (newdlg->defined_list));
  return (GTK_WIDGET (newdlg));
}


static void
add_a_server_dsn (GtkWidget * widget, ServerDSN * tbl)
{
  char *dsn, *uid, *pwd;
  int isError = 1;

  dsn = gtk_entry_get_text (GTK_ENTRY (tbl->rdsn));
  uid = gtk_entry_get_text (GTK_ENTRY (tbl->ruid));
  pwd = gtk_entry_get_text (GTK_ENTRY (tbl->rpwd));

  if (SQL_SUCCESS != SQLPrepare (tbl->lpBench->hstmt,
	  (SQLCHAR *) "vd_remote_data_source (?, '', ?, ?)", SQL_NTS))
    goto error;

  IBINDNTS (tbl->lpBench->hstmt, 1, dsn);
  IBINDNTS (tbl->lpBench->hstmt, 2, uid);
  IBINDNTS (tbl->lpBench->hstmt, 3, pwd);

  if (SQL_SUCCESS != SQLExecute (tbl->lpBench->hstmt))
    goto error;

  isError = 0;
error:
  if (isError)
    {
      SQLCHAR state[10], msg[256];
      if (SQL_SUCCESS == SQLError (SQL_NULL_HENV, SQL_NULL_HDBC,
	      tbl->lpBench->hstmt, state, NULL, msg, sizeof (msg), NULL))
	ok_cancel_dialog ((char *) msg, "Error");
    }

  SQLFreeStmt (tbl->lpBench->hstmt, SQL_CLOSE);
  SQLFreeStmt (tbl->lpBench->hstmt, SQL_UNBIND);

  if (!isError)
    {
      fill_dsns_list (tbl);
      if (tbl->dsn_info.dsns)
	{
	  GList *dsns = tbl->dsn_info.dsns, *names = tbl->dsn_info.names;
	  gtk_clist_clear (GTK_CLIST (tbl->defined_list));
	  while (dsns && names)
	    {
	      static gchar *cols[2];
	      cols[0] = (gchar *) dsns->data;
	      cols[1] = (gchar *) names->data;
	      gtk_clist_append (GTK_CLIST (tbl->defined_list), cols);
	      dsns = g_list_next (dsns);
	      names = g_list_next (names);
	    }
	  gtk_clist_columns_autosize (GTK_CLIST (tbl->defined_list));
	}
    }
}


static void
set_the_dsn_entry (GtkWidget * widget,
    gint row, gint column, GdkEventButton * event, ServerDSN * tbl)
{
  char *szSelText;

  gtk_clist_get_text (GTK_CLIST (widget), row, 0, &szSelText);
  gtk_entry_set_text (GTK_ENTRY (tbl->rdsn), szSelText);
}


static void
emit_signal_handler (GtkWidget * widget, gpointer data)
{
  gtk_signal_emit_by_name (GTK_OBJECT (data), "do_the_work");
}


static void
ServerDSN_init (ServerDSN * dlg)
{
  static gchar *titles_defined[] = { "DSN", "DBMS name" };
  static gchar *titles_available[] = { "DSN", "Comment" };
  GtkWidget *AvailableFrame, *DefinedFrame, *LoginFrame, *master_table;
  GtkWidget *helper, *label, *scrolled, *hbox;
  GtkWidget *button;

  gtk_window_set_title (GTK_WINDOW (dlg), "Virtuoso DSNs");
  gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
  gtk_container_border_width (GTK_CONTAINER (GTK_DIALOG (dlg)->vbox), 10);

  master_table = gtk_table_new (13, 1, TRUE);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), master_table, TRUE,
      TRUE, 10);

  AvailableFrame = gtk_frame_new ("Available DSNs");
  gtk_container_set_border_width (GTK_CONTAINER (AvailableFrame), 5);
  gtk_table_attach_defaults (GTK_TABLE (master_table), AvailableFrame, 0, 1,
      0, 6);

  helper = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (AvailableFrame), helper);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled), 2);
  gtk_box_pack_start (GTK_BOX (helper), scrolled, TRUE, TRUE, 2);

  dlg->available_list = gtk_clist_new_with_titles (2, titles_available);
  gtk_clist_column_titles_passive (GTK_CLIST (dlg->available_list));
  gtk_clist_column_titles_show (GTK_CLIST (dlg->available_list));
  gtk_signal_connect (GTK_OBJECT (dlg->available_list), "select_row",
      GTK_SIGNAL_FUNC (set_the_dsn_entry), dlg);
  gtk_container_add (GTK_CONTAINER (scrolled), dlg->available_list);

  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_end (GTK_BOX (helper), hbox, TRUE, TRUE, 2);

  label = gtk_label_new ("Remote DSN name");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  dlg->rdsn = gtk_entry_new_with_max_length (50);
  gtk_box_pack_end (GTK_BOX (hbox), dlg->rdsn, TRUE, TRUE, 2);

  LoginFrame = gtk_frame_new ("Login Data");
  gtk_container_set_border_width (GTK_CONTAINER (LoginFrame), 5);
  gtk_table_attach_defaults (GTK_TABLE (master_table), LoginFrame, 0, 1, 6,
      8);

  helper = gtk_hbox_new (FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (helper), 2);
  gtk_container_add (GTK_CONTAINER (LoginFrame), helper);

  label = gtk_label_new ("User name");
  gtk_box_pack_start (GTK_BOX (helper), label, FALSE, FALSE, 2);

  dlg->ruid = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (helper), dlg->ruid, TRUE, TRUE, 2);

  dlg->rpwd = gtk_entry_new ();
  gtk_box_pack_end (GTK_BOX (helper), dlg->rpwd, TRUE, TRUE, 2);

  label = gtk_label_new ("Password");
  gtk_box_pack_end (GTK_BOX (helper), label, FALSE, FALSE, 5);

  button = gtk_button_new_with_label ("Add DSN");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (add_a_server_dsn), dlg);
  gtk_table_attach (GTK_TABLE (master_table), button, 0, 1, 8, 9,
      (GtkAttachOptions) (0), GTK_EXPAND | GTK_FILL, 0, 0);

  DefinedFrame = gtk_frame_new ("Defined DSNs");
  gtk_container_set_border_width (GTK_CONTAINER (DefinedFrame), 5);
  gtk_table_attach_defaults (GTK_TABLE (master_table), DefinedFrame, 0, 1, 9,
      13);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled), 2);
  gtk_container_add (GTK_CONTAINER (DefinedFrame), scrolled);

  dlg->defined_list = gtk_clist_new_with_titles (2, titles_defined);
  gtk_clist_column_titles_passive (GTK_CLIST (dlg->defined_list));
  gtk_clist_column_titles_show (GTK_CLIST (dlg->defined_list));
  gtk_container_add (GTK_CONTAINER (scrolled), dlg->defined_list);
  /*gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled), dlg->defined_list); */

  button = gtk_button_new_with_label ("OK");
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (dlg), button);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, FALSE,
      FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (emit_signal_handler), GTK_OBJECT (dlg));
}
