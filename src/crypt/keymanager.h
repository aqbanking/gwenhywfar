/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Nov 17 2003
    copyright   : (C) 2003 by Martin Preuss
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

/** @file keymanager.h
 *
 * @brief This file contains the description of the basic key manager
 *
 * A key manager is used to store, retrieve, add and delete keys.
 * It is used by IPC functions to manage the keys to be used.
 * This is just a base "class", it really doesn't do anything.
 * However, inheriting "classes" can provide services for reading keys
 * from files, or simply hold a list of keys which is forgotten after the
 * program terminates.
 */


#ifndef GWENHYWFAR_KEYMANAGER_H
#define GWENHYWFAR_KEYMANAGER_H


#include <gwenhywfar/crypt.h>
#include <gwenhywfar/keyspec.h>

#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API
typedef struct GWEN_KEYMANAGER GWEN_KEYMANAGER;


GWENHYWFAR_API
typedef const GWEN_CRYPTKEY*
  (*GWEN_KEYMANAGER_GETKEYFN)(GWEN_KEYMANAGER *km,
                              const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
typedef int (*GWEN_KEYMANAGER_ADDKEYFN)(GWEN_KEYMANAGER *km,
                                        const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
typedef int (*GWEN_KEYMANAGER_DELKEYFN)(GWEN_KEYMANAGER *km,
                                        const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
typedef void (*GWEN_KEYMANAGER_FREEDATAFN)(GWEN_KEYMANAGER *km);



GWENHYWFAR_API
void GWEN_KeyManager_free(GWEN_KEYMANAGER *km);


GWENHYWFAR_API
const GWEN_CRYPTKEY *GWEN_KeyManager_GetKey(GWEN_KEYMANAGER *km,
                                            const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
int GWEN_KeyManager_AddKey(GWEN_KEYMANAGER *km,
                           const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
int GWEN_KeyManager_DeleteKey(GWEN_KEYMANAGER *km,
                              const GWEN_CRYPTKEY *key);


/** @name Functions for Inheritors
 *
 * These functiuons are not to be used by the application but by
 * inheritors (like "protected" in C++)
 */
/*@{*/
GWENHYWFAR_API
GWEN_KEYMANAGER *GWEN_KeyManager_new();

GWENHYWFAR_API
void GWEN_KeyManager_SetGetKeyFn(GWEN_KEYMANAGER *km,
                                 GWEN_KEYMANAGER_GETKEYFN fn);
GWENHYWFAR_API
void GWEN_KeyManager_SetAddKeyFn(GWEN_KEYMANAGER *km,
                                 GWEN_KEYMANAGER_ADDKEYFN fn);
GWENHYWFAR_API
void GWEN_KeyManager_SetDelKeyFn(GWEN_KEYMANAGER *km,
                                 GWEN_KEYMANAGER_DELKEYFN fn);
GWENHYWFAR_API
void GWEN_KeyManager_SetFreeDataFn(GWEN_KEYMANAGER *km,
                                   GWEN_KEYMANAGER_FREEDATAFN fn);
GWENHYWFAR_API
void GWEN_KeyManager_SetData(GWEN_KEYMANAGER *km,
                             void *data);
GWENHYWFAR_API
void *GWEN_KeyManager_GetData(GWEN_KEYMANAGER *km);

#ifdef __cplusplus
}
#endif


/*@}*/


#endif




