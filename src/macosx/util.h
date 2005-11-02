/*
 *  util.c
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2005 OpenLink Software <odbc-bench@openlinksw.com>
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

#ifndef _MACOSX_UTIL_H_
#define _MACOSX_UTIL_H_

#include <stdarg.h>
#include <wchar.h>

#include <Carbon/Carbon.h>
#include <CoreFoundation/CFString.h>

#ifdef __cplusplus
extern "C" {
#endif

char *OPL_CFString_to_char(const CFStringRef str);
wchar_t *OPL_CFString_to_wchar(const CFStringRef str);

CFStringRef OPL_wchar_to_CFString(const wchar_t *str);
CFStringRef OPL_char_to_CFString(const char *str);

CFStringRef OPL_CFString_vasprintf(const char *format, va_list ap);
CFStringRef OPL_CFString_asprintf(const char *format, ...);

// Case-insensitive CFString comparator
CFComparisonResult OPL_CFString_cmp(const void *a, const void *b, void * /* ctx */);

// Get FSRef from AEDesc
OSStatus OPL_GetFSRefFromAEDesc(FSRef *fsRef, AEDesc *item);

// Get file name for saving
bool OPL_getSaveFileName(char *path, size_t path_len,
					     CFStringRef title, CFStringRef defaultFileName);

// Ask to save changes
bool
OPL_getSaveChangesFileName(char *path, size_t path_len,
						   CFStringRef fileName, bool quitting);
					
#ifdef __cplusplus
}
#endif

#endif /* _MACOSX_UTIL_H_ */
