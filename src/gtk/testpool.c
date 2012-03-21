/*
 *  testpool.c
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
#include <stdio.h>
#include <string.h>

#include "odbcbench.h"
#include "odbcbench_gtk.h"
#include "testpool.h"

typedef struct test_file_s
{
  char *szCurrentFileName;
  GtkWidget *conn_pool;
  gboolean global_is_dirty;
  GtkWidget *radio_menu_item, *label, *status;
} test_file_t;

static GList *files = NULL;
static test_file_t *curr;
extern GtkWidget *windows, *dlg, *scrolled, *global_status, *status_box, *status;	/* windows menu */
static GtkWidget *last_radio = NULL;

static gchar *column_names[] = {
  "Type",
  "Name",
  "DSN",
  "Driver name",
  "Driver version",
  "DBMS"
};

static GtkWidget *hidden = NULL;


static void
set_active_item (GtkWidget * menuitem, gpointer data)
{
  test_file_t *sel = (test_file_t *) data;
  char *szFileName =
      g_strdup (sel->szCurrentFileName ? sel->szCurrentFileName : "Untitled");

  if (!hidden)
    {
      hidden = gtk_vbox_new (FALSE, 0);
    }
  if (curr)
    {
      gtk_widget_reparent (curr->conn_pool, hidden);
      gtk_widget_reparent (curr->status, hidden);
    }
  else
    gtk_widget_reparent (global_status, hidden);

  gtk_widget_reparent (sel->conn_pool, scrolled);
  gtk_widget_reparent (sel->status, status_box);
  gtk_widget_show (sel->conn_pool);
  curr = sel;
  status = sel->status;
  setTheFileName (szFileName);
}


GtkWidget *
create_test_pool ()
{
  GSList *windows_radio =
      last_radio ?
      gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (last_radio)) : NULL;
  test_file_t *t_new = (test_file_t *) g_malloc (sizeof (test_file_t));
  memset (t_new, 0, sizeof (test_file_t));
  t_new->conn_pool = gtk_clist_new_with_titles (6, column_names);
  gtk_clist_set_selection_mode (GTK_CLIST (t_new->conn_pool),
      GTK_SELECTION_MULTIPLE);
  gtk_clist_set_shadow_type (GTK_CLIST (t_new->conn_pool),
      GTK_SHADOW_ETCHED_IN);
  gtk_clist_column_titles_passive (GTK_CLIST (t_new->conn_pool));
  gtk_clist_column_titles_show (GTK_CLIST (t_new->conn_pool));

  files = g_list_append (files, t_new);
  if (dlg)
    {
      if (!hidden)
	{
	  hidden = gtk_vbox_new (FALSE, 0);
	}

      if (curr)
	{
	  gtk_widget_reparent (curr->conn_pool, hidden);
	  gtk_widget_reparent (curr->status, hidden);
	}
      else
	gtk_widget_reparent (status, hidden);
      gtk_container_add (GTK_CONTAINER (scrolled), t_new->conn_pool);
      gtk_widget_show (t_new->conn_pool);
    }
  curr = t_new;
  if (dlg)
    {
      create_status_widget (dlg);
      curr->status = status;
      gtk_container_add (GTK_CONTAINER (status_box), curr->status);

      curr->radio_menu_item = gtk_radio_menu_item_new (windows_radio);
      last_radio = curr->radio_menu_item;
      curr->label = gtk_label_new ("Untitled");
      gtk_signal_connect (GTK_OBJECT (curr->radio_menu_item), "activate",
	  GTK_SIGNAL_FUNC (set_active_item), curr);
      gtk_container_add (GTK_CONTAINER (curr->radio_menu_item), curr->label);
      gtk_widget_show (curr->label);
      gtk_widget_show (curr->radio_menu_item);
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
	  (curr->radio_menu_item), TRUE);
      gtk_menu_append (GTK_MENU (windows), curr->radio_menu_item);
    }

  setTheFileName (NULL);
  return curr->conn_pool;
}


int
add_test_to_the_pool (test_t * ptest)
{
  gchar *row[6];
  int nRow;

  switch (ptest->TestType)
    {
    case TPC_A:
      row[0] = "TPC-A";
      break;
    case TPC_C:
      row[0] = "TPC-C";
      break;
    }
  row[1] = ptest->szName;
  row[2] = ptest->szLoginDSN;
  row[3] = ptest->szDriverName;
  row[4] = ptest->szDriverVer;
  row[5] = ptest->szDBMS;

  if (!curr || !curr->conn_pool)
    return 0;

  nRow = gtk_clist_append (GTK_CLIST (curr->conn_pool), row);
  gtk_clist_set_row_data (GTK_CLIST (curr->conn_pool), nRow, ptest);
  gtk_clist_select_row (GTK_CLIST (curr->conn_pool), nRow, 0);
  gtk_clist_columns_autosize (GTK_CLIST (curr->conn_pool));

  return 1;
}


