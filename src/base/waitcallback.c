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

#define GWEN_EXTEND_WAITCALLBACK
/* #define DISABLE_DEBUGLOG */

#include "waitcallback_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/list.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/path.h>


GWEN_LIST_FUNCTIONS(GWEN_WAITCALLBACK, GWEN_WaitCallback)
GWEN_INHERIT_FUNCTIONS(GWEN_WAITCALLBACK)


static GWEN_WAITCALLBACK *gwen_waitcallback__root=0;
static GWEN_WAITCALLBACK *gwen_waitcallback__current=0;
static GWEN_WAITCALLBACK_LIST *gwen_waitcallback__list=0;


/* -------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_WaitCallback_ModuleInit(){
  gwen_waitcallback__root=GWEN_WaitCallback_new("");
  gwen_waitcallback__list=GWEN_WaitCallback_List_new();
  gwen_waitcallback__current=gwen_waitcallback__root;
  GWEN_WaitCallback_List_Add(gwen_waitcallback__current,
                             gwen_waitcallback__list);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_WaitCallback_ModuleFini(){
  GWEN_WaitCallback_List_free(gwen_waitcallback__list);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_WAITCALLBACK *GWEN_WaitCallback_new(const char *id) {
  GWEN_WAITCALLBACK *ctx;

  assert(id);
  GWEN_NEW_OBJECT(GWEN_WAITCALLBACK, ctx);
  GWEN_LIST_INIT(GWEN_WAITCALLBACK, ctx);
  GWEN_INHERIT_INIT(GWEN_WAITCALLBACK, ctx);
  ctx->registeredCallbacks=GWEN_WaitCallback_List_new();
  ctx->id=strdup(id);
  ctx->usage=1;
  return ctx;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_free(GWEN_WAITCALLBACK *ctx){
  if (ctx) {
    assert(ctx->usage);
    if (--(ctx->usage)==0) {
      GWEN_INHERIT_FINI(GWEN_WAITCALLBACK, ctx);

      GWEN_WaitCallback_List_free(ctx->registeredCallbacks);
      GWEN_WaitCallback_free(ctx->instantiatedFrom);
      free(ctx->id);

      GWEN_LIST_FINI(GWEN_WAITCALLBACK, ctx);
      GWEN_FREE_OBJECT(ctx);
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_Attach(GWEN_WAITCALLBACK *ctx){
  assert(ctx);
  ctx->usage++;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_SetProgressPos(GWEN_TYPE_UINT64 pos){
  GWEN_WAITCALLBACK *ctx;

  ctx=gwen_waitcallback__current;
  if (!ctx) {
    DBG_INFO(0, "No callback active");
  }
  else {
    ctx->pos=pos;
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_SetProgressTotal(GWEN_TYPE_UINT64 total){
  GWEN_WAITCALLBACK *ctx;

  ctx=gwen_waitcallback__current;
  if (!ctx) {
    DBG_INFO(0, "No callback active");
  }
  else {
    ctx->total=total;
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT64
GWEN_WaitCallback_GetProgressPos(GWEN_WAITCALLBACK *ctx){
  assert(ctx);
  return ctx->pos;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT64
GWEN_WaitCallback_GetProgressTotal(GWEN_WAITCALLBACK *ctx){
  assert(ctx);
  return ctx->total;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_SetCheckAbortFn(GWEN_WAITCALLBACK *ctx,
                                       GWEN_WAITCALLBACK_CHECKABORTFN fn){
  assert(ctx);
  ctx->checkAbortFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_SetInstantiateFn(GWEN_WAITCALLBACK *ctx,
                                        GWEN_WAITCALLBACK_INSTANTIATEFN fn){
  assert(ctx);
  ctx->instantiateFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_SetLogFn(GWEN_WAITCALLBACK *ctx,
                                GWEN_WAITCALLBACK_LOGFN fn){
  assert(ctx);
  ctx->logFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
void *GWEN_WaitCallback__HandlePathElement(const char *entry,
                                           void *data,
                                           unsigned int flags){
  GWEN_WAITCALLBACK *ctx;
  GWEN_WAITCALLBACK *tctx;

  assert(data);
  ctx=(GWEN_WAITCALLBACK*)data;

  if (flags & GWEN_PATH_FLAGS_ROOT) {
    ctx=gwen_waitcallback__root;
    entry++; /* skip leading slash which appears if ROOT flag is set */
    if (!(*entry))
      /* the root entry itself is wanted, return it */
      return ctx;
  }

  tctx=GWEN_WaitCallback_List_First(ctx->registeredCallbacks);
  while(tctx) {
    if (tctx->id)
      if (-1!=GWEN_Text_ComparePattern(entry, tctx->id, 0))
        return (void*)tctx;
    tctx=GWEN_WaitCallback_List_Next(tctx);
  } /* while */

  return 0;
}




