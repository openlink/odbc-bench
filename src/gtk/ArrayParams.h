/*
 *  ArrayParams.h
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

#ifndef __ARRAY_PARAMETERS_H__
#define __ARRAY_PARAMETERS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ARRAY_PARAMS(obj) GTK_CHECK_CAST(obj, ArrayParams_get_type(), ArrayParams)
#define ARRAY_PARAMS_CLASS(kclass) \
	GTK_CHECK_CLASS_CAST(obj, ArrayParams_get_type(), ArrayParamsClass)
#define IS_ARRAY_PARAMS(obj) GTK_CHECK_TYPE(obj, ArrayParams_get_type())

  typedef struct _ArrayParams ArrayParams;
  typedef struct _ArrayParamsClass ArrayParamsClass;

  struct _ArrayParamsClass
  {
    GtkVBoxClass parent_class;
  };

  struct _ArrayParams
  {
    GtkVBox parent;
    tpca_t *test;

    GtkWidget *no_params;
    GtkWidget *use_ArrayParams;
  };

  int ArrayParams_get_type (void);

  GtkWidget *ArrayParams_new (void);

  void ArrayParams_save_config (ArrayParams * props);
  void ArrayParams_load_config (ArrayParams * props,
      tpca_t * test);

#ifdef __cplusplus
}
#endif
#endif
