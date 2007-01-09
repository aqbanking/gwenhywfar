/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 06 2003
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


#include "cryptssl_rmd160_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_ERRORCODE GWEN_MdRmd160_Register(){
  GWEN_MD_PROVIDER *pr;
  GWEN_ERRORCODE err;

  pr=GWEN_MdProvider_new();
  GWEN_MdProvider_SetNewMdFn(pr, GWEN_MdRmd160_new);
  GWEN_MdProvider_SetName(pr, GWEN_MD_RMD160_NAME);
  err=GWEN_MD_RegisterProvider(pr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_MdProvider_free(pr);
    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
    return err;
  }
  return 0;
}



GWEN_MD *GWEN_MdRmd160_new(){
  GWEN_MD *md;
  RIPEMD160_CTX *ctx;

  md=GWEN_MD_new(RIPEMD160_DIGEST_LENGTH);
  ctx=(RIPEMD160_CTX*)malloc(sizeof(RIPEMD160_CTX));
  GWEN_MD_SetData(md, ctx);
  GWEN_MD_SetBeginFn(md, GWEN_MdRmd160_Begin);
  GWEN_MD_SetUpdateFn(md, GWEN_MdRmd160_Update);
  GWEN_MD_SetEndFn(md, GWEN_MdRmd160_End);
  GWEN_MD_SetFreeDataFn(md, GWEN_MdRmd160_FreeData);
  return md;
}



void GWENHYWFAR_CB GWEN_MdRmd160_FreeData(GWEN_MD *md){
  RIPEMD160_CTX *ctx;

  if (md) {
    ctx=(RIPEMD160_CTX*)GWEN_MD_GetData(md);
    assert(ctx);
    free(ctx);
  }
}



int GWEN_MdRmd160_Begin(GWEN_MD *md){
  RIPEMD160_CTX *ctx;

  assert(md);
  ctx=(RIPEMD160_CTX*)GWEN_MD_GetData(md);
  assert(ctx);
  RIPEMD160_Init(ctx);
  return 0;
}



int GWEN_MdRmd160_End(GWEN_MD *md){
  RIPEMD160_CTX *ctx;

  assert(md);
  ctx=(RIPEMD160_CTX*)GWEN_MD_GetData(md);
  assert(ctx);
  RIPEMD160_Final(GWEN_MD_GetDigestPtr(md), ctx);
  return 0;
}



int GWEN_MdRmd160_Update(GWEN_MD *md,
                         const char *buf,
                         unsigned int l){
  RIPEMD160_CTX *ctx;

  assert(md);
  ctx=(RIPEMD160_CTX*)GWEN_MD_GetData(md);
  assert(ctx);
  RIPEMD160_Update(ctx, buf, l);
  return 0;
}












