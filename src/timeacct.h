/*
 *  timeacct.h
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
#include <stdio.h>

#ifdef WIN32
#define get_msec_count GetTickCount
#define get_msec_real_time GetTickCount
#else
long get_msec_count ();
#endif

#define TA_ON 1
#define TA_OFF 2
#define TA_DISABLED 3

typedef struct timeacctstr
{
  int ta_is_on;
  char *ta_name;
  long ta_n_samples;
  long ta_total;
  long ta_max;
  long ta_min;
  long ta_entry_time;
  long ta_init_time;
}
timer_account_t;


void ta_print_out (FILE * out, timer_account_t * ta);
void ta_init (timer_account_t * ta, char *n);
void ta_enter (timer_account_t * ta);
void ta_leave (timer_account_t * ta);
void ta_add_sample (timer_account_t * ta, long this_time);
void ta_disable (timer_account_t * ta);
long rnd (long *rnd_seed);
long random_1 (long *rnd_seed, long scale);
void ta_print_buffer (char *szOut, timer_account_t * ta,
    timer_account_t * pack);
void ta_merge (timer_account_t * to, timer_account_t * from);

#ifdef WIN32
void gettimeofday (struct timeval *tv, struct timezone *tz);
#endif

#ifdef LOW_ORDER_FIRST

#define REV_LONG(l) \
  (((unsigned long)l) >> 24 |              \
  ((unsigned long)l & 0x00ff0000 ) >> 8 |  \
  ((unsigned long)l & 0x0000ff00 ) << 8 |  \
  ((unsigned long)l) << 24 )

#define TV_TO_STRING(tv) \
  (tv) -> tv_sec = REV_LONG ((tv) -> tv_sec), (tv) -> tv_usec = REV_LONG ((tv) -> tv_usec)

#define STRING_TO_TV(tv) TV_TO_STRING(tv)

#else

#define TV_TO_STRING(tv)
#define STRING_TO_TV(tv)

#endif


#define gettimestamp(ts) \
{ \
  gettimeofday ((struct timeval *) ts, NULL); \
  TV_TO_STRING ( (struct timeval *)  ts)  ; \
}
