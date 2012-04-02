/*
 *  TPCCTableProps.c
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
#include "ServerDSN.h"
#include "TPCCTableProps.h"
#include "TPCATableProps.h"

static void TPCCTableProps_class_init (TPCCTablePropsClass * tclass);
static void TPCCTableProps_init (TPCCTableProps * tableloader);


int
TPCCTableProps_get_type (void)
{
  static guint tld_type = 0;

  if (!tld_type)
    {
      GtkTypeInfo tld_info = {
	"TPCCTableProps",
	sizeof (TPCCTableProps),
	sizeof (TPCCTablePropsClass),
	(GtkClassInitFunc) TPCCTableProps_class_init,
	(GtkObjectInitFunc) TPCCTableProps_init,
	NULL,
	NULL
      };

      tld_type = gtk_type_unique (gtk_vbox_get_type (), &tld_info);
    }

  return tld_type;
}


static void
TPCCTableProps_class_init (TPCCTablePropsClass * tclass)
{
}


static void
set_dsn_combos (GtkWidget * check, gpointer widget)
{
  GtkWidget **combos = (GtkWidget **) widget;
  ServerDSN *dsn = SERVER_DSN (check);
  if (dsn->dsn_info.dsns)
    {
      int inx;
      for (inx = 0; inx < 9; inx++)
	gtk_combo_set_popdown_strings (GTK_COMBO (combos[inx]),
	    dsn->dsn_info.dsns);
    }
}


GtkWidget *
TPCCTableProps_new (test_t * lpBench)
{
  TPCCTableProps *newdlg =
      TPCC_TABLE_PROPS (gtk_type_new (TPCCTableProps_get_type ()));
  ServerDSN *dsn;
  int inx;

  newdlg->lpBench = lpBench;
  dsn = SERVER_DSN (newdlg->dsn = ServerDSN_new (lpBench));
  gtk_signal_connect (GTK_OBJECT (newdlg->dsn), "dsns_changed",
      GTK_SIGNAL_FUNC (set_dsn_combos), newdlg->combos);

  for (inx = 0; inx < 9; inx++)
    {
      if (dsn->dsn_info.dsns)
	gtk_combo_set_popdown_strings (GTK_COMBO (newdlg->combos[inx]),
	    dsn->dsn_info.dsns);
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (newdlg->combos[inx])->entry),
	  lpBench->tpc.c.tableDSNS[inx]);
    }
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (newdlg->n_ware),
      newdlg->lpBench->tpc.c.count_ware);
  return (GTK_WIDGET (newdlg));
}


void
TPCCTableProps_save_config (TPCCTableProps * newdlg)
{
  int i;
  ServerDSN *dsn = SERVER_DSN (newdlg->dsn);
  for (i = 0; i < 9; i++)
    {
      GList *found;
      char *szValue =
	  gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (newdlg->
		  combos[i])->entry));

      strncpy (newdlg->lpBench->tpc.c.tableDSNS[i], szValue, 50);
      if (!strcmp (szValue, "<local>"))
	newdlg->lpBench->tpc.c.tableDSNS[i][0] = 0;
      else if (NULL !=
	  (found =
	      g_list_find_custom (dsn->dsn_info.dsns, szValue,
		  (GCompareFunc) strcmp)))
	strncpy (newdlg->lpBench->tpc.c.tableDBMSes[i],
	    (char *) g_list_nth_data (dsn->dsn_info.names,
		g_list_position (dsn->dsn_info.dsns, found)), 50);
    }
  newdlg->lpBench->tpc.c.count_ware =
      (int) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (newdlg->
	  n_ware));
}


static void
show_server_data_sources_dialog (GtkWidget * widget)
{
  TPCCTableProps *dlg = TPCC_TABLE_PROPS (widget);
  gtk_signal_connect_object (GTK_OBJECT (dlg->dsn), "do_the_work",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg->dsn));
  gtk_signal_connect_object (GTK_OBJECT (dlg->dsn), "do_the_work",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (dlg->dsn));

  gtk_widget_show_all (GTK_WIDGET (dlg->dsn));
  gtk_main ();
}


static void
TPCCTableProps_init (TPCCTableProps * dlg)
{
  char *szNames[] = {
    "WAREHOUSE",
    "DISTRICT",
    "CUSTOMER",
    "THISTORY",
    "NEW_ORDER",
    "ORDERS",
    "ORDER_LINE",
    "ITEM",
    "STOCK"
  };
  int i;
  GtkWidget *table, *label, *remotes, *dsn_btn, *frame;
  GtkObject *adj;
  GtkWidget *vbox;

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (dlg), vbox);

  gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);

  table = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 10);
  gtk_widget_show (table);

  label = gtk_label_new ("Number of warehouses");
  gtk_box_pack_start (GTK_BOX (table), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  adj = gtk_adjustment_new (1, 1, 300, 1, 10, 10);
  dlg->n_ware = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 0.5, 0);
  gtk_box_pack_start (GTK_BOX (table), dlg->n_ware, TRUE, TRUE, 0);
  gtk_widget_show (dlg->n_ware);

  frame = gtk_frame_new ("Table destinations");
  gtk_widget_show (frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  table = gtk_table_new (10, 3, TRUE);
  gtk_widget_show (table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  dsn_btn = gtk_button_new_with_label ("DSNs ...");
  gtk_widget_show (dsn_btn);
  gtk_signal_connect_object (GTK_OBJECT (dsn_btn), "clicked",
      GTK_SIGNAL_FUNC (show_server_data_sources_dialog), GTK_OBJECT (dlg));
  gtk_table_attach_defaults (GTK_TABLE (table), dsn_btn, 2, 3, 0, 1);
  gtk_widget_set_sensitive (dsn_btn, FALSE);

  remotes = gtk_check_button_new_with_label ("Remote tables");
  gtk_widget_show (remotes);
  gtk_table_attach_defaults (GTK_TABLE (table), remotes, 1, 2, 0, 1);
  gtk_signal_connect (GTK_OBJECT (remotes), "clicked",
      GTK_SIGNAL_FUNC (enable_widget_as), dsn_btn);

  for (i = 0; i < 9; i++)
    {
      label = gtk_label_new (szNames[i]);
      gtk_widget_show (label);
      gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1 + i,
	  2 + i);

      dlg->combos[i] = gtk_combo_new ();
      gtk_widget_show (dlg->combos[i]);
      gtk_combo_set_value_in_list (GTK_COMBO (dlg->combos[i]), TRUE, TRUE);
      gtk_table_attach_defaults (GTK_TABLE (table), dlg->combos[i], 1, 3,
	  1 + i, 2 + i);
      gtk_widget_set_sensitive (GTK_WIDGET (dlg->combos[i]),
	  GTK_TOGGLE_BUTTON (remotes)->active > 0);
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dlg->combos[i])->entry), "");
      gtk_signal_connect (GTK_OBJECT (remotes), "clicked",
	  GTK_SIGNAL_FUNC (enable_widget_as), dlg->combos[i]);
    }
}
