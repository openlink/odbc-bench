/*
 *  main.c
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
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "odbcbench.h"
#include "odbcbench_gtk.h"
#include "testpool.h"
#include "LoginBox.h"
#include "results.h"
#include "results_gtk.h"
#include "ArrayParams.h"
#include "ThreadOptions.h"
#include "TPCATableProps.h"
#include "TPCCTableProps.h"
#include "TPCARunProps.h"
#include "tpca_code.h"
#include "tpccfun.h"
#include "thr.h"
#include "util.h"

GUI_t gui;

void close_test (GtkWidget * widget, gpointer data);
GtkWidget *dlg = NULL, *scrolled, *windows, *status_box, *global_status;
extern GtkWidget *status;
static GtkWidget *menubar;

static void
err_message (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  /*g_error(format, args); */
  vfprintf (stderr, format, args);
  va_end (args);
}


static void
warn_message (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  /*g_warning(format, args); */
  vfprintf (stderr, format, args);
  va_end (args);
}


static void
message (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  /*g_message(format, args); */
  vfprintf (stderr, format, args);
  va_end (args);
}


int
do_login_gtk (GtkWidget * widget, gpointer data)
{
  LoginBox *login_box = NULL;
  test_t *ptest = (test_t *) data;

  if (widget)
    {
      login_box = LOGINBOX (widget);
      strcpy (ptest->szLoginDSN, login_box->szDSN);
      strcpy (ptest->szLoginUID, login_box->szUID);
      strcpy (ptest->szLoginPWD, login_box->szPWD);
    }
  return do_login (ptest);
}


char *
do_close_selected_tests (char *filename, answer_code * rc)
{
  OList *tests = get_selected_tests_list (), *iter;
  int is_dirty = 0;
  char *retfile = NULL;
  answer_code local_rc;

  if (!rc)
    rc = &local_rc;

  *rc = DLG_YES;

  iter = tests;
  while (iter)
    {
      test_t *ptest = (test_t *) iter->data;
      if (ptest->is_dirty)
	{
	  is_dirty = TRUE;
	}
      iter = o_list_next (iter);
    }
  if (is_dirty || getIsFileDirty ())
    {
      *rc = yes_no_cancel_dialog ("Do you want to save the changes", "Close");
      if (*rc == DLG_YES)
	{
	  retfile = fill_file_name (filename, "Select output file", TRUE);
	  if (retfile)
	    {
	      if (!do_save_selected (retfile, tests))
		*rc = DLG_CANCEL;
	    }
	}
    }
  if (*rc != DLG_CANCEL)
    {
      remove_selected_tests ();
      setIsFileDirty (TRUE);
    }
  o_list_free (tests);
  return retfile;
}


void
help_about_handler (GtkWidget * widget, gpointer data)
{
  char szTemp[2048], szTitle[1024];

  sprintf (szTemp,
      "%s v.%s\n"
      "(C) 2000-2005 OpenLink Software\n"
      "Please report all bugs to <%s>\n\n"
      "This utility is released under the GNU General Public License (GPL)\n\n"
      "Disclaimer: The benchmarks in this application are loosely based\n"
      "on the TPC-A and TPC-C standard benchmarks, but this application\n"
      "does not claim to be a full or precise implementation, nor are\n"
      "the results obtained by this application necessarily comparable\n"
      "to the vendor's published results.\n", PACKAGE_NAME, PACKAGE_VERSION,
      PACKAGE_BUGREPORT);
  sprintf (szTitle, "About %s", PACKAGE);

  message_box_new (widget, szTemp, szTitle);
}


