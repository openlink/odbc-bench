/*
 *  TPCARunProps.c
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
#include "ArrayParams.h"
#include "ThreadOptions.h"
#include "TPCARunProps.h"
#include "tpca_code.h"

static char *txn_isolation_names[] = {
  "Driver Default",
  "Read uncommitted",
  "Read committed",
  "Repeatable read",
  "Serializable"
};

static char *cursor_type_names[] = {
  "None (forward only)",
  "Static",
  "Keyset Driven",
  "Dynamic",
  "Mixed"
};

static void TPCARunProps_class_init (TPCARunPropsClass * tclass);
static void TPCARunProps_init (TPCARunProps * tableloader);

int
TPCARunProps_get_type (void)
{
  static guint tld_type = 0;

  if (!tld_type)
    {
      GtkTypeInfo tld_info = {
	"TPCARunProps",
	sizeof (TPCARunProps),
	sizeof (TPCARunPropsClass),
	(GtkClassInitFunc) TPCARunProps_class_init,
	(GtkObjectInitFunc) TPCARunProps_init,
	NULL,
	NULL
      };

      tld_type = gtk_type_unique (gtk_vbox_get_type (), &tld_info);
    }

  return tld_type;
}

static void
TPCARunProps_class_init (TPCARunPropsClass * tclass)
{
}


void
enable_widget (GtkWidget * widget)
{
  gtk_widget_set_sensitive (widget, TRUE);
}


void
disable_widget (GtkWidget * widget)
{
  gtk_widget_set_sensitive (widget, FALSE);
}


static void
load_config (TPCARunProps * dlg)
{
  int index;

  switch (dlg->lpBench->tpc.a.fSQLOption)
    {
    case IDX_PLAINSQL:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->execdirect),
	  TRUE);
      break;

    case IDX_PARAMS:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->prepare_execute),
	  TRUE);
      break;

    case IDX_SPROCS:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->procedures),
	  TRUE);
      break;
    }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->async),
      dlg->lpBench->tpc.a.fExecAsync);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->trans),
      dlg->lpBench->tpc.a.fUseCommit);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->do100row),
      dlg->lpBench->tpc.a.fDoQuery);

  ArrayParams_load_config (ARRAY_PARAMS (dlg->array_params),
      &dlg->lpBench->tpc.a);

  ThreadOptions_load_config (THREAD_OPTIONS (dlg->thread_opts),
      &dlg->lpBench->tpc._);

  switch (dlg->lpBench->tpc.a.nCursorType)
    {
    default:
    case SQL_CURSOR_FORWARD_ONLY:
      index = 0;
      break;
    case SQL_CURSOR_STATIC:
      index = 1;
      break;
    case SQL_CURSOR_KEYSET_DRIVEN:
      index = 2;
      break;
    case SQL_CURSOR_DYNAMIC:
      index = 3;
      break;
    case SQL_CURSOR_MIXED:
      index = 4;
      break;
    }
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->cursor_modes[index]),
      TRUE);
  if (index)
    {
      enable_widget (dlg->rowset_size);
      enable_widget (dlg->traversal_count);
    }
  else
    {
      disable_widget (dlg->rowset_size);
      disable_widget (dlg->traversal_count);
    }
  if (index == 4)
    enable_widget (dlg->keyset_size);
  else
    disable_widget (dlg->keyset_size);

  switch (dlg->lpBench->tpc.a.txn_isolation)
    {
    default:
    case SQL_TXN_DRIVER_DEFAULT:
      index = 0;
      break;
    case SQL_TXN_READ_UNCOMMITTED:
      index = 1;
      break;
    case SQL_TXN_READ_COMMITTED:
      index = 2;
      break;
    case SQL_TXN_REPEATABLE_READ:
      index = 3;
      break;
    case SQL_TXN_SERIALIZABLE:
      index = 4;
      break;
    }
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->isolation_levels
	  [index]), TRUE);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_SPIN_BUTTON
	  (dlg->rowset_size)->adjustment), dlg->lpBench->tpc.a.nRowsetSize);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_SPIN_BUTTON
	  (dlg->keyset_size)->adjustment), dlg->lpBench->tpc.a.nKeysetSize);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_SPIN_BUTTON
	  (dlg->traversal_count)->adjustment),
      dlg->lpBench->tpc.a.nTraversalCount);
}


GtkWidget *
TPCARunProps_new (test_t * lpBench)
{
  TPCARunProps *newdlg =
      TPCA_RUN_PROPS (gtk_type_new (TPCARunProps_get_type ()));
  newdlg->lpBench = lpBench;
  load_config (newdlg);
  return (GTK_WIDGET (newdlg));
}


void
TPCARunProps_save_config (TPCARunProps * dlg)
{
  unsigned int index;

  memset (dlg->lpBench->szSQLError, 0, sizeof (dlg->lpBench->szSQLError));

  if (GTK_TOGGLE_BUTTON (dlg->execdirect)->active)
    dlg->lpBench->tpc.a.fSQLOption = IDX_PLAINSQL;
  else if (GTK_TOGGLE_BUTTON (dlg->prepare_execute)->active)
    dlg->lpBench->tpc.a.fSQLOption = IDX_PARAMS;
  else if (GTK_TOGGLE_BUTTON (dlg->procedures)->active)
    dlg->lpBench->tpc.a.fSQLOption = IDX_SPROCS;

  dlg->lpBench->tpc.a.fExecAsync = GTK_TOGGLE_BUTTON (dlg->async)->active;
  dlg->lpBench->tpc.a.fUseCommit = GTK_TOGGLE_BUTTON (dlg->trans)->active;
  dlg->lpBench->tpc.a.fDoQuery = GTK_TOGGLE_BUTTON (dlg->do100row)->active;

  ArrayParams_save_config (ARRAY_PARAMS (dlg->array_params));
  ThreadOptions_save_config (THREAD_OPTIONS (dlg->thread_opts));

  for (index = 0;
      index < sizeof (dlg->isolation_levels) / sizeof (GtkWidget *); index++)
    {
      if (GTK_TOGGLE_BUTTON (dlg->isolation_levels[index])->active)
	{
	  switch (index)
	    {
	    case 0:
	      dlg->lpBench->tpc.a.txn_isolation = SQL_TXN_DRIVER_DEFAULT;
	      break;
	    case 1:
	      dlg->lpBench->tpc.a.txn_isolation = SQL_TXN_READ_UNCOMMITTED;
	      break;
	    case 2:
	      dlg->lpBench->tpc.a.txn_isolation = SQL_TXN_READ_COMMITTED;
	      break;
	    case 3:
	      dlg->lpBench->tpc.a.txn_isolation = SQL_TXN_REPEATABLE_READ;
	      break;
	    case 4:
	      dlg->lpBench->tpc.a.txn_isolation = SQL_TXN_SERIALIZABLE;
	      break;
	    }
	  break;
	}
    }
  for (index = 0; index < sizeof (dlg->cursor_modes) / sizeof (GtkWidget *);
      index++)
    {
      if (GTK_TOGGLE_BUTTON (dlg->cursor_modes[index])->active)
	{
	  switch (index)
	    {
	    case 0:
	      dlg->lpBench->tpc.a.nCursorType = SQL_CURSOR_FORWARD_ONLY;
	      break;
	    case 1:
	      dlg->lpBench->tpc.a.nCursorType = SQL_CURSOR_STATIC;
	      break;
	    case 2:
	      dlg->lpBench->tpc.a.nCursorType = SQL_CURSOR_KEYSET_DRIVEN;
	      break;
	    case 3:
	      dlg->lpBench->tpc.a.nCursorType = SQL_CURSOR_DYNAMIC;
	      break;
	    case 4:
	      dlg->lpBench->tpc.a.nCursorType = SQL_CURSOR_MIXED;
	      break;
	    }
	  break;
	}
    }

  dlg->lpBench->tpc.a.nRowsetSize =
      (short) GTK_ADJUSTMENT (GTK_SPIN_BUTTON (dlg->rowset_size)->adjustment)->value;
  dlg->lpBench->tpc.a.nTraversalCount =
      (short) GTK_ADJUSTMENT (GTK_SPIN_BUTTON (dlg->traversal_count)->
      adjustment)->value;
}


static void
do100_signal_handler (GtkWidget * widget, gpointer data)
{
  gtk_widget_set_sensitive (GTK_WIDGET (data),
      GTK_TOGGLE_BUTTON (widget)->active);
}


static void
adjust_keyset_size_as_rowset_size (GtkWidget * _rowset_adj,
    gpointer _keyset_adj)
{
  GtkAdjustment *rowset_adj = GTK_ADJUSTMENT (_rowset_adj), *keyset_adj =
      GTK_ADJUSTMENT (_keyset_adj);
  keyset_adj->lower = rowset_adj->value;
  gtk_signal_emit_by_name (GTK_OBJECT (keyset_adj), "changed");

  if (rowset_adj->value > keyset_adj->value)
    {
      keyset_adj->value = rowset_adj->value;
      gtk_signal_emit_by_name (GTK_OBJECT (keyset_adj), "value_changed");
    }
}


static void
TPCARunProps_init (TPCARunProps * dlg)
{
  /* the original */
  GtkWidget *master_box, *left_column, *right_column;
  GtkWidget *Frame, *table;
  GtkWidget *helper, *label, *helper1;
  GSList *radio;
  GtkObject *spin, *spin1;
  unsigned int i;

  gtk_container_set_border_width (GTK_CONTAINER (dlg), 10);

  master_box = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (master_box);
  gtk_container_add (GTK_CONTAINER (dlg), master_box);

  left_column = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (left_column);
  gtk_box_pack_start (GTK_BOX (master_box), left_column, TRUE, TRUE, 0);

  dlg->thread_opts = ThreadOptions_new ();
  gtk_box_pack_start (GTK_BOX (left_column), dlg->thread_opts, TRUE, TRUE, 0);

  dlg->array_params = ArrayParams_new ();
  gtk_box_pack_start (GTK_BOX (left_column), dlg->array_params, TRUE, TRUE, 0);

  Frame = gtk_frame_new ("SQL Options");
  gtk_widget_show (Frame);
  gtk_box_pack_start (GTK_BOX (left_column), Frame, TRUE, TRUE, 0);

  helper = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (helper);
  gtk_container_add (GTK_CONTAINER (Frame), helper);

  radio = 0;

  dlg->execdirect =
      gtk_radio_button_new_with_label (radio, "ExecDirect with SQL text");
  gtk_widget_show (dlg->execdirect);
  gtk_box_pack_start (GTK_BOX (helper), dlg->execdirect, TRUE, TRUE, 0);

  radio = gtk_radio_button_group (GTK_RADIO_BUTTON (dlg->execdirect));

  dlg->prepare_execute =
      gtk_radio_button_new_with_label (radio,
      "Prepare/Execute, bound parameters");
  gtk_widget_show (dlg->prepare_execute);
  gtk_box_pack_start (GTK_BOX (helper), dlg->prepare_execute, TRUE, TRUE, 0);

  radio = gtk_radio_button_group (GTK_RADIO_BUTTON (dlg->prepare_execute));

  dlg->procedures =
      gtk_radio_button_new_with_label (radio, "Use Stored Procedures");
  gtk_widget_show (dlg->procedures);
  gtk_box_pack_start (GTK_BOX (helper), dlg->procedures, TRUE, TRUE, 0);


  Frame = gtk_frame_new ("Execution Options");
  gtk_widget_show (Frame);
  gtk_box_pack_start (GTK_BOX (left_column), Frame, TRUE, TRUE, 0);

  helper = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (helper);
  gtk_container_add (GTK_CONTAINER (Frame), helper);

  dlg->async = gtk_check_button_new_with_label ("Asynchronous");
  gtk_widget_show (dlg->async);
  gtk_box_pack_start (GTK_BOX (helper), dlg->async, TRUE, TRUE, 0);

  dlg->trans = gtk_check_button_new_with_label ("Use Transactions");
  gtk_widget_show (dlg->trans);
  gtk_box_pack_start (GTK_BOX (helper), dlg->trans, TRUE, TRUE, 0);

  dlg->do100row = gtk_check_button_new_with_label ("Do 100 row query");
  gtk_widget_show (dlg->do100row);
  gtk_box_pack_start (GTK_BOX (helper), dlg->do100row, TRUE, TRUE, 0);

  /* right column */
  right_column = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (master_box), right_column, TRUE, TRUE, 0);

  Frame = gtk_frame_new ("Transaction Isolation Levels");
  gtk_widget_show (Frame);
  gtk_box_pack_start (GTK_BOX (right_column), Frame, TRUE, TRUE, 0);

  helper = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (helper);
  gtk_container_add (GTK_CONTAINER (Frame), helper);

  for (i = 0, radio = NULL;
      i < sizeof (dlg->isolation_levels) / sizeof (GtkWidget *); i++)
    {
      dlg->isolation_levels[i] =
	  gtk_radio_button_new_with_label (radio, txn_isolation_names[i]);
      gtk_box_pack_start (GTK_BOX (helper), dlg->isolation_levels[i], TRUE,
	  TRUE, 0);
      radio =
	  gtk_radio_button_group (GTK_RADIO_BUTTON (dlg->isolation_levels
	  [i]));
    }

  Frame = gtk_frame_new ("Scrollable Cursors");
  gtk_widget_show (Frame);
  gtk_widget_set_sensitive (Frame, FALSE);
  gtk_box_pack_start (GTK_BOX (right_column), Frame, TRUE, TRUE, 0);

  gtk_signal_connect (GTK_OBJECT (dlg->do100row), "clicked",
      GTK_SIGNAL_FUNC (do100_signal_handler), Frame);
  helper1 = gtk_vbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (Frame), helper1);

  helper = gtk_vbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (helper1), helper, TRUE, TRUE, 0);

  for (i = 0, radio = NULL;
      i < sizeof (dlg->cursor_modes) / sizeof (GtkWidget *); i++)
    {
      dlg->cursor_modes[i] =
	  gtk_radio_button_new_with_label (radio, cursor_type_names[i]);
      gtk_box_pack_start (GTK_BOX (helper), dlg->cursor_modes[i], TRUE, TRUE,
	  0);
      radio =
	  gtk_radio_button_group (GTK_RADIO_BUTTON (dlg->cursor_modes[i]));
    }
  table = gtk_table_new (3, 2, TRUE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_end (GTK_BOX (helper1), table, TRUE, TRUE, 10);

  label = gtk_label_new ("Rowset size");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  spin1 = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
  dlg->rowset_size = gtk_spin_button_new (GTK_ADJUSTMENT (spin1), 0.5, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), dlg->rowset_size, 1, 2, 0, 1);

  label = gtk_label_new ("Keyset size");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

  spin = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
  dlg->keyset_size = gtk_spin_button_new (GTK_ADJUSTMENT (spin), 0.5, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), dlg->keyset_size, 1, 2, 1, 2);
  gtk_signal_connect (GTK_OBJECT (spin1), "value_changed",
      GTK_SIGNAL_FUNC (adjust_keyset_size_as_rowset_size), spin);

  label = gtk_label_new ("Traversal Count");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);

  spin = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
  dlg->traversal_count = gtk_spin_button_new (GTK_ADJUSTMENT (spin), 0.5, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), dlg->traversal_count, 1, 2, 2,
      3);

  for (i = 0; i < sizeof (dlg->cursor_modes) / sizeof (GtkWidget *); i++)
    {
      enable_widget (dlg->cursor_modes[i]);
      gtk_signal_connect_object (GTK_OBJECT (dlg->cursor_modes[i]),
	  "clicked",
	  GTK_SIGNAL_FUNC ((i == 0 ? disable_widget : enable_widget)),
	  GTK_OBJECT (dlg->rowset_size));

      gtk_signal_connect_object (GTK_OBJECT (dlg->cursor_modes[i]),
	  "clicked",
	  GTK_SIGNAL_FUNC ((i == 4 ? enable_widget : disable_widget)),
	  GTK_OBJECT (dlg->keyset_size));

      gtk_signal_connect_object (GTK_OBJECT (dlg->cursor_modes[i]),
	  "clicked",
	  GTK_SIGNAL_FUNC ((i == 0 ? disable_widget : enable_widget)),
	  GTK_OBJECT (dlg->traversal_count));
    }
  for (i = 0; i < sizeof (dlg->isolation_levels) / sizeof (GtkWidget *); i++)
    enable_widget (dlg->isolation_levels[i]);

  /* action_area 
     helper = gtk_button_new_with_label ("OK");
     GTK_WIDGET_SET_FLAGS(helper, GTK_CAN_DEFAULT);
     gtk_window_set_default(GTK_WINDOW(dlg), helper);
     gtk_signal_connect (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (emit_signal_handler), GTK_OBJECT (dlg));
     gtk_signal_connect_object (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
     gtk_signal_connect (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (run_one), GTK_OBJECT (dlg));
     gtk_signal_connect_object (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
     gtk_widget_show (helper);
     gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), helper, TRUE, TRUE, 0);

     helper = gtk_button_new_with_label ("Cancel");
     gtk_signal_connect_object (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
     gtk_widget_show (helper);
     gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), helper, TRUE, TRUE, 0);

     helper = gtk_button_new_with_label ("Run All");
     gtk_signal_connect (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (emit_signal_handler), GTK_OBJECT (dlg));
     gtk_signal_connect_object (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dlg));
     gtk_signal_connect (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (do_run_all), GTK_OBJECT(dlg));
     gtk_signal_connect_object (GTK_OBJECT (helper), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (dlg));
     gtk_widget_show (helper);
     gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), helper, TRUE, TRUE, 0);
   */
}
