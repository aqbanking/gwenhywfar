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

#include "hbcimsg_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/text.h>

#include <time.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_KEYSPEC *GWEN_HBCIMsg_GetSigners(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->signers;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_AddSigner(GWEN_HBCIMSG *hmsg,
                           GWEN_KEYSPEC *ks){
  assert(hmsg);

  if (hmsg->nodes) {
    DBG_ERROR(0, "Signers must be added before nodes !");
    return -1;
  }
  GWEN_KeySpec_Add(ks, &(hmsg->signers));
  hmsg->nSigners++;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetSignerCount(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->nSigners;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_KEYSPEC *GWEN_HBCIMsg_GetCrypter(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->crypter;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_SetCrypter(GWEN_HBCIMSG *hmsg,
                             GWEN_KEYSPEC *ks){
  assert(hmsg);
  hmsg->crypter=ks;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *GWEN_HBCIMsg_GetBuffer(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->buffer;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_SetMsgRef(GWEN_HBCIMSG *hmsg,
                            unsigned int i){
  assert(hmsg);
  hmsg->refMsgNum=i;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_SetMsgNumber(GWEN_HBCIMSG *hmsg,
                               unsigned int i){
  assert(hmsg);
  hmsg->msgNum=i;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_HBCIMSG *GWEN_HBCIMsg_new(GWEN_HBCIDIALOG *hdlg){
  GWEN_HBCIMSG *hmsg;

  GWEN_NEW_OBJECT(GWEN_HBCIMSG, hmsg);
  hmsg->dialog=hdlg;
  hmsg->buffer=GWEN_Buffer_new(0, GWEN_HBCIMSG_DEFAULTSIZE, 0, 1);
  GWEN_Buffer_SetStep(hmsg->buffer, 512);
  hmsg->msgNum=GWEN_HBCIDialog_GetNextMsgNum(hdlg);
  return hmsg;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_free(GWEN_HBCIMSG *hmsg){
  if (hmsg) {
    GWEN_KeySpec_Clear(&(hmsg->signers));
    GWEN_KeySpec_free(hmsg->crypter);
    GWEN_Buffer_free(hmsg->buffer);
    GWEN_Buffer_free(hmsg->origbuffer);
    free(hmsg);
  }
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_AddMsgTail(GWEN_HBCIMSG *hmsg){
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  int rv;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "MsgTail");
  if (!node) {
    DBG_INFO(0, "Segment \"MsgTail\"not found");
    return -1;
  }

  cfg=GWEN_DB_Group_new("msgtail");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq",
                      hmsg->lastSegment+1);
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hmsg->buffer,
                                          cfg);
  GWEN_DB_Group_free(cfg);
  if (rv) {
    DBG_INFO(0, "Could not create msgTail");
    return -1;
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_AddMsgHead(GWEN_HBCIMSG *hmsg) {
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  unsigned int msize;
  int rv;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "MsgHead");
  if (!node) {
    DBG_INFO(0, "Segment \"MsgHead\"not found");
    return -1;
  }

  cfg=GWEN_DB_Group_new("msghead");
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "dialogid",
                       GWEN_HBCIDialog_GetDialogId(hmsg->dialog));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "msgnum", hmsg->msgNum);
  if (hmsg->refMsgNum) {
    /* add message reference */
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "msgref/msgnum", hmsg->refMsgNum);
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "msgref/dialogid",
                         GWEN_HBCIDialog_GetDialogId(hmsg->dialog));
  }
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "hversion",
                      GWEN_MsgEngine_GetProtocolVersion(e));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "size", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 1);
  hbuf=GWEN_Buffer_new(0, 128, 0, 1);

  /* create first version of msgHead just to calculate the size */
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(0, "Could not create msgHead");
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  msize=GWEN_Buffer_GetUsedBytes(hmsg->buffer)+
    GWEN_Buffer_GetUsedBytes(hbuf);
  DBG_INFO(0, "Message size is: %d", msize);
  GWEN_DB_SetIntValue(cfg,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "size", msize);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 1);
  GWEN_Buffer_Reset(hbuf);

  /* create final version of msgHead (we now know the size) */
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  GWEN_DB_Group_free(cfg);
  if (rv) {
    DBG_INFO(0, "Could not create 2nd version of msgHead");
    return -1;
  }

  /* insert msgHead */
  if (GWEN_Buffer_InsertBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(0, "Could not insert msgHead");
    return -1;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetFlags(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->flags;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_SetFlags(GWEN_HBCIMSG *hmsg,
                           unsigned int f){
  assert(hmsg);
  hmsg->flags=f;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetCurrentSegmentNumber(GWEN_HBCIMSG *hmsg) {
  if (hmsg->firstSegment==0) {
    unsigned int rv;

    rv=2;
    if (hmsg->flags & GWEN_HBCIMSG_FLAGS_SIGN)
      rv+=hmsg->nSigners;
    return rv;
  }
  return hmsg->lastSegment+1;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_AddNode(GWEN_HBCIMSG *hmsg,
                         GWEN_XMLNODE *node,
                         GWEN_DB_NODE *data) {
  int rv;

  assert(hmsg);
  assert(node);
  assert(data);
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  if (hmsg->firstSegment==0) {
    /* first node */
    hmsg->firstSegment=2;
    if (hmsg->flags & GWEN_HBCIMSG_FLAGS_SIGN)
      hmsg->firstSegment+=hmsg->nSigners;
    GWEN_MsgEngine_SetIntValue(e,
                               "SegmentNumber",
                               hmsg->firstSegment);
    hmsg->lastSegment=hmsg->firstSegment-1;
  }

  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hmsg->buffer,
                                          data);
  if (rv) {
    return -1;
  }
  hmsg->lastSegment=GWEN_MsgEngine_GetIntValue(e,
                                               "SegmentNumber",
                                               1)-1;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_PrepareCryptoSeg(GWEN_HBCIMSG *hmsg,
                                  GWEN_HBCICRYPTOCONTEXT *ctx,
                                  GWEN_DB_NODE *cfg,
                                  int createCtrlRef) {
  char sdate[9];
  char stime[6];
  char ctrlref[15];
  struct tm *lt;
  time_t tt;
  GWEN_KEYSPEC *ks;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  ks=GWEN_HBCICryptoContext_GetKeySpec(ctx);
  assert(ks);

  tt=time(0);
  lt=localtime(&tt);

  if (createCtrlRef) {
    /* create control reference */
    if (strftime(ctrlref, sizeof(ctrlref),
                 "%Y%m%d%H%M%S", lt)>=sizeof(ctrlref)) {
      DBG_INFO(0, "CtrlRef string too long");
      return -1;
    }
    /* create date */
    if (strftime(sdate, sizeof(sdate),
                 "%Y%m%d", lt)>=sizeof(sdate)) {
      DBG_INFO(0, "Date string too long");
      return -1;
    }
    /* create time */
    if (strftime(stime, sizeof(stime),
                 "%H%M%S", lt)>=sizeof(stime)) {
      DBG_INFO(0, "Date string too long");
      return -1;
    }

    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "ctrlref", ctrlref);
  }

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "SecStamp/time", stime);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/bankcode",
                       GWEN_HBCICryptoContext_GetServiceCode(ctx));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/userid",
                       GWEN_KeySpec_GetOwner(ks));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/num",
                      GWEN_KeySpec_GetNumber(ks));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/version",
                      GWEN_KeySpec_GetVersion(ks));

  /* store security id */
  if (strcasecmp(GWEN_MsgEngine_GetMode(e), "RDH")==0) {
    /* RDH mode */
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "SecDetails/SecId",
                         GWEN_HBCICryptoContext_GetSecurityIdPtr(ctx));
  }
  else {
    /* DDV mode */
    GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "SecDetails/SecId",
                        GWEN_HBCICryptoContext_GetSecurityIdPtr(ctx),
                        GWEN_HBCICryptoContext_GetSecurityIdSize(ctx));
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_SignMsg(GWEN_HBCIMSG *hmsg,
                         GWEN_BUFFER *rawBuf,
                         GWEN_KEYSPEC *ks) {
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *sigbuf;
  GWEN_BUFFER *hbuf;
  unsigned int l;
  int rv;
  GWEN_HBCICRYPTOCONTEXT *ctx;
  char ctrlref[15];
  const char *p;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "SigHead");
  if (!node) {
    DBG_INFO(0, "Segment \"SigHead\"not found");
    return -1;
  }

  /* prepare context */
  ctx=GWEN_HBCICryptoContext_new();
  GWEN_HBCICryptoContext_SetKeySpec(ctx, ks);
  if (GWEN_HBCIDialog_PrepareContext(hmsg->dialog, ctx, 0)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* prepare config for segment */
  if (GWEN_HBCIMsg_PrepareCryptoSeg(hmsg, ctx, cfg, 1)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  p=GWEN_DB_GetCharValue(cfg, "ctrlref", 0, "");
  if (strlen(p)>=sizeof(ctrlref)) {
    DBG_INFO(0, "Control reference too long (14 bytes maximum)");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  strcpy(ctrlref, p);

  /* create SigHead */
  hbuf=GWEN_Buffer_new(0, 128+GWEN_Buffer_GetUsedBytes(rawBuf), 0, 1);
  cfg=GWEN_DB_Group_new("sighead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->firstSegment-1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "signseq", GWEN_HBCICryptoContext_GetSequenceNum(ctx));

  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(0, "Could not create SigHead");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* remember size of sighead for now */
  l=GWEN_Buffer_GetUsedBytes(hbuf);

  /* add raw data to to-sign data buffer */
  if (GWEN_Buffer_AppendBuffer(hbuf, rawBuf)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* sign message */
  sigbuf=GWEN_Buffer_new(0, 512, 0, 1);
  if (GWEN_HBCIDialog_Sign(hmsg->dialog, hbuf, sigbuf, ctx)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(sigbuf);
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* insert new SigHead at beginning of message buffer */
  GWEN_Buffer_InsertBytes(hmsg->buffer, GWEN_Buffer_GetStart(hbuf), l);

  GWEN_HBCICryptoContext_free(ctx);
  GWEN_DB_Group_free(cfg);


  /* create sigtail */
  GWEN_Buffer_Reset(hbuf);
  cfg=GWEN_DB_Group_new("sigtail");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->lastSegment+1);
  /* store to DB */
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "ctrlref", ctrlref);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "signature",
                      GWEN_Buffer_GetStart(sigbuf),
                      GWEN_Buffer_GetUsedBytes(sigbuf));

  /* get node */
  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "SigTail");
  if (!node) {
    DBG_INFO(0, "Segment \"SigTail\"not found");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(0, "Could not create SigHead");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* append sigtail */
  if (GWEN_Buffer_AppendBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  GWEN_Buffer_free(hbuf);
  GWEN_DB_Group_free(cfg);

  /* adjust segment numbers (for next signature and message tail */
  hmsg->firstSegment--;
  hmsg->lastSegment++;

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_EncryptMsg(GWEN_HBCIMSG *hmsg) {
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *cryptbuf;
  GWEN_BUFFER *hbuf;
  int rv;
  GWEN_HBCICRYPTOCONTEXT *ctx;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "CryptHead");
  if (!node) {
    DBG_INFO(0, "Segment \"CryptHead\"not found");
    return -1;
  }

  /* prepare context */
  ctx=GWEN_HBCICryptoContext_new();
  GWEN_HBCICryptoContext_SetKeySpec(ctx, hmsg->crypter);
  if (GWEN_HBCIDialog_PrepareContext(hmsg->dialog, ctx, 1)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* encrypt message */
  if (GWEN_HBCIDialog_Encrypt(hmsg->dialog, hmsg->buffer, cryptbuf, ctx)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(cryptbuf);
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* create CryptHead */
  hbuf=GWEN_Buffer_new(0, 256+GWEN_Buffer_GetUsedBytes(hmsg->buffer), 0, 1);
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 998);

  if (GWEN_HBCIMsg_PrepareCryptoSeg(hmsg, ctx, cfg, 0)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  if (GWEN_HBCICryptoContext_GetCryptKeyPtr(ctx)) {
    /* only set message key if there is one.
     * The original HBCI protocol creates a message key for every message,
     * but the simplified IPC protocol only creates a session key if there
     * is none or if the existing session key is too old */
    GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "CryptAlgo/MsgKey",
                        GWEN_HBCICryptoContext_GetCryptKeyPtr(ctx),
                        GWEN_HBCICryptoContext_GetCryptKeySize(ctx));
  }

  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(0, "Could not create CryptHead");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  GWEN_DB_Group_free(cfg);


  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "cryptdata",
                      GWEN_Buffer_GetStart(cryptbuf),
                      GWEN_Buffer_GetUsedBytes(cryptbuf));

  /* get node */
  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "CryptData");
  if (!node) {
    DBG_INFO(0, "Segment \"CryptData\"not found");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(0, "Could not create CryptData");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* replace existing buffer by encrypted one */
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=hbuf;
  GWEN_DB_Group_free(cfg);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_HBCIMSG *GWEN_HBCIMsg_Next(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->next;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_Add(GWEN_HBCIMSG *hmsg,
                      GWEN_HBCIMSG **head){
  assert(hmsg);
  GWEN_LIST_ADD(GWEN_HBCIMSG, hmsg, head);
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_Del(GWEN_HBCIMSG *hmsg,
                      GWEN_HBCIMSG **head){
  assert(hmsg);
  GWEN_LIST_DEL(GWEN_HBCIMSG, hmsg, head);
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetMsgRef(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->refMsgNum;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetMsgNumber(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->msgNum;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_EncodeMsg(GWEN_HBCIMSG *hmsg) {

  assert(hmsg);

  /* sign message */
  if (hmsg->flags & GWEN_HBCIMSG_FLAGS_SIGN) {
    GWEN_BUFFER *rawBuf;
    GWEN_KEYSPEC *ks;

    if (hmsg->nSigners==0) {
      DBG_ERROR(0, "Message needs signing but there are no signers");
      return -1;
    }
    rawBuf=GWEN_Buffer_dup(hmsg->buffer);
    ks=hmsg->signers;
    while (ks) {
      if (GWEN_HBCIMsg_SignMsg(hmsg, rawBuf, ks)) {
        GWEN_Buffer_free(rawBuf);
        DBG_INFO(0, "here");
        return -1;
      }
      ks=GWEN_KeySpec_Next(ks);
    } /* while */
    GWEN_Buffer_free(rawBuf);
  } /* if signing is needed */

  /* encrypt message */
  if (hmsg->flags & GWEN_HBCIMSG_FLAGS_CRYPT) {
    if (GWEN_HBCIMsg_EncryptMsg(hmsg)) {
      DBG_INFO(0, "here");
      return -1;
    }
  }

  /* add msg tail */
  if (GWEN_HBCIMsg_AddMsgTail(hmsg)) {
    DBG_INFO(0, "here");
    return -1;
  }

  /* add msg head */
  if (GWEN_HBCIMsg_AddMsgHead(hmsg)) {
    DBG_INFO(0, "here");
    return -1;
  }

  DBG_INFO(0, "Message finished");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
/* return -1 on error (with group "seg/error" set) or -2 if the message is
 * faulty */
int GWEN_HBCIMsg_ReadSegment(GWEN_MSGENGINE *e,
                             const char *gtype,
                             GWEN_BUFFER *mbuf,
                             GWEN_DB_NODE *gr,
                             unsigned int flags) {
  GWEN_XMLNODE *node;
  unsigned int posBak;
  const char *p;
  GWEN_DB_NODE *tmpdb;
  int segVer;

  /* find head segment description */
  tmpdb=GWEN_DB_Group_new("tmpdb");
  node=GWEN_MsgEngine_FindGroupByProperty(e,
                                          "id",
                                          0,
                                          "SegHead");
  if (node==0) {
    DBG_ERROR(0, "Segment description not found (internal error)");
    GWEN_DB_Group_free(tmpdb);
    return -2;
  }

  /* parse head segment */
  posBak=GWEN_Buffer_GetPos(mbuf);
  if (GWEN_MsgEngine_ParseMessage(e,
                                  node,
                                  mbuf,
                                  tmpdb,
                                  flags)) {
    DBG_ERROR(0, "Error parsing segment head");
    GWEN_DB_Group_free(tmpdb);
    return -2;
  }

  GWEN_Buffer_SetPos(mbuf, posBak);

  /* get segment code */
  segVer=GWEN_DB_GetIntValue(tmpdb,
                             "version",
                             0,
                             0);
  p=GWEN_DB_GetCharValue(tmpdb,
                         "code",
                         0,
                         0);
  if (!p) {
    DBG_ERROR(0, "No segment code for %s ? This seems to be a bad msg...",
              gtype);
    DBG_ERROR(0, "Full message (pos=%04x)", posBak);
    GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf),
                         GWEN_Buffer_GetUsedBytes(mbuf),
                         stderr, 1);
    GWEN_DB_Dump(tmpdb, stderr, 1);
    GWEN_DB_Group_free(tmpdb);
    return -1;
  }

  /* try to find corresponding XML node */
  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         gtype,
                                         "code",
                                         segVer,
                                         p);
  if (node==0) {
    GWEN_DB_NODE *storegrp;
    unsigned int startPos;

    GWEN_Buffer_SetPos(mbuf, posBak);
    startPos=posBak;

    storegrp=GWEN_DB_GetGroup(gr,
                              GWEN_PATH_FLAGS_CREATE_GROUP,
                              p);
    assert(storegrp);

    /* store the start position of this segment within the DB */
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/pos",
                        startPos);
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/error/code",
                        9000);
    GWEN_DB_SetCharValue(storegrp,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "segment/error/text",
                         "Unknown segment");
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/error/pos",
                        startPos);

    /* node not found, skip it */
    DBG_WARN(0,
             "Unknown segment \"%s\" (Segnum=%d, version=%d, ref=%d)",
             p,
             GWEN_DB_GetIntValue(tmpdb, "seq", 0, -1),
             GWEN_DB_GetIntValue(tmpdb, "version", 0, -1),
             GWEN_DB_GetIntValue(tmpdb, "ref", 0, -1));
    if (GWEN_MsgEngine_SkipSegment(e, mbuf, '?', '\'')) {
      DBG_ERROR(0, "Error skipping segment \"%s\"", p);
      GWEN_DB_Group_free(tmpdb);
      return -1;
    }
    /* store segment size within DB */
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/length",
                        GWEN_Buffer_GetPos(mbuf)-startPos);
    /* handle trust info */
    if (flags & GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO) {
      unsigned int usize;

      usize=GWEN_Buffer_GetPos(mbuf)-(startPos+1)-1;
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf)+startPos+1,
                           usize,
                           stderr, 1);
      if (GWEN_MsgEngine_AddTrustInfo(e,
                                      GWEN_Buffer_GetStart(mbuf)+startPos,
                                      usize,
                                      p,
                                      GWEN_MsgEngineTrustLevelHigh,
                                      startPos)) {
        DBG_INFO(0, "called from here");
        GWEN_DB_Group_free(tmpdb);
        return -1;
      }
    } /* if trustInfo handling wanted */
  }
  else {
    /* ok, node available, get the corresponding description and parse
     * the segment */
    const char *id;
    GWEN_DB_NODE *storegrp;
    unsigned int startPos;

    /* restore start position, since the segment head is part of a full
     * description, so we need to restart reading from the very begin */
    GWEN_Buffer_SetPos(mbuf, posBak);

    /* create group in DB for this segment */
    id=GWEN_XMLNode_GetProperty(node, "id", p);
    storegrp=GWEN_DB_GetGroup(gr,
                              GWEN_PATH_FLAGS_CREATE_GROUP,
                              id);
    assert(storegrp);

    /* store the start position of this segment within the DB */
    startPos=GWEN_Buffer_GetPos(mbuf);
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/pos",
                        startPos);

    /* parse the segment */
    if (GWEN_MsgEngine_ParseMessage(e,
                                    node,
                                    mbuf,
                                    storegrp,
                                    flags)) {
      GWEN_DB_SetIntValue(storegrp,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "segment/error/code",
                          9000);
      GWEN_DB_SetCharValue(storegrp,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "segment/error/text",
                           "Syntax error");
      GWEN_DB_SetIntValue(storegrp,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "segment/error/pos",
                          GWEN_Buffer_GetPos(mbuf)-startPos);

      DBG_ERROR(0, "Error parsing segment \"%s\"",p);
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf)+startPos,
                           GWEN_Buffer_GetUsedBytes(mbuf)-startPos,
                           stderr, 1);
      GWEN_DB_Group_free(tmpdb);
      return -1;
    }

    /* store segment size within DB */
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/length",
                        GWEN_Buffer_GetPos(mbuf)-startPos);
  }
  GWEN_DB_Group_free(tmpdb);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_ReadMessage(GWEN_MSGENGINE *e,
                             const char *gtype,
                             GWEN_BUFFER *mbuf,
                             GWEN_DB_NODE *gr,
                             unsigned int flags) {
  unsigned int segments;
  unsigned int errors;
  int rv;

  segments=0;
  errors=0;

  while(GWEN_Buffer_BytesLeft(mbuf)) {
    rv=GWEN_HBCIMsg_ReadSegment(e, gtype, mbuf, gr, flags);
    if (rv==-2) {
      DBG_INFO(0, "here");
      return -1;
    }
    else if (rv==-1) {
      DBG_INFO(0, "here");
      errors++;
    }
    segments++;
  } /* while */

  DBG_NOTICE(0, "Parsed %d segments (%d had errors)",
             segments, errors);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_PrepareCryptoSegDec(GWEN_HBCIMSG *hmsg,
                                     GWEN_HBCICRYPTOCONTEXT *ctx,
                                     GWEN_DB_NODE *n) {
  GWEN_KEYSPEC *ks;
  const void *p;
  const char *s;
  unsigned int size;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  assert(ctx);
  assert(n);

  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  ks=GWEN_KeySpec_new();

  /* prepare context */
  GWEN_KeySpec_SetOwner(ks, GWEN_DB_GetCharValue(n, "userid", 0, ""));
  GWEN_KeySpec_SetNumber(ks, GWEN_DB_GetIntValue(n, "key/keynum", 0, 0));
  GWEN_KeySpec_SetVersion(ks,
                          GWEN_DB_GetIntValue(n, "key/keyversion", 0, 0));
  s=GWEN_DB_GetCharValue(n, "key/keytype", 0, 0);
  if (!s) {
    DBG_ERROR(0, "no keytype");
    GWEN_KeySpec_free(ks);
    return -1;
  }
  GWEN_KeySpec_SetKeyName(ks, s);
  GWEN_HBCICryptoContext_SetServiceCode(ctx,
                                        GWEN_DB_GetCharValue(n,
                                                             "key/bankcode",
                                                             0,
                                                             ""));
  GWEN_KeySpec_free(hmsg->crypter);
  hmsg->crypter=ks;

  p=GWEN_DB_GetBinValue(n,
                        "CryptAlgo/MsgKey",
                        0,
                        0,0,
                        &size);
  if (p)
    GWEN_HBCICryptoContext_SetCryptKey(ctx, p, size);

  /* store security id */
  if (strcasecmp(GWEN_MsgEngine_GetMode(e), "RDH")==0) {
    /* RDH mode */
    s=GWEN_DB_GetCharValue(n,
                           "SecDetails/SecId",
                           0,
                           0);
    if (s)
      GWEN_HBCICryptoContext_SetSecurityId(ctx, s, strlen(s));
  }
  else {
    /* DDV mode */
    p=GWEN_DB_GetBinValue(n,
                          "SecDetails/SecId",
                          0,
                          0,0,
                          &size);
    if (p)
      GWEN_HBCICryptoContext_SetSecurityId(ctx, p, size);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_Decrypt(GWEN_HBCIMSG *hmsg, GWEN_DB_NODE *gr){
  GWEN_HBCICRYPTOCONTEXT *ctx;
  const void *p;
  unsigned int size;
  GWEN_DB_NODE *nhead;
  GWEN_DB_NODE *ndata;
  GWEN_BUFFER *cdbuf;
  GWEN_BUFFER *ndbuf;
  int rv;

  /* decrypt */
  ctx=GWEN_HBCICryptoContext_new();

  nhead=GWEN_DB_GetGroup(gr,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "CryptHead");
  if (!nhead) {
    DBG_ERROR(0, "No crypt head");
    GWEN_HBCICryptoContext_free(ctx);
    return -1;
  }

  ndata=GWEN_DB_GetGroup(gr,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "CryptData");
  if (!ndata) {
    DBG_ERROR(0, "No crypt data");
    GWEN_HBCICryptoContext_free(ctx);
    return -1;
  }

  rv=GWEN_HBCIMsg_PrepareCryptoSegDec(hmsg, ctx, nhead);
  if (rv) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    return -1;
  }

  p=GWEN_DB_GetBinValue(ndata,
                        "CryptData",
                        0,
                        0,0,
                        &size);
  if (!p) {
    DBG_ERROR(0, "No crypt data");
    GWEN_HBCICryptoContext_free(ctx);
    return -1;
  }

  cdbuf=GWEN_Buffer_new((void*)p, size, size, 0);
  GWEN_Buffer_SetMode(cdbuf, 0); /* no dynamic mode ! */

  ndbuf=GWEN_Buffer_new(0, GWEN_HBCIMSG_DEFAULTSIZE, 0, 1);
  GWEN_Buffer_SetStep(ndbuf, 512);

  rv=GWEN_HBCIDialog_Decrypt(hmsg->dialog,
                             cdbuf,
                             ndbuf,
                             ctx);
  if (rv) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(cdbuf);
    GWEN_Buffer_free(ndbuf);
    GWEN_HBCICryptoContext_free(ctx);
    return -1;
  }

  /* store new buffer inside message */
  GWEN_Buffer_free(hmsg->origbuffer);
  hmsg->origbuffer=hmsg->buffer;
  GWEN_Buffer_Rewind(ndbuf);
  hmsg->buffer=ndbuf;

  GWEN_Buffer_free(cdbuf);
  GWEN_HBCICryptoContext_free(ctx);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_SequenceCheck(GWEN_DB_NODE *gr) {
  GWEN_DB_NODE *n;
  unsigned int sn;
  unsigned int errors;

  sn=1;
  errors=0;
  n=GWEN_DB_GetFirstGroup(gr);
  while(n) {
    unsigned int rsn;

    rsn=GWEN_DB_GetIntValue(n, "seg/seq", 0, 0);
    if (rsn!=sn) {
      DBG_ERROR(0, "Unexpected sequence number (%d, expected %d)",
                rsn, sn);
      GWEN_DB_SetIntValue(n,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "segment/error/code",
                          9000);
      GWEN_DB_SetCharValue(n,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "segment/error/text",
                           "Unexpected segment number");
      errors++;
    }
    sn++;
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (errors)
    return -1;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_Decode(GWEN_HBCIMSG *hmsg,
                        GWEN_BUFFER *mbuf,
                        GWEN_DB_NODE *gr,
                        unsigned int flags) {
  GWEN_MSGENGINE *e;
  int rv;
  GWEN_DB_NODE *n;

  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=mbuf;
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  rv=GWEN_HBCIMsg_ReadMessage(e, "SEG", mbuf, gr, flags);
  if (rv) {
    DBG_INFO(0, "here");
    return -1;
  }

  /* find Crypt head */
  n=GWEN_DB_GetGroup(gr,
                     GWEN_DB_FLAGS_DEFAULT |
                     GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                     "CryptHead");
  if (n) {
    if (GWEN_DB_GetIntValue(n, "seg/error/code", 0, 0)>=9000) {
      DBG_ERROR(0, "Encryption error");
      return -1;
    }
    rv=GWEN_HBCIMsg_Decrypt(hmsg, gr);
    if (rv) {
      DBG_INFO(0, "here");
      return -1;
    }
    /* unlink and delete crypthead */
    GWEN_DB_UnlinkGroup(n);
    GWEN_DB_Group_free(n);

    /* unlink and delete cryptdata */
    n=GWEN_DB_GetGroup(gr,
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "CryptData");
    if (n) {
      GWEN_DB_UnlinkGroup(n);
      GWEN_DB_Group_free(n);
    }
    /* parse decrypted message part */
    n=GWEN_DB_GetGroup(gr,
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "MsgTail");
    if (n) {
      /* temporarily unlink MsgTail, it will be appended after decoding
       * the crypted part, to keep the segment sequence correct */
      GWEN_DB_UnlinkGroup(n);
    }
    rv=GWEN_HBCIMsg_ReadMessage(e, "SEG", hmsg->buffer, gr, flags);
    if (n)
      GWEN_DB_AddGroup(gr, n);
    if (rv) {
      DBG_INFO(0, "here");
      return -1;
    }
  } /* if crypthead */

  /* check segment sequence numbers */
  rv=GWEN_HBCIMsg_SequenceCheck(gr);
  if (rv) {
    DBG_INFO(0, "here");
    return -1;
  }


  return 0;
}