void
make_new_test (GtkWidget * widget, gpointer data)
{
  if (getFilesCount ())
    {
      GtkWidget *dlg, *name, *is_a, *is_c, *label, *box, *frame;
      GtkWidget *button;
      GSList *group;
      gboolean bOk = FALSE;

      dlg = gtk_dialog_new ();
      gtk_window_set_title (GTK_WINDOW (dlg), "New Test");
      gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
      gtk_container_border_width (GTK_CONTAINER (dlg), 10);

      box = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), box, TRUE, TRUE,
	  0);

      label = gtk_label_new ("Name");
      gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

      name = gtk_entry_new_with_max_length (128);
      gtk_entry_set_text (GTK_ENTRY (name), "New test");
      gtk_box_pack_start (GTK_BOX (box), name, TRUE, TRUE, 0);

      frame = gtk_frame_new ("Type");
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), frame, TRUE, TRUE,
	  0);

      box = gtk_vbox_new (TRUE, 0);
      gtk_container_add (GTK_CONTAINER (frame), box);

      is_a = gtk_radio_button_new_with_label (NULL, "TPC-A like benchmark");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (is_a), TRUE);
      gtk_box_pack_start (GTK_BOX (box), is_a, TRUE, TRUE, 0);

      group = gtk_radio_button_group (GTK_RADIO_BUTTON (is_a));
      is_c = gtk_radio_button_new_with_label (group, "TPC-C like benchmark");
      gtk_box_pack_start (GTK_BOX (box), is_c, TRUE, TRUE, 0);

      button = gtk_button_new_with_label ("OK");
      GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
      gtk_window_set_default (GTK_WINDOW (dlg), button);
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button,
	  TRUE, TRUE, 0);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
	  GTK_SIGNAL_FUNC (do_flip), &bOk);
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
	  GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
	  GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));

      button = gtk_button_new_with_label ("Cancel");
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button,
	  TRUE, TRUE, 0);
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
	  GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
	  GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));

      gtk_widget_show_all (dlg);
      gtk_main ();

      if (bOk)
	{
	  test_t *ptest = (test_t *) g_malloc (sizeof (test_t));
	  memset (ptest, 0, sizeof (test_t));

	  strncpy (ptest->szName, gtk_entry_get_text (GTK_ENTRY (name)),
	      sizeof (ptest->szName));
	  ptest->TestType = GTK_TOGGLE_BUTTON (is_a)->active ? TPC_A : TPC_C;
	  ptest->is_dirty = TRUE;
	  init_test (ptest);
	  if (!add_test_to_the_pool (ptest))
	    g_free (ptest);
	}
      gtk_widget_destroy (dlg);
    }
}


void
load_setup (GtkWidget * widget, gpointer data)
{
  char *filename;

  filename = fill_file_name (NULL, "Select file to open", TRUE);
  if (filename)
    {
      create_test_pool ();
      do_load_test (filename);
      setTheFileName (filename);
      setIsFileDirty (FALSE);
    }
}


void
make_new_setup (GtkWidget * widget, gpointer data)
{
  create_test_pool ();
  setTheFileName (NULL);
  setIsFileDirty (FALSE);
}


void
close_setup (GtkWidget * widget, gpointer data)
{
  char *retfile;
  answer_code rc;
  if (!getFilesCount ())
    return;
  for_all_in_pool ();
  retfile = do_close_selected_tests (getCurrentFileName (), &rc);
  if (rc != DLG_CANCEL)
    close_file_pool ();
  if (retfile)
    g_free (retfile);
}


void
insert_file (GtkWidget * widget, gpointer data)
{
  if (getFilesCount ())
    {
      char *filename = fill_file_name (NULL, "Select file to insert", TRUE);
      if (filename)
	{
	  do_load_test (filename);
	  g_free (filename);
	  setIsFileDirty (TRUE);
	}
    }
}


void
save_setup (GtkWidget * widget, gpointer data)
{
  OList *tests = NULL;
  if (getFilesCount ())
    {
      char *filename =
	  fill_file_name (getCurrentFileName (), "Select output file", TRUE);
      if (filename)
	{
	  setTheFileName (filename);
	  for_all_in_pool ();
	  tests = get_selected_tests_list ();
	  do_save_selected (filename, tests);
	  o_list_free (tests);
	  setIsFileDirty (FALSE);
	}
    }
}


void
save_test_as (GtkWidget * widget, gpointer data)
{
  OList *tests = get_selected_tests_list ();
  if (tests)
    {
      char *filename = fill_file_name (NULL, "Select output file", TRUE);
      if (filename)
	{
	  do_save_selected (filename, tests);
	  g_free (filename);
	}
      o_list_free (tests);
    }
}


void
save_setup_as (GtkWidget * widget, gpointer data)
{
  for_all_in_pool ();
  save_test_as (widget, data);
}


