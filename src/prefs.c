/*
 *  prefs.c
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
#include <stdio.h>
#include <stdlib.h>

#include "odbcbench.h"

struct pref_s
{
  long a_refresh_rate;
  long c_refresh_rate;
  long lock_timeout;
  long display_refresh_rate;
}
prefs =
{
10, 1, 0, 80};

long
bench_get_long_pref (OdbcBenchPref pref)
{
  switch (pref)
    {
    case A_REFRESH_RATE:
      return prefs.a_refresh_rate;
    case C_REFRESH_RATE:
      return prefs.c_refresh_rate;
    case LOCK_TIMEOUT:
      return prefs.lock_timeout;
    case DISPLAY_REFRESH_RATE:
      return prefs.display_refresh_rate;
    }
  return -1;
}


int
bench_set_long_pref (OdbcBenchPref pref, long value)
{
  switch (pref)
    {
    case A_REFRESH_RATE:
      prefs.a_refresh_rate = value;
      return 1;
    case C_REFRESH_RATE:
      prefs.c_refresh_rate = value;
      return 1;
    case LOCK_TIMEOUT:
      prefs.lock_timeout = value;
      return 1;
    case DISPLAY_REFRESH_RATE:
      prefs.display_refresh_rate = value;
      return 1;
    }
  return 0;
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

  sprintf (szTemp, "%ld", prefs.display_refresh_rate);
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
    {
      prefs.display_refresh_rate =
	  atoi (gtk_entry_get_text (GTK_ENTRY (entry)));
    }
  gtk_widget_destroy (dlg);
}

/*
static int
txn_isolation_to_index (long txn_isolation)
{
  switch (txn_isolation)
    {
      case SQL_TXN_READ_UNCOMMITTED: return 0;
      case SQL_TXN_READ_COMMITTED: return 1;
      case SQL_TXN_REPEATABLE_READ: return 2;
      case SQL_TXN_SERIALIZABLE: return 3;
      default: return 0;
    }
}
*/

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

  sprintf (szTemp, "%ld", prefs.lock_timeout);
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
    {
      prefs.lock_timeout = atoi (gtk_entry_get_text (GTK_ENTRY (entry)));
    }
  gtk_widget_destroy (dlg);
}
