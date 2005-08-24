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

#include <gwenhywfar/crypttoken_user.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GWEN_CRYPTMANAGER_NAME "crypttoken"


#define GWEN_CRYPTTOKEN_FLAGS_MANAGES_SIGNSEQ           0x00000001
#define GWEN_CRYPTTOKEN_FLAGS_PREDEF_CONTEXT_ONLY       0x00000002

#define GWEN_CRYPTTOKEN_FLAGS_DISABLE_SMALLER_SIGNATURE 0x00000004
/** this is used when checking for the pin status */
#define GWEN_CRYPTTOKEN_FLAGS_FORCE_PIN_ENTRY           0x00000008


#define GWEN_CRYPTTOKEN_GETPIN_FLAGS_CONFIRM            0x00000001
#define GWEN_CRYPTTOKEN_GETPIN_FLAGS_NUMERIC            0x00000002
#define GWEN_CRYPTTOKEN_GETPIN_FLAGS_SHOW               0x00000004
#define GWEN_CRYPTTOKEN_GETPIN_FLAGS_RETRY              0x00000008
#define GWEN_CRYPTTOKEN_GETPIN_FLAGS_ALLOW_DEFAULT      0x00000010

/**
 * This is a list of status values used by GWEN_CRYPTTOKEN in key specs.
 */
/*@{*/
#define GWEN_CRYPTTOKEN_KEYSTATUS_FREE     0
#define GWEN_CRYPTTOKEN_KEYSTATUS_NEW      1
#define GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE   2
#define GWEN_CRYPTTOKEN_KEYSTATUS_UNKNOWN  99
/*@}*/


typedef struct GWEN_CRYPTTOKEN GWEN_CRYPTTOKEN;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN,
                            GWEN_CryptToken,
                            GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN,
                               GWENHYWFAR_API)


GWEN_TYPE_UINT32 GWEN_CryptToken_Flag_fromString(const char *s);
GWEN_TYPE_UINT32 GWEN_CryptToken_Flags_fromDb(GWEN_DB_NODE *db,
                                              const char *n);
void GWEN_CryptToken_Flags_toDb(GWEN_DB_NODE *db,
                                const char *n,
                                GWEN_TYPE_UINT32 flags);


typedef enum {
  GWEN_CryptToken_HashAlgo_Unknown=-1,
  GWEN_CryptToken_HashAlgo_None=0,
  GWEN_CryptToken_HashAlgo_Any,
  /** SHA-1 */
  GWEN_CryptToken_HashAlgo_SHA1,
  /** Ripemd 160 */
  GWEN_CryptToken_HashAlgo_RMD160,
  /** MD-5 */
  GWEN_CryptToken_HashAlgo_MD5,
} GWEN_CRYPTTOKEN_HASHALGO;

GWEN_CRYPTTOKEN_HASHALGO GWEN_CryptToken_HashAlgo_fromString(const char *s);
const char *GWEN_CryptToken_HashAlgo_toString(GWEN_CRYPTTOKEN_HASHALGO a);


typedef enum {
  GWEN_CryptToken_PaddAlgo_Unknown=-1,
  GWEN_CryptToken_PaddAlgo_None=0,
  GWEN_CryptToken_PaddAlgo_Any,
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
  GWEN_CryptToken_PaddAlgo_ANSIX9_23,
} GWEN_CRYPTTOKEN_PADDALGO;

GWEN_CRYPTTOKEN_PADDALGO GWEN_CryptToken_PaddAlgo_fromString(const char *s);
const char *GWEN_CryptToken_PaddAlgo_toString(GWEN_CRYPTTOKEN_PADDALGO a);


typedef enum {
  GWEN_CryptToken_CryptAlgo_Unknown=-1,
  GWEN_CryptToken_CryptAlgo_None=0,
  GWEN_CryptToken_CryptAlgo_Any,
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
  GWEN_CryptToken_Device_Card,
  GWEN_CryptToken_Device_Any=999
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
  GWEN_CryptToken_PinEncoding_ASCII,
  GWEN_CryptToken_PinEncoding_FPIN2
} GWEN_CRYPTTOKEN_PINENCODING;

