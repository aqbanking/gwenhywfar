/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#define DISABLE_DEBUGLOG


#include "cryptkey_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <gcrypt.h>




GWEN_INHERIT_FUNCTIONS(GWEN_CRYPT_KEY)
GWEN_LIST_FUNCTIONS(GWEN_CRYPT_KEY, GWEN_Crypt_Key)
GWEN_LIST2_FUNCTIONS(GWEN_CRYPT_KEY, GWEN_Crypt_Key)




int GWEN_Crypt3_ModuleInit(void) {
  gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
  if (!gcry_check_version (NEED_LIBGCRYPT_VERSION)) {
    const char *gcrypt_version = gcry_check_version(0);
    DBG_ERROR(GWEN_LOGDOMAIN, "Libgcrypt version mismatch: "
                              "Gwen needs GCrypt >= %s, but is running with GCrypt %s",
			      NEED_LIBGCRYPT_VERSION, gcrypt_version);
    return GWEN_ERROR_GENERIC;
  }
  /*gcry_control(GCRYCTL_SET_DEBUG_FLAGS, 1u, 0);*/

  gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);

  return 0;
}



int GWEN_Crypt3_ModuleFini(void) {
  return 0;
}





GWEN_CRYPT_KEY *GWEN_Crypt_Key_new(GWEN_CRYPT_CRYPTALGOID cryptAlgoId, int keySize) {
  GWEN_CRYPT_KEY *k;

  GWEN_NEW_OBJECT(GWEN_CRYPT_KEY, k)
  k->refCount=1;
  GWEN_INHERIT_INIT(GWEN_CRYPT_KEY, k)
  GWEN_LIST_INIT(GWEN_CRYPT_KEY, k)

  k->cryptAlgoId=cryptAlgoId;
  k->keySize=keySize;

  return k;
}



GWEN_CRYPT_KEY *GWEN_Crypt_Key_fromDb(GWEN_DB_NODE *db) {
  GWEN_CRYPT_KEY *k;
  GWEN_CRYPT_CRYPTALGOID cryptAlgoId;
  int keySize;
  const char *s;

  s=GWEN_DB_GetCharValue(db, "cryptAlgoId", 0, NULL);
  if (s)
    cryptAlgoId=GWEN_Crypt_CryptAlgoId_fromString(s);
  else
    cryptAlgoId=GWEN_Crypt_CryptAlgoId_Unknown;

  if (cryptAlgoId==GWEN_Crypt_CryptAlgoId_Unknown) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown crypt algo id [%s]",
	      s?s:"---");
    return NULL;
  }

  keySize=GWEN_DB_GetIntValue(db, "keySize", 0, -1);
  if (keySize==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing keysize");
    return NULL;
  }

  k=GWEN_Crypt_Key_new(cryptAlgoId, keySize);
  if (k==NULL)
    return NULL;

  k->keyNumber=GWEN_DB_GetIntValue(db, "keyNumber", 0, 0);
  k->keyVersion=GWEN_DB_GetIntValue(db, "keyVersion", 0, 0);

  return k;
}



int GWEN_Crypt_Key_toDb(const GWEN_CRYPT_KEY *k, GWEN_DB_NODE *db) {
  assert(k);
  assert(db);

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "cryptAlgoId",
		       GWEN_Crypt_CryptAlgoId_toString(k->cryptAlgoId));
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keySize", k->keySize);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keyNumber", k->keyNumber);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keyVersion", k->keyVersion);

  return 0;
}



GWEN_CRYPT_KEY *GWEN_Crypt_Key_dup(const GWEN_CRYPT_KEY *k) {
  GWEN_CRYPT_KEY *nk;

  assert(k);
  nk=GWEN_Crypt_Key_new(k->cryptAlgoId, k->keySize);
  if (nk==NULL)
    return NULL;

  nk->keyNumber=k->keyNumber;
  nk->keyVersion=k->keyVersion;

  return nk;
}



void GWEN_Crypt_Key_free(GWEN_CRYPT_KEY *k) {
  if (k) {
    assert(k->refCount);
    if (k->refCount==1) {
      GWEN_INHERIT_FINI(GWEN_CRYPT_KEY, k)
      GWEN_LIST_FINI(GWEN_CRYPT_KEY, k)
      k->refCount=0;
      GWEN_FREE_OBJECT(k);
    }
    else
      k->refCount--;
  }
}



