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

#include "hbcidialog_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/crypt.h>



void GWEN_HBCIDialog_SetFreeDataFn(GWEN_HBCIDIALOG *hdlg,
                                   GWEN_HBCIDLG_FREEDATA_FN fn){
  assert(hdlg);
  hdlg->freeDataFn=fn;
}



void GWEN_HBCIDialog_SetResetFn(GWEN_HBCIDIALOG *hdlg,
                                GWEN_HBCIDLG_RESET_FN fn){
  assert(hdlg);
  hdlg->resetFn=fn;
}



void GWEN_HBCIDialog_SetInheritorData(GWEN_HBCIDIALOG *hdlg,
                                      void *data){
  assert(hdlg);
  if (hdlg->inheritorData && hdlg->freeDataFn)
    hdlg->freeDataFn(hdlg);
  hdlg->inheritorData=data;
}



void *GWEN_HBCIDialog_GetInheritorData(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->inheritorData;
}



GWEN_MSGENGINE *GWEN_HBCIDialog_GetMsgEngine(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->msgEngine;
}



const char *GWEN_HBCIDialog_GetDialogId(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  if (hdlg->dialogId)
    return hdlg->dialogId;
  return "0";
}



void GWEN_HBCIDialog_SetDialogId(GWEN_HBCIDIALOG *hdlg,
                                 const char *s){
  assert(hdlg);
  free(hdlg->dialogId);
  hdlg->dialogId=strdup(s);
}



unsigned int GWEN_HBCIDialog_GetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->lastReceivedMsgNum;
}



void GWEN_HBCIDialog_SetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg,
                                           unsigned int i){
  assert(hdlg);
  hdlg->lastReceivedMsgNum=i;
}



unsigned int GWEN_HBCIDialog_GetNextMsgNum(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->nextMsgNum++;
}



void GWEN_HBCIDialog_SetNextMsgNum(GWEN_HBCIDIALOG *hdlg,
                                   unsigned int i){
  assert(hdlg);
  hdlg->nextMsgNum=i;
}




GWEN_HBCIDIALOG *GWEN_HBCIDialog_new(GWEN_MSGENGINE *e,
                                     GWEN_SECCTX_MANAGER *scm){
  GWEN_HBCIDIALOG *hdlg;

  assert(e);
  assert(scm);
  GWEN_NEW_OBJECT(GWEN_HBCIDIALOG, hdlg);
  hdlg->usage=1;
  hdlg->msgEngine=e;
  hdlg->securityManager=scm;

  GWEN_HBCIDialog_Reset(hdlg);
  return hdlg;
}



void GWEN_HBCIDialog_free(GWEN_HBCIDIALOG *hdlg){
  if (hdlg) {
    if (hdlg->inheritorData && hdlg->freeDataFn)
      hdlg->freeDataFn(hdlg);
    free(hdlg->dialogId);
    free(hdlg->localName);
    free(hdlg->remoteName);
    free(hdlg);
  }
}



unsigned int GWEN_HBCIDialog_GetFlags(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->flags;
}



void GWEN_HBCIDialog_SetFlags(GWEN_HBCIDIALOG *hdlg,
                              unsigned int f){
  assert(hdlg);
  hdlg->flags=f;
}



void GWEN_HBCIDialog_Attach(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  hdlg->usage++;
}



void GWEN_HBCIDialog_Detach(GWEN_HBCIDIALOG *hdlg){
  if (hdlg) {
    assert(hdlg->usage);
    hdlg->usage--;
    if (!hdlg->usage)
      GWEN_HBCIDialog_free(hdlg);
  }
}



const char *GWEN_HBCIDialog_GetLocalName(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->localName;
}



void GWEN_HBCIDialog_SetLocalName(GWEN_HBCIDIALOG *hdlg,
                                  const char *s){
  assert(hdlg);
  assert(s);
  free(hdlg->localName);
  hdlg->localName=strdup(s);
}



const char *GWEN_HBCIDialog_GetRemoteName(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->remoteName;
}



void GWEN_HBCIDialog_SetRemoteName(GWEN_HBCIDIALOG *hdlg,
                                   const char *s){
  assert(hdlg);
  assert(s);
  free(hdlg->remoteName);
  hdlg->remoteName=strdup(s);
}



void GWEN_HBCIDialog_Reset(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  if (hdlg->resetFn)
    hdlg->resetFn(hdlg);
  free(hdlg->dialogId);
  hdlg->dialogId=strdup("0");
  hdlg->nextMsgNum=1;
  hdlg->lastReceivedMsgNum=0;
  if (!(hdlg->flags & GWEN_HBCIDIALOG_FLAGS_INITIATOR)) {
    /* only reset remote name if this is a passive connection */
    free(hdlg->remoteName);
    hdlg->remoteName=0;
  }
}



GWEN_SECCTX_MANAGER*
GWEN_HBCIDialog_GetSecurityManager(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->securityManager;
}




