/*
 *  util.h
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MACOSX_UTIL_H_
#define _MACOSX_UTIL_H_

#include <stdarg.h>
#include <wchar.h>

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
OSStatus OPL_getSaveFileName(char *path, size_t path_len,
					CFStringRef title, CFStringRef defaultFileName);

#ifdef __cplusplus
}
#endif

#endif /* _MACOSX_UTIL_H_ */
