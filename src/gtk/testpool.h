/*
 *  testpool.h
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
#ifndef __TESTPOOL_H_
#define __TESTPOOL_H_

GtkWidget *create_test_pool (void);
int add_test_to_the_pool (test_t * ptest);
void remove_selected_tests (void);
int pool_connection_count (void);
GList *get_selected_tests (void);
OList *get_selected_tests_list (void);
void for_all_in_pool (void);
void pool_update_selected (void);

void pool_set_selected_items (GList * list);
void pool_set_selected_item (test_t * test);

gboolean getIsFileDirty (void);
void setIsFileDirty (gboolean isit);
char *getCurrentFileName (void);
void setTheFileName (char *filename);
void close_file_pool (void);
int getFilesCount (void);
#endif