void
edit_login (GtkWidget * widget, gpointer data)
{
  GList *tests = get_selected_tests (), *iter;
  int nTests = g_list_length (tests);
  test_t *ptest = NULL;
  gboolean bOk = FALSE;
  GtkWidget *login;
  int rc;

  if (!nTests)
    {
      ok_cancel_dialog ("There are no selected tests", "ODBC-Bench");
      return;
    }
  if (nTests == 1)
    ptest = (test_t *) tests->data;

  login = LoginBox_new ("Login details",
      get_dsn_list (),
      ptest ? ptest->szLoginDSN : NULL,
      ptest ? ptest->szLoginUID : NULL, ptest ? ptest->szLoginPWD : NULL);

  gtk_signal_connect_object (GTK_OBJECT (login), "closed",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (login));
  gtk_signal_connect (GTK_OBJECT (login), "do_the_work",
      GTK_SIGNAL_FUNC (do_flip), &bOk);

  gtk_widget_show (login);
  gtk_main ();

  if (bOk)
    {
      iter = tests;
      if (!ptest)
	ptest = (test_t *) iter->data;

      ptest->tpc.a.uwDrvIdx = -1;
      rc = do_login_gtk (login, ptest);
      ptest->is_dirty = TRUE;
      if (ptest->szSQLError[0] || rc != TRUE)
	{
	  pane_log ("Connect Error %s : %s\r\n", ptest->szSQLState,
	      ptest->szSQLError);
	  g_list_free (tests);
	  return;
	}
      get_dsn_data (ptest);
      do_logout (ptest);

      while (NULL != (iter = g_list_next (iter)))
	{
	  test_t *ptest = (test_t *) iter->data;
	  ptest->tpc.a.uwDrvIdx = -1;
	  if (do_login_gtk (login, ptest))
	    {
	      get_dsn_data (ptest);
	      do_logout (ptest);
	    }
	  ptest->is_dirty = TRUE;
	}
      pool_update_selected ();
    }
  gtk_widget_destroy (login);

  g_list_free (tests);
}


void
edit_tables (GtkWidget * widget, gpointer data)
{
  GList *tests = get_selected_tests (), *iter;
  GtkWidget *props = NULL;
  GtkWidget *dlg, *btn;
  iter = tests;

  while (iter)
    {
      test_t *ptest = (test_t *) iter->data;
      props = NULL;
      if (!ptest->bTablePropsShown)
	switch (ptest->TestType)
	  {
	  case TPC_A:
	    props = TPCATableProps_new (ptest);
	    break;

	  case TPC_C:
	    if (_stristr (ptest->szDBMS, "Virtuoso"))
	      props = TPCCTableProps_new (ptest);
	    break;
	  }

      if (props)
	{
	  char szTitle[512];
	  dlg = gtk_dialog_new ();
	  sprintf (szTitle, "Table details : %s", ptest->szName);
	  gtk_window_set_title (GTK_WINDOW (dlg), szTitle);
	  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), props, TRUE,
	      TRUE, 0);

	  btn = gtk_button_new_with_label ("OK");
	  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
	  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (do_flip), &(ptest->bTablePropsShown));
	  if (IS_TPCA_TABLE_PROPS (props))
	    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
		GTK_SIGNAL_FUNC (TPCATableProps_save_config),
		GTK_OBJECT (props));
	  else if (IS_TPCC_TABLE_PROPS (props))
	    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
		GTK_SIGNAL_FUNC (TPCCTableProps_save_config),
		GTK_OBJECT (props));
	  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
	  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), btn,
	      TRUE, TRUE, 0);
	  if (!ptest->is_dirty)
	    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
		GTK_SIGNAL_FUNC (do_flip), &ptest->is_dirty);

	  btn = gtk_button_new_with_label ("Cancel");
	  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (do_flip), &(ptest->bTablePropsShown));
	  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
	  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), btn,
	      TRUE, TRUE, 0);

	  ptest->bTablePropsShown = TRUE;
	  gtk_widget_show_all (dlg);
	}
      iter = g_list_next (iter);
    }
  g_list_free (tests);
}


