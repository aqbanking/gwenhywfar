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

GWENHYFWAR_API typedef struct GWEN_DIRECTORYDATASTRUCT GWEN_DIRECTORYDATA;



/**
 * Constructor.
 */
GWENHYFWAR_API GWEN_DIRECTORYDATA *GWEN_Directory_new();

/**
 * Destructor
 */
GWENHYFWAR_API void GWEN_Directory_free(GWEN_DIRECTORYDATA *d);

/**
 * Opens a directory. This allows calling "Directory_Read" to succeed.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref GWEN_Directory_new().
 * @param n path and name of the directory to open
 */
GWENHYFWAR_API int GWEN_Directory_Open(GWEN_DIRECTORYDATA *d, const char *n);

/**
 * Closes a previously opened directory.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref GWEN_Directory_new().
 */
GWENHYFWAR_API int GWEN_Directory_Close(GWEN_DIRECTORYDATA *d);

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
GWENHYFWAR_API int GWEN_Directory_Read(GWEN_DIRECTORYDATA *d,
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
GWENHYFWAR_API int GWEN_Directory_Rewind(GWEN_DIRECTORYDATA *d);


/**
 * Get the current users home directory and copy it into the given buffer.
 * @return 0 if ok, !=0 on error
 */
int GWEN_Directory_GetHomeDirectory(char *buffer, unsigned int size);


#ifdef __cplusplus
}
#endif


#endif /* GWEN_DIRECTORY_H */




