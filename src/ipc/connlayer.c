/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Sep 16 2003
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

#include "connlayer_p.h"
#include "msglayer_p.h"
#include <gwenhyfwar/msglayer.h>
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_ConnectionLayer_new(GWEN_IPCMSGLAYER *ml) {
  GWEN_IPCCONNLAYER *cl;

  assert(ml);
  GWEN_NEW_OBJECT(GWEN_IPCCONNLAYER, cl);
  cl->msgLayer=ml;
  return cl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_free(GWEN_IPCCONNLAYER *cl){
  if (cl) {
    /* free the rest */
    if (cl->freeDataFn)
      cl->freeDataFn(cl);
    GWEN_MsgLayer_free(cl->msgLayer);
    cl->msgLayer=0;
    free(cl->info);
    free(cl);
  }
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_ConnectionLayer_GetInfo(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->info;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetInfo(GWEN_IPCCONNLAYER *cl,
                                  const char *s){
  assert(cl);
  assert(s);
  free(cl->info);
  cl->info=strdup(s);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET *GWEN_ConnectionLayer_GetReadSocket(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  assert(cl->msgLayer);
  return GWEN_IPCMsgLayer_GetReadSocket(cl->msgLayer);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET *GWEN_ConnectionLayer_GetWriteSocket(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  assert(cl->msgLayer);
  return GWEN_IPCMsgLayer_GetWriteSocket(cl->msgLayer);
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetFreeDataFn(GWEN_IPCCONNLAYER *cl,
                                        GWEN_IPCCONNLAYER_FREE f){
  assert(cl);
  cl->freeDataFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetWorkFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_WORK f){
  assert(cl);
  cl->workFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetAcceptFn(GWEN_IPCCONNLAYER *cl,
                                      GWEN_IPCCONNLAYER_ACCEPT f){
  assert(cl);
  cl->acceptFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetOpenFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_OPEN f){
  assert(cl);
  cl->openFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetCloseFn(GWEN_IPCCONNLAYER *cl,
                                     GWEN_IPCCONNLAYER_CLOSE f){
  assert(cl);
  cl->closeFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetDownFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_DOWN f){
  assert(cl);
  cl->downFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Accept(GWEN_IPCCONNLAYER *cl,
                                           GWEN_IPCCONNLAYER **c){
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCMSGLAYER *newml;
  GWEN_IPCCONNLAYER *newcl;
  GWEN_ERRORCODE err;

  assert(cl);

  /* let the message layer accept the new connection */
  assert(cl->msgLayer);
  ml=cl->msgLayer;
  err=GWEN_MsgLayer_Accept(ml, &newml);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
    return err;
  }

  /* let the connection layer accept the new connection */
  assert(cl->acceptFn);
  DBG_INFO(0, "Calling connectionLayer->acceptFn");
  err=cl->acceptFn(cl, newml, &newcl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
    return err;
  }
  newcl->libMark=cl->libMark;
  newcl->userMark=cl->userMark;
  newcl->flags=cl->flags | GWEN_IPCCONNLAYER_FLAGS_PASSIVE;
  *c=newcl;

  DBG_INFO(0, "Connection accepted as %d",
           GWEN_ConnectionLayer_GetId(newcl));

  return err;
}



/* --------------------------------------------------------------- FUNCTION */
void *GWEN_ConnectionLayer_GetData(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->data;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetData(GWEN_IPCCONNLAYER *cl, void *d){
  assert(cl);
  if (cl->freeDataFn)
    cl->freeDataFn(cl);
  cl->data=d;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Work(GWEN_IPCCONNLAYER *cl,
                                         int rd){
  assert(cl);
  if (cl->workFn)
    return cl->workFn(cl, rd);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYER *GWEN_ConnectionLayer_GetMsgLayer(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->msgLayer;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Open(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  if (cl->openFn)
    return cl->openFn(cl);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Close(GWEN_IPCCONNLAYER *cl,
                                          int force){
  assert(cl);
  if (cl->closeFn)
    return cl->closeFn(cl, force);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_Down(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  if (cl->downFn)
    cl->downFn(cl);
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetId(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  assert(cl->msgLayer);
  return GWEN_MsgLayer_GetId(cl->msgLayer);
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetUserMark(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->userMark;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetUserMark(GWEN_IPCCONNLAYER *cl,
                                      unsigned int i){
  assert(cl);
  cl->userMark=i;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetLibMark(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->libMark;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetLibMark(GWEN_IPCCONNLAYER *cl,
                                     unsigned int i){
  assert(cl);
  cl->libMark=i;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetFlags(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->flags;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetFlags(GWEN_IPCCONNLAYER *cl,
                                   unsigned int flags){
  assert(cl);
  cl->flags=flags;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_ConnectionLayer_GetNext(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->next;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_Append(GWEN_IPCCONNLAYER *cl,
                                 GWEN_IPCCONNLAYER *newcl){
  assert(cl);
  assert(newcl);
  GWEN_LIST_ADD(GWEN_IPCCONNLAYER, newcl, &cl);
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetType(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->typ;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetType(GWEN_IPCCONNLAYER *cl,
                                  unsigned int i){
  assert(cl);
  cl->typ=i;
}




