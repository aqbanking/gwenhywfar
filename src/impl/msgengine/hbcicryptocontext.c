/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 08 2003
    copyright   : (C) 2003 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "hbcicryptocontext_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>




/* --------------------------------------------------------------- FUNCTION */
const char*
GWEN_HBCICryptoContext_GetSecurityIdPtr(GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(ctx);
  return ctx->pSecurityId;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCICryptoContext_SetSecurityId(GWEN_HBCICRYPTOCONTEXT *ctx,
                                          const void *p, unsigned int l){
  assert(ctx);
  if (ctx->pSecurityId)
    free(ctx->pSecurityId);
  ctx->pSecurityId=0;
  if (l) {
    ctx->pSecurityId=(char*)malloc(l);
    ctx->lSecurityId=l;
    memmove(ctx->pSecurityId, p, l);
  }
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int
GWEN_HBCICryptoContext_GetSecurityIdSize(GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(ctx);
  return ctx->lSecurityId;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int
GWEN_HBCICryptoContext_GetSequenceNum(GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(ctx);
  return ctx->seq;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCICryptoContext_SetSequenceNum(GWEN_HBCICRYPTOCONTEXT *ctx,
                                           unsigned int i){
  assert(ctx);
  ctx->seq=i;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_KEYSPEC *GWEN_HBCICryptoContext_GetKeySpec(GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(ctx);
  return ctx->key;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCICryptoContext_SetKeySpec(GWEN_HBCICRYPTOCONTEXT *ctx,
                                       GWEN_KEYSPEC *ks){
  assert(ctx);
  if (ctx->key)
    GWEN_KeySpec_free(ctx->key);
  ctx->key=GWEN_KeySpec_dup(ks);
}



/* --------------------------------------------------------------- FUNCTION */
const char*
GWEN_HBCICryptoContext_GetServiceCode(GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(ctx);
  return ctx->serviceCode;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCICryptoContext_SetServiceCode(GWEN_HBCICRYPTOCONTEXT *ctx,
                                           const char *s){
  assert(ctx);
  assert(s);
  free(ctx->serviceCode);
  ctx->serviceCode=strdup(s);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_HBCICRYPTOCONTEXT *GWEN_HBCICryptoContext_new(){
  GWEN_HBCICRYPTOCONTEXT *ctx;

  GWEN_NEW_OBJECT(GWEN_HBCICRYPTOCONTEXT, ctx);
  return ctx;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCICryptoContext_free(GWEN_HBCICRYPTOCONTEXT *ctx){
  if (ctx) {
    free(ctx->pSecurityId);
    free(ctx->pCryptKey);
    GWEN_KeySpec_free(ctx->key);
    free(ctx);
  }
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCICryptoContext_SetCryptKey(GWEN_HBCICRYPTOCONTEXT *ctx,
                                        const char *p,
                                        unsigned int l){
  assert(ctx);
  if (ctx->pCryptKey)
    free(ctx->pCryptKey);
  ctx->pCryptKey=0;
  if (l) {
    ctx->pCryptKey=(char*)malloc(l);
    ctx->lCryptKey=l;
    memmove(ctx->pCryptKey, p, l);
  }
}



/* --------------------------------------------------------------- FUNCTION */
const char *
GWEN_HBCICryptoContext_GetCryptKeyPtr(GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(ctx);
  return ctx->pCryptKey;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int
GWEN_HBCICryptoContext_GetCryptKeySize(GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(ctx);
  return ctx->lCryptKey;
}







