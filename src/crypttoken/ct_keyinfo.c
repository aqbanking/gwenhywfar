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


#include "ct_keyinfo_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_LIST_FUNCTIONS(GWEN_CRYPT_TOKEN_KEYINFO, GWEN_Crypt_Token_KeyInfo)
GWEN_LIST2_FUNCTIONS(GWEN_CRYPT_TOKEN_KEYINFO, GWEN_Crypt_Token_KeyInfo)




GWEN_CRYPT_TOKEN_KEYINFO *GWEN_Crypt_Token_KeyInfo_new(uint32_t kid,
    GWEN_CRYPT_CRYPTALGOID a,
    int keySize) {
  GWEN_CRYPT_TOKEN_KEYINFO *ki;

  GWEN_NEW_OBJECT(GWEN_CRYPT_TOKEN_KEYINFO, ki)
  ki->refCount=1;
  GWEN_LIST_INIT(GWEN_CRYPT_TOKEN_KEYINFO, ki)

  ki->keyId=kid;
  ki->cryptAlgoId=a;
  ki->keySize=keySize;

  return ki;
}



void GWEN_Crypt_Token_KeyInfo_free(GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  if (ki) {
    assert(ki->refCount);
    if (ki->refCount==1) {
      free(ki->keyDescr);
      if (ki->modulusData)
        free(ki->modulusData);
      ki->modulusData=NULL;
      if (ki->exponentData)
        free(ki->exponentData);
      ki->exponentData=NULL;

      ki->refCount=0;
      GWEN_LIST_FINI(GWEN_CRYPT_TOKEN_KEYINFO, ki);
      GWEN_FREE_OBJECT(ki);
    }
    else {
      ki->refCount--;
    }
  }
}



GWEN_CRYPT_TOKEN_KEYINFO *GWEN_Crypt_Token_KeyInfo_dup(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CRYPT_TOKEN_KEYINFO *nki;

  nki=GWEN_Crypt_Token_KeyInfo_new(ki->keyId,
                                   ki->cryptAlgoId,
                                   ki->keySize);
  nki->flags=ki->flags;

  if (ki->modulusData && ki->modulusLen) {
    nki->modulusData=(uint8_t*)malloc(ki->modulusLen);
    assert(nki->modulusData);
    memmove(nki->modulusData, ki->modulusData, ki->modulusLen);
    nki->modulusLen=ki->modulusLen;
  }

  if (ki->exponentData && ki->exponentLen) {
    nki->exponentData=(uint8_t*)malloc(ki->exponentLen);
    assert(nki->exponentData);
    memmove(nki->exponentData, ki->exponentData, ki->exponentLen);
    nki->exponentLen=ki->exponentLen;
  }

  if (ki->keyDescr)
    nki->keyDescr=strdup(ki->keyDescr);

  nki->keyNumber=ki->keyNumber;
  nki->keyVersion=ki->keyVersion;
  nki->signCounter=ki->signCounter;

  return nki;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetKeyId(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->keyId;
}



GWEN_CRYPT_CRYPTALGOID GWEN_Crypt_Token_KeyInfo_GetCryptAlgoId(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->cryptAlgoId;
}



int GWEN_Crypt_Token_KeyInfo_GetKeySize(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->keySize;
}



void GWEN_Crypt_Token_KeyInfo_SetKeySize(GWEN_CRYPT_TOKEN_KEYINFO *ki, int i) {
  assert(ki);
  assert(ki->refCount);
  ki->keySize=i;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetFlags(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->flags;
}



void GWEN_Crypt_Token_KeyInfo_SetFlags(GWEN_CRYPT_TOKEN_KEYINFO *ki, uint32_t f) {
  assert(ki);
  assert(ki->refCount);
  ki->flags=f;
}



void GWEN_Crypt_Token_KeyInfo_AddFlags(GWEN_CRYPT_TOKEN_KEYINFO *ki, uint32_t f) {
  assert(ki);
  assert(ki->refCount);
  ki->flags|=f;
}



void GWEN_Crypt_Token_KeyInfo_SubFlags(GWEN_CRYPT_TOKEN_KEYINFO *ki, uint32_t f) {
  assert(ki);
  assert(ki->refCount);
  ki->flags&=~f;
}



const uint8_t *GWEN_Crypt_Token_KeyInfo_GetModulusData(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->modulusData;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetModulusLen(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->modulusLen;
}



void GWEN_Crypt_Token_KeyInfo_SetModulus(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    const uint8_t *p,
    uint32_t len) {
  assert(ki);
  assert(ki->refCount);

  assert(p);
  assert(len);

  if (ki->modulusData)
    free(ki->modulusData);
  ki->modulusData=(uint8_t*) malloc(len);
  assert(ki->modulusData);
  memmove(ki->modulusData, p, len);
  ki->modulusLen=len;
}



const uint8_t *GWEN_Crypt_Token_KeyInfo_GetExponentData(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->exponentData;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetExponentLen(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->exponentLen;
}



void GWEN_Crypt_Token_KeyInfo_SetExponent(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    const uint8_t *p,
    uint32_t len) {
  assert(ki);
  assert(ki->refCount);

  assert(p);
  assert(len);

  if (ki->exponentData)
    free(ki->exponentData);
  ki->exponentData=(uint8_t*) malloc(len);
  assert(ki->exponentData);
  memmove(ki->exponentData, p, len);
  ki->exponentLen=len;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetKeyVersion(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->keyVersion;
}



void GWEN_Crypt_Token_KeyInfo_SetKeyVersion(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    uint32_t i) {
  assert(ki);
  assert(ki->refCount);

  ki->keyVersion=i;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetKeyNumber(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->keyNumber;
}



void GWEN_Crypt_Token_KeyInfo_SetKeyNumber(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    uint32_t i) {
  assert(ki);
  assert(ki->refCount);

  ki->keyNumber=i;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetSignCounter(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->signCounter;
}



void GWEN_Crypt_Token_KeyInfo_SetSignCounter(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    uint32_t i) {
  assert(ki);
  assert(ki->refCount);

  ki->signCounter=i;
}



const char *GWEN_Crypt_Token_KeyInfo_GetKeyDescr(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->keyDescr;
}



void GWEN_Crypt_Token_KeyInfo_SetKeyDescr(GWEN_CRYPT_TOKEN_KEYINFO *ki, const char *s) {
  assert(ki);
  assert(ki->refCount);

  free(ki->keyDescr);
  if (s)
    ki->keyDescr=strdup(s);
  else
    ki->keyDescr=NULL;
}














