/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jan 17 2004
 copyright   : (C) 2004 by Martin Preuss
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

#include "waitcallback_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/list.h>


static GWEN_LIST *gwen_callbackstack=0;


GWEN_WAITCALLBACK_RESULT
GWEN_WaitCallback_internal(GWEN_TYPE_UINT32 pos,
                           GWEN_TYPE_UINT32 total,
                           GWEN_WAITCALLBACK_MODE m){
  GWEN_WAITCALLBACK_CTX *ctx;
  GWEN_WAITCALLBACK_RESULT res;
  time_t lct;

  if (!gwen_callbackstack) {
    DBG_VERBOUS(0, "No callbacks registered (1)");
    return GWEN_WaitCallbackResult_Continue;
  }
  ctx=GWEN_List_GetBack(gwen_callbackstack);
  if (!ctx) {
    DBG_WARN(0, "No callbacks registered, should not happen here");
    return GWEN_WaitCallbackResult_Continue;
  }

  if (!ctx->waitCallbackFn) {
    DBG_WARN(0, "No callback set");
    return GWEN_WaitCallbackResult_Continue;
  }

  lct=time(0);
  res=ctx->waitCallbackFn(pos, total, m, ctx);
  ctx->lastCalled=lct;
  return res;
}



GWEN_WAITCALLBACK_RESULT GWEN_WaitCallback(int count) {
  return GWEN_WaitCallbackProgress(count, 0);
}



GWEN_WAITCALLBACK_RESULT GWEN_WaitCallbackProgress(GWEN_TYPE_UINT32 pos,
                                                   GWEN_TYPE_UINT32 total) {
  return GWEN_WaitCallback_internal(pos, total,
                                    GWEN_WaitCallbackMode_Normal);
}



GWEN_WAITCALLBACK_CTX *GWEN_WaitCallback_Context_new(GWEN_WAITCALLBACK_FN fn,
                                                     void *data){
  GWEN_WAITCALLBACK_CTX *ctx;

  GWEN_NEW_OBJECT(GWEN_WAITCALLBACK_CTX, ctx);
  ctx->usage=1;
  ctx->waitCallbackFn=fn;
  ctx->data=data;
  return ctx;
}



void GWEN_WaitCallback_Context_free(GWEN_WAITCALLBACK_CTX *ctx){
  if (ctx) {
    if (ctx->usage>1) {
      DBG_ERROR(0, "Context still in use !");
      abort();
    }
    free(ctx);
  }
}



void *GWEN_WaitCallback_Context_GetData(GWEN_WAITCALLBACK_CTX *ctx){
  assert(ctx);
  return ctx->data;
}


void GWEN_WaitCallback_Enter(GWEN_WAITCALLBACK_CTX *ctx){
  assert(ctx);

  if (!gwen_callbackstack) {
    gwen_callbackstack=GWEN_List_new();
  }
  if (ctx->usage>1) {
    DBG_INFO(0, "WaitCallback entered recursively");
  }
  ctx->usage++;
  GWEN_List_PushBack(gwen_callbackstack, ctx);
  GWEN_WaitCallback_internal(0, 0, GWEN_WaitCallbackMode_Enter);
  ctx->lastEntered=time(0);
}



void GWEN_WaitCallback_Leave(){
  GWEN_WAITCALLBACK_CTX *ctx;

  assert(gwen_callbackstack);
  ctx=GWEN_List_GetBack(gwen_callbackstack);
  if (!ctx) {
    DBG_WARN(0, "No callbacks registered, should not happen here");
  }
  else {
    GWEN_WaitCallback_internal(0, 0, GWEN_WaitCallbackMode_Leave);
    GWEN_List_PopBack(gwen_callbackstack);
    if (ctx->usage<2) {
      DBG_WARN(0, "Bad usage in WaitCallback context (%d)", ctx->usage);
    }
    else
      ctx->usage--;
  }

  if (GWEN_List_GetSize(gwen_callbackstack)==0) {
    GWEN_List_free(gwen_callbackstack);
    gwen_callbackstack=0;
  }
}



time_t GWEN_WaitCallback_LastCalled(GWEN_WAITCALLBACK_CTX *ctx){
  assert(ctx);
  return ctx->lastCalled;
}



int GWEN_WaitCallback_GetDistance(GWEN_WAITCALLBACK_CTX *ctx){
  if (ctx==0) {
    if (!gwen_callbackstack) {
      DBG_VERBOUS(0, "No callbacks registered");
      return 0;
    }
    ctx=GWEN_List_GetBack(gwen_callbackstack);
    if (!ctx) {
      DBG_WARN(0, "No callbacks registered, should not happen here");
      return 0;
    }
  }
  return ctx->proposedDistance;
}



void GWEN_WaitCallback_SetDistance(GWEN_WAITCALLBACK_CTX *ctx,
                                   int d){
  assert(ctx);
  ctx->proposedDistance=d;
}



time_t GWEN_WaitCallback_LastEntered(GWEN_WAITCALLBACK_CTX *ctx){
  assert(ctx);
  return ctx->lastEntered;
}






