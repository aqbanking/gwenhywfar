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

#include "ipcxmlsecctx_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/directory.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



GWEN_IPCXMLSECCTXDATA *GWEN_IPCXMLSecCtxData_new(){
  GWEN_IPCXMLSECCTXDATA *d;

  GWEN_NEW_OBJECT(GWEN_IPCXMLSECCTXDATA, d);
  return d;
}



void GWEN_IPCXMLSecCtxData_free(GWEN_IPCXMLSECCTXDATA *d){
  if (d) {
    GWEN_CryptKey_free(d->sessionKey);
    free(d->serviceCode);
    free(d->securityId);
    GWEN_CryptKey_free(d->localSignKey);
    GWEN_CryptKey_free(d->localCryptKey);
    GWEN_CryptKey_free(d->remoteSignKey);
    GWEN_CryptKey_free(d->remoteCryptKey);
    free(d);
  }
}



GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetSessionKey(GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  return scd->sessionKey;
}



void GWEN_IPCXMLSecCtx_SetSessionKey(GWEN_SECCTX *sc,
                                     GWEN_CRYPTKEY *k){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  GWEN_CryptKey_free(scd->sessionKey);
  scd->sessionKey=k;
}



const GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetLocalSignKey(GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  return scd->localSignKey;
}



const GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetLocalCryptKey(GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  return scd->localCryptKey;
}



int GWEN_IPCXMLSecCtx_PrepareCTX(GWEN_SECCTX *sc,
                                 GWEN_HBCICRYPTOCONTEXT *ctx,
                                 int crypt){
  GWEN_IPCXMLSECCTXDATA *scd;
  GWEN_ERRORCODE err;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  if (crypt) {
    if (scd->remoteCryptKey==0) {
      DBG_ERROR(0, "No remote crypt key");
      return -1;
    }
    GWEN_HBCICryptoContext_SetKeySpec(ctx,
                                      GWEN_CryptKey_GetKeySpec(scd->remoteCryptKey));
    if (!(scd->sessionKey)) {
      GWEN_BUFFER *kbuf;
      GWEN_BUFFER *sbuf;
      int i;

      /* generate session key, if possible */
      DBG_NOTICE(0, "Generating session key");
      scd->sessionKey=GWEN_CryptKey_Factory("DES");
      assert(scd->sessionKey);
      err=GWEN_CryptKey_Generate(scd->sessionKey, 0);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO_ERR(0, err);
        return -1;
      }
      kbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_ReserveBytes(kbuf, 128);
      if (GWEN_Buffer_AppendBytes(kbuf,
				  GWEN_CryptKey_GetKeyData(scd->sessionKey),
				  16)) {
	DBG_INFO(0, "here");
	GWEN_Buffer_free(kbuf);
	return -1;
      }

      GWEN_Buffer_Rewind(kbuf);
      i=GWEN_CryptKey_GetChunkSize(scd->remoteCryptKey)-16;
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
      err=GWEN_CryptKey_Encrypt(scd->remoteCryptKey,
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
    if (scd->localSignKey==0) {
      DBG_ERROR(0, "No local sign key");
      return -1;
    }
    GWEN_HBCICryptoContext_SetKeySpec(ctx,
                                      GWEN_CryptKey_GetKeySpec(scd->localSignKey));
    /* sign */
    GWEN_HBCICryptoContext_SetSequenceNum(ctx,
                                          GWEN_IPCXMLSecCtx_GetLocalSignSeq(sc));
  }

  GWEN_HBCICryptoContext_SetMode(ctx, "RDH");
  if (scd->serviceCode)
    GWEN_HBCICryptoContext_SetServiceCode(ctx, scd->serviceCode);
  if (scd->securityId)
    GWEN_HBCICryptoContext_SetSecurityId(ctx,
                                         scd->securityId,
                                         strlen(scd->securityId)+1);
  DBG_INFO(0, "Context prepared");
  return 0;
}



int GWEN_IPCXMLSecCtx_Sign(GWEN_SECCTX *sc,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLSECCTXDATA *scd;
  GWEN_MD *md;
  GWEN_BUFFER *hashbuf;
  GWEN_ERRORCODE err;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  if (scd->localSignKey==0) {
    DBG_ERROR(0, "No local sign key");
    return -1;
  }

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
                              (const char*)GWEN_MD_GetDigestPtr(md),
                              GWEN_MD_GetDigestSize(md))) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    GWEN_MD_free(md);
    return -1;
  }
  GWEN_MD_free(md);

  /* padd */
  DBG_INFO(0, "Padding hash using ISO 9796");
  if (GWEN_SecContext_PaddWithISO9796(hashbuf)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    return -1;
  }

  /* sign hash */
  GWEN_Buffer_Rewind(hashbuf);
  err=GWEN_CryptKey_Sign(scd->localSignKey,
                         hashbuf,
                         signbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_Buffer_free(hashbuf);
    return err;
  }

  /* increment signature counter */
  scd->localSignSeq++;

  GWEN_Buffer_free(hashbuf);
  DBG_INFO(0, "Signing done");
  return 0;
}



