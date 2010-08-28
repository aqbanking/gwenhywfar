/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "ctf_context_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>





GWEN_INHERIT(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT)




GWEN_CRYPT_TOKEN_CONTEXT *GWEN_CTF_Context_new() {
  GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  GWEN_CTF_CONTEXT *fctx;

  ctx=GWEN_Crypt_Token_Context_new();
  GWEN_NEW_OBJECT(GWEN_CTF_CONTEXT, fctx)
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx, fctx,
		       GWEN_CTF_Context_freeData)

  return ctx;
}



int GWEN_CTF_Context_IsOfThisType(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  assert(ctx);
  return GWEN_INHERIT_ISOFTYPE(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
}



GWENHYWFAR_CB
void GWEN_CTF_Context_freeData(GWEN_UNUSED void *bp, void *p) {
  GWEN_CTF_CONTEXT *fctx;

  fctx=(GWEN_CTF_CONTEXT*) p;
  GWEN_Crypt_Key_free(fctx->localSignKey);
  GWEN_Crypt_Key_free(fctx->localCryptKey);
  GWEN_Crypt_Key_free(fctx->remoteSignKey);
  GWEN_Crypt_Key_free(fctx->remoteCryptKey);
  GWEN_Crypt_Key_free(fctx->localAuthKey);
  GWEN_Crypt_Key_free(fctx->remoteAuthKey);
  GWEN_Crypt_Key_free(fctx->tempLocalSignKey);

  GWEN_Crypt_Token_KeyInfo_free(fctx->localSignKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(fctx->localCryptKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(fctx->remoteSignKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(fctx->remoteCryptKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(fctx->localAuthKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(fctx->remoteAuthKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(fctx->tempLocalSignKeyInfo);

  GWEN_FREE_OBJECT(fctx);
}



GWEN_CRYPT_KEY *GWEN_CTF_Context_GetLocalSignKey(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->localSignKey;
}



void GWEN_CTF_Context_SetLocalSignKey(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_KEY *k) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Key_free(fctx->localSignKey);
  fctx->localSignKey=k;
}



GWEN_CRYPT_TOKEN_KEYINFO*GWEN_CTF_Context_GetLocalSignKeyInfo(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->localSignKeyInfo;
}



void GWEN_CTF_Context_SetLocalSignKeyInfo(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Token_KeyInfo_free(fctx->localSignKeyInfo);
  fctx->localSignKeyInfo=ki;
}



GWEN_CRYPT_KEY *GWEN_CTF_Context_GetLocalCryptKey(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->localCryptKey;
}



void GWEN_CTF_Context_SetLocalCryptKey(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_KEY *k) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Key_free(fctx->localCryptKey);
  fctx->localCryptKey=k;
}



GWEN_CRYPT_TOKEN_KEYINFO *GWEN_CTF_Context_GetLocalCryptKeyInfo(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->localCryptKeyInfo;
}



void GWEN_CTF_Context_SetLocalCryptKeyInfo(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Token_KeyInfo_free(fctx->localCryptKeyInfo);
  fctx->localCryptKeyInfo=ki;
}



GWEN_CRYPT_KEY *GWEN_CTF_Context_GetRemoteSignKey(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->remoteSignKey;
}



void GWEN_CTF_Context_SetRemoteSignKey(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_KEY *k) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Key_free(fctx->remoteSignKey);
  fctx->remoteSignKey=k;
}



GWEN_CRYPT_TOKEN_KEYINFO *GWEN_CTF_Context_GetRemoteSignKeyInfo(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->remoteSignKeyInfo;
}



void GWEN_CTF_Context_SetRemoteSignKeyInfo(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Token_KeyInfo_free(fctx->remoteSignKeyInfo);
  fctx->remoteSignKeyInfo=ki;
}



GWEN_CRYPT_KEY *GWEN_CTF_Context_GetRemoteCryptKey(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->remoteCryptKey;
}



void GWEN_CTF_Context_SetRemoteCryptKey(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_KEY *k) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Key_free(fctx->remoteCryptKey);
  fctx->remoteCryptKey=k;
}



GWEN_CRYPT_TOKEN_KEYINFO *GWEN_CTF_Context_GetRemoteCryptKeyInfo(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->remoteCryptKeyInfo;
}



void GWEN_CTF_Context_SetRemoteCryptKeyInfo(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Token_KeyInfo_free(fctx->remoteCryptKeyInfo);
  fctx->remoteCryptKeyInfo=ki;
}



GWEN_CRYPT_KEY *GWEN_CTF_Context_GetLocalAuthKey(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->localAuthKey;
}



void GWEN_CTF_Context_SetLocalAuthKey(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_KEY *k) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Key_free(fctx->localAuthKey);
  fctx->localAuthKey=k;
}



GWEN_CRYPT_TOKEN_KEYINFO *GWEN_CTF_Context_GetLocalAuthKeyInfo(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->localAuthKeyInfo;
}



void GWEN_CTF_Context_SetLocalAuthKeyInfo(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Token_KeyInfo_free(fctx->localAuthKeyInfo);
  fctx->localAuthKeyInfo=ki;
}



GWEN_CRYPT_KEY *GWEN_CTF_Context_GetRemoteAuthKey(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->remoteAuthKey;
}



void GWEN_CTF_Context_SetRemoteAuthKey(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_KEY *k) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Key_free(fctx->remoteAuthKey);
  fctx->remoteAuthKey=k;
}



GWEN_CRYPT_TOKEN_KEYINFO *GWEN_CTF_Context_GetRemoteAuthKeyInfo(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->remoteAuthKeyInfo;
}



void GWEN_CTF_Context_SetRemoteAuthKeyInfo(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Token_KeyInfo_free(fctx->remoteAuthKeyInfo);
  fctx->remoteAuthKeyInfo=ki;
}



GWEN_CRYPT_KEY *GWEN_CTF_Context_GetTempLocalSignKey(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->tempLocalSignKey;
}



void GWEN_CTF_Context_SetTempLocalSignKey(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_KEY *k) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Key_free(fctx->tempLocalSignKey);
  fctx->tempLocalSignKey=k;
}



GWEN_CRYPT_TOKEN_KEYINFO*GWEN_CTF_Context_GetTempLocalSignKeyInfo(const GWEN_CRYPT_TOKEN_CONTEXT *ctx) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  return fctx->tempLocalSignKeyInfo;
}



void GWEN_CTF_Context_SetTempLocalSignKeyInfo(GWEN_CRYPT_TOKEN_CONTEXT *ctx, GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CTF_CONTEXT *fctx;

  assert(ctx);
  fctx=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_CTF_CONTEXT, ctx);
  assert(fctx);

  GWEN_Crypt_Token_KeyInfo_free(fctx->tempLocalSignKeyInfo);
  fctx->tempLocalSignKeyInfo=ki;
}








