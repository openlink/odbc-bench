/*
 *  status.c
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
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <stdio.h>

#include "odbcbench.h"
#include "thr.h"
#include "testpool.h"
#define BARS_REFRESH_INTERVAL bench_get_long_pref (DISPLAY_REFRESH_RATE)

GtkWidget *status;
void
create_status_widget (GtkWidget *win)
{
  GtkAccelGroup *accel_group;

  status = gtk_text_new (NULL, NULL);
  gtk_text_set_editable (GTK_TEXT (status), FALSE);
  gtk_text_set_word_wrap (GTK_TEXT (status), TRUE);

  accel_group = gtk_accel_group_new ();

  gtk_widget_add_accelerator (status, "copy_clipboard", accel_group,
                              GDK_C, GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (status, "cut_clipboard", accel_group,
                              GDK_X, GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (status, "paste_clipboard", accel_group,
                              GDK_P, GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_window_add_accel_group (GTK_WINDOW (win), accel_group);

  gtk_widget_show (status);
}


void
clear_status_handler (GtkWidget * widget, gpointer data)
{
  guint end;

  end = gtk_text_get_length (GTK_TEXT (status));
  gtk_text_set_point (GTK_TEXT (status), end);
  gtk_text_backward_delete (GTK_TEXT (status), end);
}

MUTEX_T log_mutex;

void
do_pane_log (const char *format, ...)
{
  va_list args;
  char line[512];
  guint end;

  MUTEX_ENTER (log_mutex);

  va_start (args, format);
  vsprintf (line, format, args);

/*  gtk_text_freeze (GTK_TEXT (status));*/
  end = gtk_text_get_length (GTK_TEXT (status));
  gtk_text_set_point (GTK_TEXT (status), end);
  gtk_text_insert (GTK_TEXT (status), NULL, NULL, NULL, line, -1);
/*  gtk_text_thaw (GTK_TEXT (status));*/

  while (gtk_events_pending ())
    gtk_main_iteration ();

  MUTEX_LEAVE (log_mutex);
}

void (*pane_log) (const char *format, ...) = do_pane_log;

void
vBusy (void)
{
}

/* progress impl */
static BOOL gfCancel;
static GtkWidget *progress, *working_item, *status_line, *button;
static int *n_threads, n_connections = 0;
static GtkWidget ***threads_progress = NULL;
static float spec_time_secs = -1;
static float **pTrnTimes = NULL;
static testtype *test_types = NULL;
static float **fOldValues = NULL;
int nProgressIncrement = 0;
static long curr_time_msec = 0L;


static void
stop_progress_handler (GtkWidget * widget)
{
  gfCancel = TRUE;
}


static void
update_thread_tps (GtkWidget * adj, gpointer label)
{
  char szTemp[20];
  int conn, thr;

  if (spec_time_secs < 0)
    {
      conn = 0;
      thr = 0;
      goto found;
    }
  for (conn = 0; conn < n_connections; conn++)
    for (thr = 0; thr < n_threads[conn]; thr++)
      if (GTK_PROGRESS (threads_progress[conn][thr])->adjustment ==
	  GTK_ADJUSTMENT (adj))
	goto found;

  return;

found:
  if (spec_time_secs < 0)
    {				/* single threaded */
      float fCurrVal =
	  gtk_progress_get_value (GTK_PROGRESS (threads_progress[conn][thr]));
      if (fCurrVal != 0)
	{
	  sprintf (szTemp, "%10.2f tps", pTrnTimes[conn][thr] / fCurrVal);
	  gtk_label_set_text (GTK_LABEL (label), szTemp);
	}
      else
	gtk_label_set_text (GTK_LABEL (label), "       0.00 tps");
    }
  else
    {
      if (pTrnTimes[conn][thr] > 0)
	{
	  sprintf (szTemp, "%10.2f tps",
	      gtk_progress_get_value (GTK_PROGRESS (threads_progress[conn]
		      [thr])) / pTrnTimes[conn][thr]);
	  gtk_label_set_text (GTK_LABEL (label), szTemp);
	}
      else
	gtk_label_set_text (GTK_LABEL (label), "       0.00 tps");
    }
}


