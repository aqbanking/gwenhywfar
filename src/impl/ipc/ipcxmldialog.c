/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
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

#include "ipcxmldialog_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/text.h>
#include <gwenhyfwar/md.h>
#include <gwenhyfwar/crypt.h>



GWEN_IPCXMLDIALOGDATA *GWEN_IPCXMLDialogData_new(){
  GWEN_IPCXMLDIALOGDATA *d;

  GWEN_NEW_OBJECT(GWEN_IPCXMLDIALOGDATA, d);
  return d;
}



void GWEN_IPCXMLDialogData_free(GWEN_IPCXMLDIALOGDATA *d){
  if (d) {
    GWEN_CryptKey_free(d->localKey);
    GWEN_CryptKey_free(d->remoteKey);
    GWEN_CryptKey_free(d->sessionKey);
    free(d->serviceCode);
    free(d->securityId);
    free(d);
  }
}



GWEN_CRYPTKEY *GWEN_IPCXMLDialog_GetLocalKey(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  return dd->localKey;
}



void GWEN_IPCXMLDialog_SetLocalKey(GWEN_HBCIDIALOG *d,
                                   GWEN_CRYPTKEY *k){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  GWEN_CryptKey_free(dd->localKey);
  dd->localKey=k;
}



GWEN_CRYPTKEY *GWEN_IPCXMLDialog_GetRemoteKey(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  return dd->remoteKey;
}



void GWEN_IPCXMLDialog_SetRemoteKey(GWEN_HBCIDIALOG *d,
                                    GWEN_CRYPTKEY *k){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  GWEN_CryptKey_free(dd->remoteKey);
  dd->remoteKey=k;
}



GWEN_CRYPTKEY *GWEN_IPCXMLDialog_GetSessionKey(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  return dd->sessionKey;
}



void GWEN_IPCXMLDialog_SetSessionKey(GWEN_HBCIDIALOG *d,
                                     GWEN_CRYPTKEY *k){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  GWEN_CryptKey_free(dd->sessionKey);
  dd->sessionKey=k;
}



unsigned int GWEN_IPCXMLDialog_GetFlags(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  return dd->flags;
}



void GWEN_IPCXMLDialog_SetFlags(GWEN_HBCIDIALOG *d,
                                unsigned int f){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  dd->flags=f;
}



unsigned int GWEN_IPCXMLDialog_GetLocalSignSeq(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  return dd->localSignSeq;
}



void GWEN_IPCXMLDialog_SetLocalSignSeq(GWEN_HBCIDIALOG *d,
                                       unsigned int i){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  dd->localSignSeq=i;
}



unsigned int GWEN_IPCXMLDialog_GetRemoteSignSeq(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  return dd->remoteSignSeq;
}



void GWEN_IPCXMLDialog_SetRemoteSignSeq(GWEN_HBCIDIALOG *d,
                                        unsigned int i){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  dd->remoteSignSeq=i;
}







