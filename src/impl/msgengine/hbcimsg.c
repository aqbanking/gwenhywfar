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

#include <time.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_KEYSPEC *GWEN_HBCIMsg_GetSigners(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->signers;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_AddSigner(GWEN_HBCIMSG *hmsg,
                            GWEN_KEYSPEC *ks){
  assert(hmsg);
  GWEN_KeySpec_Add(ks, &(hmsg->signers));
  hmsg->nSigners++;
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
  return hmsg;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_free(GWEN_HBCIMSG *hmsg){
  if (hmsg) {
    GWEN_KeySpec_Clear(&(hmsg->signers));
    GWEN_KeySpec_free(hmsg->crypter);
    GWEN_Buffer_free(hmsg->buffer);
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
int GWEN_MsgEngineHBCI_AddMsgHead(GWEN_HBCIMSG *hmsg) {
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
  if (GWEN_HBCIDialog_PrepareContext(hmsg->dialog, ctx)) {
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
  if (GWEN_HBCIDialog_PrepareContext(hmsg->dialog, ctx)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  if (GWEN_HBCIMsg_PrepareCryptoSeg(hmsg, ctx, cfg, 0)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* create CryptHead */
  hbuf=GWEN_Buffer_new(0, 256+GWEN_Buffer_GetUsedBytes(hmsg->buffer), 0, 1);
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 998);

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

  /* encrypt message */
  if (GWEN_HBCIDialog_Encrypt(hmsg->dialog, hmsg->buffer, cryptbuf, ctx)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(cryptbuf);
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  GWEN_DB_Group_free(cfg);


  /* create cryptdata */
  cfg=GWEN_DB_Group_new("sigtail");
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
