GWEN_CRYPT_CRYPTALGOID GWEN_Crypt_Key_GetCryptAlgoId(const GWEN_CRYPT_KEY *k) {
  assert(k);
  return k->cryptAlgoId;
}



int GWEN_Crypt_Key_GetKeySize(const GWEN_CRYPT_KEY *k) {
  assert(k);
  return k->keySize;
}



int GWEN_Crypt_Key_GetKeyNumber(const GWEN_CRYPT_KEY *k) {
  assert(k);
  return k->keyNumber;
}



void GWEN_Crypt_Key_SetKeyNumber(GWEN_CRYPT_KEY *k, int i) {
  assert(k);
  k->keyNumber=i;
}



int GWEN_Crypt_Key_GetKeyVersion(const GWEN_CRYPT_KEY *k) {
  assert(k);
  return k->keyVersion;
}



void GWEN_Crypt_Key_SetKeyVersion(GWEN_CRYPT_KEY *k, int i) {
  assert(k);
  k->keyVersion=i;
}



GWEN_CRYPT_KEY_SIGN_FN GWEN_Crypt_Key_SetSignFn(GWEN_CRYPT_KEY *k, GWEN_CRYPT_KEY_SIGN_FN f) {
  GWEN_CRYPT_KEY_SIGN_FN of;

  assert(k);
  of=k->signFn;
  k->signFn=f;

  return of;
}



GWEN_CRYPT_KEY_VERIFY_FN GWEN_Crypt_Key_SetVerifyFn(GWEN_CRYPT_KEY *k, GWEN_CRYPT_KEY_VERIFY_FN f){
  GWEN_CRYPT_KEY_VERIFY_FN of;

  assert(k);
  of=k->verifyFn;
  k->verifyFn=f;

  return of;
}



GWEN_CRYPT_KEY_ENCIPHER_FN GWEN_Crypt_Key_SetEncipherFn(GWEN_CRYPT_KEY *k,
							GWEN_CRYPT_KEY_ENCIPHER_FN f) {
  GWEN_CRYPT_KEY_ENCIPHER_FN of;

  assert(k);
  of=k->encipherFn;
  k->encipherFn=f;

  return of;
}



GWEN_CRYPT_KEY_DECIPHER_FN GWEN_Crypt_Key_SetDecipherFn(GWEN_CRYPT_KEY *k,
							GWEN_CRYPT_KEY_DECIPHER_FN f) {
  GWEN_CRYPT_KEY_DECIPHER_FN of;

  assert(k);
  of=k->decipherFn;
  k->decipherFn=f;

  return of;
}



int GWEN_Crypt_Key_Sign(GWEN_CRYPT_KEY *k,
			const uint8_t *pInData,
			uint32_t inLen,
			uint8_t *pSignatureData,
			uint32_t *pSignatureLen) {
  assert(k);
  if (k->signFn)
    return k->signFn(k, pInData, inLen, pSignatureData, pSignatureLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Key_Verify(GWEN_CRYPT_KEY *k,
			  const uint8_t *pInData,
			  uint32_t inLen,
			  const uint8_t *pSignatureData,
			  uint32_t signatureLen) {
  assert(k);
  if (k->verifyFn)
    return k->verifyFn(k, pInData, inLen, pSignatureData, signatureLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Key_Encipher(GWEN_CRYPT_KEY *k,
			    const uint8_t *pInData,
			    uint32_t inLen,
			    uint8_t *pOutData,
			    uint32_t *pOutLen) {
  assert(k);
  if (k->encipherFn)
    return k->encipherFn(k, pInData, inLen, pOutData, pOutLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Key_Decipher(GWEN_CRYPT_KEY *k,
			    const uint8_t *pInData,
			    uint32_t inLen,
			    uint8_t *pOutData,
			    uint32_t *pOutLen) {
  assert(k);
  if (k->decipherFn)
    return k->decipherFn(k, pInData, inLen, pOutData, pOutLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}








