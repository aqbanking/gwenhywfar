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
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>



void GWEN_HBCIDialog_SetPrepareCtxFn(GWEN_HBCIDIALOG *hdlg,
                                     GWEN_HBCIDLG_PREPARECTX_FN fn){
  assert(hdlg);
  hdlg->prepareCtxFn=fn;
}



void GWEN_HBCIDialog_SetSignFn(GWEN_HBCIDIALOG *hdlg,
                               GWEN_HBCIDLG_SIGN_FN fn){
  assert(hdlg);
  hdlg->signFn=fn;
}



void GWEN_HBCIDialog_SetVerifyFn(GWEN_HBCIDIALOG *hdlg,
                                 GWEN_HBCIDLG_VERIFY_FN fn){
  assert(hdlg);
  hdlg->verifyFn=fn;
}



void GWEN_HBCIDialog_SetEncryptFn(GWEN_HBCIDIALOG *hdlg,
                                  GWEN_HBCIDLG_ENCRYPT_FN fn){
  assert(hdlg);
  hdlg->encryptFn=fn;
}



void GWEN_HBCIDialog_SetDecrpytFn(GWEN_HBCIDIALOG *hdlg,
                                  GWEN_HBCIDLG_DECRYPT_FN fn){
  assert(hdlg);
  hdlg->decryptFn=fn;
}



void GWEN_HBCIDialog_SetFreeDataFn(GWEN_HBCIDIALOG *hdlg,
                                   GWEN_HBCIDLG_FREEDATA_FN fn){
  assert(hdlg);
  hdlg->freeDataFn=fn;
}



void GWEN_HBCIDialog_SetInheritorData(GWEN_HBCIDIALOG *hdlg,
                                      void *data){
  assert(hdlg);
  if (hdlg->inheritorData && hdlg->freeDataFn)
    hdlg->freeDataFn(hdlg);
  hdlg->inheritorData=data;
}



GWEN_MSGENGINE *GWEN_HBCIDialog_GetMsgEngine(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->msgEngine;
}



void GWEN_HBCIDialog_SetMsgEngine(GWEN_HBCIDIALOG *hdlg,
                                  GWEN_MSGENGINE *e){
  assert(hdlg);
  hdlg->msgEngine=e;
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
  return hdlg->nextMsgNum;
}



void GWEN_HBCIDialog_SetNextMsgNum(GWEN_HBCIDIALOG *hdlg,
                                   unsigned int i){
  assert(hdlg);
  hdlg->nextMsgNum=i;
}






int GWEN_HBCIDialog_PrepareContext(GWEN_HBCIDIALOG *hdlg,
                                   GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(hdlg);
  assert(hdlg->prepareCtxFn);
  return hdlg->prepareCtxFn(hdlg, ctx);
}



int GWEN_HBCIDialog_Sign(GWEN_HBCIDIALOG *hdlg,
                         GWEN_BUFFER *msgbuf,
                         GWEN_BUFFER *signbuf,
                         GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(hdlg);
  assert(hdlg->signFn);
  return hdlg->signFn(hdlg, msgbuf, signbuf, ctx);
}



int GWEN_HBCIDialog_Verify(GWEN_HBCIDIALOG *hdlg,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(hdlg);
  assert(hdlg->verifyFn);
  return hdlg->verifyFn(hdlg, msgbuf, signbuf, ctx);
}



int GWEN_HBCIDialog_Encrypt(GWEN_HBCIDIALOG *hdlg,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *cryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(hdlg);
  assert(hdlg->encryptFn);
  return hdlg->encryptFn(hdlg, msgbuf, cryptbuf, ctx);
}



int GWEN_HBCIDialog_Decrypt(GWEN_HBCIDIALOG *hdlg,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *decryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx){
  assert(hdlg);
  assert(hdlg->decryptFn);
  return hdlg->decryptFn(hdlg, msgbuf, decryptbuf, ctx);
}



GWEN_HBCIDIALOG *GWEN_HBCIDialog_new(GWEN_MSGENGINE *e){
  GWEN_HBCIDIALOG *hdlg;

  GWEN_NEW_OBJECT(GWEN_HBCIDIALOG, hdlg);
  hdlg->msgEngine=e;
  hdlg->dialogId=strdup("0");
  hdlg->nextMsgNum=1;
  return hdlg;
}



void GWEN_HBCIDialog_free(GWEN_HBCIDIALOG *hdlg){
  if (hdlg) {
    if (hdlg->inheritorData && hdlg->freeDataFn)
      hdlg->freeDataFn(hdlg);
    free(hdlg->dialogId);
    free(hdlg);
  }
}












