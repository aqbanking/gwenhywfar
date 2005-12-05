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
 * Gwenhywfar keeps track of registered directory paths for itself
 * and for other libraries.  
 *
 * The paths are registered and stored in one global object (of
 * GWEN_DB_NODE type), so write-access to this GWEN_PathManager
 * functions is unfortunately not thread-safe.
 *
 * This technique is used internally to
 * determine some important paths.
 */
/*@{*/

/**
 * Register a path to be managed by the global GWEN_PathManager
 * module. This must be done before calling one of the other
 * functions of this group. The actual path is not yet set.
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @return Zero on success, or GWEN_ERROR_INVALID if the @c
 * pathName already exists.
 */
GWENHYWFAR_API
int GWEN_PathManager_DefinePath(const char *destLib,
                                const char *pathName);

/**
 * Unregister a path that was registered by
 * GWEN_PathManager_DefinePath().
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @return Zero on success, or GWEN_ERROR_NOT_FOUND if the @c
 * pathName was not found.
 */
GWENHYWFAR_API
int GWEN_PathManager_UndefinePath(const char *destLib,
                                  const char *pathName);


/**
 * Add a directory path entry to a registered path entry in the
 * global GWEN_PathManager.
 *
 * @callingLib The name of the library that adds this path entry.
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @pathValue The actual directory path that should be added to
 * this registered path.
 *
 * @return Zero on success, or GWEN_ERROR_NOT_FOUND if the @c
 * pathName was not found.
 */
GWENHYWFAR_API
int GWEN_PathManager_AddPath(const char *callingLib,
                             const char *destLib,
                             const char *pathName,
                             const char *pathValue);

/**
 * Add a directory path entry to a registered path entry in the
 * global GWEN_PathManager by looking up the directory path in the
 * Windows registry. 
 *
 * On non-windows platforms, the function does nothing and returns
 * zero, so you can safely call this in your multi-platform code.
 *
 * @callingLib The name of the library that adds this path entry.
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @keypath The key's path in the windows registry under
 * HKEY_LOCAL_USER, e.g. "Software\MyProgram\Paths".
 *
 * @varname The variable name of the string variable with the
 * actual directory path.
 *
 * @return Zero on success, or nonzero if the @c pathName or the
 * registry key was not found.
 */
GWENHYWFAR_API
int GWEN_PathManager_AddPathFromWinReg(const char *callingLib,
                                       const char *destLib,
                                       const char *pathName,
                                       const char *keypath,
                                       const char *varname);

/**
 * Insert a directory path entry to a registered path entry in the
 * global GWEN_PathManager. FIXME: What is the difference to
 * AddPath() ???
 *
 * @callingLib The name of the library that adds this path entry.
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @pathValue The actual directory path that should be added to
 * this registered path.
 */
GWENHYWFAR_API
int GWEN_PathManager_InsertPath(const char *callingLib,
                    const char *destLib,
                    const char *pathName,
                    const char *pathValue);

/**
 * Removes a directory path entry from a registered path entry in
 * the global GWEN_PathManager.
 *
 * @callingLib The name of the library that added this path entry.
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @pathValue The actual directory path that should be added to
 * this registered path.
 *
 * @return Zero on success, or nonzero if the @c pathName was not
 * found.
 */
GWENHYWFAR_API
int GWEN_PathManager_RemovePath(const char *callingLib,
                    const char *destLib,
                    const char *pathName,
                    const char *pathValue);

/**
 * Removes all directory path entries that were registered by the
 * @c callingLib from the global GWEN_PathManager.
 *
 * @callingLib The name of the library that added path entries.
 *
 * @return Zero on success.
 */
GWENHYWFAR_API
int GWEN_PathManager_RemovePaths(const char *callingLib);


/**
 * DOCUMENTME: What does this do?
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @return Zero on success, or GWEN_ERROR_NOT_FOUND if the @c
 * pathName was not found.
 */
GWENHYWFAR_API
int GWEN_PathManager_PathChanged(const char *destLib,
                                 const char *pathName);

/**
 * Returns a string list of all path entries that exist under the
 * registered @c pathName.
 *
 * @destLib The name of the library that this path is supposed to
 * belong to.
 *
 * @pathName A string identifier for this registered path.
 *
 * @return A newly allocated @ref GWEN_STRINGLIST with all added
 * path entries, or NULL if no path entry or no registered @c
 * pathName exists. Delete this list with GWEN_StringList_free()
 * when no longer needed.
 */
GWENHYWFAR_API
GWEN_STRINGLIST *GWEN_PathManager_GetPaths(const char *destLib,
                                           const char *pathName);

/*@}*/

#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_PATHMANAGER_H */
