/*
 *  util.c
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>

#include "odbcbench.h"
#include "LoginBox.h"
#include "thr.h"


GList *
get_dsn_list (void)
{
  char *szDSN = (char *) g_malloc (256);
  GList *list = NULL;
  SQLSMALLINT nDSN;

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
do_flip (GtkWidget * widget, gboolean * data)
{
  if (data)
    *data = *data ? FALSE : TRUE;
}


void
set_display_refresh_rate (GtkWidget * widget, gpointer data)
{
  GtkWidget *dlg, *entry, *button, *label;
  char szTemp[512];
  gboolean bOk = FALSE;

  dlg = gtk_dialog_new ();
  gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dlg)->vbox), 10);
  gtk_window_set_title (GTK_WINDOW (dlg), "Enter option");

  label = gtk_label_new ("Refresh interval for the progress bars (msec)");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), label, TRUE, TRUE, 5);

  sprintf (szTemp, "%ld", bench_get_long_pref (DISPLAY_REFRESH_RATE));
  entry = gtk_entry_new_with_max_length (10);
  gtk_entry_set_text (GTK_ENTRY (entry), szTemp);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), entry, TRUE, TRUE, 5);

  button = gtk_button_new_with_label ("OK");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE,
      TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (do_flip), &bOk);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));

  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE,
      TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));

  gtk_widget_show_all (dlg);
  gtk_main ();

  if (bOk)
    bench_set_long_pref (DISPLAY_REFRESH_RATE,
	atoi (gtk_entry_get_text (GTK_ENTRY (entry))));
  gtk_widget_destroy (dlg);
}


void
set_lock_timeout (GtkWidget * widget, gpointer data)
{
  GtkWidget *dlg, *entry, *button, *label;
  char szTemp[512];
  gboolean bOk = FALSE;

  dlg = gtk_dialog_new ();
  gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dlg)->vbox), 10);
  gtk_window_set_title (GTK_WINDOW (dlg), "Enter option");

  label = gtk_label_new ("Lock Timeout per thread (msecs)");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), label, TRUE, TRUE, 5);

  sprintf (szTemp, "%ld", bench_get_long_pref (LOCK_TIMEOUT));
  entry = gtk_entry_new_with_max_length (10);
  gtk_entry_set_text (GTK_ENTRY (entry), szTemp);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), entry, TRUE, TRUE, 5);

  button = gtk_button_new_with_label ("OK");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE,
      TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (do_flip), &bOk);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));

  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE,
      TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));

  gtk_widget_show_all (dlg);
  gtk_main ();

  if (bOk)
    bench_set_long_pref (LOCK_TIMEOUT,
	atoi (gtk_entry_get_text (GTK_ENTRY (entry))));
  gtk_widget_destroy (dlg);
}
