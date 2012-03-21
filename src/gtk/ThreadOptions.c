/*
 *  ThreadOptions.c
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
#include "thr.h"
#include "TPCARunProps.h"
#include "ThreadOptions.h"

static void ThreadOptions_class_init (ThreadOptionsClass * tclass);
static void ThreadOptions_init (ThreadOptions * tableloader);


int
ThreadOptions_get_type (void)
{
  static guint tld_type = 0;

  if (!tld_type)
    {
      GtkTypeInfo tld_info = {
	"ThreadOptions",
	sizeof (ThreadOptions),
	sizeof (ThreadOptionsClass),
	(GtkClassInitFunc) ThreadOptions_class_init,
	(GtkObjectInitFunc) ThreadOptions_init,
	NULL,
	NULL
      };

      tld_type = gtk_type_unique (gtk_vbox_get_type (), &tld_info);
    }

  return tld_type;
}


static void
ThreadOptions_class_init (ThreadOptionsClass * tclass)
{
}


GtkWidget *
ThreadOptions_new (void)
{
#if defined(PTHREADS) || defined(WIN32)
  ThreadOptions *newdlg =
      THREAD_OPTIONS (gtk_type_new (ThreadOptions_get_type ()));
  return (GTK_WIDGET (newdlg));
#else
  return NULL;
#endif
}


void
ThreadOptions_load_config (ThreadOptions * newdlg, test_common_t * test)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (newdlg->single_threaded),
      test->nThreads == 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (newdlg->multi_threaded),
      test->nThreads > 0);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_SPIN_BUTTON
	  (newdlg->no_threads)->adjustment),
      test->nThreads < 2 ? 2 : test->nThreads);
  if (!test->nThreads)
    disable_widget (newdlg->no_threads);
  else
    enable_widget (newdlg->no_threads);
  newdlg->test = test;
}


void
ThreadOptions_save_config (ThreadOptions * dlg)
{
  g_assert (dlg->test);
#if defined(PTHREADS) || defined(WIN32)
  dlg->test->nThreads =
      GTK_TOGGLE_BUTTON (dlg->single_threaded)->active ?
      0 :
      (int) GTK_ADJUSTMENT (GTK_SPIN_BUTTON (dlg->no_threads)->adjustment)->
      value;
#else
  dlg->test->nThreads = 0;
#endif
}


static void
ThreadOptions_init (ThreadOptions * dlg)
{
#if defined(PTHREADS) || defined(WIN32)
  GtkWidget *Frame, *helper, *helper1, *label;
  GSList *radio;
  GtkObject *spin;

  Frame = gtk_frame_new ("Threading options");
  gtk_widget_show (Frame);
  gtk_box_pack_start (GTK_BOX (dlg), Frame, TRUE, TRUE, 0);

  helper = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (helper);
  gtk_container_add (GTK_CONTAINER (Frame), helper);

  radio = 0;

  dlg->single_threaded =
      gtk_radio_button_new_with_label (radio, "Single Thread");
  gtk_widget_show (dlg->single_threaded);
  gtk_box_pack_start (GTK_BOX (helper), dlg->single_threaded, TRUE, TRUE, 0);

  radio = gtk_radio_button_group (GTK_RADIO_BUTTON (dlg->single_threaded));

  dlg->multi_threaded =
      gtk_radio_button_new_with_label (radio, "Multiple Threads");
  gtk_widget_show (dlg->multi_threaded);
  gtk_box_pack_start (GTK_BOX (helper), dlg->multi_threaded, TRUE, TRUE, 0);

  helper1 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (helper1);
  gtk_box_pack_start (GTK_BOX (helper), helper1, TRUE, TRUE, 0);

  label = gtk_label_new ("No Threads");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (helper1), label, TRUE, TRUE, 0);

  spin = gtk_adjustment_new (2, 2, 50000, 1, 10, 10);
  dlg->no_threads = gtk_spin_button_new (GTK_ADJUSTMENT (spin), 0.5, 0);
  gtk_box_pack_start (GTK_BOX (helper1), dlg->no_threads, TRUE, TRUE, 0);

  gtk_signal_connect_object (GTK_OBJECT (dlg->multi_threaded), "clicked",
      GTK_SIGNAL_FUNC (enable_widget), GTK_OBJECT (dlg->no_threads));
  gtk_signal_connect_object (GTK_OBJECT (dlg->single_threaded), "clicked",
      GTK_SIGNAL_FUNC (disable_widget), GTK_OBJECT (dlg->no_threads));
#endif
}