static int
num_sort (gpointer a, gpointer b)
{
  return GPOINTER_TO_INT (b) - GPOINTER_TO_INT (a);
}


void
remove_selected_tests (void)
{
  GList *selection;
  GList *rows = NULL, *iter;
  int i;

  if (!curr || !curr->conn_pool)
    return;

  selection = GTK_CLIST (curr->conn_pool)->selection;

  for (i = 0, selection = GTK_CLIST (curr->conn_pool)->selection; selection;
      selection = g_list_next (selection), i++)
    {
      int nRow = GPOINTER_TO_INT (selection->data);
      rows = g_list_append (rows, GINT_TO_POINTER (nRow));
    }
  rows = g_list_sort (rows, (GCompareFunc) num_sort);
  iter = rows;
  while (iter)
    {
      int nRow = GPOINTER_TO_INT (iter->data);
      test_t *ptest =
	  (test_t *) gtk_clist_get_row_data (GTK_CLIST (curr->conn_pool),
	  nRow);
      gtk_clist_remove (GTK_CLIST (curr->conn_pool), nRow);
      g_free (ptest);
      iter = g_list_next (iter);
    }
  g_list_free (rows);
}


int
pool_connection_count (void)
{
  int nItems = 0;
  GList *selection;

  if (!curr || !curr->conn_pool)
    return 0;

  for (selection = GTK_CLIST (curr->conn_pool)->selection; selection;
      selection = g_list_next (selection))
    nItems++;

  return nItems;
}


GList *
get_selected_tests (void)
{
  GList *selected = NULL, *selection;

  if (!curr)
    return selected;
  for (selection = GTK_CLIST (curr->conn_pool)->selection; selection;
      selection = g_list_next (selection))
    selected =
	g_list_append (selected,
	gtk_clist_get_row_data (GTK_CLIST (curr->conn_pool),
	    GPOINTER_TO_INT (selection->data)));
  return selected;
}


OList *
get_selected_tests_list (void)
{
  OList *selected = NULL;
  GList *selection;

  if (!curr)
    return selected;

  for (selection = GTK_CLIST (curr->conn_pool)->selection; selection;
      selection = g_list_next (selection))
    selected = o_list_append (selected,
	gtk_clist_get_row_data (GTK_CLIST (curr->conn_pool),
	    GPOINTER_TO_INT (selection->data)));
  return selected;
}


void
pool_set_selected_items (GList * list)
{
  GList *iter = list;
  if (!curr)
    return;
  gtk_clist_unselect_all (GTK_CLIST (curr->conn_pool));

  for (iter = list; iter; iter = g_list_next (iter))
    {
      int rowidx = gtk_clist_find_row_from_data (GTK_CLIST (curr->conn_pool),
	  iter->data);
      if (rowidx != -1)
	gtk_clist_select_row (GTK_CLIST (curr->conn_pool), rowidx, -1);
    }
}


void
pool_set_selected_item (test_t * test)
{
  if (curr)
    {
      int rowidx =
	  gtk_clist_find_row_from_data (GTK_CLIST (curr->conn_pool), test);

      if (rowidx == -1)
	return;
      gtk_clist_unselect_all (GTK_CLIST (curr->conn_pool));
      gtk_clist_select_row (GTK_CLIST (curr->conn_pool), rowidx, -1);
    }
}


void
for_all_in_pool (void)
{
  if (!curr)
    return;
  gtk_clist_select_all (GTK_CLIST (curr->conn_pool));
}


