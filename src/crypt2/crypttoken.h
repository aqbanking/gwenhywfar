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


#ifndef GWEN_CRYPTTOKEN_H
#define GWEN_CRYPTTOKEN_H

#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/plugin.h>

#define GWEN_CRYPTMANAGER_NAME "GWEN_CRYPTMANAGER"


#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_SIGN      0x00000001
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_VERIFY    0x00000002
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_ENCRYPT   0x00000004
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_DECRYPT   0x00000008
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_READABLE      0x00000010
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_WRITEABLE     0x00000020
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_HAS_SIGNSEQ   0x00000040


#define GWEN_CRYPTTOKEN_FLAGS_CONTEXT_MANAGEMENT    0x00000001
#define GWEN_CRYPTTOKEN_FLAGS_CONTEXT_ALLOW_ADD     0x00000002
#define GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_KEYS       0x00000004
#define GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_DESCR      0x00000008
#define GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_SIGNDESCR  0x00000010
#define GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_CRYPTDESCR 0x00000020
#define GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_KEYDESCRS  0x00000040

#define GWEN_CRYPTTOKEN_FLAGS_MANAGES_SIGNSEQ       0x00010000
#define GWEN_CRYPTTOKEN_FLAGS_PREDEF_CONTEXT_ONLY   0x00020000



typedef struct GWEN_CRYPTTOKEN_SIGNINFO GWEN_CRYPTTOKEN_SIGNINFO;
typedef struct GWEN_CRYPTTOKEN_CRYPTINFO GWEN_CRYPTTOKEN_CRYPTINFO;
typedef struct GWEN_CRYPTTOKEN_KEYINFO GWEN_CRYPTTOKEN_KEYINFO;
typedef struct GWEN_CRYPTTOKEN_CONTEXT GWEN_CRYPTTOKEN_CONTEXT;
typedef struct GWEN_CRYPTTOKEN GWEN_CRYPTTOKEN;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_SIGNINFO,
                            GWEN_CryptToken_SignInfo,
                            GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_CRYPTINFO,
                            GWEN_CryptToken_CryptInfo,
                            GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_KEYINFO,
                            GWEN_CryptToken_KeyInfo,
                            GWENHYWFAR_API)

GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_CONTEXT,
                            GWEN_CryptToken_Context,
                            GWENHYWFAR_API)

GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN,
                            GWEN_CryptToken,
                            GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN,
                               GWENHYWFAR_API)


typedef enum {
  GWEN_CryptToken_HashAlgo_Unknown=-1,
  GWEN_CryptToken_HashAlgo_None=0,
  /** SHA-1 */
  GWEN_CryptToken_HashAlgo_SHA1,
  /** Ripemd 160 */
  GWEN_CryptToken_HashAlgo_RMD160,
  /** MD-5 */
  GWEN_CryptToken_HashAlgo_MD5
} GWEN_CRYPTTOKEN_HASHALGO;

GWEN_CRYPTTOKEN_HASHALGO GWEN_CryptToken_HashAlgo_fromString(const char *s);
const char *GWEN_CryptToken_HashAlgo_toString(GWEN_CRYPTTOKEN_HASHALGO a);


typedef enum {
  GWEN_CryptToken_PaddAlgo_Unknown=-1,
  GWEN_CryptToken_PaddAlgo_None=0,
  /** ISO 9796/1 without appendix 4 */
  GWEN_CryptToken_PaddAlgo_ISO9796_1,
  /** ISO 9796/1 with appendix 4 */
  GWEN_CryptToken_PaddAlgo_ISO9796_1A4,
  /** ISO 9796/2 with random (SigG v1.0) */
  GWEN_CryptToken_PaddAlgo_ISO9796_2,
  /** PKCS#1 block type 01 */
  GWEN_CryptToken_PaddAlgo_PKCS1_1,
  /** PKCS#1 block type 02 */
  GWEN_CryptToken_PaddAlgo_PKCS1_2,
  /** fill left with zeroes */
  GWEN_CryptToken_PaddAlgo_LeftZero,
  /** fill right with zeroes */
  GWEN_CryptToken_PaddAlgo_RightZero,
  /** ANSI X9.23 */
  GWEN_CryptToken_PaddAlgo_ANSIX9_23
} GWEN_CRYPTTOKEN_PADDALGO;

