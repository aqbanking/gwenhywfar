/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cryptkeysym_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cryptdefs.h>
#include <gwenhywfar/text.h>



GWEN_INHERIT(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM)






int GWEN_Crypt_KeySym_Encipher(GWEN_CRYPT_KEY *k,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       uint8_t *pOutData,
			       uint32_t *pOutLen) {
  GWEN_CRYPT_KEY_SYM *xk;
  gcry_error_t err;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  err=gcry_cipher_encrypt(xk->algoHandle, pOutData, inLen, pInData, inLen);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_encrypt(): %s", gcry_strerror(err));
    return GWEN_ERROR_GENERIC;
  }
  *pOutLen=inLen;

  return 0;
}



int GWEN_Crypt_KeySym_Decipher(GWEN_CRYPT_KEY *k,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       uint8_t *pOutData,
			       uint32_t *pOutLen) {
  GWEN_CRYPT_KEY_SYM *xk;
  gcry_error_t err;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  err=gcry_cipher_decrypt(xk->algoHandle, pOutData, inLen, pInData, inLen);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_decrypt(): %s", gcry_strerror(err));
    return GWEN_ERROR_GENERIC;
  }
  *pOutLen=inLen;

  return 0;
}



GWENHYWFAR_CB
void GWEN_Crypt_KeySym_freeData(GWEN_UNUSED void *bp, void *p) {
  GWEN_CRYPT_KEY_SYM *xk;

  xk=(GWEN_CRYPT_KEY_SYM*) p;
  if (xk->keyData && xk->keyLen) {
    memset(xk->keyData, 0, xk->keyLen);
    free(xk->keyData);
  }
  xk->keyData=NULL;
  xk->keyLen=0;
  if (xk->algoValid)
    gcry_cipher_close(xk->algoHandle);
  GWEN_FREE_OBJECT(xk);
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeySym_dup(const GWEN_CRYPT_KEY *k) {
  GWEN_CRYPT_KEY *nk;
  GWEN_CRYPT_KEY_SYM *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  nk=GWEN_Crypt_KeySym_fromData(GWEN_Crypt_Key_GetCryptAlgoId(k),
                                GWEN_Crypt_Key_GetKeySize(k),
				xk->mode,
				xk->algo,
				GCRY_CIPHER_SECURE,
				xk->keyData,
				xk->keyLen);
  return nk;
}



enum gcry_cipher_modes GWEN_Crypt_KeySym__MyMode2GMode(GWEN_CRYPT_CRYPTMODE mode) {
  switch(mode) {
  case GWEN_Crypt_CryptMode_Unknown: return GCRY_CIPHER_MODE_NONE;
  case GWEN_Crypt_CryptMode_None:    return GCRY_CIPHER_MODE_NONE;
  case GWEN_Crypt_CryptMode_Ecb:     return GCRY_CIPHER_MODE_ECB;
  case GWEN_Crypt_CryptMode_Cfb:     return GCRY_CIPHER_MODE_CFB;
  case GWEN_Crypt_CryptMode_Cbc:     return GCRY_CIPHER_MODE_CBC;
  }

  return GCRY_CIPHER_MODE_NONE;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeySym_Generate(GWEN_CRYPT_CRYPTALGOID cryptAlgoId, int keySize,
					   GWEN_CRYPT_CRYPTMODE mode,
					   int algo,
                                           unsigned int flags,
					   int quality) {
  GWEN_CRYPT_KEY *k;
  GWEN_CRYPT_KEY_SYM *xk;
  int kbytes;
  uint8_t *keyData;
  gcry_error_t err;
  enum gcry_random_level q;

  k=GWEN_Crypt_Key_new(cryptAlgoId, keySize);
  assert(k);
  GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_SYM, xk);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k, xk, GWEN_Crypt_KeySym_freeData);
  GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeySym_Encipher);
  GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeySym_Decipher);

  switch(quality) {
  case 0:  q=GCRY_WEAK_RANDOM; break;
  case 1:  q=GCRY_STRONG_RANDOM; break;
  case 2:
  default: q=GCRY_VERY_STRONG_RANDOM; break;
  }

  /* open algo */
  err=gcry_cipher_open(&xk->algoHandle, algo, GWEN_Crypt_KeySym__MyMode2GMode(mode), flags);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_open(): %s", gcry_strerror(err));
    GWEN_Crypt_Key_free(k);
    return NULL;
  }
  xk->algoValid=1;
  xk->mode=mode;

