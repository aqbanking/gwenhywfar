/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Nov 19 2003
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


#ifndef GWENHYWFAR_SECCONTEXT_H
#define GWENHYWFAR_SECCONTEXT_H


#include <gwenhywfar/hbcicryptocontext.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_SECCTX_ALL General Security Context
 * @ingroup MOD_IPC_SECURITY
 *
 */
/*@{*/


/** @defgroup MOD_SECCTX Security Context
 *
 */
/*@{*/

#define GWEN_SECCTX_FLAGS_TEMP 0x0001


typedef struct GWEN_SECCTX GWEN_SECCTX;


/** @name Prototypes for Virtual Functions
 *
 */
/*@{*/
typedef int
  (*GWEN_SECCTX_PREPARECTX_FN)(GWEN_SECCTX *sctx,
                                GWEN_HBCICRYPTOCONTEXT *ctx,
                                int crypt);

typedef int
  (*GWEN_SECCTX_SIGN_FN)(GWEN_SECCTX *sctx,
                         GWEN_BUFFER *msgbuf,
                         GWEN_BUFFER *signbuf,
                         GWEN_HBCICRYPTOCONTEXT *ctx);

typedef int
  (*GWEN_SECCTX_VERIFY_FN)(GWEN_SECCTX *sctx,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx);

typedef int
  (*GWEN_SECCTX_ENCRYPT_FN)(GWEN_SECCTX *sctx,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *cryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx);

typedef int
  (*GWEN_SECCTX_DECRYPT_FN)(GWEN_SECCTX *sctx,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *decryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx);

typedef int
  (*GWEN_SECCTX_FROMDB_FN)(GWEN_SECCTX *sctx,
                           GWEN_DB_NODE *db);
typedef int
  (*GWEN_SECCTX_TODB_FN)(GWEN_SECCTX *sctx,
                         GWEN_DB_NODE *db);

typedef void
  (*GWEN_SECCTX_FREEDATA_FN)(GWEN_SECCTX *sctx);
/*@}*/



/** @name Constructor And Destructor
 *
 */
/*@{*/
GWEN_SECCTX *GWEN_SecContext_new(const char *localName,
                                 const char *remoteName);
void GWEN_SecContext_free(GWEN_SECCTX *sc);


/** @name Getters and Setters For Virtual Functions and Data
 *
 */
/*@{*/
void GWEN_SecContext_SetPrepareCtxFn(GWEN_SECCTX *sctx,
                                     GWEN_SECCTX_PREPARECTX_FN fn);
void GWEN_SecContext_SetSignFn(GWEN_SECCTX *sctx,
                               GWEN_SECCTX_SIGN_FN signFn);
void GWEN_SecContext_SetVerifyFn(GWEN_SECCTX *sctx,
                                 GWEN_SECCTX_VERIFY_FN verifyFn);
void GWEN_SecContext_SetEncryptFn(GWEN_SECCTX *sctx,
                                  GWEN_SECCTX_ENCRYPT_FN encryptFn);
void GWEN_SecContext_SetDecrpytFn(GWEN_SECCTX *sctx,
                                  GWEN_SECCTX_DECRYPT_FN decryptFn);
void GWEN_SecContext_SetFreeDataFn(GWEN_SECCTX *sctx,
                                   GWEN_SECCTX_FREEDATA_FN fn);
void GWEN_SecContext_SetFromDbFn(GWEN_SECCTX *sctx,
                                 GWEN_SECCTX_FROMDB_FN fn);
void GWEN_SecContext_SetToDbFn(GWEN_SECCTX *sctx,
                               GWEN_SECCTX_TODB_FN fn);
void *GWEN_SecContext_GetData(GWEN_SECCTX *sc);
void GWEN_SecContext_SetData(GWEN_SECCTX *sc,
                             void *d);
/*@}*/


/** @name General Information
 *
 *
 */
/*@{*/
const char *GWEN_SecContext_GetLocalName(GWEN_SECCTX *sc);
const char *GWEN_SecContext_GetRemoteName(GWEN_SECCTX *sc);

unsigned int GWEN_SecContext_GetFlags(GWEN_SECCTX *sc);
void GWEN_SecContext_SetFlags(GWEN_SECCTX *sc,
                              unsigned int fl);
/*@}*/


/** @name Lock Id
 *
 * This can be used by a context manager to store the lock id when
 * locking a context, it is not meant to be used by an application.
 */
/*@{*/
int GWEN_SecContext_GetLockId(GWEN_SECCTX *sc);
void GWEN_SecContext_SetLockId(GWEN_SECCTX *sc,
                               int id);
/*@}*/


/** @name Local Signature Sequence Counter
 *
 * This counter is used when signing data locally.
 */
/*@{*/
unsigned int GWEN_SecContext_GetLocalSignSeq(GWEN_SECCTX *sc);
void GWEN_SecContext_SetLocalSignSeq(GWEN_SECCTX *sc,
                                     unsigned int i);
/*@}*/

/** @name Remote Signature Sequence Counter
 *
 * This counter is used when verifying a signature created by a remote
 * partner.
 */
/*@{*/
unsigned int GWEN_SecContext_GetRemoteSignSeq(GWEN_SECCTX *sc);
void GWEN_SecContext_SetRemoteSignSeq(GWEN_SECCTX *sc,
                                      unsigned int i);
/*@}*/




/** @name Virtual Functions
 *
 */
/*@{*/

int GWEN_SecContext_PrepareContext(GWEN_SECCTX *sctx,
                                   GWEN_HBCICRYPTOCONTEXT *ctx,
                                   int crypt);


