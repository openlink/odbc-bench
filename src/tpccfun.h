/*
 *  tpccfun.h
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
long RandomNumber (long *rnd_seed, long x, long y);
int MakeAlphaString (long *rnd_seed, int sz1, int sz2, char *str);
long random_i_id (long *rnd_seed);
long random_c_id (long *rnd_seed);
void Lastname (int num, char *name);

void add_tpcc_result (GtkWidget * widget, test_t * lpCfg);
void scrap_log (test_t * lpBench, HSTMT stmt);
void reset_times (test_t * lpCfg);

void tpcc_init_globals (GtkWidget * widget, test_t * lpCfg);
void tpcc_create_db (GtkWidget * widget, test_t * lpCfg);
void tpcc_close_stmts (GtkWidget * widget, test_t * lpCfg);
int tpcc_run_test (GtkWidget * widget, test_t * lpCfg);
void tpcc_schema_create (GtkWidget * widget, test_t * lpBench);
void tpcc_schema_cleanup (GtkWidget * widget, test_t * lpBench);
void tpca_run_one (GtkWidget * widget, gpointer data);


void print_times_str (test_t * lpCfg, char *szBuffer);
