/*
 *  ServerDSN.h
 * 
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases 
 *  Copyright (C) 2000-2019 OpenLink Software <odbc-bench@openlinksw.com>
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
#ifndef __SERVER_DSN_H__
#define __SERVER_DSN_H__

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct dsn_info_s
  {
    GList *dsns;
    GList *names;
  }
  dsn_info_t;

#define SERVER_DSN(obj) GTK_CHECK_CAST(obj, ServerDSN_get_type(), ServerDSN)
#define SERVER_DSN_CLASS(kclass) \
	GTK_CHECK_CLASS_CAST(obj, ServerDSN_get_type(), ServerDSNClass)
#define IS_SERVER_DSN(obj) GTK_CHECK_TYPE(obj, ServerDSN_get_type())

  typedef struct _ServerDSN ServerDSN;
  typedef struct _ServerDSNClass ServerDSNClass;

  struct _ServerDSNClass
  {
    GtkDialogClass parent_class;

    void (*ServerDSN_do_the_work) (ServerDSN * serverdsn);
    void (*ServerDSN_dsns_changed) (ServerDSN * serverdsn);
  };

  struct _ServerDSN
  {

    GtkDialog parent;

    test_t *lpBench;
    dsn_info_t dsn_info;
    dsn_info_t avail_dsn_info;
    int isVirtuoso;
    GtkWidget *rdsn, *ruid, *rpwd, *available_list, *defined_list;
  };

  int ServerDSN_get_type (void);
  GtkWidget *ServerDSN_new (test_t * lpBench);

#ifdef __cplusplus
}
#endif
#endif