int GWEN_IPCXMLDialog_PrepareCTX(GWEN_HBCIDIALOG *d,
                                 GWEN_HBCICRYPTOCONTEXT *ctx,
                                 int crypt){
  GWEN_IPCXMLDIALOGDATA *dd;
  GWEN_ERRORCODE err;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  if (crypt) {
    const GWEN_KEYSPEC *ks;

    ks=GWEN_CryptKey_GetKeySpec(dd->remoteKey);
    assert(ks);
    GWEN_HBCICryptoContext_SetKeySpec(ctx, ks);
    if (!(dd->sessionKey)) {
      GWEN_BUFFER *kbuf;
      GWEN_BUFFER *sbuf;
      int i;

      /* generate session key, if possible */
      if (!(dd->remoteKey)) {
        DBG_ERROR(0, "No remote key");
        return -1;
      }

      DBG_NOTICE(0, "Generating session key");
      dd->sessionKey=GWEN_CryptKey_Factory("DES");
      assert(dd->sessionKey);
      err=GWEN_CryptKey_Generate(dd->sessionKey, 0);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO_ERR(0, err);
        return -1;
      }
      kbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_ReserveBytes(kbuf, 128);
      if (GWEN_Buffer_AppendBytes(kbuf,
				  GWEN_CryptKey_GetKeyData(dd->sessionKey),
				  16)) {
	DBG_INFO(0, "here");
	GWEN_Buffer_free(kbuf);
	return -1;
      }

      GWEN_Buffer_Rewind(kbuf);
      i=GWEN_CryptKey_GetChunkSize(dd->remoteKey)-16;
      DBG_INFO(0, "Padding with %d bytes", i);
      while(i-->0) {
	if (GWEN_Buffer_InsertByte(kbuf, (char)0)) {
	  DBG_INFO(0, "here");
	  GWEN_Buffer_free(kbuf);
	  return -1;
	}
      } /* while */
      DBG_INFO(0, "Padding done");

      sbuf=GWEN_Buffer_new(0, 256, 0, 1);
      DBG_INFO(0, "Encrypting key");
      err=GWEN_CryptKey_Encrypt(dd->remoteKey,
                                kbuf,
                                sbuf);
      if (!GWEN_Error_IsOk(err)) {
        GWEN_Buffer_free(kbuf);
        GWEN_Buffer_free(sbuf);
        DBG_INFO_ERR(0, err);
        return -1;
      }
      DBG_INFO(0, "Encrypting key: done");

      GWEN_HBCICryptoContext_SetCryptKey(ctx,
                                         GWEN_Buffer_GetStart(sbuf),
                                         GWEN_Buffer_GetUsedBytes(sbuf));
      GWEN_Buffer_free(kbuf);
      GWEN_Buffer_free(sbuf);
    } /* if no session key */
  }
  else {
    const GWEN_KEYSPEC *ks;

    /* sign */
    ks=GWEN_CryptKey_GetKeySpec(dd->localKey);
    assert(ks);
    GWEN_HBCICryptoContext_SetKeySpec(ctx, ks);
    GWEN_HBCICryptoContext_SetSequenceNum(ctx, dd->localSignSeq+1); /* DEBUG*/
  }

  GWEN_HBCICryptoContext_SetMode(ctx, "RDH");
  if (dd->serviceCode)
    GWEN_HBCICryptoContext_SetServiceCode(ctx, dd->serviceCode);
  if (dd->securityId)
    GWEN_HBCICryptoContext_SetSecurityId(ctx,
                                         dd->securityId,
                                         strlen(dd->securityId)+1);
  DBG_INFO(0, "Context prepared");
  return 0;
}



int GWEN_IPCXMLDialog_Sign(GWEN_HBCIDIALOG *d,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLDIALOGDATA *dd;
  GWEN_MD *md;
  GWEN_BUFFER *hashbuf;
  GWEN_ERRORCODE err;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  /* hash data */
  DBG_INFO(0, "Hash data");
  md=GWEN_MD_Factory("RMD160");
  if (!md) {
    DBG_ERROR(0, "RMD160 not found");
    return -1;
  }

  if (GWEN_MD_Begin(md)) {
    DBG_INFO(0, "here");
    GWEN_MD_free(md);
    return -1;
  }

  if (GWEN_MD_Update(md,
                     GWEN_Buffer_GetStart(msgbuf),
                     GWEN_Buffer_GetUsedBytes(msgbuf))) {
    DBG_INFO(0, "here");
    GWEN_MD_free(md);
    return -1;
  }

  if (GWEN_MD_End(md)) {
    DBG_INFO(0, "here");
    GWEN_MD_free(md);
    return -1;
  }
  DBG_INFO(0, "Hashing done");

  hashbuf=GWEN_Buffer_new(0,
                          GWEN_MD_GetDigestSize(md),
                          0, 1);
  if (GWEN_Buffer_AppendBytes(hashbuf,
                              GWEN_MD_GetDigestPtr(md),
                              GWEN_MD_GetDigestSize(md))) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    GWEN_MD_free(md);
    return -1;
  }
  GWEN_MD_free(md);

  /* padd */
  DBG_INFO(0, "Padding hash using ISO 9796");
  if (GWEN_HBCIDialog_PaddWithISO9796(hashbuf)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    return -1;
  }

  /* sign hash */
  GWEN_Buffer_Rewind(hashbuf);

  err=GWEN_CryptKey_Sign(dd->localKey,
                         hashbuf,
                         signbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_Buffer_free(hashbuf);
    return err;
  }

  GWEN_Buffer_free(hashbuf);
  DBG_INFO(0, "Signing done");
  return 0;
}



