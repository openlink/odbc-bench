/*
 *  TPCARunProps.h
 * 
 *  $Id$
 *
 *  odbc-bench - a TPCA and TPCC benchmark program for databases 
 *  Copyright (C) 2000-2002 OpenLink Software <odbc-bench@openlinksw.com>
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
#ifndef __TPCA_RUN_PROPS_H__
#define __TPCA_RUN_PROPS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define TPCA_RUN_PROPS(obj) GTK_CHECK_CAST(obj, TPCARunProps_get_type(), TPCARunProps)
#define TPCA_RUN_PROPS_CLASS(kclass) \
	GTK_CHECK_CLASS_CAST(obj, TPCARunProps_get_type(), TPCARunPropsClass)
#define IS_TPCA_RUN_PROPS(obj) GTK_CHECK_TYPE(obj, TPCARunProps_get_type())

  typedef struct _TPCARunProps TPCARunProps;
  typedef struct _TPCARunPropsClass TPCARunPropsClass;

  struct _TPCARunPropsClass
  {
    GtkVBoxClass parent_class;
  };

  struct _TPCARunProps
  {
    GtkVBox parent;
    test_t *lpBench;

    GtkWidget *array_params;
    GtkWidget *thread_opts;
    GtkWidget *execdirect, *prepare_execute, *procedures;
    GtkWidget *async, *trans, *do100row;
    GtkWidget *isolation_levels[5];
    GtkWidget *cursor_modes[5];
    GtkWidget *rowset_size;
    GtkWidget *keyset_size;
    GtkWidget *traversal_count;
  };

  int TPCARunProps_get_type (void);

  GtkWidget *TPCARunProps_new (test_t * lpBench);

  void TPCARunProps_save_config (TPCARunProps * props);

  void enable_widget (GtkWidget * widget);
  void disable_widget (GtkWidget * widget);


#ifdef __cplusplus
}
#endif
#endif