int GWEN_IPCXMLSecCtx_Verify(GWEN_SECCTX *sc,
                             GWEN_BUFFER *msgbuf,
                             GWEN_BUFFER *signbuf,
                             GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLSECCTXDATA *scd;
  GWEN_MD *md;
  GWEN_BUFFER *hashbuf;
  GWEN_ERRORCODE err;
  unsigned int rseq;
  const GWEN_KEYSPEC *ks, *ks2;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  if (scd->remoteSignKey==0) {
    DBG_ERROR(0, "No remote sign key");
    return -1;
  }

  /* verify that the key is the one we know */
  ks=GWEN_HBCICryptoContext_GetKeySpec(ctx);
  assert(ks);
  ks2=GWEN_CryptKey_GetKeySpec(scd->remoteSignKey);
  assert(ks2);
  if (!(
        (GWEN_Text_Compare(GWEN_KeySpec_GetOwner(ks),
                           GWEN_KeySpec_GetOwner(ks2),1)==0) &&
        (GWEN_Text_Compare(GWEN_KeySpec_GetKeyName(ks),
                           GWEN_KeySpec_GetKeyName(ks2),1)==0) &&
        (GWEN_KeySpec_GetNumber(ks)==GWEN_KeySpec_GetNumber(ks2)) &&
        (GWEN_KeySpec_GetVersion(ks)==GWEN_KeySpec_GetVersion(ks2))
       )) {
    DBG_ERROR(0,
              "Remote sign key differs from that one used for signing");
    return -1;
  }


  /* check signature sequence number */
  rseq=GWEN_HBCICryptoContext_GetSequenceNum(ctx);
  if (rseq<=GWEN_IPCXMLSecCtx_GetRemoteSignSeq(sc)) {
    DBG_ERROR(0, "bad signature sequence number (%d<%d)",
              rseq, GWEN_IPCXMLSecCtx_GetRemoteSignSeq(sc));
    //return -1; /* comment this out for debugging purposes */
  }
  GWEN_IPCXMLSecCtx_SetRemoteSignSeq(sc, rseq);

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
                              (const char*)GWEN_MD_GetDigestPtr(md),
                              GWEN_MD_GetDigestSize(md))) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    GWEN_MD_free(md);
    return -1;
  }
  GWEN_MD_free(md);

  /* padd */
  DBG_INFO(0, "Padding hash using ISO 9796");
  if (GWEN_SecContext_PaddWithISO9796(hashbuf)) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(hashbuf);
    return -1;
  }

  /* verify hash */
  err=GWEN_CryptKey_Verify(scd->remoteSignKey,
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



int GWEN_IPCXMLSecCtx_Encrypt(GWEN_SECCTX *sc,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *cryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLSECCTXDATA *scd;
  GWEN_ERRORCODE err;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  if (!(scd->sessionKey)) {
    DBG_ERROR(0, "No session key");
    return -1;
  }

  DBG_INFO(0, "Padding with ANSI X9.23");
  if (GWEN_SecContext_PaddWithANSIX9_23(msgbuf)) {
    DBG_INFO(0, "here");
    return -1;
  }
  DBG_INFO(0, "Padding with ANSI X9.23: done");

  DBG_INFO(0, "Encrypting with session key");
  err=GWEN_CryptKey_Encrypt(scd->sessionKey,
			    msgbuf,
			    cryptbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "here");
    return -1;
  }
  DBG_INFO(0, "Encrypting with session key: done");

  return 0;
}



