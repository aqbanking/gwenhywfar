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


GWEN_WAITCALLBACK_RESULT GWEN_WaitCallback_internal(int count){
  GWEN_WAITCALLBACK_CTX *ctx;

  if (!gwen_callbackstack) {
    DBG_DEBUG(0, "No callbacks registered (1)");
    return GWEN_WaitCallbackResult_Continue;
  }
  ctx=GWEN_List_GetBack(gwen_callbackstack);
  if (!ctx) {
    DBG_WARN(0, "No callbacks registered, should not happen here");
    return GWEN_WaitCallbackResult_Continue;
  }
  if (!ctx->waitCallbackFn) {
    DBG_WARN(0, "No callbacks set");
    return GWEN_WaitCallbackResult_Continue;
  }
  return ctx->waitCallbackFn((int)count, ctx);
}


GWEN_WAITCALLBACK_RESULT GWEN_WaitCallback(unsigned int count){
  return GWEN_WaitCallback_internal((int)count);
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
  GWEN_WaitCallback_internal(GWEN_WAITCALLBACK_ENTER);
}



void GWEN_WaitCallback_Leave(){
  GWEN_WAITCALLBACK_CTX *ctx;

  assert(gwen_callbackstack);
  ctx=GWEN_List_GetBack(gwen_callbackstack);
  if (!ctx) {
    DBG_WARN(0, "No callbacks registered, should not happen here");
  }
  else {
    GWEN_WaitCallback_internal(GWEN_WAITCALLBACK_LEAVE);
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