void
pool_update_selected (void)
{
  int nRow, i;
  GList *selection;
  test_t *ptest;

  if (!curr)
    return;
  for (i = 0, selection = GTK_CLIST (curr->conn_pool)->selection; selection;
      selection = g_list_next (selection), i++)
    {
      nRow = GPOINTER_TO_INT (selection->data);
      ptest =
	  (test_t *) gtk_clist_get_row_data (GTK_CLIST (curr->conn_pool),
	  nRow);

      gtk_clist_set_text (GTK_CLIST (curr->conn_pool), nRow, 0,
	  ptest->TestType == TPC_A ? "TPC-A" : "TPC-C");
      gtk_clist_set_text (GTK_CLIST (curr->conn_pool), nRow, 1,
	  ptest->szName);
      gtk_clist_set_text (GTK_CLIST (curr->conn_pool), nRow, 2,
	  ptest->szLoginDSN);
      gtk_clist_set_text (GTK_CLIST (curr->conn_pool), nRow, 3,
	  ptest->szDriverName);
      gtk_clist_set_text (GTK_CLIST (curr->conn_pool), nRow, 4,
	  ptest->szDriverVer);
      gtk_clist_set_text (GTK_CLIST (curr->conn_pool), nRow, 5,
	  ptest->szDBMS);
    }
  gtk_clist_columns_autosize (GTK_CLIST (curr->conn_pool));
}


/* file pool interface */
void
setTheFileName (char *filename)
{
  if (curr)
    {
      if (curr->szCurrentFileName)
	g_free (curr->szCurrentFileName);
      curr->szCurrentFileName = filename;

      if (dlg)
	{
	  if (curr->radio_menu_item && curr->szCurrentFileName)
	    {
	      char *szLastSlash = strrchr (filename, '/');
	      if (!szLastSlash)
		szLastSlash = strrchr (filename, '\\');
	      if (!szLastSlash)
		szLastSlash = filename;
	      else
		szLastSlash++;
	      gtk_label_set_text (GTK_LABEL (curr->label), szLastSlash);
	    }
	}
    }

  if (dlg)
    {
      char szBuffer[256], *szLastSlash = NULL;

#ifdef WIN32
      if (filename)
	szLastSlash = strrchr (filename, '\\');
#else
      if (filename)
	szLastSlash = strrchr (filename, '/');
#endif
      if (!szLastSlash)
	szLastSlash = filename;
      else
	szLastSlash++;

      if (!szLastSlash)
	szLastSlash = "Untitled";

      if (curr)
	sprintf (szBuffer, "%.250s - %s", szLastSlash, PACKAGE_NAME);
      else
	sprintf (szBuffer, "%s", PACKAGE_NAME);
      gtk_window_set_title (GTK_WINDOW (dlg), szBuffer);
    }
}


char *
getCurrentFileName (void)
{
  if (curr)
    return curr->szCurrentFileName;
  else
    return NULL;
}


gboolean
getIsFileDirty (void)
{
  if (curr)
    return curr->global_is_dirty;
  else
    return FALSE;
}


void
setIsFileDirty (gboolean isit)
{
  if (curr)
    curr->global_is_dirty = isit;
}


void
close_file_pool (void)
{
  GList *where, *newwhere = NULL;
  char *filename = NULL;
  test_file_t *new_curr = NULL;

  if (!curr)
    return;

  where = g_list_find (files, curr);
  if (!where)
    return;

  newwhere = g_list_previous (where);
  if (newwhere && newwhere->data)
    new_curr = (test_file_t *) newwhere->data;
  if (!new_curr)
    {
      newwhere = g_list_next (where);
      if (newwhere && newwhere->data)
	new_curr = (test_file_t *) newwhere->data;
    }

  files = g_list_remove (files, curr);

  if (curr->szCurrentFileName)
    g_free (curr->szCurrentFileName);
  for_all_in_pool ();
  remove_selected_tests ();
  if (curr)
    {
      gtk_widget_destroy (curr->conn_pool);
      gtk_widget_destroy (curr->status);
    }
  if (new_curr)
    {
      filename =
	  new_curr->
	  szCurrentFileName ? g_strdup (new_curr->szCurrentFileName) : NULL;
      gtk_widget_reparent (new_curr->conn_pool, scrolled);
      gtk_widget_reparent (new_curr->status, status_box);
      /*gtk_container_add (GTK_CONTAINER (scrolled), new_curr->conn_pool); */
    }
  else
    {
      gtk_widget_reparent (global_status, status_box);
    }

  gtk_widget_destroy (curr->label);
  if (last_radio == curr->radio_menu_item)
    {
      if (new_curr)
	last_radio = new_curr->radio_menu_item;
      else
	last_radio = NULL;
    }
  gtk_widget_destroy (curr->radio_menu_item);
  g_free (curr);
  curr = new_curr;
  if (curr)
    status = curr->status;
  else
    status = global_status;

  setTheFileName (filename);
}


int
getFilesCount (void)
{
  return g_list_length (files);
}
