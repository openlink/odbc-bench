/*
 *  results.c
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
#include <stdio.h>

#include "odbcbench.h"
#include "LoginBox.h"
#include "util.h"
#include "results.h"

void
do_results_logout (GtkWidget * widget, gpointer data)
{
  results_logout();
}

static void
results_login_gtk (GtkWidget * widget, gpointer data)
{
  LoginBox *box = LOGINBOX (widget);

  results_logout();
  results_login(box->szDSN, box->szUID, box->szPWD);
}

void
do_results_login (GtkWidget * widget, gpointer data)
{
  GtkWidget *box;

  box = LoginBox_new ("Results connect", get_dsn_list (), NULL, NULL, NULL);
  gtk_signal_connect (GTK_OBJECT (box), "do_the_work",
      GTK_SIGNAL_FUNC (results_login_gtk), NULL);
  gtk_widget_show (box);
}

void
do_create_results_table (GtkWidget * widget, gpointer data)
{
  create_results_table();
}

void
do_drop_results_table (GtkWidget * widget, gpointer data)
{
  drop_results_table();
}