#if 0
  kbytes=keySize/8;
  if (keySize % 8)
      kbytes++;
#else
  kbytes=keySize;
#endif
  keyData=gcry_random_bytes(kbytes, q);

  /* store key data */
  xk->keyData=keyData;
  xk->keyLen=kbytes;

  /* set key in algo */
  err=gcry_cipher_setkey(xk->algoHandle, xk->keyData, xk->keyLen);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_setkey(): %s", gcry_strerror(err));
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  return k;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeySym_fromData(GWEN_CRYPT_CRYPTALGOID cryptAlgoId, int keySize,
					   GWEN_CRYPT_CRYPTMODE mode,
					   int algo,
					   unsigned int flags,
					   const uint8_t *kd, uint32_t kl) {
  GWEN_CRYPT_KEY *k;
  GWEN_CRYPT_KEY_SYM *xk;
  gcry_error_t err;

#if 0
  if (kl!=gcry_cipher_get_algo_keylen(algo)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid key length (is %d, should be %d)",
	      (int)kl, (int)gcry_cipher_get_algo_keylen(algo));
    return NULL;
  }
#endif

  k=GWEN_Crypt_Key_new(cryptAlgoId, keySize);
  assert(k);
  GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_SYM, xk);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k, xk, GWEN_Crypt_KeySym_freeData);
  GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeySym_Encipher);
  GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeySym_Decipher);

  /* open algo */
  err=gcry_cipher_open(&xk->algoHandle, algo, GWEN_Crypt_KeySym__MyMode2GMode(mode), flags);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_open(): %s", gcry_strerror(err));
    GWEN_Crypt_Key_free(k);
    return NULL;
  }
  xk->algoValid=1;
  xk->mode=mode;
  xk->algo=algo;

  /* read key data */
  if (kd==NULL || kl==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No key data");
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  /* store key data */
  xk->keyData=(uint8_t*) malloc(kl);
  assert(xk->keyData);
  memmove(xk->keyData, kd, kl);
  xk->keyLen=kl;

  /* set key in algo */
  err=gcry_cipher_setkey(xk->algoHandle, xk->keyData, xk->keyLen);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_setkey(): %s", gcry_strerror(err));
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  return k;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeySym_fromDb(GWEN_CRYPT_CRYPTALGOID cryptAlgoId,
                                         GWEN_CRYPT_CRYPTMODE mode,
					 int algo,
					 unsigned int flags,
                                         const char *gname,
					 GWEN_DB_NODE *db) {
  gcry_error_t err;
  GWEN_CRYPT_KEY *k;
  GWEN_CRYPT_KEY_SYM *xk;
  unsigned int nbits;
  GWEN_DB_NODE *dbR;
  unsigned int len;
  const char *p;

  dbR=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, gname);
  if (dbR==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "DB does not contain an %s key (no %s group)",
	      gname, gname);
    return NULL;
  }

  k=GWEN_Crypt_Key_fromDb(db);
  if (k==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return NULL;
  }
  if (GWEN_Crypt_Key_GetCryptAlgoId(k)!=cryptAlgoId) {
    DBG_ERROR(GWEN_LOGDOMAIN, "DB does not contain an RSA key");
    GWEN_Crypt_Key_free(k);
    return NULL;
  }
  nbits=GWEN_Crypt_Key_GetKeySize(k)*8;

  /* extend key */
  GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_SYM, xk);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k, xk, GWEN_Crypt_KeySym_freeData);
  GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeySym_Encipher);
  GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeySym_Decipher);

  /* open algo */
  err=gcry_cipher_open(&xk->algoHandle, algo, GWEN_Crypt_KeySym__MyMode2GMode(mode), flags);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_open(): %s", gcry_strerror(err));
    GWEN_Crypt_Key_free(k);
    return NULL;
  }
  xk->algoValid=1;
  xk->mode=mode;
  xk->algo=algo;

  /* read key data */
  p=GWEN_DB_GetBinValue(dbR, "keyData", 0, NULL, 0, &len);
  if (p==NULL || len==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No key data");
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  /* store key data */
  xk->keyData=(uint8_t*) malloc(len);
  assert(xk->keyData);
  memmove(xk->keyData, p, len);
  xk->keyLen=len;

  /* set key in algo */
  err=gcry_cipher_setkey(xk->algoHandle, xk->keyData, xk->keyLen);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_setkey(): %s", gcry_strerror(err));
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  return k;
}