GWEN_CRYPTTOKEN_PINENCODING
GWEN_CryptToken_PinEncoding_fromString(const char *s);
const char*
  GWEN_CryptToken_PinEncoding_toString(GWEN_CRYPTTOKEN_PINENCODING pe);


#ifdef __cplusplus
}
#endif

#include <gwenhywfar/crypttoken_signinfo.h>
#include <gwenhywfar/crypttoken_cryptinfo.h>
#include <gwenhywfar/crypttoken_keyinfo.h>
#include <gwenhywfar/crypttoken_context.h>
#include <gwenhywfar/crypttoken_plugin.h>


#ifdef __cplusplus
extern "C" {
#endif



/** @name CryptToken
 *
 */
/*@{*/



typedef int (*GWEN_CRYPTTOKEN_OPEN_FN)(GWEN_CRYPTTOKEN *ct,
                                       int manage);
typedef int (*GWEN_CRYPTTOKEN_CREATE_FN)(GWEN_CRYPTTOKEN *ct);
typedef int (*GWEN_CRYPTTOKEN_CLOSE_FN)(GWEN_CRYPTTOKEN *ct);

typedef int (*GWEN_CRYPTTOKEN_CHANGEPIN_FN)(GWEN_CRYPTTOKEN *ct,
                                            GWEN_CRYPTTOKEN_PINTYPE pt);

typedef int (*GWEN_CRYPTTOKEN_SIGN_FN)(GWEN_CRYPTTOKEN *ct,
                                       const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                       const char *ptr,
                                       unsigned int len,
                                       GWEN_BUFFER *dst);
typedef int (*GWEN_CRYPTTOKEN_VERIFY_FN)(GWEN_CRYPTTOKEN *ct,
                                         const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const char *ptr,
                                         unsigned int len,
                                         const char *sigptr,
                                         unsigned int siglen);
typedef int (*GWEN_CRYPTTOKEN_ENCRYPT_FN)(GWEN_CRYPTTOKEN *ct,
                                          const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          const char *ptr,
                                          unsigned int len,
                                          GWEN_BUFFER *dst);
typedef int (*GWEN_CRYPTTOKEN_DECRYPT_FN)(GWEN_CRYPTTOKEN *ct,
                                          const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          const char *ptr,
                                          unsigned int len,
                                          GWEN_BUFFER *dst);
typedef int (*GWEN_CRYPTTOKEN_READKEY_FN)(GWEN_CRYPTTOKEN *ct,
                                          GWEN_TYPE_UINT32 kid,
                                          GWEN_CRYPTKEY **key);
typedef int (*GWEN_CRYPTTOKEN_WRITEKEY_FN)(GWEN_CRYPTTOKEN *ct,
                                           GWEN_TYPE_UINT32 kid,
                                           const GWEN_CRYPTKEY *key);
typedef int (*GWEN_CRYPTTOKEN_READKEYSPEC_FN)(GWEN_CRYPTTOKEN *ct,
                                              GWEN_TYPE_UINT32 kid,
                                              GWEN_KEYSPEC **ks);
typedef int (*GWEN_CRYPTTOKEN_WRITEKEYSPEC_FN)(GWEN_CRYPTTOKEN *ct,
                                               GWEN_TYPE_UINT32 kid,
                                               const GWEN_KEYSPEC *ks);

typedef int (*GWEN_CRYPTTOKEN_GENERATEKEY_FN)(GWEN_CRYPTTOKEN *ct,
                                              const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                              GWEN_CRYPTKEY **key);

typedef int (*GWEN_CRYPTTOKEN_GETSIGNSEQ_FN)(GWEN_CRYPTTOKEN *ct,
                                             GWEN_TYPE_UINT32 kid,
                                             GWEN_TYPE_UINT32 *signSeq);

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

typedef int
  (*GWEN_CRYPTTOKEN_FILLUSERLIST_FN)(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_USER_LIST *l);

typedef int
  (*GWEN_CRYPTTOKEN_MODIFYUSER_FN)(GWEN_CRYPTTOKEN *ct,
                                   const GWEN_CRYPTTOKEN_USER *u);

typedef int (*GWEN_CRYPTTOKEN_GETTOKENIDDATA_FN)(GWEN_CRYPTTOKEN *ct,
                                                 GWEN_BUFFER *buf);


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

GWEN_TYPE_UINT32 GWEN_CryptToken_GetFlags(const GWEN_CRYPTTOKEN *ct);
void GWEN_CryptToken_SetFlags(GWEN_CRYPTTOKEN *ct, GWEN_TYPE_UINT32 fl);
void GWEN_CryptToken_AddFlags(GWEN_CRYPTTOKEN *ct, GWEN_TYPE_UINT32 fl);
void GWEN_CryptToken_SubFlags(GWEN_CRYPTTOKEN *ct, GWEN_TYPE_UINT32 fl);

const char *GWEN_CryptToken_GetDescriptiveName(const GWEN_CRYPTTOKEN *ct);
void GWEN_CryptToken_SetDescriptiveName(GWEN_CRYPTTOKEN *ct, const char *s);

void GWEN_CryptToken_SetOpenFn(GWEN_CRYPTTOKEN *ct,
                               GWEN_CRYPTTOKEN_OPEN_FN fn);
GWEN_CRYPTTOKEN_OPEN_FN GWEN_CryptToken_GetOpenFn(const GWEN_CRYPTTOKEN *ct);

void GWEN_CryptToken_SetCreateFn(GWEN_CRYPTTOKEN *ct,
                                 GWEN_CRYPTTOKEN_CREATE_FN fn);
GWEN_CRYPTTOKEN_CREATE_FN GWEN_CryptToken_GetCreateFn(const GWEN_CRYPTTOKEN *ct);

void GWEN_CryptToken_SetCloseFn(GWEN_CRYPTTOKEN *ct,
                                GWEN_CRYPTTOKEN_CLOSE_FN fn);
GWEN_CRYPTTOKEN_CLOSE_FN
  GWEN_CryptToken_GetCloseFn(const GWEN_CRYPTTOKEN *ct);

void GWEN_CryptToken_SetChangePinFn(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_CHANGEPIN_FN fn);

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
void GWEN_CryptToken_SetReadKeySpecFn(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_READKEYSPEC_FN fn);
void GWEN_CryptToken_SetWriteKeySpecFn(GWEN_CRYPTTOKEN *ct,
                                       GWEN_CRYPTTOKEN_WRITEKEYSPEC_FN fn);
void GWEN_CryptToken_SetGenerateKeyFn(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_GENERATEKEY_FN fn);
void GWEN_CryptToken_SetFillContextListFn(GWEN_CRYPTTOKEN *ct,
                                          GWEN_CRYPTTOKEN_FILLCONTEXTLIST_FN fn);
void GWEN_CryptToken_SetFillSignInfoListFn(GWEN_CRYPTTOKEN *ct,
                                           GWEN_CRYPTTOKEN_FILLSIGNINFOLIST_FN fn);
void GWEN_CryptToken_SetFillCryptInfoListFn(GWEN_CRYPTTOKEN *ct,
                                            GWEN_CRYPTTOKEN_FILLCRYPTINFOLIST_FN fn);
void GWEN_CryptToken_SetFillKeyInfoListFn(GWEN_CRYPTTOKEN *ct,
                                          GWEN_CRYPTTOKEN_FILLKEYINFOLIST_FN fn);
void GWEN_CryptToken_SetFillUserListFn(GWEN_CRYPTTOKEN *ct,
                                       GWEN_CRYPTTOKEN_FILLUSERLIST_FN fn);
void GWEN_CryptToken_SetGetSignSeqFn(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_GETSIGNSEQ_FN fn);
void GWEN_CryptToken_SetModifyUserFn(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_MODIFYUSER_FN fn);
void
  GWEN_CryptToken_SetGetTokenIdDataFn(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_GETTOKENIDDATA_FN fn);



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                               Virtual Functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

int GWEN_CryptToken_Open(GWEN_CRYPTTOKEN *ct, int manage);

int GWEN_CryptToken_Create(GWEN_CRYPTTOKEN *ct);

int GWEN_CryptToken_Close(GWEN_CRYPTTOKEN *ct);

int GWEN_CryptToken_ChangePin(GWEN_CRYPTTOKEN *ct,
                              GWEN_CRYPTTOKEN_PINTYPE pt);

int GWEN_CryptToken_Sign(GWEN_CRYPTTOKEN *ct,
                         const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                         const char *ptr,
                         unsigned int len,
                         GWEN_BUFFER *dst);
int GWEN_CryptToken_Verify(GWEN_CRYPTTOKEN *ct,
                           const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                           const char *ptr,
                           unsigned int len,
                           const char *sigptr,
                           unsigned int siglen);
int GWEN_CryptToken_Encrypt(GWEN_CRYPTTOKEN *ct,
                            const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                            const char *ptr,
                            unsigned int len,
                            GWEN_BUFFER *dst);
int GWEN_CryptToken_Decrypt(GWEN_CRYPTTOKEN *ct,
                            const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                            const char *ptr,
                            unsigned int len,
                            GWEN_BUFFER *dst);
int GWEN_CryptToken_ReadKey(GWEN_CRYPTTOKEN *ct,
                            GWEN_TYPE_UINT32 kid,
                            GWEN_CRYPTKEY **key);
int GWEN_CryptToken_WriteKey(GWEN_CRYPTTOKEN *ct,
                             GWEN_TYPE_UINT32 kid,
                             const GWEN_CRYPTKEY *key);
int GWEN_CryptToken_ReadKeySpec(GWEN_CRYPTTOKEN *ct,
                                GWEN_TYPE_UINT32 kid,
                                GWEN_KEYSPEC **ks);
int GWEN_CryptToken_WriteKeySpec(GWEN_CRYPTTOKEN *ct,
                                 GWEN_TYPE_UINT32 kid,
                                 const GWEN_KEYSPEC *ks);
int GWEN_CryptToken_GenerateKey(GWEN_CRYPTTOKEN *ct,
                                const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                GWEN_CRYPTKEY **key);

int GWEN_CryptToken_GetSignSeq(GWEN_CRYPTTOKEN *ct,
                               GWEN_TYPE_UINT32 kid,
                               GWEN_TYPE_UINT32 *signSeq);

int GWEN_CryptToken_FillContextList(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_CONTEXT_LIST *l);
int GWEN_CryptToken_FillSignInfoList(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_SIGNINFO_LIST *l);

int GWEN_CryptToken_FillCryptInfoList(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_CRYPTINFO_LIST *l);

int GWEN_CryptToken_FillKeyInfoList(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_KEYINFO_LIST *l);

int GWEN_CryptToken_FillUserList(GWEN_CRYPTTOKEN *ct,
                                 GWEN_CRYPTTOKEN_USER_LIST *ul);

int GWEN_CryptToken_ModifyUser(GWEN_CRYPTTOKEN *ct,
                               const GWEN_CRYPTTOKEN_USER *u);

int GWEN_CryptToken_GetTokenIdData(GWEN_CRYPTTOKEN *ct, GWEN_BUFFER *buf);


const GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_GetSignInfoByAlgos(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_HASHALGO hashAlgo,
                                     GWEN_CRYPTTOKEN_PADDALGO paddAlgo);

const GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_GetSignInfoById(GWEN_CRYPTTOKEN *ct,
                                  GWEN_TYPE_UINT32 id);

const GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_GetCryptInfoByAlgos(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_CRYPTALGO cryptAlgo,
                                      GWEN_CRYPTTOKEN_PADDALGO paddAlgo);

const GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_GetCryptInfoById(GWEN_CRYPTTOKEN *ct,
                                   GWEN_TYPE_UINT32 id);


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
                         unsigned int chunkSize,
                         GWEN_BUFFER *buf);

int GWEN_CryptToken_Unpadd(GWEN_CRYPTTOKEN_PADDALGO algo,
                           GWEN_BUFFER *buf);

int GWEN_CryptToken_AddContext(GWEN_CRYPTTOKEN *ct,
                               const GWEN_CRYPTTOKEN_CONTEXT *ctx);

/*@}*/


/** @name CryptToken Helper Functions
 *
 */
/*@{*/

int GWEN_CryptToken_TransformPin(GWEN_CRYPTTOKEN_PINENCODING peSrc,
                                 GWEN_CRYPTTOKEN_PINENCODING peDst,
                                 unsigned char *buffer,
                                 unsigned int bufLength,
                                 unsigned int *pinLength);

/*@}*/

#ifdef __cplusplus
}
#endif






#endif /* GWEN_CRYPTTOKEN_H */