int GWEN_IPCXMLSecCtx_Decrypt(GWEN_SECCTX *sc,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *decryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx){
  GWEN_IPCXMLSECCTXDATA *scd;
  GWEN_ERRORCODE err;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  if (GWEN_HBCICryptoContext_GetCryptKeyPtr(ctx)) {
    /* new session key, store it */
    GWEN_BUFFER *kbuf;
    GWEN_BUFFER *sbuf;
    GWEN_CRYPTKEY *key;
    const GWEN_KEYSPEC *ks, *ks2;
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
    if (scd->localCryptKey==0) {
      DBG_ERROR(0, "No local crypt key");
      return -1;
    }

    /* verify that the key is the one we know */
    ks=GWEN_HBCICryptoContext_GetKeySpec(ctx);
    assert(ks);
    ks2=GWEN_CryptKey_GetKeySpec(scd->localCryptKey);
    assert(ks2);
    if (!(
          (GWEN_Text_Compare(GWEN_KeySpec_GetOwner(ks),
                             GWEN_KeySpec_GetOwner(ks2),1)==0) &&
          (GWEN_Text_Compare(GWEN_KeySpec_GetKeyName(ks),
                             GWEN_KeySpec_GetKeyName(ks2),1)==0) &&
          (GWEN_KeySpec_GetNumber(ks)==GWEN_KeySpec_GetNumber(ks2)) &&
          (GWEN_KeySpec_GetVersion(ks)==GWEN_KeySpec_GetVersion(ks2))
         )) {
      DBG_ERROR(0,
                "Local crypt key differs from that one used for encryption");
      return -1;
    }

    err=GWEN_CryptKey_Decrypt(scd->localCryptKey,
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
    GWEN_CryptKey_free(scd->sessionKey);
    scd->sessionKey=key;
  }

  /* now decrypt the message */
  err=GWEN_CryptKey_Decrypt(scd->sessionKey,
			    msgbuf,
			    decryptbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "here");
    return -1;
  }

  if (GWEN_SecContext_UnpaddWithANSIX9_23(decryptbuf)) {
    DBG_INFO(0, "here");
    return -1;
  }

  return 0;
}



void GWEN_IPCXMLSecCtx_FreeData(GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  DBG_ERROR(0, "Freeing SecCtx-Data");
  GWEN_IPCXMLSecCtxData_free(scd);
}



void GWEN_IPCXMLSecCtx_Reset(GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  GWEN_CryptKey_free(scd->sessionKey);
  scd->sessionKey=0;
}



const char *GWEN_IPCXMLSecCtx_GetServiceCode(GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  return scd->serviceCode;
}



void GWEN_IPCXMLSecCtx_SetServiceCode(GWEN_SECCTX *sc,
                                      const char *s){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  assert(s);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  free(scd->serviceCode);
  scd->serviceCode=strdup(s);
}



const char *GWEN_IPCXMLSecCtx_GetSecurityId(GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  return scd->securityId;
}



void GWEN_IPCXMLSecCtx_SetSecurityId(GWEN_SECCTX *sc,
                                     const char *s){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  assert(s);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  free(scd->securityId);
  scd->securityId=strdup(s);
}



unsigned int GWEN_IPCXMLSecCtx_GetLocalSignSeq(GWEN_SECCTX *sc) {
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  return scd->localSignSeq;
}



void GWEN_IPCXMLSecCtx_SetLocalSignSeq(GWEN_SECCTX *sc,
                                       unsigned int i) {
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  scd->localSignSeq=i;
}



unsigned int GWEN_IPCXMLSecCtx_GetRemoteSignSeq(GWEN_SECCTX *sc) {
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  return scd->remoteSignSeq;
}



void GWEN_IPCXMLSecCtx_SetRemoteSignSeq(GWEN_SECCTX *sc,
                                        unsigned int i) {
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  scd->remoteSignSeq=i;
}



GWEN_SECCTX *GWEN_IPCXMLSecCtx_new(const char *localName,
                                   const char *remoteName){
  GWEN_SECCTX *sc;
  GWEN_IPCXMLSECCTXDATA *scd;

  sc=GWEN_SecContext_new(localName, remoteName);
  scd=GWEN_IPCXMLSecCtxData_new();
  scd->localSignSeq=1;
  scd->remoteSignSeq=0;

  GWEN_SecContext_SetPrepareCtxFn(sc, GWEN_IPCXMLSecCtx_PrepareCTX);
  GWEN_SecContext_SetSignFn(sc, GWEN_IPCXMLSecCtx_Sign);
  GWEN_SecContext_SetVerifyFn(sc, GWEN_IPCXMLSecCtx_Verify);
  GWEN_SecContext_SetEncryptFn(sc, GWEN_IPCXMLSecCtx_Encrypt);
  GWEN_SecContext_SetDecrpytFn(sc, GWEN_IPCXMLSecCtx_Decrypt);
  GWEN_SecContext_SetFreeDataFn(sc, GWEN_IPCXMLSecCtx_FreeData);
  GWEN_SecContext_SetFromDbFn(sc, GWEN_IPCXMLSecCtx_FromDB);
  GWEN_SecContext_SetToDbFn(sc, GWEN_IPCXMLSecCtx_ToDB);

  GWEN_SecContext_SetData(sc, scd);

  return sc;
}


