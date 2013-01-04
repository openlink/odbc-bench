/*
 *  TPCATableProps.h
 * 
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases 
 *  Copyright (C) 2000-2013 OpenLink Software <odbc-bench@openlinksw.com>
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
#ifndef __TPCA_TABLE_PROPS_H__
#define __TPCA_TABLE_PROPS_H__

#include "ServerDSN.h"
#ifdef __cplusplus
extern "C"
{
#endif

/*-------------------------------------------------------------------------- */
/*|     Defines and Macros                                                   */
/*-------------------------------------------------------------------------- */
#define NUMITEMS(p1) (sizeof(p1)/sizeof(p1[0]))

#define TPCA_TABLE_PROPS(obj) GTK_CHECK_CAST(obj, TPCATableProps_get_type(), TPCATableProps)
#define TPCA_TABLE_PROPS_CLASS(kclass) \
	GTK_CHECK_CLASS_CAST(obj, TPCATableProps_get_type(), TPCATablePropsClass)
#define IS_TPCA_TABLE_PROPS(obj) GTK_CHECK_TYPE(obj, TPCATableProps_get_type())

  typedef struct _TPCATableProps TPCATableProps;
  typedef struct _TPCATablePropsClass TPCATablePropsClass;

  struct _TPCATablePropsClass
  {
    GtkVBoxClass parent_class;
  };

  struct _TPCATableProps
  {

    GtkVBox parent;

    GtkWidget *create_branch, *create_teller, *create_account,
	*create_history;
    GtkWidget *load_branch, *load_teller, *load_account;
    GtkWidget *create_indexes, *create_procedures;

    GtkWidget *num_branches, *num_tellers, *num_accounts;
    GtkWidget *dbmstype;

    GtkWidget *branch_dsn, *teller_dsn, *account_dsn, *history_dsn;
    test_t *ptest;
    ServerDSN *pDSN;
  };

  int TPCATableProps_get_type (void);

  GtkWidget *TPCATableProps_new (test_t * ptest);

  void enable_widget_as (GtkWidget * check, gpointer widget);

  void TPCATableProps_save_config (TPCATableProps * pDlg);
#ifdef __cplusplus
}
#endif

#endif
