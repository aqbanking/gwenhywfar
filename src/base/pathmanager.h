/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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



#ifndef GWENHYWFAR_PATHMANAGER_H
#define GWENHYWFAR_PATHMANAGER_H

#include <gwenhywfar/error.h>
#include <gwenhywfar/stringlist.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @name Path Management
 *
 * Gwen is able to keep track of some paths for other libraries.
 * This technique is used internally to determine some important paths.
 */
/*@{*/

/**
 * This function defines a path. This must be done before calling
 * one of the other functions of this group.
 */
GWENHYWFAR_API
int GWEN_PathManager_DefinePath(const char *destLib,
                                const char *pathName);

GWENHYWFAR_API
int GWEN_PathManager_UndefinePath(const char *destLib,
                                  const char *pathName);


GWENHYWFAR_API
int GWEN_PathManager_AddPath(const char *callingLib,
                             const char *destLib,
                             const char *pathName,
                             const char *pathValue);

GWENHYWFAR_API
int GWEN_PathManager_AddPathFromWinReg(const char *callingLib,
                                       const char *destLib,
                                       const char *pathName,
                                       const char *keypath,
                                       const char *varname);

GWENHYWFAR_API
int GWEN_PathManager_InsertPath(const char *callingLib,
                    const char *destLib,
                    const char *pathName,
                    const char *pathValue);

GWENHYWFAR_API
int GWEN_PathManager_RemovePath(const char *callingLib,
                    const char *destLib,
                    const char *pathName,
                    const char *pathValue);

GWENHYWFAR_API
int GWEN_PathManager_RemovePaths(const char *callingLib);

GWENHYWFAR_API
int GWEN_PathManager_PathChanged(const char *destLib,
                                 const char *pathName);

GWENHYWFAR_API
GWEN_STRINGLIST *GWEN_PathManager_GetPaths(const char *destLib,
                                           const char *pathName);

/*@}*/

#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_PATHMANAGER_H */
