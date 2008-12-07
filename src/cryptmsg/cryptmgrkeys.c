/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cryptmgrkeys_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/crypthead.h>
#include <gwenhywfar/text.h>



GWEN_INHERIT(GWEN_CRYPTMGR, GWEN_CRYPTMGR_KEYS);



GWEN_CRYPTMGR *GWEN_CryptMgrKeys_new(const char *localName,
				     GWEN_CRYPT_KEY *localKey,
				     const char *peerName,
				     GWEN_CRYPT_KEY *peerKey,
				     int ownKeys) {
  GWEN_CRYPTMGR *cm;
  GWEN_CRYPTMGR_KEYS *xcm;

  cm=GWEN_CryptMgr_new();
  GWEN_NEW_OBJECT(GWEN_CRYPTMGR_KEYS, xcm);
  GWEN_INHERIT_SETDATA(GWEN_CRYPTMGR, GWEN_CRYPTMGR_KEYS, cm, xcm,
		       GWEN_CryptMgrKeys_FreeData);

  if (localKey) {
    xcm->localKey=localKey;
    GWEN_CryptMgr_SetLocalKeyNumber(cm, GWEN_Crypt_Key_GetKeyNumber(localKey));
    GWEN_CryptMgr_SetLocalKeyVersion(cm, GWEN_Crypt_Key_GetKeyVersion(localKey));
  }
  else
    xcm->ownLocalKey=0;

  if (peerKey) {
    xcm->peerKey=peerKey;
    GWEN_CryptMgr_SetPeerKeyNumber(cm, GWEN_Crypt_Key_GetKeyNumber(peerKey));
    GWEN_CryptMgr_SetPeerKeyVersion(cm, GWEN_Crypt_Key_GetKeyVersion(peerKey));
    xcm->ownPeerKey=ownKeys;
  }
  else
    xcm->ownPeerKey=0;

  if (localName)
    GWEN_CryptMgr_SetLocalKeyName(cm, localName);

  if (peerName)
    GWEN_CryptMgr_SetPeerKeyName(cm, peerName);

  GWEN_CryptMgr_SetSignDataFn(cm, GWEN_CryptMgrKeys_SignData);
  GWEN_CryptMgr_SetVerifyDataFn(cm, GWEN_CryptMgrKeys_VerifyData);
  GWEN_CryptMgr_SetEncryptKeyFn(cm, GWEN_CryptMgrKeys_EncryptKey);
  GWEN_CryptMgr_SetDecryptKeyFn(cm, GWEN_CryptMgrKeys_DecryptKey);

  return cm;
}



void GWEN_CryptMgrKeys_FreeData(void *bp, void *p) {
  GWEN_CRYPTMGR_KEYS *xcm;

  xcm=(GWEN_CRYPTMGR_KEYS*) p;

  if (xcm->ownLocalKey)
    GWEN_Crypt_Key_free(xcm->localKey);
  if (xcm->ownPeerKey)
    GWEN_Crypt_Key_free(xcm->peerKey);
}



void GWEN_CryptMgrKeys_SetPeerKey(GWEN_CRYPTMGR *cm,
				  GWEN_CRYPT_KEY *peerKey,
				  int ownKey) {
  GWEN_CRYPTMGR_KEYS *xcm;

  assert(cm);
  xcm=GWEN_INHERIT_GETDATA(GWEN_CRYPTMGR, GWEN_CRYPTMGR_KEYS, cm);
  assert(xcm);

  if (xcm->ownPeerKey)
    GWEN_Crypt_Key_free(xcm->peerKey);
  xcm->peerKey=peerKey;
  xcm->ownPeerKey=ownKey;
}



int GWEN_CryptMgrKeys_SignData(GWEN_CRYPTMGR *cm,
			       const uint8_t *pData, uint32_t lData,
			       GWEN_BUFFER *dbuf) {
  GWEN_CRYPTMGR_KEYS *xcm;
  GWEN_MDIGEST *md;
  int rv;
  GWEN_BUFFER *tbuf;
  int ksize;
  uint32_t signatureLen;

  assert(cm);
  xcm=GWEN_INHERIT_GETDATA(GWEN_CRYPTMGR, GWEN_CRYPTMGR_KEYS, cm);
  assert(xcm);

  if (xcm->localKey==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No local key");
    return GWEN_ERROR_GENERIC;
  }

  ksize=GWEN_Crypt_Key_GetKeySize(xcm->localKey);

  /* hash pData */
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }
  rv=GWEN_MDigest_Update(md, pData, lData);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  /* padd */
  tbuf=GWEN_Buffer_new(0, ksize, 0, 1);
  GWEN_Buffer_AppendBytes(tbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);
  GWEN_Padd_PaddWithIso9796_2(tbuf, ksize);

  /* sign */
  GWEN_Buffer_AllocRoom(dbuf, ksize);
  signatureLen=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
  rv=GWEN_Crypt_Key_Sign(xcm->localKey,
			 (uint8_t*)GWEN_Buffer_GetStart(tbuf),
			 GWEN_Buffer_GetUsedBytes(tbuf),
			 (uint8_t*)GWEN_Buffer_GetPosPointer(dbuf),
			 &signatureLen);
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  GWEN_Buffer_IncrementPos(dbuf, signatureLen);
  GWEN_Buffer_AdjustUsedBytes(dbuf);

  return 0;
}