/* -------------------------------------------------------------- FUNCTION */
GWEN_WAITCALLBACK *GWEN_WaitCallback__FindCallback(const char *s) {
  GWEN_WAITCALLBACK *ctx;
  void *p;

  /* always start at root */
  assert(gwen_waitcallback__root);
  ctx=gwen_waitcallback__root;
  if (ctx->instantiatedFrom)
    ctx=ctx->instantiatedFrom;

  p=GWEN_Path_Handle(s,
		     (void*)ctx,
		     GWEN_PATH_FLAGS_CHECKROOT,
		     GWEN_WaitCallback__HandlePathElement);
  if (!p) {
    DBG_DEBUG(0, "Callback \"%s\" not found", s);
    return 0;
  }

  return (GWEN_WAITCALLBACK*)p;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_WaitCallback_Register(const char *id,
                               GWEN_WAITCALLBACK *ctx){
  GWEN_WAITCALLBACK *tctx;

  assert(id);
  assert(ctx);

  tctx=GWEN_WaitCallback__FindCallback(id);
  if (!tctx) {
    DBG_ERROR(0, "Callback \"%s\" not found", id);
    return -1;
  }
  GWEN_WaitCallback_List_Insert(ctx, tctx->registeredCallbacks);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_WAITCALLBACK_RESULT GWEN_WaitCallback(){
  GWEN_WAITCALLBACK *ctx;
  GWEN_WAITCALLBACK_RESULT rv;

  ctx=gwen_waitcallback__current;
  if (!ctx){
    DBG_INFO(0, "No callback currently selected");
    rv=GWEN_WaitCallbackResult_Continue;
  }
  else {
    if (ctx->originalCtx) {
      if (!ctx->originalCtx->checkAbortFn) {
        DBG_DEBUG(0, "No checkAbort function set");
        rv=GWEN_WaitCallbackResult_Continue;
      }
      else {
        rv=ctx->originalCtx->checkAbortFn(ctx->originalCtx,
                                          ctx->level);
        ctx->originalCtx->lastCalled=time(0);
      }
    } /* if there is an original context */
    else {
      if (!ctx->checkAbortFn) {
        DBG_DEBUG(0, "No checkAbort function set");
        rv=GWEN_WaitCallbackResult_Continue;
      }
      else {
        rv=ctx->checkAbortFn(ctx, 0);
        ctx->lastCalled=time(0);
      }
    }
  }
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_WAITCALLBACK *GWEN_WaitCallback_Instantiate(GWEN_WAITCALLBACK *ctx) {
  GWEN_WAITCALLBACK *nctx;

  if (ctx->instantiateFn) {
    nctx=ctx->instantiateFn(ctx);
  }
  else {
    nctx=GWEN_WaitCallback_new(ctx->id);
  }
  nctx->instantiatedFrom=ctx;
  GWEN_WaitCallback_Attach(nctx->instantiatedFrom);
  return nctx;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_WAITCALLBACK_RESULT GWEN_WaitCallbackProgress(GWEN_TYPE_UINT64 pos){
  GWEN_WAITCALLBACK *ctx;

  ctx=gwen_waitcallback__current;
  if (!ctx){
    DBG_INFO(0, "No callback currently selected");
    return GWEN_WaitCallbackResult_Continue;
  }
  else {
    ctx->pos=pos;
    return GWEN_WaitCallback();
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_Enter(const char *id){
  GWEN_WAITCALLBACK *ctx;
  GWEN_WAITCALLBACK *nctx;

  ctx=GWEN_WaitCallback__FindCallback(id);
  if (!ctx) {
    if (gwen_waitcallback__current) {
      DBG_INFO(0, "Callback \"%s\" not found, faking it", id);

      nctx=GWEN_WaitCallback_new(id);
      nctx->previousCtx=gwen_waitcallback__current;
      if (gwen_waitcallback__current->originalCtx) {
        nctx->originalCtx=gwen_waitcallback__current->originalCtx;
        nctx->level=gwen_waitcallback__current->level+1;
      }
      else {
        nctx->originalCtx=gwen_waitcallback__current;
        nctx->level=1;
      }
      gwen_waitcallback__current=nctx;
      nctx->lastEntered=time(0);
      GWEN_WaitCallback_List_Add(nctx, gwen_waitcallback__list);
    }
    else {
      DBG_INFO(0, "Callback \"%s\" not found and none is currently selected",
               id);
    }
  } /* if ctx not found */
  else {
    /* ctx found, select it */
    DBG_DEBUG(0, "Callback \"%s\" found", id);
    nctx=GWEN_WaitCallback_Instantiate(ctx);
    assert(nctx);
    nctx->previousCtx=gwen_waitcallback__current;
    gwen_waitcallback__current=nctx;
    nctx->lastEntered=time(0);
    GWEN_WaitCallback_List_Add(nctx, gwen_waitcallback__list);
    DBG_DEBUG(0, "Active callbacks: %d",
              GWEN_WaitCallback_List_GetCount(gwen_waitcallback__list));
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_Leave(){
  GWEN_WAITCALLBACK *ctx;

  assert(gwen_waitcallback__current);
  DBG_DEBUG(0, "Leaving callback context \"%s\"",
            gwen_waitcallback__current->id);
  ctx=gwen_waitcallback__current->previousCtx;
  GWEN_WaitCallback_free(gwen_waitcallback__current);
  gwen_waitcallback__current=ctx;
  if (ctx) {
    DBG_VERBOUS(0, "Returned to callback \"%s\"", ctx->id);
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_Log(unsigned int loglevel, const char *s){
  GWEN_WAITCALLBACK *ctx;

  DBG_DEBUG(0, "Callback Log: \"%s\"", s);
  ctx=gwen_waitcallback__current;
  if (!ctx){
    DBG_INFO(0, "No callback currently selected");
  }
  else {
    if (ctx->originalCtx) {
      if (!ctx->originalCtx->logFn) {
        DBG_INFO(0, "No log function set in \"%s\"", ctx->originalCtx->id);
      }
      else {
        ctx->originalCtx->logFn(ctx->originalCtx,
                                ctx->level,
                                loglevel,
                                s);
      }
    } /* if there is an original context */
    else {
      if (!ctx->logFn) {
        DBG_INFO(0, "No log function set in \"%s\"", ctx->id);
      }
      else {
        ctx->logFn(ctx, 0, loglevel, s);
      }
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
time_t GWEN_WaitCallback_LastCalled(GWEN_WAITCALLBACK *ctx){
  assert(ctx);
  return ctx->lastCalled;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_WaitCallback_GetDistance(GWEN_WAITCALLBACK *ctx){
  if (ctx==0) {
    ctx=gwen_waitcallback__current;
    if (!ctx) {
      DBG_DEBUG(0, "No callback currently selected");
      return 0;
    }
  }
  return ctx->proposedDistance;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_WaitCallback_SetDistance(GWEN_WAITCALLBACK *ctx,
                                   int d){
  assert(ctx);
  ctx->proposedDistance=d;
}



/* -------------------------------------------------------------- FUNCTION */
time_t GWEN_WaitCallback_LastEntered(GWEN_WAITCALLBACK *ctx){
  assert(ctx);
  return ctx->lastEntered;
}



/* -------------------------------------------------------------- FUNCTION */
const char *GWEN_WaitCallback_GetId(GWEN_WAITCALLBACK *ctx){
  assert(ctx);
  return ctx->id;
}