int GWEN_IPCXMLSecCtx_FromDB(GWEN_SECCTX *sc,
                             GWEN_DB_NODE *db){
  const char *p;
  GWEN_DB_NODE *gr;
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=GWEN_IPCXMLSecCtxData_new();
  assert(scd);
  scd->localSignSeq=GWEN_DB_GetIntValue(db, "localsignseq", 0, 0);
  scd->remoteSignSeq=GWEN_DB_GetIntValue(db, "remotesignseq", 0, 0);
  p=GWEN_DB_GetCharValue(db, "serviceCode", 0, 0);
  if (p)
    GWEN_IPCXMLSecCtx_SetServiceCode(sc, p);
  p=GWEN_DB_GetCharValue(db, "securityId", 0, 0);
  if (p)
    GWEN_IPCXMLSecCtx_SetSecurityId(sc, p);

  /* read the keys */
  gr=GWEN_DB_GetGroup(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "remotesignkey");
  if (gr) {
    GWEN_CryptKey_free(scd->remoteSignKey);
    scd->remoteSignKey=GWEN_CryptKey_FromDb(gr);
    if (scd->remoteSignKey==0) {
      DBG_ERROR(0, "Could not read key");
      return 0;
    }
  } /* if "remotesignkey" group */

  gr=GWEN_DB_GetGroup(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "remotecryptkey");
  if (gr) {
    GWEN_CryptKey_free(scd->remoteCryptKey);
    scd->remoteCryptKey=GWEN_CryptKey_FromDb(gr);
    if (scd->remoteCryptKey==0) {
      DBG_ERROR(0, "Could not read key");
      return 0;
    }
  } /* if "remotecryptkey" group */

  GWEN_SecContext_SetData(sc, scd);
  return 0;
}



int GWEN_IPCXMLSecCtx_ToDB(GWEN_SECCTX *sc,
                           GWEN_DB_NODE *db){
  GWEN_IPCXMLSECCTXDATA *scd;
  GWEN_DB_NODE *gr;
  GWEN_ERRORCODE err;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "localsignseq",
                      scd->localSignSeq);
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "remotesignseq",
                      scd->remoteSignSeq);
  if (scd->remoteSignKey) {
    gr=GWEN_DB_GetGroup(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_PATH_FLAGS_CREATE_GROUP,
                        "remotesignkey");
    assert(gr);
    err=GWEN_CryptKey_ToDb(scd->remoteSignKey, gr, 1);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      return -1;
    }
  }

  if (scd->remoteCryptKey) {
    gr=GWEN_DB_GetGroup(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_PATH_FLAGS_CREATE_GROUP,
                        "remotecryptkey");
    assert(gr);
    err=GWEN_CryptKey_ToDb(scd->remoteCryptKey, gr, 1);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      return -1;
    }
  }

  return 0;
}



void GWEN_IPCXMLSecCtx_SetLocalSignKey(GWEN_SECCTX *sc,
                                       const GWEN_CRYPTKEY *key){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  assert(key);
  if (scd->localSignKey)
    GWEN_CryptKey_free(scd->localSignKey);
  scd->localSignKey=GWEN_CryptKey_dup(key);
}



void GWEN_IPCXMLSecCtx_SetLocalCryptKey(GWEN_SECCTX *sc,
                                        const GWEN_CRYPTKEY *key){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  assert(key);
  if (scd->localCryptKey)
    GWEN_CryptKey_free(scd->localCryptKey);
  scd->localCryptKey=GWEN_CryptKey_dup(key);
}



void GWEN_IPCXMLSecCtx_SetRemoteSignKey(GWEN_SECCTX *sc,
                                        GWEN_CRYPTKEY *key){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  GWEN_CryptKey_free(scd->remoteSignKey);
  scd->remoteSignKey=key;
}



void GWEN_IPCXMLSecCtx_SetRemoteCryptKey(GWEN_SECCTX *sc,
                                         GWEN_CRYPTKEY *key){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  GWEN_CryptKey_free(scd->remoteCryptKey);
  scd->remoteCryptKey=key;
}






GWEN_SECCTX *GWEN_IPCXMLSecCtxtMgr_FindContext(GWEN_SECCTX_MANAGER *scm,
                                               const char *localName,
                                               const char *remoteName){
  GWEN_LIST_ITERATOR *it;
  GWEN_IPCXMLSECCTXMGRDATA *scmd;

  assert(scm);
  assert(localName);
  assert(remoteName);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);
  assert(scmd->contextList);

  DBG_WARN(0, "Looking for context \"%s:%s\"", localName, remoteName);
  it=GWEN_List_First(scmd->contextList);
  if (it) {
    GWEN_SECCTX *sc;

    sc=(GWEN_SECCTX*)GWEN_ListIterator_Data(it);
    while(sc) {
      if ((GWEN_Text_Compare(GWEN_SecContext_GetLocalName(sc),
                             localName, 1)==0) &&
          (GWEN_Text_Compare(GWEN_SecContext_GetRemoteName(sc),
                             remoteName, 1)==0)){
        GWEN_ListIterator_free(it);
        return sc;
      }
      sc=(GWEN_SECCTX*)GWEN_ListIterator_Next(it);
    } /* while */
  }
  GWEN_ListIterator_free(it);

  DBG_INFO(0, "Context \"%s:%s\" not found.", localName, remoteName);
  return 0;
}



