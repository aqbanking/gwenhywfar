/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Wed Mar 31 2004
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


/** @file dbio.h
 * @short This file provides the GWEN DB import/export framework.
 */

#ifndef GWENHYWFAR_DBIO_H
#define GWENHYWFAR_DBIO_H

#include <gwenhywfar/gwenhywfarapi.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct GWEN_DBIO GWEN_DBIO;
#ifdef __cplusplus
}
#endif


/**
 * name of the folder below Gwen's PLUGIN folder which holds DBIO plugins
 */
#define GWEN_DBIO_FOLDER "dbio"


#include <gwenhywfar/path.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/error.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_DBIO, GWEN_DBIO, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_DBIO, GWENHYWFAR_API)
/* No trailing semicolon here because this is a macro call */

typedef enum {
  GWEN_DBIO_CheckFileResultOk=0,
  GWEN_DBIO_CheckFileResultNotOk,
  GWEN_DBIO_CheckFileResultUnknown
} GWEN_DBIO_CHECKFILE_RESULT;

typedef GWEN_DBIO* (*GWEN_DBIO_FACTORYFN)(void);

typedef int (*GWEN_DBIO_IMPORTFN)(GWEN_DBIO *dbio,
				  GWEN_BUFFEREDIO *bio,
				  GWEN_TYPE_UINT32 flags,
                                  GWEN_DB_NODE *db,
                                  GWEN_DB_NODE *params);

typedef int (*GWEN_DBIO_EXPORTFN)(GWEN_DBIO *dbio,
				  GWEN_BUFFEREDIO *bio,
				  GWEN_TYPE_UINT32 flags,
                                  GWEN_DB_NODE *db,
                                  GWEN_DB_NODE *params);

typedef GWEN_DBIO_CHECKFILE_RESULT (*GWEN_DBIO_CHECKFILEFN)(GWEN_DBIO *dbio,
                                     const char *fname);


/** @name Functions To Be Used By Applications
 *
 */
/*@{*/
/**
 * Reads data from the given GWEN_BUFFEREDIO and stores the data read
 * into the given DB. The stream represented by the buffered io is expected
 * to have the format for this particular GWEN_DBIO.
 */
GWENHYWFAR_API
int GWEN_DBIO_Import(GWEN_DBIO *dbio,
                     GWEN_BUFFEREDIO *bio,
                     GWEN_TYPE_UINT32 flags,
                     GWEN_DB_NODE *db,
                     GWEN_DB_NODE *params);

/**
 * Writes data to the given GWEN_BUFFEREDIO in the format of this particular
 * GWEN_DBIO.
 */
GWENHYWFAR_API
int GWEN_DBIO_Export(GWEN_DBIO *dbio,
                     GWEN_BUFFEREDIO *bio,
                     GWEN_TYPE_UINT32 flags,
                     GWEN_DB_NODE *db,
                     GWEN_DB_NODE *params);

/**
 * Checks whether the given file is supported by the given DBIO.
 */
GWENHYWFAR_API
GWEN_DBIO_CHECKFILE_RESULT GWEN_DBIO_CheckFile(GWEN_DBIO *dbio,
					       const char *fname);


/**
 * Releases the ressources associated with the given GWEN_DBIO if the usage
 * counter reaches zero.
 */
GWENHYWFAR_API
void GWEN_DBIO_free(GWEN_DBIO *dbio);

/**
 * Increments the internal usage counter. This counter is decremented
 * upon @ref GWEN_DBIO_free.
 */
GWENHYWFAR_API
void GWEN_DBIO_Attach(GWEN_DBIO *dbio);

/**
 * Returns the name of a GWEN_DBIO.
 */
GWENHYWFAR_API
const char *GWEN_DBIO_GetName(const GWEN_DBIO *dbio);

/**
 * Returns a descriptive text about this particular GWEN_DBIO.
 */
GWENHYWFAR_API
const char *GWEN_DBIO_GetDescription(const GWEN_DBIO *dbio);

