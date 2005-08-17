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
#include <gwenhywfar/plugin.h>

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



/** @name GWEN_DBIO plugins
 *
 */
/*@{*/
typedef GWEN_DBIO* (*GWEN_DBIO_PLUGIN_FACTORYFN)(GWEN_PLUGIN *pl);

GWEN_PLUGIN *GWEN_DBIO_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                                  const char *name,
                                  const char *fileName);
void GWEN_DBIO_Plugin_SetFactoryFn(GWEN_PLUGIN *pl,
                                   GWEN_DBIO_PLUGIN_FACTORYFN f);
GWEN_DBIO *GWEN_DBIO_Plugin_Factory(GWEN_PLUGIN *pl);
/*@}*/



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
 * This function creates a GWEN_DBIO of the given name. It therefore loads
 * the appropriate plugin if necessary.
 * The caller becomes the owner of the object returned, so he/she is
 * responsible for freeing it (Note: Previous version kept the ownership
 * so that the caller was not allowed to free the object. This has changed).
 */
GWENHYWFAR_API
GWEN_DBIO *GWEN_DBIO_GetPlugin(const char *modname);

/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_DBIO_H */