int GWEN_CryptMgrKeys_VerifyData(GWEN_CRYPTMGR *cm,
				 const uint8_t *pData, uint32_t lData,
				 const uint8_t *pSignature, uint32_t lSignature) {
  GWEN_CRYPTMGR_KEYS *xcm;
  GWEN_MDIGEST *md;
  int rv;
  GWEN_BUFFER *tbuf;
  int ksize;
  uint32_t l;

  assert(cm);
  xcm=GWEN_INHERIT_GETDATA(GWEN_CRYPTMGR, GWEN_CRYPTMGR_KEYS, cm);
  assert(xcm);

  if (xcm->peerKey==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No peer key");
    return GWEN_ERROR_GENERIC;
  }

  ksize=GWEN_Crypt_Key_GetKeySize(xcm->peerKey);

  /* the padding algo uses random numbers, so we must use the encrypt function and
   * compare the decoded and unpadded signature with the hash of the source data
   */
  tbuf=GWEN_Buffer_new(0, ksize+16, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
  rv=GWEN_Crypt_Key_Encipher(xcm->peerKey,
			     pSignature, lSignature,
			     (uint8_t*)GWEN_Buffer_GetPosPointer(tbuf),
			     &l);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(tbuf, l);
  GWEN_Buffer_AdjustUsedBytes(tbuf);

  /* unpadd */
  rv=GWEN_Padd_UnpaddWithIso9796_2(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  /* tbuf now contains the hash */

  /* hash source data */
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  rv=GWEN_MDigest_Update(md, pData, lData);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  if (GWEN_MDigest_GetDigestSize(md)!=GWEN_Buffer_GetUsedBytes(tbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid signature");
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_VERIFY;
  }

  if (memcmp(GWEN_MDigest_GetDigestPtr(md),
             GWEN_Buffer_GetStart(tbuf),
	     GWEN_MDigest_GetDigestSize(md))!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid signature");
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_VERIFY;
  }

  GWEN_MDigest_free(md);
  GWEN_Buffer_free(tbuf);

  return 0;
}



int GWEN_CryptMgrKeys_EncryptKey(GWEN_CRYPTMGR *cm,
				 const uint8_t *pData, uint32_t lData,
				 GWEN_BUFFER *dbuf) {
  GWEN_CRYPTMGR_KEYS *xcm;
  int rv;
  GWEN_BUFFER *tbuf;
  int ksize;
  uint32_t l;

  assert(cm);
  xcm=GWEN_INHERIT_GETDATA(GWEN_CRYPTMGR, GWEN_CRYPTMGR_KEYS, cm);
  assert(xcm);

  if (xcm->peerKey==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No peer key");
    return GWEN_ERROR_GENERIC;
  }

  ksize=GWEN_Crypt_Key_GetKeySize(xcm->peerKey);

  /* padd key data */
  tbuf=GWEN_Buffer_new(0, ksize, 0, 1);
  GWEN_Buffer_AppendBytes(tbuf, (const char*) pData, lData);
  rv=GWEN_Padd_PaddWithIso9796_2(tbuf, ksize);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_AllocRoom(dbuf, ksize);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
  rv=GWEN_Crypt_Key_Encipher(xcm->peerKey,
			     (const uint8_t*)GWEN_Buffer_GetStart(tbuf),
			     GWEN_Buffer_GetUsedBytes(tbuf),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(dbuf),
			     &l);
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  GWEN_Buffer_IncrementPos(dbuf, l);
  GWEN_Buffer_AdjustUsedBytes(dbuf);

  return 0;
}



int GWEN_CryptMgrKeys_DecryptKey(GWEN_CRYPTMGR *cm,
				 const uint8_t *pData, uint32_t lData,
				 GWEN_BUFFER *dbuf) {
  GWEN_CRYPTMGR_KEYS *xcm;
  int rv;
  GWEN_BUFFER *tbuf;
  int ksize;
  uint32_t l;

  assert(cm);
  xcm=GWEN_INHERIT_GETDATA(GWEN_CRYPTMGR, GWEN_CRYPTMGR_KEYS, cm);
  assert(xcm);

  if (xcm->localKey==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No local key");
    return GWEN_ERROR_GENERIC;
  }

  ksize=GWEN_Crypt_Key_GetKeySize(xcm->localKey);

  tbuf=GWEN_Buffer_new(0, ksize, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
  rv=GWEN_Crypt_Key_Decipher(xcm->localKey,
			     pData, lData,
			     (uint8_t*)GWEN_Buffer_GetPosPointer(tbuf),
			     &l);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(tbuf, l);
  GWEN_Buffer_AdjustUsedBytes(tbuf);

  /* unpadd data */
  rv=GWEN_Padd_UnpaddWithIso9796_2(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_AppendBuffer(dbuf, tbuf);
  GWEN_Buffer_free(tbuf);

  return 0;
}