void
do_ShowProgress (GtkWidget * parent, gchar * title, gboolean bForceSingle,
    float fMax)
{

  GtkWidget *Frame, *helper, *thread_hbox = NULL, *thread_tps;
  int i;
  int nConn, n_max_threads = 1;
  GtkWidget *MasterFrame, *master_table;
  GList *tests = get_selected_tests (), *iter;

  curr_time_msec = get_msec_count ();
  spec_time_secs = -1;
  gfCancel = FALSE;
  progress = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (progress), title);

  master_table = gtk_table_new (2, 1, TRUE);
  gtk_widget_show (master_table);
  gtk_container_border_width (GTK_CONTAINER (master_table), 10);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (progress)->vbox), master_table,
      TRUE, TRUE, 0);

  working_item = gtk_label_new ("                                        ");
  gtk_label_set_justify (GTK_LABEL (working_item), GTK_JUSTIFY_CENTER);
  gtk_table_attach_defaults (GTK_TABLE (master_table), working_item, 0, 1, 0,
      1);
  gtk_widget_show (working_item);

  status_line =
      gtk_label_new
      ("                                                            ");
  gtk_label_set_justify (GTK_LABEL (status_line), GTK_JUSTIFY_CENTER);
  gtk_table_attach_defaults (GTK_TABLE (master_table), status_line, 0, 1, 1,
      2);
  gtk_widget_show (status_line);

  if (threads_progress)
    {
      for (i = 0; i < n_connections; i++)
	g_free (threads_progress[i]);
      for (i = 0; i < n_connections; i++)
	g_free (fOldValues[i]);
      g_free (fOldValues);
      g_free (threads_progress);
      g_free (n_threads);
      g_free (test_types);
      threads_progress = NULL;
    }
  if (pTrnTimes)
    {
      for (i = 0; i < n_connections; i++)
	g_free (pTrnTimes[i]);
      g_free (pTrnTimes);
      pTrnTimes = NULL;
    }

  n_connections = pool_connection_count ();
  n_threads = (int *) g_malloc (n_connections * sizeof (int));
  test_types = (testtype *) g_malloc (n_connections * sizeof (testtype));
  threads_progress =
      (GtkWidget ***) g_malloc (n_connections * sizeof (GtkWidget ***));
  pTrnTimes = (float **) g_malloc (n_connections * sizeof (float *));
  fOldValues = (float **) g_malloc (n_connections * sizeof (float *));

  for (iter = tests, i = 0; iter; iter = g_list_next (iter), i++)
    {
      test_t *ptest = (test_t *) iter->data;
      n_threads[i] =
	  bForceSingle ? 1 : (ptest->tpc._.nThreads ? ptest->tpc.
	  _.nThreads : 1);

      pTrnTimes[i] = (float *) g_malloc (n_threads[i] * sizeof (float));
      fOldValues[i] = (float *) g_malloc (n_threads[i] * sizeof (float));
      memset (pTrnTimes[i], 0, n_threads[i] * sizeof (float));
      memset (fOldValues[i], 0, n_threads[i] * sizeof (float));
      threads_progress[i] =
	  (GtkWidget **) g_malloc (n_threads[i] * sizeof (GtkWidget *));

      if (n_max_threads < n_threads[i])
	n_max_threads = n_threads[i];
      test_types[i] = ptest->TestType;
    }

  spec_time_secs = (n_max_threads == 1 && n_connections == 1) ? -1 : fMax;

  MasterFrame =
      gtk_frame_new (n_max_threads > 1 ? "Thread progress" : "Progress");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (progress)->vbox), MasterFrame,
      TRUE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (MasterFrame), 10);
  gtk_widget_show (MasterFrame);

  master_table = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (MasterFrame), master_table);
  gtk_widget_show (master_table);

  for (iter = tests, nConn = 0; iter; iter = g_list_next (iter), nConn++)
    {
      test_t *ptest = (test_t *) iter->data;

      Frame = gtk_frame_new (ptest->szName);
      gtk_box_pack_start (GTK_BOX (master_table), Frame, TRUE, TRUE, 5);
      gtk_container_border_width (GTK_CONTAINER (Frame), 10);
      gtk_widget_show (Frame);

      helper = gtk_table_new (n_threads[nConn], 1, TRUE);
      gtk_container_add (GTK_CONTAINER (Frame), helper);
      gtk_widget_show (helper);

      for (i = 0; i < n_threads[nConn]; i++)
	{
	  if (!bForceSingle)
	    {
	      thread_hbox = gtk_table_new (1, 4, TRUE);
	      gtk_table_set_col_spacings (GTK_TABLE (thread_hbox), 10);
	      gtk_table_attach_defaults (GTK_TABLE (helper), thread_hbox, 0,
		  1, i, i + 1);
	      gtk_widget_show (thread_hbox);
	    }

	  threads_progress[nConn][i] = gtk_progress_bar_new ();
	  gtk_progress_configure (GTK_PROGRESS (threads_progress[nConn][i]),
	      0, 0, fMax);
	  gtk_progress_bar_set_bar_style (GTK_PROGRESS_BAR (threads_progress
		  [nConn][i]), GTK_PROGRESS_CONTINUOUS);

	  if (n_threads[nConn] > 1 || n_connections > 1)
	    {
	      if (IS_A (*ptest))
		gtk_progress_set_format_string (GTK_PROGRESS (threads_progress
			[nConn][i]), "%v txns");
	      else
		gtk_progress_set_format_string (GTK_PROGRESS (threads_progress
			[nConn][i]), "%v packs");
	    }
	  else
	    gtk_progress_set_format_string (GTK_PROGRESS (threads_progress
		    [nConn][i]), "%p%%");
	  gtk_progress_set_show_text (GTK_PROGRESS (threads_progress[nConn]
		  [i]), TRUE);

	  if (!bForceSingle)
	    {
	      gtk_table_attach_defaults (GTK_TABLE (thread_hbox),
		  threads_progress[nConn][i], 0, 3, 0, 1);
	      thread_tps = gtk_label_new ("       0.00 tps");
	      gtk_table_attach_defaults (GTK_TABLE (thread_hbox), thread_tps,
		  3, 4, 0, 1);
	      gtk_signal_connect (GTK_OBJECT (GTK_PROGRESS (threads_progress
			  [nConn][i])->adjustment), "value_changed",
		  GTK_SIGNAL_FUNC (update_thread_tps), thread_tps);

	      gtk_widget_show (thread_tps);
	    }
	  else
	    gtk_table_attach_defaults (GTK_TABLE (helper),
		threads_progress[nConn][i], 0, 1, i, i + 1);
	  gtk_widget_show (threads_progress[nConn][i]);
	}
    }

  /* action area */
  button = gtk_button_new_with_label ("Cancel");
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (progress), button);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (stop_progress_handler), GTK_OBJECT (progress));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (progress)->action_area), button,
      TRUE, TRUE, 0);
  gtk_widget_show (button);

  gtk_widget_show (progress);
}