int GWEN_IPCXMLSecCtxMgr_LockFile(const char *path) {
  int fid;
  struct flock fl;

  if (GWEN_Directory_GetPath(path, GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(0, "Could not create path \"%s\"", path);
    return -1;
  }

  /* file exists, try to open it */
  fid=open(path, O_RDWR);
  if (fid==-1) {
    DBG_ERROR(0, "Error on open(%s): %s",
              path, strerror(errno));
    return -1;
  }

  /* now lock it (wait for lock if necessary) */
  fl.l_type=F_WRLCK;
  fl.l_whence=SEEK_SET;
  fl.l_start=0;
  fl.l_len=0;
  if (fcntl(fid, F_SETLKW, &fl)) {
    close(fid);
    DBG_ERROR(0, "Error on fcntl(%s, F_SETLKW): %s",
              path, strerror(errno));
    return -1;
  }
  return fid;
}



int GWEN_IPCXMLSecCtxMgr_UnlockFile(int fid) {
  struct flock fl;

  /* unlock file */
  fl.l_type=F_UNLCK;
  fl.l_whence=SEEK_SET;
  fl.l_start=0;
  fl.l_len=0;
  if (fcntl(fid, F_SETLKW, &fl)) {
    close(fid);
    DBG_ERROR(0, "Error on fcntl(%d, F_SETLKW): %s",
              fid, strerror(errno));
    return -1;
  }
  if (close(fid)) {
    DBG_ERROR(0, "Error on close(%d): %s",
              fid, strerror(errno));
    return -1;
  }
  return 0;
}




GWEN_SECCTX *GWEN_IPCXMLSecCtxMgr_GetContext(GWEN_SECCTX_MANAGER *scm,
                                             const char *localName,
                                             const char *remoteName){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;
  char path[256];

  assert(scm);
  assert(localName);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);
  assert(scmd->dir);

  if (remoteName) {
    /* try to read the file */
    DBG_INFO(0, "Trying to read context \"%s:%s\"",
             localName, remoteName);
    if (strlen(scmd->dir)+
        strlen(localName)+
        strlen(remoteName)+
        4>sizeof(path)) {
      DBG_ERROR(0, "Path too long");
      return 0;
    }
    strcpy(path, scmd->dir);
    strcat(path, "/");
    strcat(path, localName);
    strcat(path, "/");
    strcat(path, remoteName);
    strcat(path, ".ctx");
    if (GWEN_Directory_GetPath(path,
                               GWEN_PATH_FLAGS_VARIABLE |
                               GWEN_PATH_FLAGS_PATHMUSTEXIST)) {
      GWEN_SECCTX *ctx;

      DBG_INFO(0,
               "File for context \"%s:%s\" not found, will check for tmp",
               localName, remoteName);
      ctx=GWEN_IPCXMLSecCtxtMgr_FindContext(scm,
                                            localName,
                                            remoteName);
      if (!ctx) {
        DBG_INFO(0, "Context \"%s:%s\" not found",
                 localName, remoteName);
        return 0;
      }
      GWEN_SecContext_SetFlags(ctx, GWEN_SECCTX_FLAGS_TEMP);
      if (GWEN_IPCXMLSecCtx_GetLocalSignKey(ctx)==0 &&
          scmd->localSignKey)
        GWEN_IPCXMLSecCtx_SetLocalSignKey(ctx,scmd->localSignKey);
      if (GWEN_IPCXMLSecCtx_GetLocalCryptKey(ctx)==0 &&
          scmd->localCryptKey)
        GWEN_IPCXMLSecCtx_SetLocalCryptKey(ctx,scmd->localCryptKey);
      return ctx;
    }
    else {
      GWEN_DB_NODE *db;
      GWEN_SECCTX *ctx;
      int fid;

      DBG_INFO(0, "Trying to load file for context \"%s:%s\"",
               localName, remoteName);
      /* file exists, try to lock it */
      fid=GWEN_IPCXMLSecCtxMgr_LockFile(path);
      if (fid==-1) {
        return 0;
      }

      /* try to read the file */
      db=GWEN_DB_Group_new("context");
      if (GWEN_DB_ReadFile(db, path,
                           GWEN_DB_FLAGS_DEFAULT |
                           GWEN_PATH_FLAGS_CREATE_GROUP)) {
        DBG_ERROR(0, "Error reading file \"%s\"", path);
        GWEN_DB_Group_free(db);
        GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
        return 0;
      }

      /* create context */
      ctx=GWEN_IPCXMLSecCtx_new(localName, remoteName);
      if (GWEN_SecContext_FromDB(ctx, db)) {
        GWEN_SecContext_free(ctx);
        DBG_ERROR(0, "Could not read context \"%s:%s\"from DB",
                  localName, remoteName);
        GWEN_DB_Group_free(db);
        GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
        return 0;
      }
      /* store lock id (which actually is the file descriptor) */
      GWEN_SecContext_SetLockId(ctx, fid);

      /* store local files if they are not already set */
      if (GWEN_IPCXMLSecCtx_GetLocalSignKey(ctx)==0 &&
          scmd->localSignKey)
        GWEN_IPCXMLSecCtx_SetLocalSignKey(ctx,scmd->localSignKey);
      if (GWEN_IPCXMLSecCtx_GetLocalCryptKey(ctx)==0 &&
          scmd->localCryptKey)
        GWEN_IPCXMLSecCtx_SetLocalCryptKey(ctx,scmd->localCryptKey);
      /* done */
      return ctx;
    }
  }
  else {
    GWEN_SECCTX *ctx;

    /* create a local-only context */
    DBG_INFO(0, "Creating local-only context");
    ctx=GWEN_IPCXMLSecCtx_new(localName, remoteName);
    if (scmd->localSignKey)
      GWEN_IPCXMLSecCtx_SetLocalSignKey(ctx,scmd->localSignKey);
    if (scmd->localCryptKey)
      GWEN_IPCXMLSecCtx_SetLocalCryptKey(ctx,scmd->localCryptKey);
    return ctx;
  }
}



