/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 ***************************************************************************/

/* Replacement for the POSIX header, which MSVC does not have. configure
   adds this folder to the include path only when building with MSVC. */

#ifndef GWEN_COMPAT_MSVC_UNISTD_H
#define GWEN_COMPAT_MSVC_UNISTD_H

#include <io.h>      /* access, close, dup, isatty, lseek, read, unlink, write */
#include <process.h> /* getpid */
#include <direct.h>  /* chdir, getcwd, rmdir */

#ifndef F_OK
# define F_OK 0
#endif
#ifndef W_OK
# define W_OK 2
#endif
#ifndef R_OK
# define R_OK 4
#endif
/* access() of MSVC rejects the execute bit, and files have no such
   permission on Windows. */
#ifndef X_OK
# define X_OK 0
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO  0
# define STDOUT_FILENO 1
# define STDERR_FILENO 2
#endif

#endif