/**
 * Returns the GWEN_DBIO of the given name if it has already been
 * registered.
 */
GWENHYWFAR_API
GWEN_DBIO *GWEN_DBIO_Find(const char *name);

/**
 * Returns the path to DBIO plugins. On Windows this path is looked upon
 * from the registry key "Software\\Gwenhywfar\\Paths" in HKEY_CURRENT_USER.
 * If this key does not exist or this is called on a non-win32 platform the
 * compile-time value is returned.
 * @return 0 if value found, 1 if on win32 and the key did not exist, -1 on error
 * @param pbuf Buffer to which the path is appended.
 */
GWENHYWFAR_API
int GWEN_DBIO_GetPluginPath(GWEN_BUFFER *pbuf);

/*@}*/




/** @name Functions To Be Used By Inheritors
 *
 */
/*@{*/
/**
 * Creates the base object which is to be extended by the inheritor.
 */
GWENHYWFAR_API
GWEN_DBIO *GWEN_DBIO_new(const char *name, const char *descr);

/**
 * Sets the import function for this kind of GWEN_DBIO.
 */
GWENHYWFAR_API
void GWEN_DBIO_SetImportFn(GWEN_DBIO *dbio, GWEN_DBIO_IMPORTFN f);

/**
 * Sets the export function for this kind of GWEN_DBIO.
 */
GWENHYWFAR_API
void GWEN_DBIO_SetExportFn(GWEN_DBIO *dbio, GWEN_DBIO_EXPORTFN f);

GWENHYWFAR_API
void GWEN_DBIO_SetCheckFileFn(GWEN_DBIO *dbio, GWEN_DBIO_CHECKFILEFN f);

/*@}*/


/** @name Functions To Be Used By Administration Functions
 *
 * Functions in this group are to be used for administration purposes
 * only.
 */
/*@{*/

/**
 * If the GWEN_DBIO has been dynamically loaded (as is the case with plugins)
 * then the GWEN_LIBLOADER used can be stored within the GWEN_DBIO.
 * That way as soon as the GWEN_DBIO is released the associated dynamic
 * library is automatically unloaded.
 */
GWENHYWFAR_API
GWEN_LIBLOADER *GWEN_DBIO_GetLibLoader(const GWEN_DBIO *dbio);

/**
 * If the GWEN_DBIO has been dynamically loaded (as is the case with plugins)
 * then the GWEN_LIBLOADER used can be stored within the GWEN_DBIO.
 * That way as soon as the GWEN_DBIO is released the associated dynamic
 * library is automatically unloaded.
 */
GWENHYWFAR_API
void GWEN_DBIO_SetLibLoader(GWEN_DBIO *dbio, GWEN_LIBLOADER *ll);

/**
 * Register a new GWEN_DBIO with Gwenhywfar. This always inserts the new
 * GWEN_DBIO at the beginning of the internal list thus replacing an
 * eventually existing GWEN_DBIO with the same name.
 */
GWENHYWFAR_API
int GWEN_DBIO_Register(GWEN_DBIO *dbio);

/**
 * This function only loads the given plugin file, it does not register it.
 */
GWENHYWFAR_API
GWEN_DBIO *GWEN_DBIO_LoadPluginFile(const char *modname, const char *fname);

/**
 * This functions searches for a plugin which supports the given type in
 * the usual place ("GWENHYWFAR_PLUGIN/dbio/"). It does not register the
 * plugin.
 */
GWENHYWFAR_API
GWEN_DBIO *GWEN_DBIO_LoadPlugin(const char *modname);

/**
 * This functions returns the DBIO of the given name. If it already is
 * registered, then simply the registered one is returned. Otherwise this
 * function tries to load the appropriate plugin and registers it before
 * returning it to the caller.
 * The caller MUST NOT free the plugin returned, this module keeps the
 * ownership of the plugin.
 */
GWENHYWFAR_API
GWEN_DBIO *GWEN_DBIO_GetPlugin(const char *modname);

/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_DBIO_H */


