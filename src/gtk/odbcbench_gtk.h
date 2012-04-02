/*
 *  odbcbench.h
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void do_pane_log (const char *format, ...);	/* adds a text line(s) to the status log */

/* status log handling routines - status.c */
void clear_status_handler (GtkWidget * widget, gpointer data);	/* clears the status text */
void vBusy (void);
void create_status_widget (GtkWidget * win);

typedef enum
{ DLG_OK, DLG_CANCEL, DLG_YES, DLG_NO }
answer_code;
/* dialog box functions - dialog.c */
GtkWidget *message_box_new (GtkWidget * Parent, const gchar * Text, const gchar * Title);	/* shows a dialog box with the specified attributes */
answer_code ok_cancel_dialog (const gchar * Text, const gchar * Title);
answer_code yes_no_cancel_dialog (const gchar * Text, const gchar * Title);
void login_dialog (GtkWidget * Parent, gpointer data);	/* show a login combo with a dsn_list as specified */
char *fill_file_name (char *szFileName, char *caption, int add_xmls);

/* progress implementations - dialog.c */
void do_ShowProgress (void *parent, char *title, BOOL bForceSingle,
    float nMax);
void do_SetWorkingItem (char *pszWorking);
void do_SetProgressText (char *pszProgress, int nConn, int thread_no,
    float percent, int nTrnPerCall, long secs_remain, double tpca_dDiffSum);
void do_StopProgress (void);
int do_fCancel (void);
void do_ShowCancel (int fShow);
void do_RestartProgress (void);
void do_MarkFinished (int nConn, int nThread);
BOOL isCancelled (void);

void set_display_refresh_rate (GtkWidget * widget, gpointer data);
void set_lock_timeout (GtkWidget * widget, gpointer data);
void do_flip (GtkWidget * widget, gboolean * data);
