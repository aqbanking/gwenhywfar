/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Dec 03 2002
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

/**
 * @file gwenhywfar/directory.h
 * @short This file contains the directory handling module
 */


#ifndef GWEN_DIRECTORY_H
#define GWEN_DIRECTORY_H


#include <gwenhywfar/gwenhywfarapi.h>

#ifdef __cplusplus
extern "C" {
#endif

GWENHYWFAR_API typedef struct GWEN_DIRECTORYDATASTRUCT GWEN_DIRECTORYDATA;


/**
 * @defgroup MOD_DIRECTORY Directory Functions
 * @ingroup MOD_OS
 * @short Provides secure interprocess communication
 * @author Martin Preuss<martin@libchipcard.de>
 */
/*@{*/

/**
 * Constructor.
 */
GWENHYWFAR_API GWEN_DIRECTORYDATA *GWEN_Directory_new();

/**
 * Destructor
 */
GWENHYWFAR_API void GWEN_Directory_free(GWEN_DIRECTORYDATA *d);

/**
 * Opens a directory. This allows calling "Directory_Read" to succeed.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref GWEN_Directory_new().
 * @param n path and name of the directory to open
 */
GWENHYWFAR_API int GWEN_Directory_Open(GWEN_DIRECTORYDATA *d, const char *n);

/**
 * Closes a previously opened directory.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref GWEN_Directory_new().
 */
GWENHYWFAR_API int GWEN_Directory_Close(GWEN_DIRECTORYDATA *d);

/**
 * Reads the next entry from a directory and stores the name of that
 * entry in the given buffer. The entry returned is relative to the
 * open directory.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref GWEN_Directory_new().
 * @param buffer pointer to a buffer to receive the name
 * @param len size of the buffer
 */
GWENHYWFAR_API int GWEN_Directory_Read(GWEN_DIRECTORYDATA *d,
                                       char *buffer,
                                       unsigned int len);

/**
 * Rewinds the internal pointers, so that the next call to
 * @author Martin Preuss<martin@libchipcard.de>
 * @ref GWEN_Directory_Read will return the first entry of the open directory.
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref GWEN_Directory_new().
 */
GWENHYWFAR_API int GWEN_Directory_Rewind(GWEN_DIRECTORYDATA *d);


/**
 * Get the current users home directory and copy it into the given buffer.
 * @return 0 if ok, !=0 on error
 */
int GWEN_Directory_GetHomeDirectory(char *buffer, unsigned int size);

/**
 * Gets a file path. It uses @ref MOD_PATH to either create or check for
 * the existence of a given path.
 * @return 0 if ok, !=0 on error
 * @param path path to inspect
 * @param flags flags, please see @ref GWEN_PATH_FLAGS_PATHMUSTEXIST et. al.
 */
int GWEN_Directory_GetPath(const char *path, unsigned int flags);


/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif /* GWEN_DIRECTORY_H */