int GWEN_IPCXMLSecCtxMgr_AddContext(GWEN_SECCTX_MANAGER *scm,
                                    GWEN_SECCTX *sc,
                                    int tmp){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;
  char path[256];
  const char *localName;
  const char *remoteName;
  GWEN_DB_NODE *db;
  int fid;

  assert(scm);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);

  localName=GWEN_SecContext_GetLocalName(sc);
  remoteName=GWEN_SecContext_GetRemoteName(sc);

  if (!remoteName) {
    DBG_ERROR(0, "Will not add local-only context \"%s\"",
              localName);
    return -1;
  }

  /* check for the existence of a file for the context */
  DBG_INFO(0, "Trying to read context \"%s:%s\"",
           localName, remoteName);
  if (strlen(scmd->dir)+
      strlen(localName)+
      strlen(remoteName)+
      4>sizeof(path)) {
    DBG_ERROR(0, "Path too long");
    return 0;
  }
  strcpy(path, scmd->dir);
  strcat(path, "/");
  strcat(path, localName);
  strcat(path, "/");
  strcat(path, remoteName);
  strcat(path, ".ctx");
  if (GWEN_Directory_GetPath(path,
                             GWEN_PATH_FLAGS_VARIABLE |
                             GWEN_PATH_FLAGS_PATHMUSTEXIST)==0) {
    DBG_ERROR(0, "Context \"%s:%s\" already exists, not adding it",
              localName, remoteName);
    return -1;
  }

  if (GWEN_IPCXMLSecCtxtMgr_FindContext(scm, localName, remoteName)) {
    DBG_ERROR(0, "Context \"%s:%s\" already exists locally, not adding it",
              localName, remoteName);
    return -1;
  }

  if (tmp) {
    DBG_INFO(0, "Adding temporary context \"%s:%s\"",
             localName, remoteName);
    GWEN_List_PushBack(scmd->contextList, sc);
    return 0;
  }

  fid=GWEN_IPCXMLSecCtxMgr_LockFile(path);
  if (fid==-1) {
    DBG_ERROR(0, "Could not lock context file for \"%s:%s\"",
              localName, remoteName);
    return -1;
  }

  db=GWEN_DB_Group_new("context");
  if (GWEN_SecContext_ToDB(sc, db)) {
    DBG_ERROR(0, "Could not write context \"%s:%s\" to DB",
              localName, remoteName);
    GWEN_DB_Group_free(db);
    GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
    return -1;
  }

  if (GWEN_DB_WriteFile(db, path,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_ERROR(0, "Error writing file \"%s\"", path);
    GWEN_DB_Group_free(db);
    GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
    return -1;
  }

  GWEN_DB_Group_free(db);
  GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
  GWEN_SecContext_free(sc);
  return 0;
}



