/*
 *  TPCATableProps.c
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
#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "odbcbench.h"
#include "TPCATableProps.h"
#include "results.h"
#include "tpca_code.h"

static void TPCATableProps_class_init (TPCATablePropsClass * tclass);
static void TPCATableProps_init (TPCATableProps * tableloader);

int
TPCATableProps_get_type (void)
{
  static guint tld_type = 0;

  if (!tld_type)
    {
      GtkTypeInfo tld_info = {
	"TPCATableProps",
	sizeof (TPCATableProps),
	sizeof (TPCATablePropsClass),
	(GtkClassInitFunc) TPCATableProps_class_init,
	(GtkObjectInitFunc) TPCATableProps_init,
	NULL,
	NULL
      };

      tld_type = gtk_type_unique (gtk_vbox_get_type (), &tld_info);
    }

  return tld_type;
}

static void
TPCATableProps_class_init (TPCATablePropsClass * tclass)
{
}

void
enable_widget_as (GtkWidget * check, gpointer widget)
{
  gtk_widget_set_sensitive (GTK_WIDGET (widget),
      GTK_TOGGLE_BUTTON (check)->active > 0);
  if (GTK_IS_COMBO (widget))
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (widget)->entry), "");
}


static void
show_server_data_sources_dialog (GtkWidget * check, gpointer widget)
{
  TPCATableProps *tbl = TPCA_TABLE_PROPS (widget);
  gtk_signal_connect_object (GTK_OBJECT (tbl->pDSN), "do_the_work",
      GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (tbl->pDSN));
  gtk_signal_connect_object (GTK_OBJECT (tbl->pDSN), "do_the_work",
      GTK_SIGNAL_FUNC (gtk_main_quit), GTK_OBJECT (tbl->pDSN));

  gtk_widget_show_all (GTK_WIDGET (tbl->pDSN));
  gtk_main ();
}


static void
set_dsn_combos (GtkWidget * check, gpointer widget)
{
  TPCATableProps *tbl = TPCA_TABLE_PROPS (widget);
  ServerDSN *dsn = SERVER_DSN (check);
  if (dsn->dsn_info.dsns)
    {
      gtk_combo_set_popdown_strings (GTK_COMBO (tbl->branch_dsn),
	  dsn->dsn_info.dsns);
      gtk_combo_set_popdown_strings (GTK_COMBO (tbl->teller_dsn),
	  dsn->dsn_info.dsns);
      gtk_combo_set_popdown_strings (GTK_COMBO (tbl->account_dsn),
	  dsn->dsn_info.dsns);
      gtk_combo_set_popdown_strings (GTK_COMBO (tbl->history_dsn),
	  dsn->dsn_info.dsns);
    }
}

static void
make_sub_controls (TPCATableProps * tableloader)
{
  GtkWidget *CreateTablesFrame, *LoadTablesFrame, *IndexesFrame,
      *RecordsToInsertFrame;
  GtkWidget *main_hbox, *first_vbox, *second_vbox, *remotes = NULL;
  GtkWidget *helper, *label;
  GList *list = NULL;
  int i, ofs;

  do_login(tableloader->ptest);
  tableloader->pDSN = (ServerDSN *) ServerDSN_new (tableloader->ptest);
  if (tableloader->ptest->tpc.a.uwDrvIdx == -1)
    {
      tableloader->ptest->tpc.a.uwDrvIdx =
	  getDriverTypeIndex (tableloader->ptest->szDBMS);
      tableloader->ptest->is_dirty = 1;
    }
  gtk_signal_connect (GTK_OBJECT (tableloader->pDSN), "dsns_changed",
      GTK_SIGNAL_FUNC (set_dsn_combos), GTK_OBJECT (tableloader));

  gtk_container_border_width (GTK_CONTAINER (&tableloader->parent), 10);
  main_hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (tableloader), main_hbox);

  first_vbox = gtk_vbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (main_hbox), first_vbox, TRUE, TRUE, 0);

  CreateTablesFrame = gtk_frame_new ("Create Tables");
  gtk_box_pack_start (GTK_BOX (first_vbox), CreateTablesFrame, TRUE, TRUE, 0);

  helper =
      gtk_table_new (tableloader->pDSN->isVirtuoso ? 5 : 4,
      tableloader->pDSN->isVirtuoso ? 3 : 1, TRUE);
  gtk_container_border_width (GTK_CONTAINER (helper), 5);
  gtk_container_add (GTK_CONTAINER (CreateTablesFrame), helper);

  if (tableloader->pDSN->isVirtuoso)
    {
      GtkWidget *dsn_btn = gtk_button_new_with_label ("DSNs ...");
      gtk_signal_connect (GTK_OBJECT (dsn_btn), "clicked",
	  GTK_SIGNAL_FUNC (show_server_data_sources_dialog), tableloader);
      gtk_table_attach_defaults (GTK_TABLE (helper), dsn_btn, 2, 3, 0, 1);
      gtk_widget_set_sensitive (dsn_btn, FALSE);

      remotes = gtk_check_button_new_with_label ("Remote tables");
      gtk_table_attach_defaults (GTK_TABLE (helper), remotes, 1, 2, 0, 1);

      gtk_signal_connect (GTK_OBJECT (remotes), "clicked",
	  GTK_SIGNAL_FUNC (enable_widget_as), dsn_btn);
      ofs = 1;
    }
  else
    ofs = 0;

  tableloader->create_branch = gtk_check_button_new_with_label ("Branch");
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->create_branch,
      0, 1, ofs, ofs + 1);

  tableloader->create_teller = gtk_check_button_new_with_label ("Teller");
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->create_teller,
      0, 1, ofs + 1, ofs + 2);

  tableloader->create_account = gtk_check_button_new_with_label ("Account");
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->create_account,
      0, 1, ofs + 2, ofs + 3);

  tableloader->create_history = gtk_check_button_new_with_label ("History");
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->create_history,
      0, 1, ofs + 3, ofs + 4);

  tableloader->branch_dsn = gtk_combo_new ();
  gtk_combo_set_value_in_list (GTK_COMBO (tableloader->branch_dsn), TRUE,
      TRUE);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (tableloader->branch_dsn)->entry),
      "");
  if (tableloader->pDSN->isVirtuoso)
    {
      if (tableloader->pDSN->dsn_info.dsns)
	gtk_combo_set_popdown_strings (GTK_COMBO (tableloader->branch_dsn),
	    tableloader->pDSN->dsn_info.dsns);
      gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->branch_dsn,
	  1, 3, ofs + 0, ofs + 1);
      gtk_widget_set_sensitive (tableloader->branch_dsn, FALSE);
      gtk_signal_connect (GTK_OBJECT (remotes), "clicked",
	  GTK_SIGNAL_FUNC (enable_widget_as), tableloader->branch_dsn);
    }

  tableloader->teller_dsn = gtk_combo_new ();
  gtk_combo_set_value_in_list (GTK_COMBO (tableloader->teller_dsn), TRUE,
      TRUE);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (tableloader->teller_dsn)->entry),
      "");
  if (tableloader->pDSN->isVirtuoso)
    {
      if (tableloader->pDSN->dsn_info.dsns)
	gtk_combo_set_popdown_strings (GTK_COMBO (tableloader->teller_dsn),
	    tableloader->pDSN->dsn_info.dsns);
      gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->teller_dsn,
	  1, 3, ofs + 1, ofs + 2);
      gtk_widget_set_sensitive (tableloader->teller_dsn, FALSE);
      gtk_signal_connect (GTK_OBJECT (remotes), "clicked",
	  GTK_SIGNAL_FUNC (enable_widget_as), tableloader->teller_dsn);
    }

  tableloader->account_dsn = gtk_combo_new ();
  gtk_combo_set_value_in_list (GTK_COMBO (tableloader->account_dsn), TRUE,
      TRUE);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (tableloader->account_dsn)->entry),
      "");
  if (tableloader->pDSN->isVirtuoso)
    {
      if (tableloader->pDSN->dsn_info.dsns)
	gtk_combo_set_popdown_strings (GTK_COMBO (tableloader->account_dsn),
	    tableloader->pDSN->dsn_info.dsns);
      gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->account_dsn,
	  1, 3, ofs + 2, ofs + 3);
      gtk_widget_set_sensitive (tableloader->account_dsn, FALSE);
      gtk_signal_connect (GTK_OBJECT (remotes), "clicked",
	  GTK_SIGNAL_FUNC (enable_widget_as), tableloader->account_dsn);
    }

  tableloader->history_dsn = gtk_combo_new ();
  gtk_combo_set_value_in_list (GTK_COMBO (tableloader->history_dsn), TRUE,
      TRUE);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (tableloader->history_dsn)->entry),
      "");
  if (tableloader->pDSN->isVirtuoso)
    {
      if (tableloader->pDSN->dsn_info.dsns)
	gtk_combo_set_popdown_strings (GTK_COMBO (tableloader->history_dsn),
	    tableloader->pDSN->dsn_info.dsns);
      gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->history_dsn,
	  1, 3, ofs + 3, ofs + 4);
      gtk_widget_set_sensitive (tableloader->history_dsn, FALSE);
      gtk_signal_connect (GTK_OBJECT (remotes), "clicked",
	  GTK_SIGNAL_FUNC (enable_widget_as), tableloader->history_dsn);
    }

  LoadTablesFrame = gtk_frame_new ("Load Tables");
  gtk_container_border_width (GTK_CONTAINER (LoadTablesFrame), 5);
  gtk_box_pack_start (GTK_BOX (first_vbox), LoadTablesFrame, TRUE, TRUE, 0);

  helper = gtk_table_new (3, 1, TRUE);
  gtk_container_border_width (GTK_CONTAINER (helper), 5);
  gtk_container_add (GTK_CONTAINER (LoadTablesFrame), helper);

  tableloader->load_branch = gtk_check_button_new_with_label ("Branch");
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->load_branch, 0,
      1, 0, 1);

  tableloader->load_teller = gtk_check_button_new_with_label ("Teller");
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->load_teller, 0,
      1, 1, 2);

  tableloader->load_account = gtk_check_button_new_with_label ("Account");
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->load_account, 0,
      1, 2, 3);

  second_vbox = gtk_vbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (main_hbox), second_vbox, TRUE, TRUE, 0);

  IndexesFrame = gtk_frame_new ("Indexes");
  gtk_container_border_width (GTK_CONTAINER (IndexesFrame), 5);
  gtk_box_pack_start (GTK_BOX (second_vbox), IndexesFrame, TRUE, TRUE, 0);

  helper = gtk_hbox_new (TRUE, 10);
  gtk_container_border_width (GTK_CONTAINER (helper), 5);
  gtk_container_add (GTK_CONTAINER (IndexesFrame), helper);

  tableloader->create_indexes =
      gtk_check_button_new_with_label ("Create Indexes");
  gtk_box_pack_start (GTK_BOX (helper), tableloader->create_indexes, TRUE,
      TRUE, 0);

  tableloader->create_procedures =
      gtk_check_button_new_with_label ("Create Procedures");
  gtk_box_pack_start (GTK_BOX (helper), tableloader->create_procedures, TRUE,
      TRUE, 0);

  RecordsToInsertFrame = gtk_frame_new ("Records to Insert");
  gtk_container_border_width (GTK_CONTAINER (RecordsToInsertFrame), 5);
  gtk_box_pack_start (GTK_BOX (second_vbox), RecordsToInsertFrame, TRUE, TRUE,
      0);

  helper = gtk_table_new (3, 2, FALSE);
  gtk_container_border_width (GTK_CONTAINER (helper), 5);
  gtk_container_add (GTK_CONTAINER (RecordsToInsertFrame), helper);

  label = gtk_label_new ("Number of branches");
  gtk_table_attach_defaults (GTK_TABLE (helper), label, 0, 1, 0, 1);

  tableloader->num_branches = gtk_entry_new_with_max_length (10);
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->num_branches, 1,
      2, 0, 1);

  label = gtk_label_new ("Number of Tellers");
  gtk_table_attach_defaults (GTK_TABLE (helper), label, 0, 1, 1, 2);

  tableloader->num_tellers = gtk_entry_new_with_max_length (10);
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->num_tellers, 1,
      2, 1, 2);

  label = gtk_label_new ("Number of Accounts");
  gtk_table_attach_defaults (GTK_TABLE (helper), label, 0, 1, 2, 3);

  tableloader->num_accounts = gtk_entry_new_with_max_length (10);
  gtk_table_attach_defaults (GTK_TABLE (helper), tableloader->num_accounts, 1,
      2, 2, 3);

  helper = gtk_vbox_new (TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (helper), 5);
  gtk_box_pack_start (GTK_BOX (second_vbox), helper, TRUE, TRUE, 0);

  label = gtk_label_new ("Schema for DBMS");
  gtk_box_pack_start (GTK_BOX (helper), label, TRUE, TRUE, 0);

  for (i = 0; i < getDriverMapSize (); i++)
    list = g_list_append (list, getDriverDBMSName (i));
  tableloader->dbmstype = gtk_combo_new ();
  gtk_combo_set_popdown_strings (GTK_COMBO (tableloader->dbmstype), list);
  gtk_box_pack_start (GTK_BOX (helper), tableloader->dbmstype, TRUE, TRUE, 0);

  gtk_widget_show_all (main_hbox);
}

static void
TPCATableProps_init (TPCATableProps * tableloader)
{
}

static void
load_config (TPCATableProps * pDlg)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->create_branch),
      pDlg->ptest->tpc.a.fCreateBranch);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->create_teller),
      pDlg->ptest->tpc.a.fCreateTeller);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->create_account),
      pDlg->ptest->tpc.a.fCreateAccount);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->create_history),
      pDlg->ptest->tpc.a.fCreateHistory);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->load_branch),
      pDlg->ptest->tpc.a.fLoadBranch);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->load_teller),
      pDlg->ptest->tpc.a.fLoadTeller);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->load_account),
      pDlg->ptest->tpc.a.fLoadAccount);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->create_indexes),
      pDlg->ptest->tpc.a.fCreateIndex);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pDlg->create_procedures),
      pDlg->ptest->tpc.a.fCreateProcedure);

  sprintf (pDlg->ptest->szTemp, "%lu", pDlg->ptest->tpc.a.udwMaxBranch);
  gtk_entry_set_text (GTK_ENTRY (pDlg->num_branches), pDlg->ptest->szTemp);
  sprintf (pDlg->ptest->szTemp, "%lu", pDlg->ptest->tpc.a.udwMaxTeller);
  gtk_entry_set_text (GTK_ENTRY (pDlg->num_tellers), pDlg->ptest->szTemp);
  sprintf (pDlg->ptest->szTemp, "%lu", pDlg->ptest->tpc.a.udwMaxAccount);
  gtk_entry_set_text (GTK_ENTRY (pDlg->num_accounts), pDlg->ptest->szTemp);

  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pDlg->dbmstype)->entry),
      getDriverDBMSName (pDlg->ptest->tpc.a.uwDrvIdx));

  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pDlg->branch_dsn)->entry),
      pDlg->ptest->tpc.a.szBranchDSN[0] ? pDlg->ptest->tpc.
      a.szBranchDSN : "");
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pDlg->teller_dsn)->entry),
      pDlg->ptest->tpc.a.szTellerDSN[0] ? pDlg->ptest->tpc.
      a.szTellerDSN : "");
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pDlg->account_dsn)->entry),
      pDlg->ptest->tpc.a.szAccountDSN[0] ? pDlg->ptest->tpc.
      a.szAccountDSN : "");
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (pDlg->history_dsn)->entry),
      pDlg->ptest->tpc.a.szHistoryDSN[0] ? pDlg->ptest->tpc.
      a.szHistoryDSN : "");
}

void
TPCATableProps_save_config (TPCATableProps * pDlg)
{
  int i;

  pDlg->ptest->is_dirty = TRUE;
  pDlg->ptest->tpc.a.fCreateBranch =
      GTK_TOGGLE_BUTTON (pDlg->create_branch)->active;
  pDlg->ptest->tpc.a.fCreateTeller =
      GTK_TOGGLE_BUTTON (pDlg->create_teller)->active;
  pDlg->ptest->tpc.a.fCreateAccount =
      GTK_TOGGLE_BUTTON (pDlg->create_account)->active;
  pDlg->ptest->tpc.a.fCreateHistory =
      GTK_TOGGLE_BUTTON (pDlg->create_history)->active;

  pDlg->ptest->tpc.a.fLoadBranch =
      GTK_TOGGLE_BUTTON (pDlg->load_branch)->active;
  pDlg->ptest->tpc.a.fLoadTeller =
      GTK_TOGGLE_BUTTON (pDlg->load_teller)->active;
  pDlg->ptest->tpc.a.fLoadAccount =
      GTK_TOGGLE_BUTTON (pDlg->load_account)->active;

  pDlg->ptest->tpc.a.fCreateIndex =
      GTK_TOGGLE_BUTTON (pDlg->create_indexes)->active;
  pDlg->ptest->tpc.a.fCreateProcedure =
      GTK_TOGGLE_BUTTON (pDlg->create_procedures)->active;

  pDlg->ptest->tpc.a.udwMaxBranch =
      atoi (gtk_entry_get_text (GTK_ENTRY (pDlg->num_branches)));
  pDlg->ptest->tpc.a.udwMaxTeller =
      atoi (gtk_entry_get_text (GTK_ENTRY (pDlg->num_tellers)));
  pDlg->ptest->tpc.a.udwMaxAccount =
      atoi (gtk_entry_get_text (GTK_ENTRY (pDlg->num_accounts)));

  strcpy (pDlg->ptest->szTemp,
      gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (pDlg->dbmstype)->entry)));
  pDlg->ptest->tpc.a.uwDrvIdx = 16;
  for (i = 0; i < getDriverMapSize (); i++)
    if (!strcmp (pDlg->ptest->szTemp, getDriverDBMSName (i)))
      {
	pDlg->ptest->tpc.a.uwDrvIdx = i;
	break;
      }

  pDlg->ptest->tpc.a.szBranchDSN[0] = pDlg->ptest->tpc.a.szBranchDBMS[0] =
      pDlg->ptest->tpc.a.szAccountDSN[0] =
      pDlg->ptest->tpc.a.szAccountDBMS[0] =
      pDlg->ptest->tpc.a.szTellerDSN[0] = pDlg->ptest->tpc.a.szTellerDBMS[0] =
      pDlg->ptest->tpc.a.szHistoryDSN[0] =
      pDlg->ptest->tpc.a.szHistoryDBMS[0] = 0;
  if (pDlg->pDSN->dsn_info.dsns)
    {
      GList *found;

      strcpy (pDlg->ptest->tpc.a.szBranchDSN,
	  gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (pDlg->
		      branch_dsn)->entry)));

      if (!strcmp (pDlg->ptest->tpc.a.szBranchDSN, "<local>"))
	pDlg->ptest->tpc.a.szBranchDSN[0] = 0;
      else
	  if (NULL != (found = g_list_find_custom (pDlg->pDSN->dsn_info.dsns,
		 pDlg->ptest->tpc.a.szBranchDSN, (GCompareFunc) strcmp)))
	strncpy (pDlg->ptest->tpc.a.szBranchDBMS,
	    (char *) g_list_nth_data (pDlg->pDSN->dsn_info.names,
g_list_position (pDlg->pDSN->dsn_info.dsns, found)), 50);

      strcpy (pDlg->ptest->tpc.a.szTellerDSN,
	  gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (pDlg->
		      teller_dsn)->entry)));
      if (!strcmp (pDlg->ptest->tpc.a.szTellerDSN, "<local>"))
	pDlg->ptest->tpc.a.szTellerDSN[0] = 0;
      else
	  if (NULL != (found = g_list_find_custom (pDlg->pDSN->dsn_info.dsns,
		 pDlg->ptest->tpc.a.szTellerDSN, (GCompareFunc) strcmp)))
	strncpy (pDlg->ptest->tpc.a.szTellerDBMS,
	    (char *) g_list_nth_data (pDlg->pDSN->dsn_info.names,
g_list_position (pDlg->pDSN->dsn_info.dsns, found)), 50);

      strcpy (pDlg->ptest->tpc.a.szAccountDSN,
	  gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (pDlg->
		      account_dsn)->entry)));

      if (!strcmp (pDlg->ptest->tpc.a.szAccountDSN, "<local>"))
	pDlg->ptest->tpc.a.szAccountDSN[0] = 0;
      else
	  if (NULL != (found = g_list_find_custom (pDlg->pDSN->dsn_info.dsns,
		 pDlg->ptest->tpc.a.szAccountDSN, (GCompareFunc) strcmp)))
	strncpy (pDlg->ptest->tpc.a.szAccountDBMS,
	    (char *) g_list_nth_data (pDlg->pDSN->dsn_info.names,
g_list_position (pDlg->pDSN->dsn_info.dsns, found)), 50);

      strcpy (pDlg->ptest->tpc.a.szHistoryDSN,
	  gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (pDlg->
		      history_dsn)->entry)));

      if (!strcmp (pDlg->ptest->tpc.a.szHistoryDSN, "<local>"))
	pDlg->ptest->tpc.a.szHistoryDSN[0] = 0;
      else
	  if (NULL != (found = g_list_find_custom (pDlg->pDSN->dsn_info.dsns,
		 pDlg->ptest->tpc.a.szHistoryDSN, (GCompareFunc) strcmp)))
	strncpy (pDlg->ptest->tpc.a.szHistoryDBMS,
	    (char *) g_list_nth_data (pDlg->pDSN->dsn_info.names,
g_list_position (pDlg->pDSN->dsn_info.dsns, found)), 50);
    }
}

GtkWidget *
TPCATableProps_new (test_t * ptest)
{
  TPCATableProps *newdlg =
      TPCA_TABLE_PROPS (gtk_type_new (TPCATableProps_get_type ()));
  newdlg->ptest = ptest;
  make_sub_controls (newdlg);
  load_config (newdlg);
  return (GTK_WIDGET (newdlg));
}