int GWEN_IPCXMLDialog_Verify(GWEN_HBCIDIALOG *d,
                             GWEN_BUFFER *msgbuf,
                             GWEN_BUFFER *signbuf,
                             GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLDIALOGDATA *dd;
  GWEN_MD *md;
  GWEN_BUFFER *hashbuf;
  GWEN_ERRORCODE err;
  unsigned int rseq;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  if (!(dd->remoteKey)) {
    DBG_ERROR(0, "No remote key");
    return -1;
  }

  /* check signature sequence number */
  rseq=GWEN_HBCICryptoContext_GetSequenceNum(ctx);
  if (rseq<=dd->remoteSignSeq) {
    DBG_ERROR(0, "bad signature sequence number (%d<%d)",
              rseq, dd->remoteSignSeq);
    return -1;
  }
  dd->remoteSignSeq=rseq;

  /* hash data */
  md=GWEN_MD_Factory("RMD160");
  if (!md) {
    DBG_ERROR(0, "RMD160 not found");
    return -1;
  }

  if (GWEN_MD_Begin(md)) {
    DBG_INFO(0, "here");
    GWEN_MD_free(md);
    return -1;
  }

  if (GWEN_MD_Update(md,
                     GWEN_Buffer_GetStart(msgbuf),
                     GWEN_Buffer_GetUsedBytes(msgbuf))) {
    DBG_INFO(0, "here");
    GWEN_MD_free(md);
    return -1;
  }

  if (GWEN_MD_End(md)) {
    DBG_INFO(0, "here");
    GWEN_MD_free(md);
    return -1;
  }

  hashbuf=GWEN_Buffer_new(0, GWEN_MD_GetDigestSize(md), 0, 1);
  if (GWEN_Buffer_AppendBytes(hashbuf,
                              GWEN_MD_GetDigestPtr(md),
                              GWEN_MD_GetDigestSize(md))) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    GWEN_MD_free(md);
    return -1;
  }
  GWEN_MD_free(md);

  /* padd */
  DBG_INFO(0, "Padding hash using ISO 9796");
  if (GWEN_HBCIDialog_PaddWithISO9796(hashbuf)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    return -1;
  }

  /* verify hash */
  err=GWEN_CryptKey_Verify(dd->remoteKey,
                           hashbuf,
                           signbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    return err;
  }
  GWEN_Buffer_free(hashbuf);

  return 0;


}



int GWEN_IPCXMLDialog_Encrypt(GWEN_HBCIDIALOG *d,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *cryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLDIALOGDATA *dd;
  GWEN_ERRORCODE err;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  if (!(dd->sessionKey)) {
    DBG_ERROR(0, "No remote key");
    return -1;
  }

  DBG_INFO(0, "Padding with ANSI X9.23");
  if (GWEN_HBCIDialog_PaddWithANSIX9_23(msgbuf)) {
    DBG_INFO(0, "here");
    return -1;
  }
  DBG_INFO(0, "Padding with ANSI X9.23: done");

  DBG_INFO(0, "Encrypting with session key");
  err=GWEN_CryptKey_Encrypt(dd->sessionKey,
			    msgbuf,
			    cryptbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "here");
    return -1;
  }
  DBG_INFO(0, "Encrypting with session key: done");

  return 0;
}