int GWEN_IPCXMLSecCtxMgr_DelContext(GWEN_SECCTX_MANAGER *scm,
                                    GWEN_SECCTX *sc){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;
  char path[256];
  const char *localName;
  const char *remoteName;

  assert(scm);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);

  localName=GWEN_SecContext_GetLocalName(sc);
  remoteName=GWEN_SecContext_GetRemoteName(sc);

  /* try to read the file */
  DBG_INFO(0, "Trying to read context \"%s:%s\"",
           localName, remoteName);
  if (strlen(scmd->dir)+
      strlen(localName)+
      strlen(remoteName)+
      4>sizeof(path)) {
    DBG_ERROR(0, "Path too long");
    return 0;
  }
  strcpy(path, scmd->dir);
  strcat(path, "/");
  strcat(path, localName);
  strcat(path, "/");
  strcat(path, remoteName);
  strcat(path, ".ctx");
  if (GWEN_Directory_GetPath(path,
                             GWEN_PATH_FLAGS_VARIABLE |
                             GWEN_PATH_FLAGS_PATHMUSTEXIST)==0) {
    int fid;

    fid=GWEN_SecContext_GetLockId(sc);

    /* file exists, remove it */
    DBG_INFO(0, "Removing file for context \"%s:%s\"",
             localName, remoteName);
    if (unlink(path)) {
      DBG_ERROR(0, "Error on unlink(%s): %s",
                path, strerror(errno));
      GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
      return -1;
    }
    GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
    GWEN_SecContext_free(sc);
    return 0;
  }
  else {
    GWEN_LIST_ITERATOR *it;

    /* remove temporary context */
    it=GWEN_List_First(scmd->contextList);
    if (it) {
      GWEN_SECCTX *ctx;

      ctx=(GWEN_SECCTX*)GWEN_ListIterator_Data(it);
      while(ctx) {
        if ((GWEN_Text_Compare(GWEN_SecContext_GetLocalName(ctx),
                               localName, 1)==0) &&
            (GWEN_Text_Compare(GWEN_SecContext_GetRemoteName(ctx),
                               remoteName, 1)==0)){
          GWEN_ListIterator_free(it);
          GWEN_SecContext_free(ctx);
          return 0;
        }
        ctx=(GWEN_SECCTX*)GWEN_ListIterator_Next(it);
      } /* while */
    }
    GWEN_ListIterator_free(it);
    DBG_INFO(0, "Context \"%s:%s\" not found",
             localName, remoteName);
    return -1;
  }
}



int GWEN_IPCXMLSecCtxMgr_ReleaseContext(GWEN_SECCTX_MANAGER *scm,
                                        GWEN_SECCTX *sc,
                                        int aban){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;
  char path[256];
  const char *localName;
  const char *remoteName;

  assert(scm);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);

  localName=GWEN_SecContext_GetLocalName(sc);
  remoteName=GWEN_SecContext_GetRemoteName(sc);

  if (!remoteName) {
    DBG_INFO(0, "Releasing local context \"%s\"", localName);
    GWEN_SecContext_free(sc);
    return 0;
  }

  if (GWEN_SecContext_GetFlags(sc) & GWEN_SECCTX_FLAGS_TEMP) {
    if (GWEN_IPCXMLSecCtxtMgr_FindContext(scm, localName, remoteName)==0) {
      DBG_ERROR(0, "Context \"%s:%s\" not found",
                localName, remoteName);
      return -1;
    }
    GWEN_SecContext_free(sc);
    return 0;
  }
  else {
    /* try to write the file */
    if (strlen(scmd->dir)+
        strlen(localName)+
        strlen(remoteName)+
        4>sizeof(path)) {
      DBG_ERROR(0, "Path too long");
      return 0;
    }
    strcpy(path, scmd->dir);
    strcat(path, "/");
    strcat(path, localName);
    strcat(path, "/");
    strcat(path, remoteName);
    strcat(path, ".ctx");
    if (GWEN_Directory_GetPath(path,
                               GWEN_PATH_FLAGS_VARIABLE |
                               GWEN_PATH_FLAGS_PATHMUSTEXIST)==0) {
      int fid;

      fid=GWEN_SecContext_GetLockId(sc);

      /* file exists, save it */
      if (!aban) {
        GWEN_DB_NODE *db;

        /* only write file if not in abandon mode */
        db=GWEN_DB_Group_new("context");
        if (GWEN_SecContext_ToDB(sc, db)) {
          DBG_ERROR(0, "Could not write context \"%s:%s\" to DB",
                    localName, remoteName);
          GWEN_DB_Group_free(db);
          GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
          return -1;
        }

        if (GWEN_DB_WriteFile(db, path,
                              GWEN_DB_FLAGS_DEFAULT |
                              GWEN_PATH_FLAGS_CREATE_GROUP)) {
          DBG_ERROR(0, "Error writing file \"%s\"", path);
          GWEN_DB_Group_free(db);
          GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
          return -1;
        }
      } /* if not abandon */
      else {
        DBG_INFO(0, "Abandoning context \"%s:%s\"",
                 localName, remoteName);
      }

      GWEN_IPCXMLSecCtxMgr_UnlockFile(fid);
      DBG_INFO(0, "Context \"%s:%s\" released",
               localName, remoteName);
      GWEN_SecContext_free(sc);
      return 0;
    }
    else {
      /* file does not exist */
      DBG_ERROR(0, "Context \"%s:%s\" not found",
                localName, remoteName);
      return -1;
    }
  }
}



