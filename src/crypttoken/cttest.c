/***************************************************************************
    begin       : Sun Jan 13 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "cttest_p.h"




GWEN_CRYPT_TOKEN *GWEN_Crypt_TokenTest_new(int maximumContexts, int maximumKeys) {
  GWEN_CRYPT_TOKEN *ct;
  GWEN_CRYPT_TOKEN_TEST *lct;

  ct=GWEN_Crypt_Token_new(GWEN_Crypt_Token_Device_File, "ctTest", NULL);
  assert(ct);

  GWEN_NEW_OBJECT(GWEN_CRYPT_TOKEN_TEST, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_TEST, ct, lct,
                       GWEN_Crypt_TokenTest_freeData);

  GWEN_Crypt_Token_SetOpenFn(ct, GWEN_Crypt_TokenTest_Open);
  GWEN_Crypt_Token_SetCreateFn(ct, GWEN_Crypt_TokenTest_Create);
  GWEN_Crypt_Token_SetCloseFn(ct, GWEN_Crypt_TokenTest_Close);
  GWEN_Crypt_Token_SetGetKeyIdListFn(ct, GWEN_Crypt_TokenTest__GetKeyIdList);
  GWEN_Crypt_Token_SetGetKeyInfoFn(ct, GWEN_Crypt_TokenTest__GetKeyInfo);
  GWEN_Crypt_Token_SetSetKeyInfoFn(ct, GWEN_Crypt_TokenTest__SetKeyInfo);
  GWEN_Crypt_Token_SetGetContextIdListFn(ct, GWEN_Crypt_TokenTest__GetContextIdList);
  GWEN_Crypt_Token_SetGetContextFn(ct, GWEN_Crypt_TokenTest__GetContext);
  GWEN_Crypt_Token_SetSetContextFn(ct, GWEN_Crypt_TokenTest__SetContext);
  GWEN_Crypt_Token_SetSignFn(ct, GWEN_Crypt_TokenTest__Sign);
  GWEN_Crypt_Token_SetVerifyFn(ct, GWEN_Crypt_TokenTest__Verify);
  GWEN_Crypt_Token_SetEncipherFn(ct, GWEN_Crypt_TokenTest__Encipher);
  GWEN_Crypt_Token_SetDecipherFn(ct, GWEN_Crypt_TokenTest__Decipher);
  GWEN_Crypt_Token_SetGenerateKeyFn(ct, GWEN_Crypt_TokenTest__GenerateKey);
  GWEN_Crypt_Token_SetActivateKeyFn(ct, GWEN_Crypt_TokenTest__ActivateKey);

  lct->contextList=GWEN_Crypt_Token_Context_List_new();
  lct->keyList=GWEN_CryptKey_List_new();

  lct->maximumContexts=maximumContexts;
  lct->maximumKeys=maximumKeys;
  return ct;
}



void GWEN_Crypt_TokenTest_freeData(void *bp, void *p) {
  GWEN_CRYPT_TOKEN_TEST *lct;

  lct=(GWEN_CRYPT_TOKEN_TEST*) p;
  GWEN_CryptKey_List_free(lct->keyList);
  GWEN_Crypt_Token_Context_List_free(lct->contextList);
}



int GWEN_Crypt_TokenTest_Create(GWEN_CRYPT_TOKEN *ct, uint32_t gid) {
  int i;

  for (i=0; i<lct->maximumContexts; i++) {
    GWEN_CRYPT_TOKEN_CONTEXT *ctx;
    int keyBaseId;

    ctx=GWEN_Crypt_Token_Context_new();
    GWEN_Crypt_Token_Context_SetId(ctx, i+1);

    keyBaseId=(i*10);
    GWEN_Crypt_Token_Context_SetSignKeyId(ctx, keyBaseId);
    GWEN_Crypt_Token_Context_SetVerifyKeyId(ctx, keyBaseId); /* same key for both */

    GWEN_Crypt_Token_Context_SetEncipherKeyId(ctx, keyBaseId+1);
    GWEN_Crypt_Token_Context_SetDecipherKeyId(ctx, keyBaseId+1); /* same key for both */

    GWEN_Crypt_Token_Context_SetAuthSignKeyId(ctx, keyBaseId+2);
    GWEN_Crypt_Token_Context_SetAuthVerifyKeyId(ctx, keyBaseId+2); /* same key for both */

  }
}


