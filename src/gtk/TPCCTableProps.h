/*
 *  TPCCTableProps.h
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
#ifndef __TPCC_TABLE_PROPS_H__
#define __TPCC_TABLE_PROPS_H__

#include <gtk/gtkcontainer.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define TPCC_TABLE_PROPS(obj) GTK_CHECK_CAST(obj, TPCCTableProps_get_type(), TPCCTableProps)
#define TPCC_TABLE_PROPS_CLASS(kclass) \
	GTK_CHECK_CLASS_CAST(obj, TPCCTableProps_get_type(), TPCCTablePropsClass)
#define IS_TPCC_TABLE_PROPS(obj) GTK_CHECK_TYPE(obj, TPCCTableProps_get_type())

  typedef struct _TPCCTableProps TPCCTableProps;
  typedef struct _TPCCTablePropsClass TPCCTablePropsClass;

  struct _TPCCTablePropsClass
  {
    GtkVBoxClass parent_class;
  };

  struct _TPCCTableProps
  {
    GtkVBox parent;

    test_t *lpBench;
    GtkWidget *dsn;

    GtkWidget *combos[9];
    GtkWidget *n_ware;
  };

  int TPCCTableProps_get_type (void);

  GtkWidget *TPCCTableProps_new (test_t * lpBench);

  void TPCCTableProps_save_config (TPCCTableProps * props);

#ifdef __cplusplus
}
#endif
#endif