void
edit_run (GtkWidget * widget, gpointer data)
{
  GList *tests = get_selected_tests (), *iter;
  GtkWidget *props = NULL;
  GtkWidget *dlg, *btn;
  iter = tests;

  while (iter)
    {
      test_t *ptest = (test_t *) iter->data;

      props = NULL;
      if (!ptest->bRunPropsShown)
	switch (ptest->TestType)
	  {
	  case TPC_A:
	    props = TPCARunProps_new (ptest);
	    break;

	  case TPC_C:
	    props = ThreadOptions_new ();
	    ThreadOptions_load_config (THREAD_OPTIONS (props), &ptest->tpc._);
	    break;
	  }

      if (props)
	{
	  char szTitle[512];
	  dlg = gtk_dialog_new ();
	  sprintf (szTitle, "Run details : %s", ptest->szName);
	  gtk_window_set_title (GTK_WINDOW (dlg), szTitle);
	  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), props, TRUE,
	      TRUE, 0);

	  btn = gtk_button_new_with_label ("OK");
	  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
	  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (do_flip), &ptest->bRunPropsShown);
	  if (IS_TPCA_RUN_PROPS (props))
	    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
		GTK_SIGNAL_FUNC (TPCARunProps_save_config),
		GTK_OBJECT (props));
	  else if (IS_THREAD_OPTIONS (props))
	    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
		GTK_SIGNAL_FUNC (ThreadOptions_save_config),
		GTK_OBJECT (props));
	  else if (IS_ARRAY_PARAMS (props))
	    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
		GTK_SIGNAL_FUNC (ArrayParams_save_config),
		GTK_OBJECT (props));
	  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
	  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), btn,
	      TRUE, TRUE, 0);
	  if (!ptest->is_dirty)
	    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
		GTK_SIGNAL_FUNC (do_flip), &ptest->is_dirty);

	  btn = gtk_button_new_with_label ("Cancel");
	  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (do_flip), &ptest->bRunPropsShown);
	  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	      GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
	  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), btn,
	      TRUE, TRUE, 0);

	  ptest->bRunPropsShown = TRUE;
	  gtk_widget_show_all (dlg);
	}
      iter = g_list_next (iter);
    }
  g_list_free (tests);
}


void
create_tables (GtkWidget * widget, gpointer data)
{
  GList *tests = get_selected_tests (), *iter;
  int rc;
  iter = tests;

  if (tests)
    pane_log ("CREATING SCHEMA STARTED\r\n");
  if (menubar)
    gtk_widget_set_sensitive (GTK_WIDGET (menubar), FALSE);
  while (iter)
    {
      test_t *ptest = (test_t *) iter->data;
      pool_set_selected_item (ptest);
      rc = do_login_gtk (NULL, ptest);
      if (ptest->hdbc && rc)
	{
	  switch (ptest->TestType)
	    {
	    case TPC_A:
	      fBuildBench (ptest);
	      break;

	    case TPC_C:
	      tpcc_schema_create (NULL, ptest);
	      tpcc_create_db (NULL, ptest);
	      break;
	    }
	  do_logout (ptest);
	}
      iter = g_list_next (iter);
    }
  pool_set_selected_items (tests);
  g_list_free (tests);
  if (menubar)
    gtk_widget_set_sensitive (GTK_WIDGET (menubar), TRUE);
  if (tests)
    pane_log ("CREATING SCHEMA FINISHED\r\n");
}


void
drop_tables (GtkWidget * widget, gpointer data)
{
  GList *tests = get_selected_tests (), *iter;
  int rc;
  iter = tests;

  if (tests)
    pane_log ("CLEANUP STARTED\r\n");
  if (menubar)
    gtk_widget_set_sensitive (GTK_WIDGET (menubar), FALSE);
  while (iter)
    {
      test_t *ptest = (test_t *) iter->data;
      pool_set_selected_item (ptest);
      rc = do_login_gtk (NULL, ptest);
      if (ptest->hdbc && rc)
	{
	  switch (ptest->TestType)
	    {
	    case TPC_A:
	      fCleanup (ptest);
	      break;

	    case TPC_C:
	      tpcc_schema_cleanup (NULL, ptest);
	      break;
	    }
	  do_logout (ptest);
	}
      iter = g_list_next (iter);
    }
  pool_set_selected_items (tests);
  g_list_free (tests);
  if (menubar)
    gtk_widget_set_sensitive (GTK_WIDGET (menubar), TRUE);
  if (tests)
    pane_log ("CLEANUP FINISHED\r\n");
}


