/*
 *  ArrayParams.c
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
#include "tpca.h"
#include "TPCARunProps.h"
#include "ArrayParams.h"

static void ArrayParams_class_init (ArrayParamsClass * aclass);
static void ArrayParams_init (ArrayParams * tableloader);

int
ArrayParams_get_type (void)
{
  static guint tld_type = 0;

  if (!tld_type)
    {
      GtkTypeInfo tld_info = {
	"ArrayParams",
	sizeof (ArrayParams),
	sizeof (ArrayParamsClass),
	(GtkClassInitFunc) ArrayParams_class_init,
	(GtkObjectInitFunc) ArrayParams_init,
	NULL,
	NULL
      };

      tld_type = gtk_type_unique (gtk_vbox_get_type (), &tld_info);
    }

  return tld_type;
}

static void
ArrayParams_class_init (ArrayParamsClass * aclass)
{
}


GtkWidget *
ArrayParams_new (void)
{
  ArrayParams *newdlg =
      ARRAY_PARAMS (gtk_type_new (ArrayParams_get_type ()));
  return (GTK_WIDGET (newdlg));
}


void
ArrayParams_load_config (ArrayParams * newdlg, tpca_t * test)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (newdlg->use_ArrayParams),
       test->nArrayParSize > 0); 
  gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_SPIN_BUTTON(newdlg->no_params)->adjustment),
      test->nArrayParSize < 10 ? 10 : test->nArrayParSize);
  if (!test->nArrayParSize)
    disable_widget (newdlg->no_params);
  else
    enable_widget (newdlg->no_params);
  newdlg->test = test;
}


void
ArrayParams_save_config (ArrayParams * dlg)
{
  g_assert (dlg->test);
  dlg->test->nArrayParSize =
      !GTK_TOGGLE_BUTTON (dlg->use_ArrayParams)->active ?
      0 :
      (int) GTK_ADJUSTMENT (GTK_SPIN_BUTTON (dlg->no_params)->adjustment)->value;
}


static void
emit_signal_handler (GtkWidget * widget, gpointer data)
{
  ArrayParams * dlg = ARRAY_PARAMS(data);

  if (GTK_TOGGLE_BUTTON (dlg->use_ArrayParams)->active)
    enable_widget (dlg->no_params);
  else
    disable_widget (dlg->no_params);
}



static void
ArrayParams_init (ArrayParams * dlg)
{
  GtkWidget *Frame, *helper, *helper1, *label;
  GtkObject *spin;

  Frame = gtk_frame_new ("Array Optimization");
  gtk_widget_show (Frame);
  gtk_box_pack_start (GTK_BOX (dlg), Frame, TRUE, TRUE, 0);

  helper = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (helper);
  gtk_container_add (GTK_CONTAINER (Frame), helper);

  dlg->use_ArrayParams = gtk_check_button_new_with_label ("Use Array Parameters");
  gtk_widget_show (dlg->use_ArrayParams);
  gtk_box_pack_start (GTK_BOX (helper), dlg->use_ArrayParams, TRUE, TRUE, 0);

  helper1 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (helper1);
  gtk_box_pack_start (GTK_BOX (helper), helper1, TRUE, TRUE, 0);

  label = gtk_label_new ("No Array Params");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (helper1), label, TRUE, TRUE, 0);

  spin = gtk_adjustment_new (100, 10, 50000, 1, 10, 10);
  dlg->no_params = gtk_spin_button_new (GTK_ADJUSTMENT (spin), 0.5, 0);
  gtk_box_pack_start (GTK_BOX (helper1), dlg->no_params, TRUE, TRUE, 0);

  gtk_signal_connect (GTK_OBJECT (dlg->use_ArrayParams), "clicked",
      GTK_SIGNAL_FUNC (emit_signal_handler), dlg);
}