int GWEN_Crypt_KeySym_toDb(const GWEN_CRYPT_KEY *k, GWEN_DB_NODE *db, const char *gname) {
  GWEN_CRYPT_KEY_SYM *xk;
  GWEN_DB_NODE *dbR;
  int rv;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  /* let key module wirte basic key info */
  rv=GWEN_Crypt_Key_toDb(k, db);
  if (rv)
    return rv;

  /* write sym stuff into our own group */
  dbR=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, gname);
  assert(dbR);

  GWEN_DB_SetBinValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keyData", xk->keyData, xk->keyLen);

  return 0;
}



int GWEN_Crypt_KeySym_SetKeyData(GWEN_CRYPT_KEY *k, const uint8_t *kd, uint32_t kl) {
  GWEN_CRYPT_KEY_SYM *xk;
  gcry_error_t err;

  if (!kd || !kl) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty keydata not allowed");
    return GWEN_ERROR_INVALID;
  }

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  if (xk->keyData && xk->keyLen) {
    memset(xk->keyData, 0, xk->keyLen);
    free(xk->keyData);
  }

  /* store key data */
  xk->keyData=(uint8_t*)malloc(kl);
  assert(xk->keyData);
  memmove(xk->keyData, kd, kl);
  xk->keyLen=kl;

  /* set key in algo */
  err=gcry_cipher_setkey(xk->algoHandle, xk->keyData, xk->keyLen);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_setkey(): %s", gcry_strerror(err));
    GWEN_Crypt_Key_free(k);
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



uint8_t *GWEN_Crypt_KeySym_GetKeyDataPtr(const GWEN_CRYPT_KEY *k) {
  GWEN_CRYPT_KEY_SYM *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  return xk->keyData;
}



uint32_t GWEN_Crypt_KeySym_GetKeyDataLen(const GWEN_CRYPT_KEY *k) {
  GWEN_CRYPT_KEY_SYM *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  return xk->keyLen;
}




