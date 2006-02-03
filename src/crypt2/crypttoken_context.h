/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTTOKEN_CONTEXT_H
#define GWEN_CRYPTTOKEN_CONTEXT_H

#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/plugin.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_CRYPTTOKEN_CONTEXT GWEN_CRYPTTOKEN_CONTEXT;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_CONTEXT,
                            GWEN_CryptToken_Context,
                            GWENHYWFAR_API)


typedef enum {
  GWEN_CryptToken_ContextType_Unknown=0,
  GWEN_CryptToken_ContextType_HBCI
} GWEN_CRYPTTOKEN_CONTEXTTYPE;


GWENHYWFAR_API
GWEN_CRYPTTOKEN_CONTEXTTYPE GWEN_CryptToken_ContextType_fromString(const char *s);

GWENHYWFAR_API
const char *GWEN_CryptToken_ContextType_toString(GWEN_CRYPTTOKEN_CONTEXTTYPE t);



/** @name Context
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_new();

GWENHYWFAR_API
void GWEN_CryptToken_Context_free(GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_CONTEXT*
  GWEN_CryptToken_Context_dup(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_fromDb(GWEN_DB_NODE *db);

GWENHYWFAR_API
int GWEN_CryptToken_Context_toDb(const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                 GWEN_DB_NODE *db);


GWENHYWFAR_API
GWEN_TYPE_UINT32
  GWEN_CryptToken_Context_GetId(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void GWEN_CryptToken_Context_SetId(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                   GWEN_TYPE_UINT32 id);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_CONTEXTTYPE
  GWEN_CryptToken_Context_GetContextType(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void GWEN_CryptToken_Context_SetContextType(GWEN_CRYPTTOKEN_CONTEXT *ctx,
					    GWEN_CRYPTTOKEN_CONTEXTTYPE t);


GWENHYWFAR_API
const char*
  GWEN_CryptToken_Context_GetDescription(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void GWEN_CryptToken_Context_SetDescription(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                           const char *s);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetSignKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void
  GWEN_CryptToken_Context_SetSignKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetLocalAuthKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void
  GWEN_CryptToken_Context_SetLocalAuthKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                              const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetRemoteAuthKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void
  GWEN_CryptToken_Context_SetRemoteAuthKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                               const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetVerifyKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void
  GWEN_CryptToken_Context_SetVerifyKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                           const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetEncryptKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void
  GWEN_CryptToken_Context_SetEncryptKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                            const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetDecryptKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void
  GWEN_CryptToken_Context_SetDecryptKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                            const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_Context_GetSignInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void GWEN_CryptToken_Context_SetSignInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const GWEN_CRYPTTOKEN_SIGNINFO *si);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_Context_GetCryptInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void GWEN_CryptToken_Context_SetCryptInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          const GWEN_CRYPTTOKEN_CRYPTINFO *ci);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_Context_GetAuthInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);

GWENHYWFAR_API
void GWEN_CryptToken_Context_SetAuthInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const GWEN_CRYPTTOKEN_SIGNINFO *si);

/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* GWEN_CRYPTTOKEN_CONTEXT_H */