GWEN_IPCXMLSECCTXMGRDATA *GWEN_IPCXMLSecCtxMgrData_new() {
  GWEN_IPCXMLSECCTXMGRDATA *scmd;

  GWEN_NEW_OBJECT(GWEN_IPCXMLSECCTXMGRDATA, scmd);
  scmd->contextList=GWEN_List_new();
  return scmd;
};



void GWEN_IPCXMLSecCtxMgrData_free(GWEN_SECCTX_MANAGER *scm) {
  if (scm) {
    GWEN_IPCXMLSECCTXMGRDATA *scmd;
    GWEN_LIST_ITERATOR *it;

    scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);

    free(scmd->dir);
    GWEN_CryptKey_free(scmd->localSignKey);
    GWEN_CryptKey_free(scmd->localCryptKey);

    /* free all temporary contexts */
    it=GWEN_List_First(scmd->contextList);
    if (it) {
      GWEN_SECCTX *sc;

      sc=(GWEN_SECCTX*)GWEN_ListIterator_Data(it);
      while(sc) {
        GWEN_SecContext_free(sc);
        sc=(GWEN_SECCTX*)GWEN_ListIterator_Next(it);
      } /* while */
      GWEN_ListIterator_free(it);
    }
    GWEN_List_free(scmd->contextList);
  } /* if scm */
}



GWEN_SECCTX_MANAGER *GWEN_IPCXMLSecCtxMgr_new(const char *serviceCode,
                                              const char *dir){
  GWEN_SECCTX_MANAGER *scm;
  GWEN_IPCXMLSECCTXMGRDATA *scmd;

  assert(serviceCode);
  assert(dir);

  scm=GWEN_SecContextMgr_new(serviceCode);
  scmd=GWEN_IPCXMLSecCtxMgrData_new();
  GWEN_SecContextMgr_SetData(scm, scmd);
  scmd->dir=strdup(dir);

  GWEN_SecContextMgr_SetGetFn(scm, GWEN_IPCXMLSecCtxMgr_GetContext);
  GWEN_SecContextMgr_SetAddFn(scm, GWEN_IPCXMLSecCtxMgr_AddContext);
  GWEN_SecContextMgr_SetDelFn(scm, GWEN_IPCXMLSecCtxMgr_DelContext);
  GWEN_SecContextMgr_SetReleaseFn(scm, GWEN_IPCXMLSecCtxMgr_ReleaseContext);
  GWEN_SecContextMgr_SetFreeDataFn(scm, GWEN_IPCXMLSecCtxMgrData_free);

  return scm;
}



void GWEN_IPCXMLSecCtxMgr_SetLocalSignKey(GWEN_SECCTX_MANAGER *scm,
                                          const GWEN_CRYPTKEY *key){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;

  assert(scm);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);

  assert(key);
  if (scmd->localSignKey)
    GWEN_CryptKey_free(scmd->localSignKey);
  scmd->localSignKey=GWEN_CryptKey_dup(key);
}



void GWEN_IPCXMLSecCtxMgr_SetLocalCryptKey(GWEN_SECCTX_MANAGER *scm,
                                           const GWEN_CRYPTKEY *key){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;

  assert(scm);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);

  assert(key);
  if (scmd->localCryptKey)
    GWEN_CryptKey_free(scmd->localCryptKey);
  scmd->localCryptKey=GWEN_CryptKey_dup(key);
}



const GWEN_CRYPTKEY*
GWEN_IPCXMLSecCtxMgr_GetLocalSignKey(GWEN_SECCTX_MANAGER *scm){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;

  assert(scm);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);

  return scmd->localSignKey;
}



const GWEN_CRYPTKEY*
GWEN_IPCXMLSecCtxMgr_GetLocalCryptKey(GWEN_SECCTX_MANAGER *scm){
  GWEN_IPCXMLSECCTXMGRDATA *scmd;

  assert(scm);
  scmd=(GWEN_IPCXMLSECCTXMGRDATA*)GWEN_SecContextMgr_GetData(scm);
  assert(scmd);

  return scmd->localCryptKey;
}








