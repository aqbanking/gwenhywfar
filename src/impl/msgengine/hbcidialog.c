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
#include <gwenhyfwar/text.h>
#include <gwenhyfwar/crypt.h>



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
  return hdlg->nextMsgNum++;
}



void GWEN_HBCIDialog_SetNextMsgNum(GWEN_HBCIDIALOG *hdlg,
                                   unsigned int i){
  assert(hdlg);
  hdlg->nextMsgNum=i;
}






int GWEN_HBCIDialog_PrepareContext(GWEN_HBCIDIALOG *hdlg,
                                   GWEN_HBCICRYPTOCONTEXT *ctx,
                                   int crypt){
  assert(hdlg);
  assert(hdlg->prepareCtxFn);
  return hdlg->prepareCtxFn(hdlg, ctx, crypt);
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
  hdlg->usage=1;
  hdlg->msgEngine=e;

  GWEN_HBCIDialog_Reset(hdlg);
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




/*
 * This code has been taken from OpenHBCI (rsakey.cpp, written by Fabian
 * Kaiser)
 */
unsigned char GWEN_HBCIDialog_permutate(unsigned char input) {
  unsigned char leftNibble;
  unsigned char rightNibble;
  static const unsigned char lookUp[2][16] =
    {{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
    {14,3,5,8,9,4,2,15,0,13,11,6,7,10,12,1}};

  rightNibble = input & 15;
  leftNibble = input & 240;
  leftNibble = leftNibble / 16;
  rightNibble = lookUp[1][rightNibble];
  leftNibble = lookUp[1][leftNibble];
  leftNibble = leftNibble * 16;

  return leftNibble + rightNibble;
}



/*
 * The original code (in C++) has been written by Fabian Kaiser for OpenHBCI
 * (file rsakey.cpp). Moved to C by me (Martin Preuss)
 */
int GWEN_HBCIDialog_PaddWithISO9796(GWEN_BUFFER *src) {
  char *p;
  unsigned int l;
  unsigned int i;
  unsigned char buffer[GWEN_HBCIDIALOG_KEYSIZE];
  unsigned char hash[20];
  unsigned char c;

  p=GWEN_Buffer_GetStart(src);
  l=GWEN_Buffer_GetUsedBytes(src);
  memmove(hash, p, l);

  /* src+src+src */
  if (GWEN_Buffer_AppendBytes(src, hash, l)) {
    DBG_INFO(0, "here");
    return -1;
  }

  if (GWEN_Buffer_AppendBytes(src, hash, l)) {
    DBG_INFO(0, "here");
    return -1;
  }

  /* src=src(20,40) */
  if (GWEN_Buffer_Crop(src, 20, 40)) {
    DBG_INFO(0, "here");
    return -1;
  }

  memset(buffer, 0, sizeof(buffer));

  /* append redundancy */
  p=GWEN_Buffer_GetStart(src);
  for (i=0; i<=47; i++) {
    int j1, j2, j3;

    j1=1 + sizeof(buffer) - (2*i);
    j2=40-i;
    j3=sizeof(buffer) - (2*i);

    if (j1>=0 && j1<sizeof(buffer) && j2>=0) {
      buffer[j1]=p[j2];
    }
    if (j3>=0 && j3<sizeof(buffer) && j2>=0) {
      buffer[j3]=GWEN_HBCIDialog_permutate(p[j2]);
    }
  } /* for */

  /* copy last 16 bytes to the beginning */
  memmove(buffer, buffer+(sizeof(buffer)-16), 16);

  p=buffer;
  /* finish */
  c=p[sizeof(buffer)-1];
  c = (c & 15) * 16;
  c += 6;
  p[sizeof(buffer)-1]=c;
  p[0] = p[0] & 127;
  p[0] = p[0] | 64;
  p[sizeof(buffer) - 40] = p[sizeof(buffer) - 40] ^ 1;

  GWEN_Buffer_Reset(src);
  if (GWEN_Buffer_AppendBytes(src, buffer, sizeof(buffer))) {
    DBG_INFO(0, "here");
    return -1;
  }

  return 0;
}




int GWEN_HBCIDialog_PaddWithANSIX9_23(GWEN_BUFFER *src) {
  unsigned char paddLength;
  unsigned int i;

  paddLength=8-(GWEN_Buffer_GetUsedBytes(src) % 8);
  for (i=0; i<paddLength; i++)
    GWEN_Buffer_AppendByte(src, paddLength);
  return 0;
}



int GWEN_HBCIDialog_UnpaddWithANSIX9_23(GWEN_BUFFER *src) {
  const char *p;
  unsigned int lastpos;
  unsigned char paddLength;

  lastpos=GWEN_Buffer_GetUsedBytes(src);
  if (lastpos<8) {
    DBG_ERROR(0, "Buffer too small");
    return -1;
  }
  lastpos--;

  p=GWEN_Buffer_GetStart(src)+lastpos;
  paddLength=*p;
  if (paddLength<1 || paddLength>8) {
    DBG_ERROR(0, "Invalid padding (%d bytes ?)", paddLength);
    return -1;
  }
  GWEN_Buffer_SetUsedBytes(src, GWEN_Buffer_GetUsedBytes(src)-paddLength);
  GWEN_Buffer_SetPos(src, lastpos-paddLength);
  return 0;
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



const char *GWEN_HBCIDialog_GetOwner(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  return hdlg->owner;
}



void GWEN_HBCIDialog_SetOwner(GWEN_HBCIDIALOG *hdlg,
                              const char *s){
  assert(hdlg);
  assert(s);
  free(hdlg->owner);
  hdlg->owner=strdup(s);
}


void GWEN_HBCIDialog_Reset(GWEN_HBCIDIALOG *hdlg){
  assert(hdlg);
  if (hdlg->resetFn)
    hdlg->resetFn(hdlg);
  free(hdlg->owner);
  hdlg->owner=0;
  free(hdlg->dialogId);
  hdlg->dialogId=strdup("0");
  hdlg->nextMsgNum=1;
}