int GWEN_Crypt_TokenTest_Open(GWEN_CRYPT_TOKEN *ct, int admin, uint32_t gid) {
}



int GWEN_Crypt_TokenTest_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t gid) {
}



int GWEN_Crypt_TokenTest__GetKeyIdList(GWEN_CRYPT_TOKEN *ct,
                                       uint32_t *pIdList,
                                       uint32_t *pCount,
                                       uint32_t gid) {
}



const GWEN_CRYPT_TOKEN_KEYINFO* GWEN_Crypt_TokenTest__GetKeyInfo(GWEN_CRYPT_TOKEN *ct,
                                                                 uint32_t id,
                                                                 uint32_t flags,
                                                                 uint32_t gid) {
}



int GWEN_Crypt_TokenTest__SetKeyInfo(GWEN_CRYPT_TOKEN *ct,
                                     uint32_t id,
                                     const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                                     uint32_t gid) {
}



int GWEN_Crypt_TokenTest__GetContextIdList(GWEN_CRYPT_TOKEN *ct,
                                           uint32_t *pIdList,
                                           uint32_t *pCount,
                                           uint32_t gid) {
}



const GWEN_CRYPT_TOKEN_CONTEXT* GWEN_Crypt_TokenTest__GetContext(GWEN_CRYPT_TOKEN *ct,
                                                                 uint32_t id,
                                                                 uint32_t gid) {
}



int GWEN_Crypt_TokenTest__SetContext(GWEN_CRYPT_TOKEN *ct,
                                     uint32_t id,
                                     const GWEN_CRYPT_TOKEN_CONTEXT *nctx,
                                     uint32_t gid) {
}



int GWEN_Crypt_TokenTest__Sign(GWEN_CRYPT_TOKEN *ct,
                               uint32_t keyId,
                               GWEN_CRYPT_PADDALGO *a,
                               const uint8_t *pInData,
                               uint32_t inLen,
                               uint8_t *pSignatureData,
                               uint32_t *pSignatureLen,
                               uint32_t *pSeqCounter,
                               uint32_t gid) {
}



int GWEN_Crypt_TokenTest__Verify(GWEN_CRYPT_TOKEN *ct,
                                 uint32_t keyId,
                                 GWEN_CRYPT_PADDALGO *a,
                                 const uint8_t *pInData,
                                 uint32_t inLen,
                                 const uint8_t *pSignatureData,
                                 uint32_t signatureLen,
                                 uint32_t seqCounter,
                                 uint32_t gid) {
}



int GWEN_Crypt_TokenTest__Encipher(GWEN_CRYPT_TOKEN *ct,
                                   uint32_t keyId,
                                   GWEN_CRYPT_PADDALGO *a,
                                   const uint8_t *pInData,
                                   uint32_t inLen,
                                   uint8_t *pOutData,
                                   uint32_t *pOutLen,
                                   uint32_t gid) {
}



int GWEN_Crypt_TokenTest__Decipher(GWEN_CRYPT_TOKEN *ct,
                                   uint32_t keyId,
                                   GWEN_CRYPT_PADDALGO *a,
                                   const uint8_t *pInData,
                                   uint32_t inLen,
                                   uint8_t *pOutData,
                                   uint32_t *pOutLen,
                                   uint32_t gid) {
}



int GWEN_Crypt_TokenTest__GenerateKey(GWEN_CRYPT_TOKEN *ct,
                                      uint32_t keyId,
                                      const GWEN_CRYPT_CRYPTALGO *a,
                                      uint32_t gid) {
}







