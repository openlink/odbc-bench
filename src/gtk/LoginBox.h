/*
 *  LoginBox.h
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
#ifndef __LOGINBOX_H__
#define __LOGINBOX_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define LOGINBOX(obj) GTK_CHECK_CAST(obj, LoginBox_get_type(), LoginBox)
#define LOGINBOX_CLASS(kclass) \
	GTK_CHECK_CLASS_CAST(obj, LoginBox_get_type(), LoginBoxClass)
#define IS_LOGINBOX(obj) GTK_CHECK_TYPE(obj, LoginBox_get_type())

  typedef struct _LoginBox LoginBox;
  typedef struct _LoginBoxClass LoginBoxClass;

  struct _LoginBoxClass
  {
    GtkDialogClass parent_class;

    void (*LoginBox_do_the_work) (LoginBox * login_box);
    void (*LoginBox_closed) (LoginBox * login_box);
  };

  struct _LoginBox
  {

    GtkDialog parent;

    GtkWidget *dsn, *uid, *pwd;

    gchar szDSN[50], szUID[50], szPWD[50];
  };

  int LoginBox_get_type (void);
  GtkWidget *LoginBox_new (gchar * title, GList * dsn_list, gchar * szDSN,
      gchar * szUID, gchar * szPWD);

  gchar *LoginBox_dsn (LoginBox * box, gchar * new_dsn);
  gchar *LoginBox_uid (LoginBox * box, gchar * new_uid);
  gchar *LoginBox_pwd (LoginBox * box, gchar * new_pwd);

#ifdef __cplusplus
}
#endif
#endif
