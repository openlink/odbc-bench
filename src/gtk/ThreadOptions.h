/*
 *  ThreadOptions.h
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
#ifndef __THREAD_OPTIONS_H__
#define __THREAD_OPTIONS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define THREAD_OPTIONS(obj) GTK_CHECK_CAST(obj, ThreadOptions_get_type(), ThreadOptions)
#define THREAD_OPTIONS_CLASS(kclass) \
	GTK_CHECK_CLASS_CAST(obj, ThreadOptions_get_type(), ThreadOptionsClass)
#define IS_THREAD_OPTIONS(obj) GTK_CHECK_TYPE(obj, ThreadOptions_get_type())

  typedef struct _ThreadOptions ThreadOptions;
  typedef struct _ThreadOptionsClass ThreadOptionsClass;

  struct _ThreadOptionsClass
  {
    GtkVBoxClass parent_class;
  };

  struct _ThreadOptions
  {
    GtkVBox parent;
    test_common_t *test;

    GtkWidget *no_threads_spin, *no_threads;
    GtkWidget *single_threaded, *multi_threaded;
  };

  int ThreadOptions_get_type (void);

  GtkWidget *ThreadOptions_new (void);

  void ThreadOptions_save_config (ThreadOptions * props);
  void ThreadOptions_load_config (ThreadOptions * props,
      test_common_t * test);

#ifdef __cplusplus
}
#endif
#endif
