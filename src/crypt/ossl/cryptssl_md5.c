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


#include "cryptssl_md5_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_ERRORCODE GWEN_MdMd5_Register(){
  GWEN_MD_PROVIDER *pr;
  GWEN_ERRORCODE err;

  pr=GWEN_MdProvider_new();
  GWEN_MdProvider_SetNewMdFn(pr, GWEN_MdMd5_new);
  GWEN_MdProvider_SetName(pr, GWEN_MD_MD5_NAME);
  err=GWEN_MD_RegisterProvider(pr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_MdProvider_free(pr);
    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
    return err;
  }
  return 0;
}



GWEN_MD *GWEN_MdMd5_new(){
  GWEN_MD *md;
  MD5_CTX *ctx;

  md=GWEN_MD_new(MD5_DIGEST_LENGTH);
  ctx=(MD5_CTX*)malloc(sizeof(MD5_CTX));
  GWEN_MD_SetData(md, ctx);
  GWEN_MD_SetBeginFn(md, GWEN_MdMd5_Begin);
  GWEN_MD_SetUpdateFn(md, GWEN_MdMd5_Update);
  GWEN_MD_SetEndFn(md, GWEN_MdMd5_End);
  GWEN_MD_SetFreeDataFn(md, GWEN_MdMd5_FreeData);
  return md;
}



void GWEN_MdMd5_FreeData(GWEN_MD *md){
  MD5_CTX *ctx;

  if (md) {
    ctx=(MD5_CTX*)GWEN_MD_GetData(md);
    assert(ctx);
    free(ctx);
  }
}



int GWEN_MdMd5_Begin(GWEN_MD *md){
  MD5_CTX *ctx;

  assert(md);
  ctx=(MD5_CTX*)GWEN_MD_GetData(md);
  assert(ctx);
  MD5_Init(ctx);
  return 0;
}



int GWEN_MdMd5_End(GWEN_MD *md){
  MD5_CTX *ctx;

  assert(md);
  ctx=(MD5_CTX*)GWEN_MD_GetData(md);
  assert(ctx);
  MD5_Final(GWEN_MD_GetDigestPtr(md), ctx);
  return 0;
}



int GWEN_MdMd5_Update(GWEN_MD *md,
                         const char *buf,
                         unsigned int l){
  MD5_CTX *ctx;

  assert(md);
  ctx=(MD5_CTX*)GWEN_MD_GetData(md);
  assert(ctx);
  MD5_Update(ctx, buf, l);
  return 0;
}