static void
set_file_name (GtkWidget * widget)
{
  char *filename = fill_file_name (NULL, "Select output file name", TRUE);
  if (filename)
    {
      gtk_entry_set_text (GTK_ENTRY (widget), filename);
      g_free (filename);
    }
}


void
run_selected (GtkWidget * widget, gpointer data)
{
  GtkWidget *dlg, *label, *entry, *hbox, *btn, *filename;
  gboolean bOk = FALSE, bRunAll = FALSE, canRunAll = TRUE;
  int nTests = pool_connection_count ();
  OList *tests = get_selected_tests_list (), *iter;
  char *szFileName = NULL;

  if (!nTests)
    return;
  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dlg), "Run Duration");

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), hbox, TRUE, TRUE, 5);

  label = gtk_label_new ("Test duration (mins)");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

  entry = gtk_entry_new_with_max_length (3);
  gtk_entry_set_text (GTK_ENTRY (entry), "1");
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 5);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), hbox, TRUE, TRUE, 5);

  label = gtk_label_new ("Output file name");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

  filename = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (filename), "results.xml");
  gtk_box_pack_start (GTK_BOX (hbox), filename, TRUE, TRUE, 5);

  btn = gtk_button_new_with_label ("Select ...");
  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
      GTK_SIGNAL_FUNC (set_file_name), GTK_OBJECT (filename));
  gtk_box_pack_start (GTK_BOX (hbox), btn, TRUE, TRUE, 0);

  btn = gtk_button_new_with_label ("Start");
  GTK_WIDGET_SET_FLAGS (btn, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (dlg), btn);
  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
      GTK_SIGNAL_FUNC (do_flip), &bOk);
  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), btn, TRUE,
      TRUE, 0);


  iter = tests;
  while (iter)
    {
      if (((test_t *) iter->data)->TestType != TPC_A)
	canRunAll = FALSE;
      iter = o_list_next (iter);
    }
  if (canRunAll)
    {
      btn = gtk_button_new_with_label ("Run All");
      gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	  GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
	  GTK_SIGNAL_FUNC (do_flip), &bOk);
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
	  GTK_SIGNAL_FUNC (do_flip), &bRunAll);
      gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
	  GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), btn, TRUE,
	  TRUE, 0);
    }

  btn = gtk_button_new_with_label ("Cancel");
  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), btn, TRUE,
      TRUE, 0);

  gtk_widget_show_all (dlg);
  gtk_main ();

  do_RestartProgress ();

  if (bOk)
    {
      test_t *ptest;
      int nMinutes = atoi (gtk_entry_get_text (GTK_ENTRY (entry)));

      szFileName = gtk_entry_get_text (GTK_ENTRY (filename));
      if (!szFileName)
	szFileName = "results.xml";
      szFileName = fill_file_name (szFileName, "dummy", TRUE);

      gtk_widget_destroy (dlg);
      if (nMinutes < 1)
	{
	  goto end;
	}

      if (menubar)
	gtk_widget_set_sensitive (GTK_WIDGET (menubar), FALSE);
      do_run_selected (tests, nTests, szFileName, nMinutes, bRunAll);
      if (menubar)
	gtk_widget_set_sensitive (GTK_WIDGET (menubar), TRUE);
    }

end:
  if (szFileName)
    g_free (szFileName);
  o_list_free (tests);
}


void
destroy_handler (GtkWidget * widget, gpointer data)
{
  char *file;
  answer_code rc;
  while (getFilesCount ())
    {
      for_all_in_pool ();
      file = do_close_selected_tests (getCurrentFileName (), &rc);
      if (file)
	g_free (file);
      if (rc == DLG_CANCEL)
	return;
      close_file_pool ();
    }
  do_free_env (1);
  gtk_main_quit ();
}


