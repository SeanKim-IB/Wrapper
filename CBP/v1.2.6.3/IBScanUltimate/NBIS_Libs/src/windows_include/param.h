/*
 * param.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 */

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#ifndef WINCE
#include <sys/types.h>
#endif
#include <limits.h>

/* These are useful for cross-compiling */ 
#define BIG_ENDIAN	4321
#define LITTLE_ENDIAN	1234
#define BYTE_ORDER	LITTLE_ENDIAN

#define PATH_MAX	260
#define MAXPATHLEN PATH_MAX

#endif