GWEN_CRYPTTOKEN_PADDALGO GWEN_CryptToken_PaddAlgo_fromString(const char *s);
const char *GWEN_CryptToken_PaddAlgo_toString(GWEN_CRYPTTOKEN_PADDALGO a);


typedef enum {
  GWEN_CryptToken_CryptAlgo_Unknown=-1,
  GWEN_CryptToken_CryptAlgo_None=0,
  /** RSA */
  GWEN_CryptToken_CryptAlgo_RSA,
  /** DSA */
  GWEN_CryptToken_CryptAlgo_DSA,
  /* DES */
  GWEN_CryptToken_CryptAlgo_DES,
  /** triple-key DES */
  GWEN_CryptToken_CryptAlgo_DES_3K
} GWEN_CRYPTTOKEN_CRYPTALGO;

GWEN_CRYPTTOKEN_CRYPTALGO GWEN_CryptToken_CryptAlgo_fromString(const char *s);
const char *GWEN_CryptToken_CryptAlgo_toString(GWEN_CRYPTTOKEN_CRYPTALGO a);



typedef enum {
  GWEN_CryptToken_Device_Unknown=-1,
  GWEN_CryptToken_Device_None=0,
  GWEN_CryptToken_Device_File,
  GWEN_CryptToken_Device_Card
} GWEN_CRYPTTOKEN_DEVICE;

GWEN_CRYPTTOKEN_DEVICE GWEN_CryptToken_Device_fromString(const char *s);
const char *GWEN_CryptToken_Device_toString(GWEN_CRYPTTOKEN_DEVICE d);



typedef enum {
  GWEN_CryptToken_PinType_Unknown=-1,
  GWEN_CryptToken_PinType_None=0,
  GWEN_CryptToken_PinType_Access,
  GWEN_CryptToken_PinType_Manage
} GWEN_CRYPTTOKEN_PINTYPE;

GWEN_CRYPTTOKEN_PINTYPE GWEN_CryptToken_PinType_fromString(const char *s);
const char *GWEN_CryptToken_PinType_toString(GWEN_CRYPTTOKEN_PINTYPE pt);



typedef enum {
  GWEN_CryptToken_PinEncoding_Unknown=-1,
  GWEN_CryptToken_PinEncoding_None=0,
  GWEN_CryptToken_PinEncoding_Bin,
  GWEN_CryptToken_PinEncoding_BCD,
  GWEN_CryptToken_PinEncoding_ASCII
} GWEN_CRYPTTOKEN_PINENCODING;

GWEN_CRYPTTOKEN_PINENCODING
GWEN_CryptToken_PinEncoding_fromString(const char *s);
const char*
  GWEN_CryptToken_PinEncoding_toString(GWEN_CRYPTTOKEN_PINENCODING pe);




/** @name SignInfo
 *
 */
/*@{*/
GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_new();
void GWEN_CryptToken_SignInfo_free(GWEN_CRYPTTOKEN_SIGNINFO *si);
GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_SignInfo_dup(const GWEN_CRYPTTOKEN_SIGNINFO *si);
GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_fromDb(GWEN_DB_NODE *db);
void GWEN_CryptToken_SignInfo_toDb(const GWEN_CRYPTTOKEN_SIGNINFO *si,
                                   GWEN_DB_NODE *db);
GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_fromXml(GWEN_XMLNODE *n);


GWEN_TYPE_UINT32
  GWEN_CryptToken_SignInfo_GetId(const GWEN_CRYPTTOKEN_SIGNINFO *si);
void GWEN_CryptToken_SignInfo_SetId(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                    GWEN_TYPE_UINT32 id);

GWEN_CRYPTTOKEN_HASHALGO
  GWEN_CryptToken_SignInfo_GetHashAlgo(const GWEN_CRYPTTOKEN_SIGNINFO *si);
void GWEN_CryptToken_SignInfo_SetHashAlgo(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                          GWEN_CRYPTTOKEN_HASHALGO a);

GWEN_CRYPTTOKEN_PADDALGO
  GWEN_CryptToken_SignInfo_GetPaddAlgo(const GWEN_CRYPTTOKEN_SIGNINFO *si);
void GWEN_CryptToken_SignInfo_SetPaddAlgo(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                          GWEN_CRYPTTOKEN_PADDALGO a);
/*@}*/



