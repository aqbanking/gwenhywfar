/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_CRYPTMGR_KEYS_P_H
#define GWEN_CRYPTMGR_CRYPTMGR_KEYS_P_H


#include <gwenhywfar/cryptmgrkeys.h>
#include <gwenhywfar/cryptmgr_be.h>


typedef struct GWEN_CRYPTMGR_KEYS GWEN_CRYPTMGR_KEYS;
struct GWEN_CRYPTMGR_KEYS {
  GWEN_CRYPT_KEY *localKey;
  GWEN_CRYPT_KEY *peerKey;
  int ownKeys;
};

static GWENHYWFAR_CB
void GWEN_CryptMgrKeys_FreeData(void *bp, void *p);

static GWENHYWFAR_CB 
int GWEN_CryptMgrKeys_SignData(GWEN_CRYPTMGR *cm,
			       const uint8_t *pData, uint32_t lData,
			       GWEN_BUFFER *dbuf);
static GWENHYWFAR_CB 
int GWEN_CryptMgrKeys_EncryptKey(GWEN_CRYPTMGR *cm,
				  const uint8_t *pData, uint32_t lData,
				  GWEN_BUFFER *dbuf);

static GWENHYWFAR_CB 
int GWEN_CryptMgrKeys_VerifyData(GWEN_CRYPTMGR *cm,
				 const uint8_t *pData, uint32_t lData,
				 const uint8_t *pSignature, uint32_t lSignature);

static GWENHYWFAR_CB 
int GWEN_CryptMgrKeys_DecryptKey(GWEN_CRYPTMGR *cm,
				 const uint8_t *pData, uint32_t lData,
				 GWEN_BUFFER *dbuf);


#endif