void
do_SetWorkingItem (char *pszWorking)
{
  /*long time_now = get_msec_count();
     if (time_now - curr_time_msec > BARS_REFRESH_INTERVAL) */
  gtk_label_set_text (GTK_LABEL (working_item), pszWorking);
}

void
do_SetProgressText (char *pszProgress, int nConn, int thread_no,
    float percent, int nTrnPerCall)
{
  long time_now = get_msec_count ();
  if (time_now - curr_time_msec > BARS_REFRESH_INTERVAL)
    gtk_label_set_text (GTK_LABEL (status_line), pszProgress);
  if (nConn >= 0 && nConn < n_connections && thread_no >= 0
      && thread_no < n_threads[nConn])
    {
      GtkProgress *pToSet = GTK_PROGRESS (threads_progress[nConn][thread_no]);
      if (spec_time_secs < 0)
	{
	  if (test_types[nConn] == TPC_A)
	    pTrnTimes[nConn][thread_no] +=
	      (bench_get_long_pref (A_REFRESH_RATE) * nTrnPerCall);
	  else
	    pTrnTimes[nConn][thread_no] += 1;
	  if (time_now - curr_time_msec > BARS_REFRESH_INTERVAL)
	    {
	      gtk_progress_set_value (pToSet, percent);
	    }
	}
      else
	{
	  /* that is a benchmark run - so adjust ! */
	  int fastest_conn_a = -1, fastest_conn_c =
	      -1, fastest_thr_a, fastest_thr_c;
	  float fMaxTps_a = 0, fCurrTps, fMaxTps_c = 0;
	  int conn, thr;

	  pTrnTimes[nConn][thread_no] = percent;
	  fOldValues[nConn][thread_no] +=
	      test_types[nConn] == TPC_A ? nProgressIncrement * nTrnPerCall: 1;
	  if (time_now - curr_time_msec > BARS_REFRESH_INTERVAL)
	    {
	      gtk_progress_set_value (pToSet, fOldValues[nConn][thread_no]);

	      for (conn = 0; conn < n_connections; conn++)
		{
		  for (thr = 0; thr < n_threads[conn]; thr++)
		    {
		      if (pTrnTimes[conn][thr] <= 0)
			continue;
		      else
			{
			  fCurrTps = gtk_progress_get_value (GTK_PROGRESS
			      (threads_progress[conn][thr])) /
			      pTrnTimes[conn][thr];
			  if (test_types[conn] == TPC_A
			      && (fastest_conn_a == -1
				  || fMaxTps_a < fCurrTps))
			    {
			      fastest_conn_a = conn;
			      fastest_thr_a = thr;
			      fMaxTps_a = fCurrTps;
			    }
			  if (test_types[conn] == TPC_C
			      && (fastest_conn_c == -1
				  || fMaxTps_c < fCurrTps))
			    {
			      fastest_conn_c = conn;
			      fastest_thr_c = thr;
			      fMaxTps_c = fCurrTps;
			    }
			}
		    }
		}
	      if (fastest_conn_a != -1)
		{
		  for (conn = 0; conn < n_connections; conn++)
		    for (thr = 0; thr < n_threads[conn]; thr++)
		      {
			if (test_types[conn] == TPC_A)
			  {
			    GtkProgress *pCurr =
				GTK_PROGRESS (threads_progress[conn][thr]);
			    float fCur = fOldValues[conn][thr];
			    if (fCur >= 0
				&& fCur <= fMaxTps_a * spec_time_secs)
			      gtk_progress_configure (pCurr, fCur, 0,
				  fMaxTps_a * spec_time_secs);
			  }
		      }
		}
	      if (fastest_conn_c != -1)
		{
		  for (conn = 0; conn < n_connections; conn++)
		    for (thr = 0; thr < n_threads[conn]; thr++)
		      {
			if (test_types[conn] == TPC_C)
			  {
			    GtkProgress *pCurr =
				GTK_PROGRESS (threads_progress[conn][thr]);
			    if (fMaxTps_c * spec_time_secs > 0)
			      {
				float currVal = fOldValues[conn][thr];
				if (currVal >= 0
				    && currVal <= fMaxTps_c * spec_time_secs)
				  gtk_progress_configure (pCurr, currVal, 0,
				      fMaxTps_c * spec_time_secs);
			      }
			  }
		      }
		}
	    }
	}
    }
  if (time_now - curr_time_msec > BARS_REFRESH_INTERVAL)
    curr_time_msec = get_msec_count ();
}


