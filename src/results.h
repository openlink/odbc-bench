/*
 *  results.h
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

#ifndef __RESULTS_H_
#define __RESULTS_H_

#ifdef __cplusplus
extern "C" {
#endif

void results_logout ();
int results_login (char *szDSN, char *szUID, char *sz_PWD);
void create_results_table ();
void drop_results_table ();
void do_add_results_record (char *test_type, char *result_test_type,
    SQLHENV env, SQLHDBC dbc, SQLHSTMT stmt,
    char *szDSN, float ftps, double dDiffSum, long nTrnCnt,
    float fsub1, float fsub2, float fAvgTPTime,
    char *szDriverName, char *szDriverVer, int driver_has_results,
    char *szState, char *szMessage);

#ifdef __cplusplus
}
#endif

#endif
