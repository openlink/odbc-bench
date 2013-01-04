/*
 *  prefs.c
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
#include <stdio.h>
#include <stdlib.h>

#include "odbcbench.h"


struct pref_s
{
  long a_refresh_rate;
  long c_refresh_rate;
  long lock_timeout;
  long display_refresh_rate;
} prefs = {10, 1, 0, 80};


long
bench_get_long_pref (OdbcBenchPref pref)
{
  switch (pref)
    {
    case A_REFRESH_RATE:
      return prefs.a_refresh_rate;

    case C_REFRESH_RATE:
      return prefs.c_refresh_rate;

    case LOCK_TIMEOUT:
      return prefs.lock_timeout;

    case DISPLAY_REFRESH_RATE:
      return prefs.display_refresh_rate;
    }

  return -1;
}


int
bench_set_long_pref (OdbcBenchPref pref, long value)
{
  switch (pref)
    {
    case A_REFRESH_RATE:
      prefs.a_refresh_rate = value;
      return 1;

    case C_REFRESH_RATE:
      prefs.c_refresh_rate = value;
      return 1;

    case LOCK_TIMEOUT:
      prefs.lock_timeout = value;
      return 1;

    case DISPLAY_REFRESH_RATE:
      prefs.display_refresh_rate = value;
      return 1;
    }

  return 0;
}