int GWEN_SecContext_Sign(GWEN_SECCTX *sctx,
                         GWEN_BUFFER *msgbuf,
                         GWEN_BUFFER *signbuf,
                         GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_SecContext_Verify(GWEN_SECCTX *sctx,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_SecContext_Encrypt(GWEN_SECCTX *sctx,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *cryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_SecContext_Decrypt(GWEN_SECCTX *sctx,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *decryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_SecContext_FromDB(GWEN_SECCTX *sc, GWEN_DB_NODE *db);
int GWEN_SecContext_ToDB(GWEN_SECCTX *sc, GWEN_DB_NODE *db);
/*@}*/


/** @name Padding Functions
 * These functions are used for padding when encrypting/decrypting data
 * using 2-key-triple-DES or when signing data.
 * The original code (in C++) has been written by
 * <strong>Fabian Kaiser</strong> for <strong>OpenHBCI</strong>
 * (file rsakey.cpp). Translated to C and slightly modified by me
 * (Martin Preuss)
 */
/*@{*/
/**
 */
int GWEN_SecContext_PaddWithISO9796(GWEN_BUFFER *src);

/**
 * This function is used to pad the plain text data to a multiple of 8 bytes
 * size before encryoting it.
 */
int GWEN_SecContext_PaddWithANSIX9_23(GWEN_BUFFER *src);

/**
 * This function is used to remove padding from plain text data after
 * decrypting it.
 */
int GWEN_SecContext_UnpaddWithANSIX9_23(GWEN_BUFFER *src);
/*@}*/ /* name */


/*@}*/ /* defgroup */


/** @defgroup MOD_SECCTX_MANAGER Security Context Manager
 *
 */
/*@{*/
typedef struct GWEN_SECCTX_MANAGER GWEN_SECCTX_MANAGER;

/** @name Prototypes for Virtual Functions
 *
 */
/*@{*/
typedef GWEN_SECCTX*
  (*GWEN_SECCTXMGR_GETCONTEXT_FN)(GWEN_SECCTX_MANAGER *scm,
                                  const char *localName,
                                  const char *remoteName);


typedef int
  (*GWEN_SECCTXMGR_ADDCONTEXT_FN)(GWEN_SECCTX_MANAGER *scm,
                                  GWEN_SECCTX *sc,
                                  int tmp);


typedef int
  (*GWEN_SECCTXMGR_DELCONTEXT_FN)(GWEN_SECCTX_MANAGER *scm,
                                  GWEN_SECCTX *sc);

typedef int
  (*GWEN_SECCTXMGR_RELEASECONTEXT_FN)(GWEN_SECCTX_MANAGER *scm,
                                      GWEN_SECCTX *sc,
                                      int aban);

typedef void
  (*GWEN_SECCTXMGR_FREEDATA_FN)(GWEN_SECCTX_MANAGER *scm);
/*@}*/


/** @name Constructor And Destructor
 *
 */
/*@{*/
GWEN_SECCTX_MANAGER *GWEN_SecContextMgr_new(const char *serviceCode);
void GWEN_SecContextMgr_free(GWEN_SECCTX_MANAGER *scm);
/*@}*/

/** @name Setters And Getters For Virtual Functions And Data
 *
 */
/*@{*/
void GWEN_SecContextMgr_SetGetFn(GWEN_SECCTX_MANAGER *scm,
                                 GWEN_SECCTXMGR_GETCONTEXT_FN fn);

void GWEN_SecContextMgr_SetAddFn(GWEN_SECCTX_MANAGER *scm,
                                 GWEN_SECCTXMGR_ADDCONTEXT_FN fn);

void GWEN_SecContextMgr_SetDelFn(GWEN_SECCTX_MANAGER *scm,
                                 GWEN_SECCTXMGR_DELCONTEXT_FN fn);
void GWEN_SecContextMgr_SetReleaseFn(GWEN_SECCTX_MANAGER *scm,
                                     GWEN_SECCTXMGR_RELEASECONTEXT_FN fn);

void GWEN_SecContextMgr_SetFreeDataFn(GWEN_SECCTX_MANAGER *scm,
                                      GWEN_SECCTXMGR_FREEDATA_FN fn);

void *GWEN_SecContextMgr_GetData(GWEN_SECCTX_MANAGER *scm);
void GWEN_SecContextMgr_SetData(GWEN_SECCTX_MANAGER *scm, void *d);
/*@}*/


/** @name Managing Contexts
 *
 */
/*@{*/
/**
 * This function keeps ownership of the given context, so you MUST NOT free
 * the returned context, if any.
 */
GWEN_SECCTX *GWEN_SecContextMgr_GetContext(GWEN_SECCTX_MANAGER *scm,
                                           const char *localName,
                                           const char *remoteName);

/**
 * This function releases a context acquired via
 * @ref GWEN_SecContextMgr_GetContext.
 */
int GWEN_SecContextMgr_ReleaseContext(GWEN_SECCTX_MANAGER *scm,
                                      GWEN_SECCTX *sc,
                                      int aban);


/**
 * This function takes over ownership of the given context.
 */
int GWEN_SecContextMgr_AddContext(GWEN_SECCTX_MANAGER *scm,
                                  GWEN_SECCTX *sc,
                                  int tmp);


/**
 * This function most probably frees the given context, so you MUST NOT
 * use it after calling this function.
 */
int GWEN_SecContextMgr_DelContext(GWEN_SECCTX_MANAGER *scm,
                                  GWEN_SECCTX *sc);

/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
const char *GWEN_SecContextMgr_GetServiceCode(GWEN_SECCTX_MANAGER *scm);
/*@}*/ /* name */

/*@}*/ /* defgroup */
/*@}*/ /* defgroup (all) */

#ifdef __cplusplus
}
#endif


#endif



