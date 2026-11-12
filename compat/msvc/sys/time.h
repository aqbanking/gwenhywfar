/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 ***************************************************************************/

/* Replacement for the POSIX header, which MSVC does not have. configure
   adds this folder to the include path only when building with MSVC.
   On Windows, struct timeval is declared by the Winsock headers. */

#ifndef GWEN_COMPAT_MSVC_SYS_TIME_H
#define GWEN_COMPAT_MSVC_SYS_TIME_H

#include <winsock2.h>

#endif
