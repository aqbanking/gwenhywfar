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
#include <gwenhyfwar/list.h>

#include <time.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_KEYSPEC *GWEN_HBCIMsg_GetSigners(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->signers;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_AddSigner(GWEN_HBCIMSG *hmsg,
                           const GWEN_KEYSPEC *ks){
  assert(hmsg);
  assert(ks);

  if (hmsg->nodes) {
    DBG_ERROR(0, "Signers must be added before nodes !");
    return -1;
  }
  GWEN_KeySpec_Add(GWEN_KeySpec_dup(ks), &(hmsg->signers));
  hmsg->nSigners++;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetSignerCount(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->nSigners;
}



/* --------------------------------------------------------------- FUNCTION */
const GWEN_KEYSPEC *GWEN_HBCIMsg_GetCrypter(const GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->crypter;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_SetCrypter(GWEN_HBCIMSG *hmsg,
                             const GWEN_KEYSPEC *ks){
  assert(hmsg);
  hmsg->crypter=GWEN_KeySpec_dup(ks);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *GWEN_HBCIMsg_GetBuffer(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->buffer;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *GWEN_HBCIMsg_TakeBuffer(GWEN_HBCIMSG *hmsg){
  GWEN_BUFFER *bf;

  assert(hmsg);
  bf=hmsg->buffer;
  hmsg->buffer=0;
  return bf;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_SetBuffer(GWEN_HBCIMSG *hmsg,
                            GWEN_BUFFER *bf){
  assert(hmsg);
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=bf;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetMsgLayerId(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->msgLayerId;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_HBCIMsg_GetNodes(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->nodes;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_SetMsgLayerId(GWEN_HBCIMSG *hmsg,
                                unsigned int i){
  assert(hmsg);
  hmsg->msgLayerId=i;
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
  GWEN_HBCIDialog_Attach(hdlg);
  hmsg->buffer=GWEN_Buffer_new(0, GWEN_HBCIMSG_DEFAULTSIZE, 0, 1);
  GWEN_Buffer_SetStep(hmsg->buffer, 512);
  hmsg->msgNum=GWEN_HBCIDialog_GetNextMsgNum(hdlg);
  return hmsg;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_free(GWEN_HBCIMSG *hmsg){
  if (hmsg) {
    GWEN_KeySpec_Clear(&(hmsg->signers));
    DBG_INFO(0, "Freeing Keyspec");
    GWEN_KeySpec_free(hmsg->crypter);
    GWEN_Buffer_free(hmsg->buffer);
    GWEN_Buffer_free(hmsg->origbuffer);
    GWEN_HBCIDialog_Detach(hmsg->dialog);
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
  GWEN_Buffer_SetPos(hmsg->buffer, GWEN_Buffer_GetUsedBytes(hmsg->buffer));
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
    GWEN_Buffer_free(hbuf);
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
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  /* insert msgHead */
  GWEN_Buffer_SetPos(hmsg->buffer, 0);
  if (GWEN_Buffer_InsertBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(0, "Could not insert msgHead");
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  GWEN_Buffer_free(hbuf);
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
unsigned int GWEN_HBCIMsg_AddNode(GWEN_HBCIMSG *hmsg,
                                  GWEN_XMLNODE *node,
                                  GWEN_DB_NODE *data) {
  int rv;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  assert(node);
  assert(data);

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
    DBG_INFO(0, "here");
    return 0;
  }
  hmsg->lastSegment=GWEN_MsgEngine_GetIntValue(e,
                                               "SegmentNumber",
                                               1)-1;
  hmsg->nodes++;
  return hmsg->lastSegment;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_PrepareCryptoSeg(GWEN_HBCIMSG *hmsg,
                                  GWEN_HBCICRYPTOCONTEXT *ctx,
                                  GWEN_DB_NODE *cfg,
                                  int createCtrlRef) {
  char sdate[9];
  char stime[7];
  char ctrlref[15];
  struct tm *lt;
  time_t tt;
  const GWEN_KEYSPEC *ks;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  ks=GWEN_HBCICryptoContext_GetKeySpec(ctx);
  assert(ks);

  /* check local context */
  if (!GWEN_HBCIDialog_GetLocalContext(hmsg->dialog)) {
    DBG_ERROR(0, "No local context, aborting");
    return -1;
  }
  else {
    if (GWEN_Text_Compare(GWEN_HBCIDialog_GetLocalContext(hmsg->dialog),
                          GWEN_KeySpec_GetOwner(ks),1)!=0) {
      DBG_ERROR(0, "Key owner does not match local context");
      return -1;
    }
  }

  tt=time(0);
  lt=localtime(&tt);

  if (createCtrlRef) {
    /* create control reference */
    if (!strftime(ctrlref, sizeof(ctrlref),
                  "%Y%m%d%H%M%S", lt)) {
      DBG_INFO(0, "CtrlRef string too long");
      return -1;
    }

    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "ctrlref", ctrlref);
  }
  /* create date */
  if (!strftime(sdate, sizeof(sdate),
                "%Y%m%d", lt)) {
    DBG_INFO(0, "Date string too long");
    return -1;
  }
  /* create time */
  if (!strftime(stime, sizeof(stime),
                "%H%M%S", lt)) {
    DBG_INFO(0, "Date string too long");
    return -1;
  }

  DBG_INFO(0, "Date and Time: %s / %s",
           sdate, stime);

  if (GWEN_HBCIDialog_GetFlags(hmsg->dialog) &
      GWEN_HBCIDIALOG_FLAGS_INITIATOR)
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "SecDetails/dir", 2);
  else
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "SecDetails/dir", 1);
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
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/keytype",
                       GWEN_KeySpec_GetKeyName(ks));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/keynum",
                      GWEN_KeySpec_GetNumber(ks));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/keyversion",
                      GWEN_KeySpec_GetVersion(ks));

  /* store security id */
  if (strcasecmp(GWEN_HBCICryptoContext_GetMode(ctx), "RDH")==0) {
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
  GWEN_SECCTX_MANAGER *scm;
  GWEN_SECCTX *sc;
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

  cfg=GWEN_DB_Group_new("sighead");

  /* prepare context */
  ctx=GWEN_HBCICryptoContext_new();
  GWEN_HBCICryptoContext_SetKeySpec(ctx, ks);
  scm=GWEN_HBCIDialog_GetSecurityManager(hmsg->dialog);
  assert(scm);
  GWEN_HBCICryptoContext_SetServiceCode(ctx,
                                        GWEN_SecContext_GetServiceCode(scm));
  sc=GWEN_SecContextMgr_GetContext(scm, GWEN_KeySpec_GetOwner(ks));
  if (!sc) {
    DBG_ERROR(0,
              "Unknown security context \"%s\"",
              GWEN_KeySpec_GetOwner(ks));
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  if (GWEN_SecContext_PrepareContext(sc, ctx, 0)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* prepare config for segment */
  if (GWEN_HBCIMsg_PrepareCryptoSeg(hmsg, ctx, cfg, 1)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  p=GWEN_DB_GetCharValue(cfg, "ctrlref", 0, "");
  if (strlen(p)>=sizeof(ctrlref)) {
    DBG_INFO(0, "Control reference too long (14 bytes maximum)");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  strcpy(ctrlref, p);

  /* create SigHead */
  hbuf=GWEN_Buffer_new(0, 128+GWEN_Buffer_GetUsedBytes(rawBuf), 0, 1);
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
  if (GWEN_SecContext_Sign(sc, hbuf, sigbuf, ctx)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(sigbuf);
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  DBG_INFO(0, "Signing done");

  /* insert new SigHead at beginning of message buffer */
  DBG_INFO(0, "Inserting signature head");
  GWEN_Buffer_Rewind(hmsg->buffer);
  GWEN_Buffer_InsertBytes(hmsg->buffer, GWEN_Buffer_GetStart(hbuf), l);

  GWEN_HBCICryptoContext_free(ctx);
  GWEN_DB_Group_free(cfg);

  /* create sigtail */
  DBG_INFO(0, "Creating signature tail");
  GWEN_Buffer_Reset(hbuf);
  cfg=GWEN_DB_Group_new("sigtail");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->lastSegment+1);
  /* store to DB */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "signature",
                      GWEN_Buffer_GetStart(sigbuf),
                      GWEN_Buffer_GetUsedBytes(sigbuf));

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "ctrlref", ctrlref);
  GWEN_Buffer_free(sigbuf);

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
    DBG_INFO(0, "Could not create SigTail");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* append sigtail */
  DBG_INFO(0, "Appending signature tail");
  if (GWEN_Buffer_AppendBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  DBG_INFO(0, "Appending signature tail: done");

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
  GWEN_SECCTX_MANAGER *scm;
  GWEN_SECCTX *sc;
  GWEN_HBCICRYPTOCONTEXT *ctx;
  GWEN_MSGENGINE *e;

  DBG_INFO(0, "Encrypting message");
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
  DBG_INFO(0, "Preparing context for encryption");
  ctx=GWEN_HBCICryptoContext_new();
  GWEN_HBCICryptoContext_SetKeySpec(ctx, hmsg->crypter);
  scm=GWEN_HBCIDialog_GetSecurityManager(hmsg->dialog);
  assert(scm);
  GWEN_HBCICryptoContext_SetServiceCode(ctx,
                                        GWEN_SecContext_GetServiceCode(scm));
  sc=GWEN_SecContextMgr_GetContext(scm,
                                   GWEN_KeySpec_GetOwner(hmsg->crypter));
  if (!sc) {
    DBG_ERROR(0,
              "Unknown security context \"%s\"",
              GWEN_KeySpec_GetOwner(hmsg->crypter));
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  if (GWEN_SecContext_PrepareContext(sc, ctx, 1)) {
    DBG_INFO(0, "here");
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  DBG_INFO(0, "Preparing context for encryption: done");

  /* encrypt message */
  DBG_INFO(0, "Encrypting message");
  cryptbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(hmsg->buffer)+256,0,1);
  GWEN_Buffer_Rewind(hmsg->buffer);
  if (GWEN_SecContext_Encrypt(sc, hmsg->buffer, cryptbuf, ctx)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(cryptbuf);
    GWEN_HBCICryptoContext_free(ctx);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  DBG_INFO(0, "Encrypting message: done");

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
  GWEN_HBCICryptoContext_free(ctx);


  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "cryptdata",
                      GWEN_Buffer_GetStart(cryptbuf),
                      GWEN_Buffer_GetUsedBytes(cryptbuf));
  GWEN_Buffer_free(cryptbuf);

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

  DBG_NOTICE(0, "Encoding message");
  GWEN_Buffer_Dump(hmsg->buffer, stderr, 2);

  GWEN_MsgEngine_SetIntValue(GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog),
                             "MessageNumber",
                             hmsg->msgNum);

  /* sign message */
  DBG_INFO(0, "Letting all signers sign");
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
  DBG_INFO(0, "Letting all signers sign: done");
  GWEN_Buffer_Dump(hmsg->buffer, stderr, 2);

  /* encrypt message */
  if (hmsg->flags & GWEN_HBCIMSG_FLAGS_CRYPT) {
    DBG_INFO(0, "Encrypting message");
    if (GWEN_HBCIMsg_EncryptMsg(hmsg)) {
      DBG_INFO(0, "here");
      return -1;
    }
    DBG_INFO(0, "Encrypting message: done");
  }

  /* add msg tail */
  DBG_INFO(0, "Adding message tail");
  if (GWEN_HBCIMsg_AddMsgTail(hmsg)) {
    DBG_INFO(0, "here");
    return -1;
  }
  DBG_INFO(0, "Adding message tail: done");
  GWEN_Buffer_Dump(hmsg->buffer, stderr, 2);

  /* add msg head */
  DBG_INFO(0, "Adding message head");
  if (GWEN_HBCIMsg_AddMsgHead(hmsg)) {
    DBG_INFO(0, "here");
    return -1;
  }
  DBG_INFO(0, "Adding message head: done");
  GWEN_Buffer_Dump(hmsg->buffer, stderr, 2);

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

      DBG_ERROR(0, "Error here:");
      GWEN_Buffer_Dump(mbuf, stderr, 2);
      return -1; /* DEBUG */
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
                                     GWEN_DB_NODE *n,
                                     int crypt) {
  GWEN_KEYSPEC *ks;
  const void *p;
  const char *s;
  unsigned int size;
  GWEN_MSGENGINE *e;
  int rdhMode;

  assert(hmsg);
  assert(ctx);
  assert(n);

  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  ks=GWEN_KeySpec_new();

  /* prepare context */
  GWEN_KeySpec_SetOwner(ks, GWEN_DB_GetCharValue(n, "key/userid", 0, ""));
  GWEN_KeySpec_SetNumber(ks, GWEN_DB_GetIntValue(n, "key/keynum", 0, 0));
  GWEN_KeySpec_SetKeyName(ks, GWEN_DB_GetCharValue(n, "key/keytype", 0, ""));
  GWEN_KeySpec_SetVersion(ks,
                          GWEN_DB_GetIntValue(n, "key/keyversion", 0, 0));
  s=GWEN_DB_GetCharValue(n, "key/keytype", 0, 0);
  if (!s) {
    DBG_ERROR(0, "no keytype");
    DBG_INFO(0, "Freeing Keyspec");
    GWEN_KeySpec_free(ks);
    return -1;
  }
  GWEN_KeySpec_SetKeyName(ks, s);

  /* check local context */
  if (!GWEN_HBCIDialog_GetLocalContext(hmsg->dialog)) {
    DBG_ERROR(0, "No local context, aborting");
    return -1;
  }
  else {
    if (GWEN_Text_Compare(GWEN_HBCIDialog_GetLocalContext(hmsg->dialog),
                          GWEN_KeySpec_GetOwner(ks),1)!=0) {
      DBG_ERROR(0, "Key owner does not match local context");
      return -1;
    }
  }

  GWEN_HBCICryptoContext_SetServiceCode(ctx,
                                        GWEN_DB_GetCharValue(n,
                                                             "key/bankcode",
                                                             0,
                                                             ""));
  GWEN_HBCICryptoContext_SetKeySpec(ctx, ks);
  DBG_INFO(0, "Freeing Keyspec");
  GWEN_KeySpec_free(ks);

  p=GWEN_DB_GetBinValue(n,
                        "CryptAlgo/MsgKey",
                        0,
                        0,0,
                        &size);
  if (p)
    GWEN_HBCICryptoContext_SetCryptKey(ctx, p, size);

  /* get security mode */
  if (crypt) {
    int m;

    m=GWEN_DB_GetIntValue(n, "CryptAlgo/keytype", 0, -1);
    if (m==-1) {
      DBG_ERROR(0, "Bad security mode (non-DDV, non-RDH)");
      return -1;
    }
    if (m==6)
      rdhMode=1;
    else if (m!=5) {
      DBG_ERROR(0, "Bad security mode (non-DDV, non-RDH: %d)", m);
      return -1;
    }
  }
  else {
    int m;

    GWEN_HBCICryptoContext_SetSequenceNum(ctx,
                                          GWEN_DB_GetIntValue(n,
                                                              "signseq", 0,
                                                              0));
    m=GWEN_DB_GetIntValue(n, "SignAlgo/algo", 0, -1);
    if (m==-1) {
      DBG_ERROR(0, "Bad security mode (non-DDV, non-RDH)");
      return -1;
    }
    if (m==10)
      rdhMode=1;
    else if (m!=1) {
      DBG_ERROR(0, "Bad security mode (non-DDV, non-RDH: %d)", m);
      return -1;
    }
  }
  if (rdhMode)
    GWEN_HBCICryptoContext_SetMode(ctx, "RDH");
  else
    GWEN_HBCICryptoContext_SetMode(ctx, "DDV");

  /* store security id */
  if (rdhMode) {
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
  GWEN_SECCTX_MANAGER *scm;
  GWEN_SECCTX *sc;
  int rv;
  const GWEN_KEYSPEC *ks;

  scm=GWEN_HBCIDialog_GetSecurityManager(hmsg->dialog);
  assert(scm);

  /* decrypt */
  ctx=GWEN_HBCICryptoContext_new();
  GWEN_HBCICryptoContext_SetServiceCode(ctx,
                                        GWEN_SecContext_GetServiceCode(scm));

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

  rv=GWEN_HBCIMsg_PrepareCryptoSegDec(hmsg, ctx, nhead, 1);
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
  ks=GWEN_HBCICryptoContext_GetKeySpec(ctx);
  assert(ks);
  sc=GWEN_SecContextMgr_GetContext(scm, GWEN_KeySpec_GetOwner(ks));
  if (!sc) {
    DBG_ERROR(0,
              "Unknown security context \"%s\"",
              GWEN_KeySpec_GetOwner(ks));
    GWEN_Buffer_free(cdbuf);
    GWEN_Buffer_free(ndbuf);
    GWEN_HBCICryptoContext_free(ctx);
    return -1;
  }
  rv=GWEN_SecContext_Decrypt(sc,
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

  /* store crypter */
  GWEN_HBCIMsg_SetCrypter(hmsg, GWEN_HBCICryptoContext_GetKeySpec(ctx));

  GWEN_Buffer_free(cdbuf);
  GWEN_HBCICryptoContext_free(ctx);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_SequenceCheck(GWEN_DB_NODE *gr) {
  GWEN_DB_NODE *n;
  unsigned int sn;
  unsigned int errors;

  DBG_INFO(0, "Sequence check");
  sn=1;
  errors=0;
  n=GWEN_DB_GetFirstGroup(gr);
  while(n) {
    unsigned int rsn;

    rsn=GWEN_DB_GetIntValue(n, "head/seq", 0, 0);
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
  DBG_INFO(0, "Sequence check ok");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_Verify(GWEN_HBCIMSG *hmsg,
                        GWEN_DB_NODE *gr,
                        unsigned int flags) {
  GWEN_LIST *sigheads;
  GWEN_LIST *sigtails;
  GWEN_DB_NODE *n;
  int nonSigHeads;
  int nSigheads;
  unsigned int dataBegin;
  char *dataStart;
  unsigned int dataLength;
  unsigned int i;
  GWEN_SECCTX_MANAGER *scm;

  sigheads=GWEN_List_new();

  scm=GWEN_HBCIDialog_GetSecurityManager(hmsg->dialog);
  assert(scm);

  /* enumerate signature heads */
  nonSigHeads=0;
  nSigheads=0;
  n=GWEN_DB_GetFirstGroup(gr);
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigHead")==0) {
      /* found a signature head */
      if (nonSigHeads) {
        DBG_ERROR(0, "Found some unsigned parts at the beginning");
        GWEN_List_free(sigheads);
        return -1;
      }
      GWEN_List_PushBack(sigheads, n);
      nSigheads++;
    }
    else if (strcasecmp(GWEN_DB_GroupName(n), "MsgHead")!=0) {
      if (nSigheads)
        break;
      nonSigHeads++;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    if (nSigheads) {
      DBG_ERROR(0, "Found Signature heads but no other segments");
      GWEN_List_free(sigheads);
      return -1;
    }
    DBG_INFO(0, "No signatures");
    GWEN_List_free(sigheads);
    return 0;
  }

  /* store begin of signed data */
  dataBegin=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
  if (!dataBegin) {
    DBG_ERROR(0, "No position specifications in segment");
    GWEN_List_free(sigheads);
    return -1;
  }

  /* now get first signature tail */
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")==0) {
      unsigned int currpos;

      /* found a signature tail */
      currpos=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
      if (!currpos || dataBegin>currpos) {
        DBG_ERROR(0, "Bad position specification in Signature tail");
        GWEN_List_free(sigheads);
        return -1;
      }
      dataLength=currpos-dataBegin;
      break;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(0, "No signature tail found");
    GWEN_List_free(sigheads);
    return -1;
  }

  sigtails=GWEN_List_new();
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")!=0)
      break;
    GWEN_List_PushBack(sigtails, n);
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(0, "Message tail expected");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return -1;
  }

  if (strcasecmp(GWEN_DB_GroupName(n), "MsgTail")!=0) {
    DBG_ERROR(0, "Unexpected segment (msg tail expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return -1;
  }

  n=GWEN_DB_GetNextGroup(n);
  if (n) {
    DBG_ERROR(0, "Unexpected segment (end expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return -1;
  }

  if (GWEN_List_GetSize(sigheads)!=
      GWEN_List_GetSize(sigtails)) {
    DBG_ERROR(0,
              "Number of signature heads (%d) does not match "
              "number of signature tails (%d)",
              GWEN_List_GetSize(sigheads),
              GWEN_List_GetSize(sigtails));
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return -1;
  }

  /* ok, now verify all signatures */
  dataStart=GWEN_Buffer_GetStart(hmsg->buffer)+dataBegin;
  for (i=0; i< GWEN_List_GetSize(sigtails); i++) {
    GWEN_DB_NODE *sighead;
    GWEN_DB_NODE *sigtail;
    GWEN_BUFFER *dbuf;
    GWEN_BUFFER *sigbuf;
    const void *p;
    unsigned int size;
    int rv;
    GWEN_SECCTX *sc;
    GWEN_HBCICRYPTOCONTEXT *ctx;
    const GWEN_KEYSPEC *ks;

    /* get signature tail */
    sigtail=(GWEN_DB_NODE*)GWEN_List_GetBack(sigtails);

    /* get corresponding signature head */
    sighead=(GWEN_DB_NODE*)GWEN_List_GetFront(sigheads);

    if (!sighead || !sigtail) {
      DBG_ERROR(0, "No signature head/tail left (internal error)");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return -1;
    }

    GWEN_List_PopBack(sigtails);
    GWEN_List_PopFront(sigheads);

    /* some checks */
    if (strcasecmp(GWEN_DB_GetCharValue(sighead, "ctrlref", 0, ""),
                   GWEN_DB_GetCharValue(sigtail, "ctrlref", 0, ""))!=0) {
      DBG_ERROR(0, "Non-matching signature tail");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return -1;
    }

    /* prepare data buffer */
    dbuf=GWEN_Buffer_new(0,
                         dataLength+GWEN_DB_GetIntValue(sighead,
                                                        "segment/length",
                                                        0,
                                                        0),
                         0,1);
    GWEN_Buffer_AppendBytes(dbuf,
                            GWEN_Buffer_GetStart(hmsg->buffer)+
                            GWEN_DB_GetIntValue(sighead,
                                                "segment/pos",
                                                0,
                                                0),
                            GWEN_DB_GetIntValue(sighead,
                                                "segment/length",
                                                0,
                                                0));
    GWEN_Buffer_AppendBytes(dbuf, dataStart, dataLength);

    /* prepare signature buffer */
    p=GWEN_DB_GetBinValue(sigtail, "signature", 0, 0, 0, &size);
    if (!p) {
      DBG_ERROR(0, "No signature");
      GWEN_Buffer_free(dbuf);
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return -1;
    }

    GWEN_Buffer_Rewind(dbuf);

    sigbuf=GWEN_Buffer_new((char*)p, size, size, 0);
    GWEN_Buffer_SetMode(sigbuf, 0);

    /* prepare context */
    ctx=GWEN_HBCICryptoContext_new();
    GWEN_HBCICryptoContext_SetServiceCode(ctx,
                                          GWEN_SecContext_GetServiceCode(scm));
    if (GWEN_HBCIMsg_PrepareCryptoSegDec(hmsg, ctx, sighead, 0)) {
      GWEN_HBCICryptoContext_free(ctx);
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(dbuf);
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      DBG_INFO(0, "here");
      return -1;
    }

    ks=GWEN_HBCICryptoContext_GetKeySpec(ctx);
    assert(ks);
    sc=GWEN_SecContextMgr_GetContext(scm, GWEN_KeySpec_GetOwner(ks));
    if (!sc) {
      DBG_ERROR(0,
                "Unknown security context \"%s\"",
                GWEN_KeySpec_GetOwner(ks));
      GWEN_HBCICryptoContext_free(ctx);
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(dbuf);
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return -1;
    }

    /* verify signature */
    rv=GWEN_SecContext_Verify(sc, dbuf, sigbuf, ctx);
    GWEN_Buffer_free(sigbuf);
    GWEN_Buffer_free(dbuf);

    if (rv) {
      DBG_ERROR(0, "Invalid signature");
      GWEN_HBCICryptoContext_free(ctx);
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return -1;
    }

    /* add signer */
    GWEN_HBCIMsg_AddSigner(hmsg, GWEN_HBCICryptoContext_GetKeySpec(ctx));
    GWEN_HBCICryptoContext_free(ctx);
  } /* for */


  GWEN_List_free(sigheads);
  GWEN_List_free(sigtails);
  DBG_INFO(0, "Signature valid");
  return 0;
}





/* --------------------------------------------------------------- FUNCTION */
int GWEN_HBCIMsg_DecodeMsg(GWEN_HBCIMSG *hmsg,
                           GWEN_DB_NODE *gr,
                           unsigned int flags) {
  GWEN_MSGENGINE *e;
  int rv;
  GWEN_DB_NODE *n;

  DBG_NOTICE(0, "Decoding this message:");
  GWEN_Buffer_Dump(GWEN_HBCIMsg_GetBuffer(hmsg), stderr, 2);

  e=GWEN_HBCIDialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  GWEN_Buffer_Rewind(hmsg->buffer);
  rv=GWEN_HBCIMsg_ReadMessage(e, "SEG", hmsg->buffer, gr, flags);
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
    if (GWEN_DB_GetIntValue(n, "segment/error/code", 0, 0)>=9000) {
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

  /* verify signatures */
  rv=GWEN_HBCIMsg_Verify(hmsg, gr, flags);
  if (rv) {
    DBG_INFO(0, "here");
    return -1;
  }

  DBG_INFO(0, "Decoded message is:");
  GWEN_HBCIMsg_Dump(hmsg, stderr, 1);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_HBCIMsg_Dump(const GWEN_HBCIMSG *hmsg, FILE *f, int indent) {
  unsigned int i;
  GWEN_KEYSPEC *ks;

  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "GWEN_HBCIMsg\n");
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "==================================================\n");
  for (i=0; i<indent; i++) fprintf(f, " ");
  if (hmsg->origbuffer) {
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, "Original buffer      :\n");
    GWEN_Buffer_Dump(hmsg->origbuffer, f, indent+2);
  }
  else {
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, "Original buffer      : none\n");
  }
  if (hmsg->buffer) {
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, "Buffer:\n");
    GWEN_Buffer_Dump(hmsg->buffer, f, indent+2);
  }
  else {
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, "Buffer               : none\n");
  }

  for (i=0; i<indent; i++) fprintf(f, " ");
  if (hmsg->crypter) {
    fprintf(f, "Crypter:\n");
    GWEN_KeySpec_Dump(hmsg->crypter, f, indent+2);
  }
  else {
    fprintf(f, "Crypter: none\n");
  }
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Signers (%d):\n", hmsg->nSigners);
  ks=hmsg->signers;
  while(ks) {
    GWEN_KeySpec_Dump(ks, f, indent+2);
    ks=GWEN_KeySpec_Next(ks);
  } /* while */
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "MsgLayer             : %d\n", hmsg->msgLayerId);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Nodes                : %d\n", hmsg->nodes);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Msg number           : %d\n", hmsg->msgNum);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Reference msg number : %d\n", hmsg->refMsgNum);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "First segment        : %d\n", hmsg->firstSegment);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Last segment         : %d\n", hmsg->lastSegment);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Flags                :");
  if (hmsg->flags & GWEN_HBCIMSG_FLAGS_SIGN)
    fprintf(f, " SIGN");
  if (hmsg->flags & GWEN_HBCIMSG_FLAGS_CRYPT)
    fprintf(f, " CRYPT");
  fprintf(f, "\n");

}



unsigned int GWEN_HBCIMsg_GetDialogNumber(GWEN_HBCIMSG *hmsg){
  assert(hmsg);
  return hmsg->dialogNumber;
}



void GWEN_HBCIMsg_SetDialogNumber(GWEN_HBCIMSG *hmsg,
                                  unsigned int i){
  assert(hmsg);
  hmsg->dialogNumber=i;
}