/** @name CryptInfo
 *
 */
/*@{*/
GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_new();
void GWEN_CryptToken_CryptInfo_free(GWEN_CRYPTTOKEN_CRYPTINFO *ci);
GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_CryptInfo_dup(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);

GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_fromDb(GWEN_DB_NODE *db);
void GWEN_CryptToken_CryptInfo_toDb(const GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                    GWEN_DB_NODE *db);
GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_fromXml(GWEN_XMLNODE *n);


GWEN_TYPE_UINT32
  GWEN_CryptToken_CryptInfo_GetId(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
void GWEN_CryptToken_CryptInfo_SetId(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                     GWEN_TYPE_UINT32 id);

GWEN_CRYPTTOKEN_CRYPTALGO
  GWEN_CryptToken_CryptInfo_GetCryptAlgo(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
void GWEN_CryptToken_CryptInfo_SetCryptAlgo(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                            GWEN_CRYPTTOKEN_CRYPTALGO a);

GWEN_CRYPTTOKEN_PADDALGO
  GWEN_CryptToken_CryptInfo_GetPaddAlgo(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
void GWEN_CryptToken_CryptInfo_SetPaddAlgo(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                           GWEN_CRYPTTOKEN_PADDALGO a);
/*@}*/



/** @name KeyInfo
 *
 */
/*@{*/
GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_new();
void GWEN_CryptToken_KeyInfo_free(GWEN_CRYPTTOKEN_KEYINFO *ki);
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_KeyInfo_dup(const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_fromDb(GWEN_DB_NODE *db);
void GWEN_CryptToken_KeyInfo_toDb(const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                  GWEN_DB_NODE *db);
GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_fromXml(GWEN_XMLNODE *n);

int GWEN_CryptToken_KeyInfo_GetKeyId(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetKeyId(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                      int id);

int GWEN_CryptToken_KeyInfo_GetKeySize(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetKeySize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                        int i);

int GWEN_CryptToken_KeyInfo_GetChunkSize(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetChunkSize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                          int i);

GWEN_CRYPTTOKEN_CRYPTALGO
  GWEN_CryptToken_KeyInfo_GetCryptAlgo(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetCryptAlgo(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                          GWEN_CRYPTTOKEN_CRYPTALGO a);

GWEN_TYPE_UINT32
  GWEN_CryptToken_KeyInfo_GetKeyFlags(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl);
void GWEN_CryptToken_KeyInfo_AddKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl);
void GWEN_CryptToken_KeyInfo_SubKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl);

const char*
  GWEN_CryptToken_KeyInfo_GetDescription(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetDescription(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                            const char *s);

/*@}*/





/** @name Context
 *
 */
/*@{*/
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_new();
void GWEN_CryptToken_Context_free(GWEN_CRYPTTOKEN_CONTEXT *ctx);
GWEN_CRYPTTOKEN_CONTEXT*
  GWEN_CryptToken_Context_dup(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_fromDb(GWEN_DB_NODE *db);
void GWEN_CryptToken_Context_toDb(const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                  GWEN_DB_NODE *db);
GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_fromXml(GWEN_XMLNODE *n);


GWEN_TYPE_UINT32
  GWEN_CryptToken_Context_GetId(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_Context_SetId(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                   GWEN_TYPE_UINT32 id);

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
  GWEN_CryptToken_GetSignInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_SetSignInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                 const GWEN_CRYPTTOKEN_SIGNINFO *si);

const GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_GetCryptInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx);
void GWEN_CryptToken_SetCryptInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                  const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
/*@}*/





/** @name CryptToken
 *
 */
/*@{*/



typedef int (*GWEN_CRYPTTOKEN_OPEN_FN)(GWEN_CRYPTTOKEN *ct,
                                       int manage);
typedef int (*GWEN_CRYPTTOKEN_CLOSE_FN)(GWEN_CRYPTTOKEN *ct);

typedef int (*GWEN_CRYPTTOKEN_SIGN_FN)(GWEN_CRYPTTOKEN *ct,
                                       const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *dst);
typedef int (*GWEN_CRYPTTOKEN_VERIFY_FN)(GWEN_CRYPTTOKEN *ct,
                                         const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         GWEN_BUFFER *src,
                                         GWEN_BUFFER *dst);
typedef int (*GWEN_CRYPTTOKEN_ENCRYPT_FN)(GWEN_CRYPTTOKEN *ct,
                                          const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          GWEN_BUFFER *src,
                                          GWEN_BUFFER *dst);
typedef int (*GWEN_CRYPTTOKEN_DECRYPT_FN)(GWEN_CRYPTTOKEN *ct,
                                          const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          GWEN_BUFFER *src,
                                          GWEN_BUFFER *dst);
typedef int (*GWEN_CRYPTTOKEN_READKEY_FN)(GWEN_CRYPTTOKEN *ct,
                                          GWEN_TYPE_UINT32 kid,
                                          GWEN_CRYPTKEY **key);
typedef int (*GWEN_CRYPTTOKEN_WRITEKEY_FN)(GWEN_CRYPTTOKEN *ct,
                                           GWEN_TYPE_UINT32 kid,
                                           const GWEN_CRYPTKEY *key);
typedef int (*GWEN_CRYPTTOKEN_GENERATEKEY_FN)(GWEN_CRYPTTOKEN *ct,
                                              const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                              GWEN_CRYPTKEY **key);

typedef int (*GWEN_CRYPTTOKEN_GETSIGNSEQ_FN)(GWEN_CRYPTTOKEN *ct,
                                             GWEN_TYPE_UINT32 kid,
                                             GWEN_TYPE_UINT32 *signSeq);

typedef int
  (*GWEN_CRYPTTOKEN_ADDCONTEXT_FN)(GWEN_CRYPTTOKEN *ct,
                                   const GWEN_CRYPTTOKEN_CONTEXT *ctx);
typedef int
  (*GWEN_CRYPTTOKEN_FILLCONTEXTLIST_FN)(GWEN_CRYPTTOKEN *ct,
                                        GWEN_CRYPTTOKEN_CONTEXT_LIST *l);
typedef int
  (*GWEN_CRYPTTOKEN_FILLSIGNINFOLIST_FN)(GWEN_CRYPTTOKEN *ct,
                                         GWEN_CRYPTTOKEN_SIGNINFO_LIST *l);

typedef int
  (*GWEN_CRYPTTOKEN_FILLCRYPTINFOLIST_FN)(GWEN_CRYPTTOKEN *ct,
                                          GWEN_CRYPTTOKEN_CRYPTINFO_LIST *l);

typedef int
  (*GWEN_CRYPTTOKEN_FILLKEYINFOLIST_FN)(GWEN_CRYPTTOKEN *ct,
                                        GWEN_CRYPTTOKEN_KEYINFO_LIST *l);




GWEN_CRYPTTOKEN *GWEN_CryptToken_new(GWEN_PLUGIN_MANAGER *pm,
                                     GWEN_CRYPTTOKEN_DEVICE devType,
                                     const char *typeName,
                                     const char *subTypeName,
                                     const char *name);
void GWEN_CryptToken_free(GWEN_CRYPTTOKEN *ct);

GWEN_CRYPTTOKEN *GWEN_CryptToken_fromXml(GWEN_PLUGIN_MANAGER *pm,
                                         GWEN_CRYPTTOKEN_DEVICE devType,
                                         GWEN_XMLNODE *n);

int GWEN_CryptToken_ReadXml(GWEN_CRYPTTOKEN *ct, GWEN_XMLNODE *n);

GWEN_PLUGIN_MANAGER*
GWEN_CryptToken_GetCryptManager(const GWEN_CRYPTTOKEN *ct);
const char *GWEN_CryptToken_GetTokenName(const GWEN_CRYPTTOKEN *ct);
void GWEN_CryptToken_SetTokenName(GWEN_CRYPTTOKEN *ct, const char *s);

const char *GWEN_CryptToken_GetTokenType(const GWEN_CRYPTTOKEN *ct);
const char *GWEN_CryptToken_GetTokenSubType(const GWEN_CRYPTTOKEN *ct);
GWEN_CRYPTTOKEN_DEVICE
GWEN_CryptToken_GetDeviceType(const GWEN_CRYPTTOKEN *ct);


void GWEN_CryptToken_SetOpenFn(GWEN_CRYPTTOKEN *ct,
                               GWEN_CRYPTTOKEN_OPEN_FN fn);
void GWEN_CryptToken_SetCloseFn(GWEN_CRYPTTOKEN *ct,
                                GWEN_CRYPTTOKEN_CLOSE_FN fn);
void GWEN_CryptToken_SetSignFn(GWEN_CRYPTTOKEN *ct,
                               GWEN_CRYPTTOKEN_SIGN_FN fn);
void GWEN_CryptToken_SetVerifyFn(GWEN_CRYPTTOKEN *ct,
                                 GWEN_CRYPTTOKEN_VERIFY_FN fn);
void GWEN_CryptToken_SetEncryptFn(GWEN_CRYPTTOKEN *ct,
                                  GWEN_CRYPTTOKEN_ENCRYPT_FN fn);
void GWEN_CryptToken_SetDecryptFn(GWEN_CRYPTTOKEN *ct,
                                  GWEN_CRYPTTOKEN_DECRYPT_FN fn);
void GWEN_CryptToken_SetReadKeyFn(GWEN_CRYPTTOKEN *ct,
                                  GWEN_CRYPTTOKEN_READKEY_FN fn);
void GWEN_CryptToken_SetWriteKeyFn(GWEN_CRYPTTOKEN *ct,
                                   GWEN_CRYPTTOKEN_WRITEKEY_FN fn);
void GWEN_CryptToken_SetGenerateKeyFn(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_GENERATEKEY_FN fn);
void GWEN_CryptToken_SetAddContextFn(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_ADDCONTEXT_FN fn);
void GWEN_CryptToken_SetFillContextListFn(GWEN_CRYPTTOKEN *ct,
                                          GWEN_CRYPTTOKEN_FILLCONTEXTLIST_FN fn);
void GWEN_CryptToken_SetFillSignInfoListFn(GWEN_CRYPTTOKEN *ct,
                                           GWEN_CRYPTTOKEN_FILLSIGNINFOLIST_FN fn);
void GWEN_CryptToken_SetFillCryptInfoListFn(GWEN_CRYPTTOKEN *ct,
                                            GWEN_CRYPTTOKEN_FILLCRYPTINFOLIST_FN fn);
void GWEN_CryptToken_SetFillKeyInfoListFn(GWEN_CRYPTTOKEN *ct,
                                          GWEN_CRYPTTOKEN_FILLKEYINFOLIST_FN fn);
void GWEN_CryptToken_SetGetSignSeqFn(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_GETSIGNSEQ_FN fn);



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                               Virtual Functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

int GWEN_CryptToken_Open(GWEN_CRYPTTOKEN *ct, int manage);

int GWEN_CryptToken_Close(GWEN_CRYPTTOKEN *ct);

int GWEN_CryptToken_Sign(GWEN_CRYPTTOKEN *ct,
                         const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                         GWEN_BUFFER *src,
                         GWEN_BUFFER *dst);
int GWEN_CryptToken_Verify(GWEN_CRYPTTOKEN *ct,
                           const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                           GWEN_BUFFER *src,
                           GWEN_BUFFER *dst);
int GWEN_CryptToken_Encrypt(GWEN_CRYPTTOKEN *ct,
                            const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                            GWEN_BUFFER *src,
                            GWEN_BUFFER *dst);
int GWEN_CryptToken_Decrypt(GWEN_CRYPTTOKEN *ct,
                            const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                            GWEN_BUFFER *src,
                            GWEN_BUFFER *dst);
int GWEN_CryptToken_ReadKey(GWEN_CRYPTTOKEN *ct,
                            GWEN_TYPE_UINT32 kid,
                            GWEN_CRYPTKEY **key);
int GWEN_CryptToken_WriteKey(GWEN_CRYPTTOKEN *ct,
                             GWEN_TYPE_UINT32 kid,
                             const GWEN_CRYPTKEY *key);
int GWEN_CryptToken_GenerateKey(GWEN_CRYPTTOKEN *ct,
                                const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                GWEN_CRYPTKEY **key);

int GWEN_CryptToken_GetSignSeq(GWEN_CRYPTTOKEN *ct,
                               GWEN_TYPE_UINT32 kid,
                               GWEN_TYPE_UINT32 *signSeq);

int GWEN_CryptToken_AddContext(GWEN_CRYPTTOKEN *ct,
                               const GWEN_CRYPTTOKEN_CONTEXT *ctx);
int GWEN_CryptToken_FillContextList(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_CONTEXT_LIST *l);
int GWEN_CryptToken_FillSignInfoList(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_SIGNINFO_LIST *l);

int GWEN_CryptToken_FillCryptInfoList(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_CRYPTINFO_LIST *l);

int GWEN_CryptToken_FillKeyInfoList(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_KEYINFO_LIST *l);




const GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_GetSignInfoByAlgos(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_HASHALGO hashAlgo,
                                     GWEN_CRYPTTOKEN_PADDALGO paddAlgo);

const GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_GetCryptInfoByAlgos(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_CRYPTALGO cryptAlgo,
                                      GWEN_CRYPTTOKEN_PADDALGO paddAlgo);

const GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_GetKeyInfoById(GWEN_CRYPTTOKEN *ct,
                                 GWEN_TYPE_UINT32 kid);

const GWEN_CRYPTTOKEN_CONTEXT*
  GWEN_CryptToken_GetContextById(GWEN_CRYPTTOKEN *ct,
                                 GWEN_TYPE_UINT32 id);


int GWEN_CryptToken_Hash(GWEN_CRYPTTOKEN_HASHALGO algo,
                         const char *src,
                         unsigned int slen,
                         GWEN_BUFFER *dstBuf);

int GWEN_CryptToken_Padd(GWEN_CRYPTTOKEN_PADDALGO algo,
                         unsigned int dstSize,
                         GWEN_BUFFER *buf);

int GWEN_CryptToken_Unpadd(GWEN_CRYPTTOKEN_PADDALGO algo,
                           GWEN_BUFFER *buf);


/*@}*/




/** @name CryptToken_Plugin
 *
 */
/*@{*/

typedef GWEN_CRYPTTOKEN*
  (*GWEN_CRYPTTOKEN_PLUGIN_CREATETOKEN_FN)(GWEN_PLUGIN *pl,
                                           const char *subTypeName,
                                           const char *name);

typedef int
  (*GWEN_CRYPTTOKEN_PLUGIN_CHECKTOKEN_FN)(GWEN_PLUGIN *pl,
                                          GWEN_BUFFER *subTypeName,
                                          GWEN_BUFFER *name);



GWEN_PLUGIN *GWEN_CryptToken_Plugin_new(GWEN_PLUGIN_MANAGER *mgr,
                                        GWEN_CRYPTTOKEN_DEVICE devType,
                                        const char *typeName,
                                        const char *fileName);


GWEN_CRYPTTOKEN*
GWEN_CryptToken_Plugin_CreateToken(GWEN_PLUGIN *pl,
                                   const char *subTypeName,
                                   const char *name);

/**
 * <p>This function is used to let a crypt token plugin check whether it
 * supports a given plugin.</p>
 * <p>Initially the given buffers may contain some values to narrow the
 * search. For chip cards the <i>name</i> argument may contain the serial
 * number of the card (if known). For file based crypt tokens this argument
 * must contain the path to the file to check.</p>
 *
 * This function should return one of the following error codes:
 * <ul>
 *  <li>GWEN_SUCCESS: CryptToken is supported by this plugin, the buffers
 *   for typeName, subTypeName and name are updated accordingly</li>
 *  <li>GWEN_ERROR_CT_NOT_IMPLEMENTED: function not implmeneted</li>
 *  <li>GWEN_ERROR_CT_NOT_SUPPORTED: medium not supported by this plugin</li>
 *  <li>GWEN_ERROR_CT_BAD_NAME: Medium is supported but the name doesn't
 *      match of the currently checked medium</li>
 *  <li>GWEN_ERROR_CT_IO_ERROR: any type of IO error occurred</li>
 *  <li>other codes as appropriate</li>
 * </ul>
 */
int GWEN_CryptToken_Plugin_CheckToken(GWEN_PLUGIN *pl,
                                      GWEN_BUFFER *subTypeName,
                                      GWEN_BUFFER *name);


GWEN_PLUGIN_MANAGER*
  GWEN_CryptToken_Plugin_GetCryptManager(const GWEN_PLUGIN *pl);


GWEN_CRYPTTOKEN_DEVICE
  GWEN_CryptToken_Plugin_GetDeviceType(const GWEN_PLUGIN *pl);


void GWEN_CryptToken_Plugin_SetCreateTokenFn(GWEN_PLUGIN *pl,
                                             GWEN_CRYPTTOKEN_PLUGIN_CREATETOKEN_FN fn);
void GWEN_CryptToken_Plugin_SetCheckTokenFn(GWEN_PLUGIN *pl,
                                            GWEN_CRYPTTOKEN_PLUGIN_CHECKTOKEN_FN fn);


/*@}*/



/** @name CryptManager
 *
 */
/*@{*/
typedef int (*GWEN_CRYPTMANAGER_GETPIN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                           GWEN_CRYPTTOKEN *token,
                                           GWEN_CRYPTTOKEN_PINTYPE pt,
                                           GWEN_CRYPTTOKEN_PINENCODING *pe,
                                           unsigned char *buffer,
                                           unsigned int minLength,
                                           unsigned int maxLength,
                                           unsigned int *pinLength);

typedef int (*GWEN_CRYPTMANAGER_BEGIN_ENTER_PIN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                    GWEN_CRYPTTOKEN *token,
                                                    GWEN_CRYPTTOKEN_PINTYPE pt);
typedef int (*GWEN_CRYPTMANAGER_END_ENTER_PIN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                   GWEN_CRYPTTOKEN *token,
                                                   GWEN_CRYPTTOKEN_PINTYPE pt,
                                                   int ok);
typedef int (*GWEN_CRYPTMANAGER_INSERT_TOKEN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                 GWEN_CRYPTTOKEN *token);
typedef int (*GWEN_CRYPTMANAGER_INSERT_CORRECT_TOKEN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                         GWEN_CRYPTTOKEN *token);
typedef int (*GWEN_CRYPTMANAGER_SHOW_MESSAGE_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                 GWEN_CRYPTTOKEN *token,
                                                 const char *msg);




GWEN_PLUGIN_MANAGER *GWEN_CryptManager_new();


void GWEN_CryptManager_SetGetPinFn(GWEN_PLUGIN_MANAGER *cm,
                                   GWEN_CRYPTMANAGER_GETPIN_FN fn);

void GWEN_CryptManager_SetBeginEnterPinFn(GWEN_PLUGIN_MANAGER *cm,
                                          GWEN_CRYPTMANAGER_BEGIN_ENTER_PIN_FN fn);
void GWEN_CryptManager_SetEndEnterPinFn(GWEN_PLUGIN_MANAGER *cm,
                                         GWEN_CRYPTMANAGER_END_ENTER_PIN_FN fn);

void GWEN_CryptManager_SetInsertTokenFn(GWEN_PLUGIN_MANAGER *cm,
                                        GWEN_CRYPTMANAGER_INSERT_TOKEN_FN fn);
void GWEN_CryptManager_SetInsertCorrectTokenFn(GWEN_PLUGIN_MANAGER *cm,
                                               GWEN_CRYPTMANAGER_INSERT_CORRECT_TOKEN_FN fn);
void GWEN_CryptManager_SetShowMessageFn(GWEN_PLUGIN_MANAGER *cm,
                                        GWEN_CRYPTMANAGER_SHOW_MESSAGE_FN fn);



int GWEN_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                             GWEN_CRYPTTOKEN *token,
                             GWEN_CRYPTTOKEN_PINTYPE pt,
                             GWEN_CRYPTTOKEN_PINENCODING *pe,
                             unsigned char *buffer,
                             unsigned int minLength,
                             unsigned int maxLength,
                             unsigned int *pinLength);

int GWEN_CryptManager_BeginEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                    GWEN_CRYPTTOKEN *token,
                                    GWEN_CRYPTTOKEN_PINTYPE pt);
int GWEN_CryptManager_EndEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token,
                                  GWEN_CRYPTTOKEN_PINTYPE pt,
                                  int ok);
int GWEN_CryptManager_InsertToken(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token);
int GWEN_CryptManager_InsertCorrectToken(GWEN_PLUGIN_MANAGER *cm,
                                         GWEN_CRYPTTOKEN *token);

int GWEN_CryptManager_ShowMessage(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token,
                                  const char *msg);
/*@}*/




#endif /* GWEN_CRYPTTOKEN_H */


