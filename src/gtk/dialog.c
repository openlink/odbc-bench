/*
 *  dialog.c
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
#include <gtk/gtk.h>

#include "odbcbench.h"
#include "odbcbench_gtk.h"

GtkWidget *
message_box_new (GtkWidget * Parent, const gchar * Text, const gchar * Title)
{
  GtkWidget *dlg;
  GtkWidget *text;
  GtkWidget *ok_button;

  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dlg), Title ? Title : "Message");
  gtk_container_border_width (GTK_CONTAINER (dlg), 10);
  text = gtk_label_new (Text ? Text : "Message");
  gtk_label_set_justify (text, GTK_JUSTIFY_LEFT);
  gtk_widget_show (text);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), text, TRUE, TRUE, 0);

  ok_button = gtk_button_new_with_label ("OK");
  GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (dlg), ok_button);
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
  GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (ok_button);
  gtk_widget_grab_focus (ok_button);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), ok_button,
      TRUE, TRUE, 0);
  gtk_widget_show (ok_button);

  gtk_widget_show (dlg);
  return (dlg);
}


answer_code
ok_cancel_dialog (const gchar * Text, const gchar * Title)
{
  GtkWidget *dlg;
  GtkWidget *text;
  GtkWidget *ok_button;
  gboolean bOk = FALSE;

  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dlg), Title ? Title : "Message");
  gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
  gtk_container_border_width (GTK_CONTAINER (dlg), 10);
  text = gtk_label_new (Text ? Text : "Message");
  gtk_widget_show (text);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), text, TRUE, TRUE, 0);

  ok_button = gtk_button_new_with_label ("OK");
  GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (dlg), ok_button);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), ok_button,
      TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (do_flip), &bOk);
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
  gtk_widget_show (ok_button);

  ok_button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), ok_button,
      TRUE, TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
  gtk_widget_show (ok_button);

  gtk_widget_show (dlg);
  gtk_main ();

  gtk_widget_destroy (dlg);

  return bOk ? DLG_OK : DLG_CANCEL;
}

/*
static void
set_to_ok (GtkWidget *widget, answer_code *data)
{
  *data = DLG_OK;
}
*/
static void
set_to_yes (GtkWidget * widget, answer_code * data)
{
  *data = DLG_YES;
}

static void
set_to_no (GtkWidget * widget, answer_code * data)
{
  *data = DLG_NO;
}

answer_code
yes_no_cancel_dialog (const gchar * Text, const gchar * Title)
{
  GtkWidget *dlg;
  GtkWidget *text;
  GtkWidget *ok_button;
  answer_code state = DLG_CANCEL;

  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dlg), Title ? Title : "Message");
  gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
  gtk_container_border_width (GTK_CONTAINER (dlg), 10);
  text = gtk_label_new (Text ? Text : "Message");
  gtk_widget_show (text);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), text, TRUE, TRUE, 0);

  ok_button = gtk_button_new_with_label ("Yes");
  GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (dlg), ok_button);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), ok_button,
      TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (set_to_yes), &state);
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
  gtk_widget_show (ok_button);

  ok_button = gtk_button_new_with_label ("No");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), ok_button,
      TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (set_to_no), &state);
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
  gtk_widget_show (ok_button);

  ok_button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), ok_button,
      TRUE, TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
  gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg));
  gtk_widget_show (ok_button);

  gtk_widget_show (dlg);
  gtk_main ();

  gtk_widget_destroy (dlg);

  return state;
}


char *
fill_file_name (char *szFileName, char *caption, int add_xmls)
{
  char *szName;

  if (!szFileName)
    {
      GtkWidget *filew = gtk_file_selection_new (caption);
      gboolean bOk = FALSE;

      gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
	  "clicked", (GtkSignalFunc) do_flip, &bOk);
      gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
	      (filew)->ok_button), "clicked", (GtkSignalFunc) gtk_main_quit,
	  GTK_OBJECT (filew));
      gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
	      (filew)->cancel_button), "clicked",
	  (GtkSignalFunc) gtk_main_quit, GTK_OBJECT (filew));
      gtk_widget_show_all (filew);

      gtk_main ();
      gtk_widget_hide (filew);
      if (bOk)
	{
	  char *sel =
	      gtk_file_selection_get_filename (GTK_FILE_SELECTION (filew));
	  char *szExt;
	  if (!sel)
	    {
	      gtk_widget_destroy (filew);
	      return NULL;
	    }

	  szExt = strrchr (sel, '.');
	  if (!szExt && add_xmls)
	    {
	      szName = g_malloc (strlen (sel) + 5);
	      strcpy (szName, sel);
	      strcat (szName, ".xml");
	    }
	  else
	    {
	      szName = g_malloc (strlen (sel) + 1);
	      strcpy (szName, sel);
	    }
	  gtk_widget_destroy (filew);
	}
      else
	{
	  gtk_widget_destroy (filew);
	  return NULL;
	}
    }
  else
    {
      char *szExt = strrchr (szFileName, '.');
      if (!szExt && add_xmls)
	{
	  szName = g_malloc (strlen (szFileName) + 5);
	  strcpy (szName, szFileName);
	  strcat (szName, ".xml");
	}
      else
	{
	  szName = g_strdup (szFileName);
	}
    }
  return szName;
}
