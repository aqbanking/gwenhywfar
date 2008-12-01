/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_CRYPTMGR_H
#define GWEN_CRYPTMGR_CRYPTMGR_H


#include <gwenhywfar/buffer.h>


typedef struct GWEN_CRYPTMGR GWEN_CRYPTMGR;


enum {
  GWEN_CryptProfile_None=0,
  /**
   * Padding for key    : ISO 9796-2
   * Encryption for key : RSA 2048
   * Padding for data   : ANSIX9_23
   * Encryption for data: BLOWFISH-256
   */
  GWEN_CryptProfile_1=1
} GWEN_CRYPT_PROFILE;


enum {
  GWEN_SignatureProfile_None=0,
  /**
   * Hash: RMD-160
   * Padd: ISO 9796-2
   * Sign: RSA 2048
   */
  GWEN_SignatureProfile_1=1
} GWEN_SIGNATURE_PROFILE;


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API
void GWEN_CryptMgr_free(GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
const char *GWEN_CryptMgr_GetLocalKeyName(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
int GWEN_CryptMgr_GetLocalKeyNumber(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
int GWEN_CryptMgr_GetLocalKeyVersion(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
const char *GWEN_CryptMgr_GetPeerKeyName(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
int GWEN_CryptMgr_GetPeerKeyNumber(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
int GWEN_CryptMgr_GetPeerKeyVersion(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
int GWEN_CryptMgr_GetCryptProfile(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
void GWEN_CryptMgr_SetCryptProfile(GWEN_CRYPTMGR *cm, int i);

GWENHYWFAR_API
int GWEN_CryptMgr_GetSignatureProfile(const GWEN_CRYPTMGR *cm);

GWENHYWFAR_API
void GWEN_CryptMgr_SetSignatureProfile(GWEN_CRYPTMGR *cm, int i);



GWENHYWFAR_API
int GWEN_CryptMgr_Sign(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf);

GWENHYWFAR_API
int GWEN_CryptMgr_Encrypt(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf);

GWENHYWFAR_API
int GWEN_CryptMgr_Verify(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf);

GWENHYWFAR_API
int GWEN_CryptMgr_Decrypt(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf);


GWENHYWFAR_API
int GWEN_CryptMgr_Encode(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf);

GWENHYWFAR_API
int GWEN_CryptMgr_Decode(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf);


#ifdef __cplusplus
}
#endif


#endif