void
close_test (GtkWidget * widget, gpointer data)
{
  GList *tests = get_selected_tests (), *iter;
  char *file;
  setIsFileDirty (FALSE);
  for (iter = tests; iter; iter = g_list_next (iter))
    {
      test_t *ptest = (test_t *) iter->data;
      ptest->is_dirty = FALSE;
    }
  file = do_close_selected_tests (NULL, NULL);
  setIsFileDirty (TRUE);
  g_free (file);
  g_list_free (tests);
}


#ifdef PIPE_DEBUG
static void
pipe_trough_isql_handler (GtkWidget * widget, gpointer data)
{
  GList *tests = get_selected_tests (), *iter;
  for (iter = tests; iter; iter = g_list_next (iter))
    {
      test_t *ptest = (test_t *) iter->data;
      if (do_login_gtk (NULL, ptest))
	{
	  pipe_trough_isql (ptest->hdbc, "gogo.sql", 1);
	  do_logout (ptest);
	}
    }
  g_list_free (tests);
}
#endif


static GtkItemFactoryEntry menu_items[] = {
  {"/_File", NULL, NULL, 0, "<Branch>"},
  {"/File/New", "<control>N", (GtkItemFactoryCallback) make_new_setup, 0,
      NULL},
  {"/File/_Open...", "<control>O", (GtkItemFactoryCallback) load_setup, 0,
      NULL},
  {"/File/_Save", "<control>S", (GtkItemFactoryCallback) save_setup, 0, NULL},
  {"/File/Save _As...", "<control>A", (GtkItemFactoryCallback) save_setup_as,
      0, NULL},
  {"/File/_Close", NULL, (GtkItemFactoryCallback) close_setup, 0, NULL},
  {"/File/sep1", NULL, NULL, 0, "<Separator>"},
  {"/File/_Clear log", NULL, (GtkItemFactoryCallback) clear_status_handler, 0,
      NULL},
#ifdef PIPE_DEBUG
  {"/File/pipe trough isql", NULL,
      (GtkItemFactoryCallback) pipe_trough_isql_handler, 0, NULL},
#endif
  {"/File/sep2", NULL, NULL, 0, "<Separator>"},
  {"/File/E_xit", "<control>X", (GtkItemFactoryCallback) destroy_handler, 0,
      NULL},

  {"/_Edit", NULL, NULL, 0, "<Branch>"},
  {"/Edit/New Benchmark Item...", "<control>Insert",
      (GtkItemFactoryCallback) make_new_test, 0, NULL},
  {"/Edit/Delete selected items", NULL, (GtkItemFactoryCallback) close_test,
      0, NULL},
  {"/Edit/Save selected items as...", NULL,
      (GtkItemFactoryCallback) save_test_as, 0, NULL},
  {"/Edit/sep1", NULL, NULL, 0, "<Separator>"},
  {"/Edit/_Login details...", "<control>L",
      (GtkItemFactoryCallback) edit_login, 0, NULL},
  {"/Edit/_Table details...", "<control>T",
      (GtkItemFactoryCallback) edit_tables, 0, NULL},
  {"/Edit/R_un details...", "<control>U", (GtkItemFactoryCallback) edit_run,
      0, NULL},
  {"/Edit/sep2", NULL, NULL, 0, "<Separator>"},
  {"/Edit/_Insert file...", "<control>I",
      (GtkItemFactoryCallback) insert_file, 0, NULL},

  {"/_Action", NULL, NULL, 0, "<Branch>"},
  {"/Action/_Create tables&procedures", NULL,
      (GtkItemFactoryCallback) create_tables, 0, NULL},
  {"/Action/_Drop tables&procedures", NULL,
      (GtkItemFactoryCallback) drop_tables, 0, NULL},
  {"/Action/sep1", NULL, NULL, 0, "<Separator>"},
  {"/Action/Run _Selected", "<control>R",
      (GtkItemFactoryCallback) run_selected, 0, NULL},

  {"/_Results", NULL, NULL, 0, "<Branch>"},
  {"/Results/_Connect...", NULL, (GtkItemFactoryCallback) do_results_login, 0,
      NULL},
  {"/Results/_Disconnect", NULL, (GtkItemFactoryCallback) do_results_logout,
      0, NULL},
  {"/Results/sep1", NULL, NULL, 0, "<Separator>"},
  {"/Results/C_reate the table", NULL,
      (GtkItemFactoryCallback) do_create_results_table, 0, NULL},
  {"/Results/Dr_op the table", NULL,
      (GtkItemFactoryCallback) do_drop_results_table, 0, NULL},

  {"/_Preferences", NULL, NULL, 0, "<Branch>"},

  {"/Preferences/Display refresh rate...", NULL,
	(GtkItemFactoryCallback) set_display_refresh_rate,
      0, NULL},
  {"/Preferences/Lock timeout...", NULL,
      (GtkItemFactoryCallback) set_lock_timeout, 0, NULL},

  {"/_Window", NULL, NULL, 0, "<Branch>"},

  {"/_Help", NULL, NULL, 0, "<LastBranch>"},
  {"/Help/_About", "<control>F1", (GtkItemFactoryCallback) help_about_handler,
      0, NULL},
};


