/*
 *  LoginBox.c
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
#include <string.h>
#include <gtk/gtk.h>

#include "LoginBox.h"

static void LoginBox_class_init (LoginBoxClass * lclass);
static void LoginBox_init (LoginBox * login_box);

int
LoginBox_get_type (void)
{
  static guint e_type = 0;

  if (!e_type)
    {
      GtkTypeInfo e_info = {
	"LoginBox",
	sizeof (LoginBox),
	sizeof (LoginBoxClass),
	(GtkClassInitFunc) LoginBox_class_init,
	(GtkObjectInitFunc) LoginBox_init,
	NULL,
	NULL
      };

      e_type = gtk_type_unique (gtk_dialog_get_type (), &e_info);
    }

  return e_type;
}


enum
{
  DO_THE_WORK_SIGNAL,
  CLOSED,
  LAST_SIGNAL
};

static guint LoginBox_signals[LAST_SIGNAL] = { 0 };


static void
LoginBox_class_init (LoginBoxClass * lclass)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass *) lclass;

  LoginBox_signals[DO_THE_WORK_SIGNAL] = gtk_signal_new ("do_the_work",
      GTK_RUN_FIRST,
      object_class->type,
      GTK_SIGNAL_OFFSET (LoginBoxClass, LoginBox_do_the_work),
      gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);

  LoginBox_signals[CLOSED] = gtk_signal_new ("closed",
      GTK_RUN_FIRST,
      object_class->type,
      GTK_SIGNAL_OFFSET (LoginBoxClass, LoginBox_closed),
      gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);

  gtk_object_class_add_signals (object_class, LoginBox_signals, LAST_SIGNAL);

  lclass->LoginBox_do_the_work = NULL;
  lclass->LoginBox_closed = NULL;
}


static void
emit_signal_handler (GtkWidget * widget, gpointer data)
{
  LoginBox_dsn (LOGINBOX (data), NULL);
  LoginBox_uid (LOGINBOX (data), NULL);
  LoginBox_pwd (LOGINBOX (data), NULL);
  gtk_signal_emit_by_name (GTK_OBJECT (data), "do_the_work");
}


static void
emit_closed_handler (GtkWidget * widget, gpointer data)
{
  gtk_signal_emit_by_name (GTK_OBJECT (data), "closed");
}


static void
LoginBox_init (LoginBox * login_box)
{

  GtkWidget *table;
  GtkWidget *label, *button;

  gtk_window_set_title (GTK_WINDOW (login_box), "Login");
  gtk_container_border_width (GTK_CONTAINER (login_box), 10);

  table = gtk_table_new (3, 5, TRUE);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (login_box)->vbox), table, TRUE,
      TRUE, 0);

  label = gtk_label_new ("Data source");
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  login_box->dsn = gtk_combo_new ();
  gtk_combo_set_use_arrows (GTK_COMBO (login_box->dsn), TRUE);
  gtk_widget_show (login_box->dsn);
  gtk_table_attach_defaults (GTK_TABLE (table), login_box->dsn, 1, 5, 0, 1);

  label = gtk_label_new ("User ID");
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

  login_box->uid = gtk_entry_new_with_max_length (50);
  gtk_widget_show (login_box->uid);
  gtk_table_attach_defaults (GTK_TABLE (table), login_box->uid, 1, 5, 1, 2);

  label = gtk_label_new ("Password");
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);

  login_box->pwd = gtk_entry_new_with_max_length (50);
  gtk_entry_set_visibility (GTK_ENTRY (login_box->pwd), FALSE);
  gtk_widget_show (login_box->pwd);
  gtk_table_attach_defaults (GTK_TABLE (table), login_box->pwd, 1, 5, 2, 3);

  /* action_area */
  button = gtk_button_new_with_label ("Login");
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_window_set_default (GTK_WINDOW (login_box), button);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_grab_remove), GTK_OBJECT (login_box));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (login_box));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (emit_closed_handler), login_box);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (emit_signal_handler), login_box);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (login_box)->action_area), button,
      TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("Cancel");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (emit_closed_handler), login_box);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (login_box)->action_area), button,
      TRUE, TRUE, 0);
  gtk_window_set_modal (GTK_WINDOW (login_box), TRUE);
  gtk_grab_add (GTK_WIDGET (login_box));
}


GtkWidget *
LoginBox_new (gchar * title, GList * dsn_list, gchar * szDSN, gchar * szUID,
    gchar * szPWD)
{
  LoginBox *login_box = LOGINBOX (gtk_type_new (LoginBox_get_type ()));
  gtk_window_set_title (GTK_WINDOW (login_box), title ? title : "Login");
  if (dsn_list)
    gtk_combo_set_popdown_strings (GTK_COMBO (login_box->dsn), dsn_list);
  LoginBox_dsn (login_box, szDSN);
  LoginBox_uid (login_box, szUID);
  LoginBox_pwd (login_box, szPWD);
  return (GTK_WIDGET (login_box));
}


gchar *
LoginBox_dsn (LoginBox * box, gchar * new_dsn)
{
  if (!box)
    return NULL;

  strncpy (box->szDSN,
      gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (box->dsn)->entry)), 50);
  if (new_dsn)
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (box->dsn)->entry), new_dsn);
  else
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (box->dsn)->entry), "");

  return box->szDSN;
}


gchar *
LoginBox_uid (LoginBox * box, gchar * new_uid)
{
  if (!box)
    return NULL;

  strncpy (box->szUID, gtk_entry_get_text (GTK_ENTRY (box->uid)), 50);
  if (new_uid)
    gtk_entry_set_text (GTK_ENTRY (box->uid), new_uid);
  else
    gtk_entry_set_text (GTK_ENTRY (box->uid), "");

  return box->szUID;
}


gchar *
LoginBox_pwd (LoginBox * box, gchar * new_pwd)
{
  if (!box)
    return NULL;

  strncpy (box->szPWD, gtk_entry_get_text (GTK_ENTRY (box->pwd)), 50);
  if (new_pwd)
    gtk_entry_set_text (GTK_ENTRY (box->pwd), new_pwd);
  else
    gtk_entry_set_text (GTK_ENTRY (box->pwd), "");

  return box->szPWD;
}
