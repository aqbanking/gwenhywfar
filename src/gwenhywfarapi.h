/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Sep 02 2002
    copyright   : (C) 2002 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GWENHYWFARAPI_H

#ifdef __declspec
# if BUILDING_GWEN_DLL
#  define GWENHYWFAR_API __declspec (dllexport)
# else /* Not BUILDING_GWENHYWFAR_DLL */
#  define GWENHYWFAR_API __declspec (dllimport)
# endif /* Not BUILDING_GWENHYWFAR_DLL */
#else
# define GWENHYWFAR_API
#endif

/* Convenience macros to test the versions of glibc and gcc. Taken
   from <features.h> which does not contain this on MinGW systems.  */
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif /* __GNUC_PREREQ */


/* Taken from <sys/cdefs.h> which does not contain this on MinGW
   systems.  */
#ifndef __STRING
# define __STRING(x)     #x
#endif /* __STRING */


/* This is needed for PalmOS, because it define some functions needed */
#include <string.h>
#include <gwenhywfar/system.h>


#endif