void
get_main_menu (GtkWidget * window, GtkWidget ** menubar)
{
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

  accel_group = gtk_accel_group_new ();

  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
      accel_group);

  gtk_item_factory_create_items_ac (item_factory, nmenu_items, menu_items,
      window, 2);

  /* Attach the new accelerator group to the window. */
  gtk_accel_group_attach (accel_group, GTK_OBJECT (window));

  if (menubar)
    /* Finally, return the actual menu bar created by the item factory. */
    {
      *menubar = gtk_item_factory_get_widget (item_factory, "<main>");
      windows = gtk_item_factory_get_widget (item_factory, "/Window");
    }
}


extern int do_command_line (int argc, char *argv[]);
#ifndef WIN32
int
main (int argc, char *argv[])
#else
extern char **command_line_to_argv (char *pszSysCmdLine, int *_argc);

int WINAPI
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
    int nCmdShow)
#endif
{
  GtkWidget *table;
#ifdef WIN32
  int argc;
  char **argv;
  argv = command_line_to_argv (GetCommandLine (), &argc);
#endif
  do_alloc_env ();

  memset (&gui, 0, sizeof (gui));
  gui.main_quit = gtk_main_quit;
  gui.err_message = err_message;
  gui.warn_message = warn_message;
  gui.message = message;
  gui.add_test_to_the_pool = add_test_to_the_pool;
  gui.for_all_in_pool = for_all_in_pool;
  gui.do_MarkFinished = do_MarkFinished;

  gui.isCancelled = isCancelled;
  gui.ShowProgress = do_ShowProgress;
  gui.SetWorkingItem = do_SetWorkingItem;
  gui.SetProgressText = do_SetProgressText;
  gui.StopProgress = do_StopProgress;
  gui.fCancel = do_fCancel;
  gui.vBusy = vBusy;

  if (argc > 2)
    return do_command_line (argc, argv);

  gtk_init (&argc, &argv);
  gtk_set_locale ();
  gtk_rc_add_default_file ("odbcbenchrc");

  dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
      GTK_SIGNAL_FUNC (destroy_handler), NULL);
  setTheFileName (NULL);
  gtk_widget_set_usize (GTK_WIDGET (dlg), 420, 320);

  table = gtk_table_new (11, 1, TRUE);
  gtk_container_add (GTK_CONTAINER (dlg), table);
  gtk_widget_show (table);

  get_main_menu (dlg, &menubar);
  gtk_table_attach (GTK_TABLE (table), menubar, 0, 1, 0, 1,
      GTK_FILL | GTK_EXPAND, (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (menubar);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled), 2);
  gtk_widget_show (scrolled);
  gtk_table_attach_defaults (GTK_TABLE (table), scrolled, 0, 1, 1, 5);

  status_box = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (status_box);
  create_status_widget (dlg);
  global_status = status;
  gtk_container_add (GTK_CONTAINER (status_box), global_status);
  gtk_table_attach_defaults (GTK_TABLE (table), status_box, 0, 1, 5, 11);

  gtk_widget_show (dlg);
  create_test_pool ();
  if (argc > 1)
    {
      do_load_test (argv[1]);
      setTheFileName (fill_file_name (argv[1], NULL, TRUE));
      setIsFileDirty (FALSE);
    }
  else
    {
      setTheFileName (NULL);
      setIsFileDirty (FALSE);
    }
  gtk_main ();

  return (0);
}