void
do_MarkFinished (int nConn, int thread_no)
{
  if (nConn >= 0 && nConn < n_connections && thread_no >= 0
      && thread_no < n_threads[nConn])
    {
      gtk_widget_set_name (threads_progress[nConn][thread_no],
	  "thread finished");
      gtk_progress_set_format_string (GTK_PROGRESS (threads_progress[nConn]
	      [thread_no]), "Finished");
    }
  spec_time_secs = -1;
}


void
do_StopProgress (void)
{
  int i;

  if (progress)
    {
      gtk_widget_destroy (progress);
      progress = NULL;
    }

  if (threads_progress)
    {
      for (i = 0; i < n_connections; i++)
	g_free (threads_progress[i]);
      g_free (threads_progress);
      g_free (n_threads);
      threads_progress = NULL;
    }
  if (pTrnTimes)
    {
      for (i = 0; i < n_connections; i++)
	g_free (pTrnTimes[i]);
      g_free (pTrnTimes);
      pTrnTimes = NULL;
    }

  n_threads = 0;
  n_connections = 0;
}

int
do_fCancel (void)
{
  if (!progress)
    return FALSE;

  while (gtk_events_pending ())
    gtk_main_iteration ();

  return gfCancel;
}

void
do_ShowCancel (int fShow)
{
  if (fShow)
    gtk_widget_show (button);
  else
    gtk_widget_hide (button);
}

void
do_RestartProgress (void)
{
  gfCancel = FALSE;
}

BOOL
isCancelled(void)
{
  return gfCancel;
}