GWEN_CRYPT_KEY *GWEN_Crypt_KeyDes3K_Generate(GWEN_CRYPT_CRYPTMODE mode,
					     int keySize,
					     int quality){
  uint8_t kd[16];
  GWEN_CRYPT_KEY *k;

  GWEN_Crypt_Random(quality, kd, 16);
  k=GWEN_Crypt_KeyDes3K_fromData(mode, keySize, kd, 16);
  memset(kd, 0, 16);

  return k;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyDes3K_fromData(GWEN_CRYPT_CRYPTMODE mode, int keySize,
					     const uint8_t *kd, uint32_t kl) {
  if (kl==16) {
    uint8_t new_kd[24];
    GWEN_CRYPT_KEY *k;

    /* 3key DES with only two keys, copy key1 as key3 */
    memmove(new_kd, kd, 16);
    memmove(new_kd+16, new_kd, 8);
    k=GWEN_Crypt_KeySym_fromData(GWEN_Crypt_CryptAlgoId_Des3K, 24,
				 mode, GCRY_CIPHER_3DES, GCRY_CIPHER_SECURE, new_kd, 24);
    memset(new_kd, 0, 24);
    return k;
  }
  else
    return GWEN_Crypt_KeySym_fromData(GWEN_Crypt_CryptAlgoId_Des3K, keySize,
				      mode, GCRY_CIPHER_3DES, GCRY_CIPHER_SECURE, kd, kl);
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyDes3K_fromDb(GWEN_CRYPT_CRYPTMODE mode,
					   GWEN_DB_NODE *db) {
  return GWEN_Crypt_KeySym_fromDb(GWEN_Crypt_CryptAlgoId_Des3K, mode,
				  GCRY_CIPHER_3DES, GCRY_CIPHER_SECURE, "des3k", db);
}



int GWEN_Crypt_KeyDes3K_toDb(const GWEN_CRYPT_KEY *k, GWEN_DB_NODE *db) {
  return GWEN_Crypt_KeySym_toDb(k, db, "des3k");
}



int GWEN_Crypt_KeyDes3K_SetKeyData(GWEN_CRYPT_KEY *k, const uint8_t *kd, uint32_t kl) {
  if (kl==16) {
    uint8_t new_kd[24];
    int rv;

    /* 3key DES with only two keys, copy key1 as key3 */
    memmove(new_kd, kd, 16);
    memmove(new_kd+16, new_kd, 8);
    rv=GWEN_Crypt_KeySym_SetKeyData(k, new_kd, 24);
    memset(new_kd, 0, 24);
    return rv;
  }
  else
    return GWEN_Crypt_KeySym_SetKeyData(k, kd, kl);
}



uint8_t *GWEN_Crypt_KeyDes3K_GetKeyDataPtr(const GWEN_CRYPT_KEY *k) {
  return GWEN_Crypt_KeySym_GetKeyDataPtr(k);
}



uint32_t GWEN_Crypt_KeyDes3K_GetKeyDataLen(const GWEN_CRYPT_KEY *k) {
  return GWEN_Crypt_KeySym_GetKeyDataLen(k);
}



int GWEN_Crypt_KeyDes3K_SetIV(GWEN_CRYPT_KEY *k,
			      const uint8_t *kd,
			      uint32_t kl) {
  GWEN_CRYPT_KEY_SYM *xk;
  gcry_error_t err;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_SYM, k);
  assert(xk);

  if (kd==NULL || kl==0) {
    const uint8_t iv[]={
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    err=gcry_cipher_setiv(xk->algoHandle, iv, sizeof(iv));
  }
  else
    err=gcry_cipher_setiv(xk->algoHandle, kd, kl);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_cipher_setiv(): %s", gcry_strerror(err));
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}







GWEN_CRYPT_KEY *GWEN_Crypt_KeyBlowFish_Generate(GWEN_CRYPT_CRYPTMODE mode,
						int keySize,
						int quality){
  return GWEN_Crypt_KeySym_Generate(GWEN_Crypt_CryptAlgoId_BlowFish, keySize, mode,
				    GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_SECURE, quality);
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyBlowFish_fromData(GWEN_CRYPT_CRYPTMODE mode, int keySize,
						const uint8_t *kd, uint32_t kl) {
  return GWEN_Crypt_KeySym_fromData(GWEN_Crypt_CryptAlgoId_BlowFish, keySize, mode,
				    GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_SECURE,
				    kd, kl);
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyBlowFish_fromDb(GWEN_CRYPT_CRYPTMODE mode,
					      GWEN_DB_NODE *db) {
  return GWEN_Crypt_KeySym_fromDb(GWEN_Crypt_CryptAlgoId_BlowFish, mode,
				  GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_SECURE, "blowFish", db);
}



int GWEN_Crypt_KeyBlowFish_toDb(const GWEN_CRYPT_KEY *k, GWEN_DB_NODE *db) {
  return GWEN_Crypt_KeySym_toDb(k, db, "blowFish");
}



int GWEN_Crypt_KeyBlowFish_SetKeyData(GWEN_CRYPT_KEY *k, const uint8_t *kd, uint32_t kl) {
  return GWEN_Crypt_KeySym_SetKeyData(k, kd, kl);
}



uint8_t *GWEN_Crypt_KeyBlowFish_GetKeyDataPtr(const GWEN_CRYPT_KEY *k) {
  return GWEN_Crypt_KeySym_GetKeyDataPtr(k);
}



uint32_t GWEN_Crypt_KeyBlowFish_GetKeyDataLen(const GWEN_CRYPT_KEY *k) {
  return GWEN_Crypt_KeySym_GetKeyDataLen(k);
}










