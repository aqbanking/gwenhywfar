/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
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

#ifndef GWENHYWFAR_IPCXMSECCTX_H
#define GWENHYWFAR_IPCXMSECCTX_H


#include <gwenhywfar/crypt.h>
#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/seccontext.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/ipcxmlkeymanager.h>


/** @defgroup MOD_IPCXML_SECCTX_ALL IPCXML Security Context Management
 * @ingroup MOD_IPC_SECURITY
 * @brief Extends @ref MOD_SECCTX
 *
 */
/*@{*/

/** @defgroup MOD_IPCXML_SECCTX IPCXML Security Context
 *
 */
/*@{*/

typedef struct GWEN_IPCXMLSECCTXDATA GWEN_IPCXMLSECCTXDATA;


GWEN_SECCTX *GWEN_IPCXMLSecCtx_new(const char *localName,
                                   const char *remoteName);


GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetSessionKey(GWEN_SECCTX *d);
void GWEN_IPCXMLSecCtx_SetSessionKey(GWEN_SECCTX *d,
                                     GWEN_CRYPTKEY *k);

const char *GWEN_IPCXMLSecCtx_GetServiceCode(GWEN_SECCTX *d);
void GWEN_IPCXMLSecCtx_SetServiceCode(GWEN_SECCTX *d,
                                      const char *s);

unsigned int GWEN_IPCXMLSecCtx_GetLocalSignSeq(GWEN_SECCTX *sc);
void GWEN_IPCXMLSecCtx_SetLocalSignSeq(GWEN_SECCTX *sc,
                                       unsigned int i);
unsigned int GWEN_IPCXMLSecCtx_GetRemoteSignSeq(GWEN_SECCTX *sc);
void GWEN_IPCXMLSecCtx_SetRemoteSignSeq(GWEN_SECCTX *sc,
                                        unsigned int i);

/**
 * Does NOT take over ownership of the given key.
 */
void GWEN_IPCXMLSecCtx_SetLocalSignKey(GWEN_SECCTX *sc,
                                       const GWEN_CRYPTKEY *key);

/**
 * Does NOT take over ownership of the given key.
 */
void GWEN_IPCXMLSecCtx_SetLocalCryptKey(GWEN_SECCTX *sc,
                                        const GWEN_CRYPTKEY *key);

/**
 * Returns a pointer to the remote sign key, if any.
 */
const GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetRemoteSignKey(GWEN_SECCTX *sc);

/**
 * Takes over ownership of the given key.
 */
void GWEN_IPCXMLSecCtx_SetRemoteSignKey(GWEN_SECCTX *sc,
                                        GWEN_CRYPTKEY *key);

/**
 * Returns a pointer to the remote crypt key, if any.
 */
const GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetRemoteCryptKey(GWEN_SECCTX *sc);

/**
 * Takes over ownership of the given key.
 */
void GWEN_IPCXMLSecCtx_SetRemoteCryptKey(GWEN_SECCTX *sc,
                                         GWEN_CRYPTKEY *key);

/*@}*/


/** @defgroup MOD_IPCXML_SECCTXMGR IPCXML Security Context Manager
 *
 */
/*@{*/


GWEN_SECCTX_MANAGER *GWEN_IPCXMLSecCtxMgr_new(const char *serviceCode,
                                              const char *dir);

/**
 * Does NOT take over ownership of the given key.
 */
void GWEN_IPCXMLSecCtxMgr_SetLocalSignKey(GWEN_SECCTX_MANAGER *scm,
                                          const GWEN_CRYPTKEY *key);


const GWEN_CRYPTKEY*
  GWEN_IPCXMLSecCtxMgr_GetLocalSignKey(GWEN_SECCTX_MANAGER *scm);

/**
 * Does NOT take over ownership of the given key.
 */
void GWEN_IPCXMLSecCtxMgr_SetLocalCryptKey(GWEN_SECCTX_MANAGER *scm,
                                           const GWEN_CRYPTKEY *key);

const GWEN_CRYPTKEY*
  GWEN_IPCXMLSecCtxMgr_GetLocalCryptKey(GWEN_SECCTX_MANAGER *scm);

/*@}*/
/*@}*/

#endif