int GWEN_IPCXMLDialog_Decrypt(GWEN_HBCIDIALOG *d,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *decryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLDIALOGDATA *dd;
  GWEN_ERRORCODE err;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  if (GWEN_HBCICryptoContext_GetCryptKeyPtr(ctx)) {
    /* new session key, store it */
    GWEN_BUFFER *kbuf;
    GWEN_BUFFER *sbuf;
    GWEN_CRYPTKEY *key;
    char *km;

    DBG_NOTICE(0, "Storing new session key");
    sbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Buffer_AppendBytes(sbuf,
				GWEN_HBCICryptoContext_GetCryptKeyPtr(ctx),
				GWEN_HBCICryptoContext_GetCryptKeySize(ctx))){
      DBG_INFO(0, "here");
      GWEN_Buffer_free(sbuf);
      return -1;
    }
    kbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_Rewind(sbuf);
    err=GWEN_CryptKey_Decrypt(dd->localKey,
			      sbuf,
			      kbuf);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO(0, "here");
      GWEN_Buffer_free(sbuf);
      GWEN_Buffer_free(kbuf);
      return -1;
    }

    key=GWEN_CryptKey_Factory("DES");
    assert(key);
    km=(char*)malloc(16);
    assert(km);
    memmove(km,
	    GWEN_Buffer_GetStart(kbuf)+
	    GWEN_Buffer_GetUsedBytes(kbuf)-16,
	    16);

    GWEN_CryptKey_SetKeyData(key, km);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(kbuf);
  }

  /* now decrypt the message */
  err=GWEN_CryptKey_Decrypt(dd->sessionKey,
			    msgbuf,
			    decryptbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "here");
    return -1;
  }

  if (GWEN_HBCIDialog_UnpaddWithANSIX9_23(decryptbuf)) {
    DBG_INFO(0, "here");
    return -1;
  }

  return 0;
}



void GWEN_IPCXMLDialog_FreeData(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);
  GWEN_IPCXMLDialogData_free(dd);
}



const char *GWEN_IPCXMLDialog_GetServiceCode(GWEN_HBCIDIALOG *d){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  return dd->serviceCode;
}



void GWEN_IPCXMLDialog_SetServiceCode(GWEN_HBCIDIALOG *d,
                                      const char *s){
  GWEN_IPCXMLDIALOGDATA *dd;

  assert(d);
  assert(s);
  dd=(GWEN_IPCXMLDIALOGDATA*)GWEN_HBCIDialog_GetInheritorData(d);
  assert(dd);

  free(dd->serviceCode);
  dd->serviceCode=strdup(s);
}



GWEN_HBCIDIALOG *GWEN_IPCXMLDialog_new(GWEN_MSGENGINE *e){
  GWEN_HBCIDIALOG *d;
  GWEN_IPCXMLDIALOGDATA *dd;

  d=GWEN_HBCIDialog_new(e);
  dd=GWEN_IPCXMLDialogData_new();
  GWEN_HBCIDialog_SetPrepareCtxFn(d, GWEN_IPCXMLDialog_PrepareCTX);
  GWEN_HBCIDialog_SetSignFn(d, GWEN_IPCXMLDialog_Sign);
  GWEN_HBCIDialog_SetVerifyFn(d, GWEN_IPCXMLDialog_Verify);
  GWEN_HBCIDialog_SetEncryptFn(d, GWEN_IPCXMLDialog_Encrypt);
  GWEN_HBCIDialog_SetDecrpytFn(d, GWEN_IPCXMLDialog_Decrypt);
  GWEN_HBCIDialog_SetFreeDataFn(d, GWEN_IPCXMLDialog_FreeData);

  GWEN_HBCIDialog_SetInheritorData(d, dd);

  return d;
}











