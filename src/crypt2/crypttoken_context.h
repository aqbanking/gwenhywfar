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


typedef struct GWEN_CRYPTTOKEN_CONTEXT GWEN_CRYPTTOKEN_CONTEXT;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_CONTEXT,
                            GWEN_CryptToken_Context,
                            GWENHYWFAR_API)


typedef enum {
  GWEN_CryptToken_ContextType_Unknown=0,
  GWEN_CryptToken_ContextType_HBCI
} GWEN_CRYPTTOKEN_CONTEXTTYPE;


GWEN_CRYPTTOKEN_CONTEXTTYPE GWEN_CryptToken_ContextType_fromString(const char *s);
const char *GWEN_CryptToken_ContextType_toString(GWEN_CRYPTTOKEN_CONTEXTTYPE t);



/** @name Context
 *
 */
/*@{*/
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_new();
void GWEN_CryptToken_Context_free(GWEN_CRYPTTOKEN_CONTEXT *ctx);
GWEN_CRYPTTOKEN_CONTEXT*
  GWEN_CryptToken_Context_dup(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_fromDb(GWEN_DB_NODE *db);
int GWEN_CryptToken_Context_toDb(const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                 GWEN_DB_NODE *db);
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_fromXml(GWEN_XMLNODE *n);


GWEN_TYPE_UINT32
  GWEN_CryptToken_Context_GetId(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_Context_SetId(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                   GWEN_TYPE_UINT32 id);

GWEN_CRYPTTOKEN_CONTEXTTYPE
  GWEN_CryptToken_Context_GetContextType(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_Context_SetContextType(GWEN_CRYPTTOKEN_CONTEXT *ctx,
					    GWEN_CRYPTTOKEN_CONTEXTTYPE t);


const char*
  GWEN_CryptToken_Context_GetDescription(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_Context_SetDescription(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                           const char *s);

const GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetSignKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void
  GWEN_CryptToken_Context_SetSignKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const GWEN_CRYPTTOKEN_KEYINFO *ki);

const GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetVerifyKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void
  GWEN_CryptToken_Context_SetVerifyKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                           const GWEN_CRYPTTOKEN_KEYINFO *ki);

const GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetEncryptKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void
  GWEN_CryptToken_Context_SetEncryptKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                            const GWEN_CRYPTTOKEN_KEYINFO *ki);

const GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_Context_GetDecryptKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void
  GWEN_CryptToken_Context_SetDecryptKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                            const GWEN_CRYPTTOKEN_KEYINFO *ki);

const GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_Context_GetSignInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_Context_SetSignInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const GWEN_CRYPTTOKEN_SIGNINFO *si);

const GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_Context_GetCryptInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_Context_SetCryptInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
/*@}*/




#endif /* GWEN_CRYPTTOKEN_CONTEXT_H */


