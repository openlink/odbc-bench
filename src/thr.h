/*
 *  thr.h
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
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#else
#define PTHREADS
#include <pthread.h>
#endif

/* mutexes */
#if defined(PTHREADS)

typedef pthread_mutex_t MUTEX_T;
typedef pthread_t THREAD_T;
#define GET_EXIT_STATUS(th, result) \
	pthread_join(th, (void **)result)

#define START_THREAD(th, worker, data) \
	pthread_create(&(th), NULL, worker, &(data))

#define MUTEX_INIT(mutex) \
	pthread_mutex_init(&mutex, NULL)

#define MUTEX_FREE(mutex) \
	pthread_mutex_destroy(&mutex)

#define MUTEX_ENTER(mutex) \
	pthread_mutex_lock(&mutex)

#define MUTEX_LEAVE(mutex) \
	pthread_mutex_unlock(&mutex)

#elif defined(WIN32)

typedef HANDLE MUTEX_T;
typedef HANDLE THREAD_T;
#define GET_EXIT_STATUS(th, result) \
	GetExitCodeThread(th, (LPDWORD)result)

#define START_THREAD(th, worker, data) \
	th = CreateThread(NULL, 0, worker, &(data), 0, &(thrid))

#define MUTEX_INIT(mutex) \
	mutex = CreateMutex(NULL, FALSE, NULL)

#define MUTEX_FREE(mutex) \
	CloseHandle(mutex)

#define MUTEX_ENTER(mutex) \
	WaitForSingleObject(mutex, INFINITE)

#define MUTEX_LEAVE(mutex) \
	ReleaseMutex(mutex)

#endif

#if defined (PTHREADS) || defined(WIN32)
int do_threads_run (int nConnCount, OList * tests, int nMinutes,
    char *szTitle);
int do_threads_run_all (int nTests, OList * tests, int nMinutes,
    char *filename);
extern MUTEX_T log_mutex;
extern long nProgressIncrement;
#define THREADED
void DoRunAll (test_t * test, char *filename);
#endif
