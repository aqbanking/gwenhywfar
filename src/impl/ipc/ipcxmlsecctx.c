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

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  if (scd->remoteSignKey==0) {
    DBG_ERROR(0, "No remote sign key");
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

  scd->localSignKey=key;
}



void GWEN_IPCXMLSecCtx_SetLocalCryptKey(GWEN_SECCTX *sc,
                                        const GWEN_CRYPTKEY *key){
  GWEN_IPCXMLSECCTXDATA *scd;

  assert(sc);
  scd=(GWEN_IPCXMLSECCTXDATA*)GWEN_SecContext_GetData(sc);
  assert(scd);

  scd->localCryptKey=key;
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








