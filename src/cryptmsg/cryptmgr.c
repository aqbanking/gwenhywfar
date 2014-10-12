/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "cryptmgr_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwentime.h>

#include <gwenhywfar/crypthead.h>
#include <gwenhywfar/sighead.h>
#include <gwenhywfar/sigtail.h>
#include <gwenhywfar/tag16.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>




GWEN_INHERIT_FUNCTIONS(GWEN_CRYPTMGR)



GWEN_CRYPTMGR *GWEN_CryptMgr_new(void) {
  GWEN_CRYPTMGR *cm;

  GWEN_NEW_OBJECT(GWEN_CRYPTMGR, cm);
  GWEN_INHERIT_INIT(GWEN_CRYPTMGR, cm);

  return cm;
}



void GWEN_CryptMgr_free(GWEN_CRYPTMGR *cm) {
  if (cm) {
    GWEN_INHERIT_FINI(GWEN_CRYPTMGR, cm);
    free(cm->localKeyName);
    free(cm->peerKeyName);

    GWEN_FREE_OBJECT(cm);
  }
}



const char *GWEN_CryptMgr_GetLocalKeyName(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->localKeyName;
}



void GWEN_CryptMgr_SetLocalKeyName(GWEN_CRYPTMGR *cm, const char *s) {
  assert(cm);
  free(cm->localKeyName);
  if (s) cm->localKeyName=strdup(s);
  else cm->localKeyName=NULL;
}



int GWEN_CryptMgr_GetLocalKeyNumber(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->localKeyNumber;
}



void GWEN_CryptMgr_SetLocalKeyNumber(GWEN_CRYPTMGR *cm, int i) {
  assert(cm);
  cm->localKeyNumber=i;
}



int GWEN_CryptMgr_GetLocalKeyVersion(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->localKeyVersion;
}



void GWEN_CryptMgr_SetLocalKeyVersion(GWEN_CRYPTMGR *cm, int i) {
  assert(cm);
  cm->localKeyVersion=i;
}



const char *GWEN_CryptMgr_GetPeerKeyName(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->peerKeyName;
}



void GWEN_CryptMgr_SetPeerKeyName(GWEN_CRYPTMGR *cm, const char *s) {
  assert(cm);
  free(cm->peerKeyName);
  if (s) cm->peerKeyName=strdup(s);
  else cm->peerKeyName=NULL;
}



int GWEN_CryptMgr_GetPeerKeyNumber(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->peerKeyNumber;
}



void GWEN_CryptMgr_SetPeerKeyNumber(GWEN_CRYPTMGR *cm, int i) {
  assert(cm);
  cm->peerKeyNumber=i;
}



int GWEN_CryptMgr_GetPeerKeyVersion(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->peerKeyVersion;
}



void GWEN_CryptMgr_SetPeerKeyVersion(GWEN_CRYPTMGR *cm, int i) {
  assert(cm);
  cm->peerKeyVersion=i;
}



int GWEN_CryptMgr_GetCryptProfile(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->cryptProfile;
}



void GWEN_CryptMgr_SetCryptProfile(GWEN_CRYPTMGR *cm, int i) {
  assert(cm);
  cm->cryptProfile=i;
}



int GWEN_CryptMgr_GetSignatureProfile(const GWEN_CRYPTMGR *cm) {
  assert(cm);
  return cm->signatureProfile;
}



void GWEN_CryptMgr_SetSignatureProfile(GWEN_CRYPTMGR *cm, int i) {
  assert(cm);
  cm->signatureProfile=i;
}





int GWEN_CryptMgr_SignData(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  assert(cm);
  if (cm->signDataFn)
    return cm->signDataFn(cm, pData, lData, dbuf);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_CryptMgr_EncryptKey(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  assert(cm);
  if (cm->encryptKeyFn)
    return cm->encryptKeyFn(cm, pData, lData, dbuf);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_CryptMgr_VerifyData(GWEN_CRYPTMGR *cm,
                             const uint8_t *pData, uint32_t lData,
                             const uint8_t *pSignature, uint32_t lSignature) {
  assert(cm);
  if (cm->verifyDataFn)
    return cm->verifyDataFn(cm, pData, lData, pSignature, lSignature);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_CryptMgr_DecryptKey(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  assert(cm);
  if (cm->decryptKeyFn)
    return cm->decryptKeyFn(cm, pData, lData, dbuf);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_CRYPTMGR_SIGNDATA_FN GWEN_CryptMgr_SetSignDataFn(GWEN_CRYPTMGR *cm,
    GWEN_CRYPTMGR_SIGNDATA_FN f) {
  GWEN_CRYPTMGR_SIGNDATA_FN of;

  assert(cm);
  of=cm->signDataFn;
  cm->signDataFn=f;
  return of;
}



GWEN_CRYPTMGR_VERIFYDATA_FN GWEN_CryptMgr_SetVerifyDataFn(GWEN_CRYPTMGR *cm,
    GWEN_CRYPTMGR_VERIFYDATA_FN f) {
  GWEN_CRYPTMGR_VERIFYDATA_FN of;

  assert(cm);
  of=cm->verifyDataFn;
  cm->verifyDataFn=f;
  return of;
}



GWEN_CRYPTMGR_ENCRYPTKEY_FN GWEN_CryptMgr_SetEncryptKeyFn(GWEN_CRYPTMGR *cm,
    GWEN_CRYPTMGR_ENCRYPTKEY_FN f) {
  GWEN_CRYPTMGR_ENCRYPTKEY_FN of;

  assert(cm);
  of=cm->encryptKeyFn;
  cm->encryptKeyFn=f;
  return of;
}



GWEN_CRYPTMGR_DECRYPTKEY_FN GWEN_CryptMgr_SetDecryptKeyFn(GWEN_CRYPTMGR *cm,
    GWEN_CRYPTMGR_DECRYPTKEY_FN f) {
  GWEN_CRYPTMGR_DECRYPTKEY_FN of;

  assert(cm);
  of=cm->decryptKeyFn;
  cm->decryptKeyFn=f;
  return of;
}



int GWEN_CryptMgr_Sign(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  GWEN_SIGHEAD *sh;
  GWEN_SIGTAIL *st;
  GWEN_TIME *ti;
  uint32_t pos;
  uint32_t shPos;
  uint8_t *p;
  uint32_t l;
  int rv;
  GWEN_BUFFER *sigbuf;

  assert(cm);
  GWEN_Buffer_AppendByte(dbuf, GWEN_CRYPTMGR_TLV_SIGNEDOBJECT);
  pos=GWEN_Buffer_GetPos(dbuf);
  GWEN_Buffer_AppendByte(dbuf, 0);
  GWEN_Buffer_AppendByte(dbuf, 0);

  /* prepare signature head */
  sh=GWEN_SigHead_new();
  GWEN_SigHead_SetKeyName(sh, cm->localKeyName);
  GWEN_SigHead_SetKeyNumber(sh, cm->localKeyNumber);
  GWEN_SigHead_SetKeyVersion(sh, cm->localKeyVersion);
  ti=GWEN_CurrentTime();
  GWEN_SigHead_SetDateTime(sh, ti);
  GWEN_Time_free(ti);
  GWEN_SigHead_SetSignatureProfile(sh, cm->signatureProfile);
  GWEN_SigHead_SetSignatureNumber(sh, 1);

  /* write signature head to buffer */
  shPos=GWEN_Buffer_GetPos(dbuf);
  rv=GWEN_SigHead_toBuffer(sh, dbuf, GWEN_CRYPTMGR_TLV_SIGHEAD);
  GWEN_SigHead_free(sh);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* write data to buffer */
  if (pData && lData)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPTMGR_TLV_SIGDATA,
                                (const char*)pData,
                                lData,
                                dbuf);

  /* sign data: signature head TLV + data TLV */
  sigbuf=GWEN_Buffer_new(0, 300, 0, 1);
  p=((uint8_t*)GWEN_Buffer_GetStart(dbuf))+shPos;
  l=GWEN_Buffer_GetPos(dbuf)-shPos;
  rv=GWEN_CryptMgr_SignData(cm, p, l, sigbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sigbuf);
    return rv;
  }

  /* create signature tail */
  st=GWEN_SigTail_new();
  GWEN_SigTail_SetSignature(st,
                            (const uint8_t*)GWEN_Buffer_GetStart(sigbuf),
                            GWEN_Buffer_GetUsedBytes(sigbuf));
  GWEN_Buffer_free(sigbuf);
  GWEN_SigTail_SetSignatureNumber(st, 1);

  /* write signature tail */
  rv=GWEN_SigTail_toBuffer(st, dbuf, GWEN_CRYPTMGR_TLV_SIGTAIL);
  GWEN_SigTail_free(st);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* write complete size */
  l=GWEN_Buffer_GetPos(dbuf)-pos-2;
  p=(uint8_t*)GWEN_Buffer_GetStart(dbuf)+pos;
  *(p++)=l & 0xff;
  *p=(l>>8) & 0xff;

  return 0;
}



int GWEN_CryptMgr_Encrypt(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  GWEN_CRYPTHEAD *ch;
  uint32_t pos;
  uint8_t *p;
  uint32_t l;
  int rv;
  GWEN_BUFFER *cryptbuf;
  GWEN_BUFFER *tbuf;
  GWEN_CRYPT_KEY *mkey;

  assert(cm);

  /* generate a message key */
  mkey=GWEN_Crypt_KeyBlowFish_Generate(GWEN_Crypt_CryptMode_Cbc, 256/8, 2);
  if (mkey==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unable to generate BLOWFISH key");
    return GWEN_ERROR_GENERIC;
  }

  GWEN_Buffer_AppendByte(dbuf, GWEN_CRYPTMGR_TLV_ENCRYPTEDOBJECT);
  pos=GWEN_Buffer_GetPos(dbuf);
  GWEN_Buffer_AppendByte(dbuf, 0);
  GWEN_Buffer_AppendByte(dbuf, 0);

  /* prepare signature head */
  ch=GWEN_CryptHead_new();
  GWEN_CryptHead_SetKeyName(ch, cm->peerKeyName);
  GWEN_CryptHead_SetKeyNumber(ch, cm->peerKeyNumber);
  GWEN_CryptHead_SetKeyVersion(ch, cm->peerKeyVersion);
  GWEN_CryptHead_SetCryptProfile(ch, cm->signatureProfile);

  /* encrypt key */
  cryptbuf=GWEN_Buffer_new(0, lData+256, 0, 1);
  rv=GWEN_CryptMgr_EncryptKey(cm,
                              GWEN_Crypt_KeyBlowFish_GetKeyDataPtr(mkey),
                              GWEN_Crypt_KeyBlowFish_GetKeyDataLen(mkey),
                              cryptbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(cryptbuf);
    GWEN_CryptHead_free(ch);
    GWEN_Crypt_Key_free(mkey);
    return rv;
  }
  GWEN_CryptHead_SetKey(ch,
                        (const uint8_t*)GWEN_Buffer_GetStart(cryptbuf),
                        GWEN_Buffer_GetUsedBytes(cryptbuf));
  GWEN_Buffer_free(cryptbuf);

  /* write crypt head to buffer */
  rv=GWEN_CryptHead_toBuffer(ch, dbuf, GWEN_CRYPTMGR_TLV_CRYPTHEAD);
  GWEN_CryptHead_free(ch);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_Key_free(mkey);
    return rv;
  }

  /* padd plain text data */
  tbuf=GWEN_Buffer_new(0, lData+256, 0, 1);
  GWEN_Buffer_AppendBytes(tbuf, (const char*)pData, lData);
  GWEN_Padd_PaddWithAnsiX9_23(tbuf);

  /* encrypt with message key */
  cryptbuf=GWEN_Buffer_new(0, lData+256, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(cryptbuf);
  rv=GWEN_Crypt_Key_Encipher(mkey,
                             (const uint8_t*)GWEN_Buffer_GetStart(tbuf),
                             GWEN_Buffer_GetUsedBytes(tbuf),
                             (uint8_t*)GWEN_Buffer_GetStart(cryptbuf),
                             &l);
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(cryptbuf);
    GWEN_Crypt_Key_free(mkey);
    return rv;
  }
  GWEN_Buffer_IncrementPos(cryptbuf, l);
  GWEN_Buffer_AdjustUsedBytes(cryptbuf);

  /* write encrypted data */
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPTMGR_TLV_CRYPTDATA,
                              GWEN_Buffer_GetStart(cryptbuf),
                              GWEN_Buffer_GetUsedBytes(cryptbuf),
                              dbuf);
  GWEN_Buffer_free(cryptbuf);
  GWEN_Crypt_Key_free(mkey);

  /* write complete size */
  l=GWEN_Buffer_GetPos(dbuf)-pos-2;
  p=(uint8_t*)GWEN_Buffer_GetStart(dbuf)+pos;
  *(p++)=l & 0xff;
  *p=(l>>8) & 0xff;

  return 0;
}



int GWEN_CryptMgr_Verify(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  GWEN_TAG16 *tag;
  const uint8_t *p;
  uint32_t l;
  GWEN_SIGHEAD *sh=NULL;
  GWEN_SIGTAIL *st=NULL;
  const uint8_t *pSignedData=NULL;
  uint32_t lSignedData=0;
  int rv;

  assert(cm);
  if (lData<3) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
    return GWEN_ERROR_BAD_DATA;
  }

  tag=GWEN_Tag16_fromBuffer2(pData, lData, 0);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Data doesn't contain a valid TLV");
    return GWEN_ERROR_BAD_DATA;
  }

  if (GWEN_Tag16_GetTagType(tag)!=GWEN_CRYPTMGR_TLV_SIGNEDOBJECT) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Data does not contain asigned object");
    GWEN_Tag16_free(tag);
    return GWEN_ERROR_BAD_DATA;
  }

  p=GWEN_Tag16_GetTagData(tag);
  l=GWEN_Tag16_GetTagLength(tag);

  /* read sighead */
  if (l) {
    GWEN_TAG16 *subtag;

    subtag=GWEN_Tag16_fromBuffer2(p, l, 0);
    if (subtag) {
      if (GWEN_Tag16_GetTagType(subtag)==GWEN_CRYPTMGR_TLV_SIGHEAD) {
        sh=GWEN_SigHead_fromBuffer(GWEN_Tag16_GetTagData(subtag),
                                   GWEN_Tag16_GetTagLength(subtag));
        if (sh) {
          pSignedData=p;
          lSignedData=GWEN_Tag16_GetTagSize(subtag);
        }
      }
      p+=GWEN_Tag16_GetTagSize(subtag);
      l-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    }
  }

  /* read and store signed data */
  if (l) {
    GWEN_TAG16 *subtag;

    subtag=GWEN_Tag16_fromBuffer2(p, l, 0);
    if (subtag) {
      if (GWEN_Tag16_GetTagType(subtag)==GWEN_CRYPTMGR_TLV_SIGDATA) {
        GWEN_Buffer_AppendBytes(dbuf,
                                GWEN_Tag16_GetTagData(subtag),
                                GWEN_Tag16_GetTagLength(subtag));
        if ((pSignedData+lSignedData)==p) {
          lSignedData+=GWEN_Tag16_GetTagSize(subtag);
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "data TLV must follow sighead TLV");
          GWEN_Tag16_free(subtag);
          GWEN_SigHead_free(sh);
          GWEN_Tag16_free(tag);
          return GWEN_ERROR_BAD_DATA;
        }
      }
      p+=GWEN_Tag16_GetTagSize(subtag);
      l-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    }
  }

  /* read sigtail (contains the signature) */
  if (l) {
    GWEN_TAG16 *subtag;

    subtag=GWEN_Tag16_fromBuffer2(p, l, 0);
    if (subtag) {
      if (GWEN_Tag16_GetTagType(subtag)==GWEN_CRYPTMGR_TLV_SIGTAIL) {
        st=GWEN_SigTail_fromBuffer(GWEN_Tag16_GetTagData(subtag),
                                   GWEN_Tag16_GetTagLength(subtag));
      }
      p+=GWEN_Tag16_GetTagSize(subtag);
      l-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    }
  }

  /* check for all needed components */
  if (!(sh && st && pSignedData && lSignedData)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Signed object is not complete");
    GWEN_SigTail_free(st);
    GWEN_SigHead_free(sh);
    GWEN_Tag16_free(tag);
    return GWEN_ERROR_BAD_DATA;
  }

  if (GWEN_SigHead_GetSignatureNumber(sh)!=GWEN_SigTail_GetSignatureNumber(st)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Sighead doesn't match sigtail");
    GWEN_SigTail_free(st);
    GWEN_SigHead_free(sh);
    GWEN_Tag16_free(tag);
    return GWEN_ERROR_BAD_DATA;
  }

  /* store or check peer key info */
  if (cm->peerKeyName==NULL) {
    /* store peer info */
    GWEN_CryptMgr_SetPeerKeyName(cm, GWEN_SigHead_GetKeyName(sh));
    GWEN_CryptMgr_SetPeerKeyNumber(cm, GWEN_SigHead_GetKeyNumber(sh));
    GWEN_CryptMgr_SetPeerKeyVersion(cm, GWEN_SigHead_GetKeyVersion(sh));
  }
  else {
    const char *s;

    /* compare peer info with expected info */
    s=GWEN_SigHead_GetKeyName(sh);
    if (!(cm->peerKeyName && s && (strcasecmp(cm->peerKeyName, s)==0) &&
          (cm->peerKeyNumber==GWEN_SigHead_GetKeyNumber(sh)) &&
          (cm->peerKeyVersion==GWEN_SigHead_GetKeyVersion(sh)))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected peer key information in signature");
      GWEN_SigTail_free(st);
      GWEN_SigHead_free(sh);
      GWEN_Tag16_free(tag);

      return GWEN_ERROR_BAD_DATA;
    }
  }

  /* verify signature */
  rv=GWEN_CryptMgr_VerifyData(cm,
                              pSignedData, lSignedData,
                              GWEN_SigTail_GetSignaturePtr(st),
                              GWEN_SigTail_GetSignatureLen(st));
  GWEN_SigTail_free(st);
  GWEN_SigHead_free(sh);
  GWEN_Tag16_free(tag);

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_CryptMgr_Decrypt(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  GWEN_TAG16 *tag;
  const uint8_t *p;
  uint32_t l;
  GWEN_CRYPTHEAD *ch=NULL;
  const uint8_t *pEncryptedData=NULL;
  uint32_t lEncryptedData=0;
  int rv;
  GWEN_BUFFER *tbuf;
  GWEN_CRYPT_KEY *mkey;

  assert(cm);
  if (lData<3) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
    return GWEN_ERROR_BAD_DATA;
  }

  tag=GWEN_Tag16_fromBuffer2(pData, lData, 0);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Data doesn't contain a valid TLV");
    return GWEN_ERROR_BAD_DATA;
  }

  if (GWEN_Tag16_GetTagType(tag)!=GWEN_CRYPTMGR_TLV_ENCRYPTEDOBJECT) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Data does not contain an encrypted object");
    GWEN_Tag16_free(tag);
    return GWEN_ERROR_BAD_DATA;
  }

  p=GWEN_Tag16_GetTagData(tag);
  l=GWEN_Tag16_GetTagLength(tag);

  /* read crypthead */
  if (l) {
    GWEN_TAG16 *subtag;

    subtag=GWEN_Tag16_fromBuffer2(p, l, 0);
    if (subtag) {
      if (GWEN_Tag16_GetTagType(subtag)==GWEN_CRYPTMGR_TLV_CRYPTHEAD) {
        ch=GWEN_CryptHead_fromBuffer(GWEN_Tag16_GetTagData(subtag),
                                     GWEN_Tag16_GetTagLength(subtag));
      }
      p+=GWEN_Tag16_GetTagSize(subtag);
      l-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    }
  }

  /* read encrypted data */
  if (l) {
    GWEN_TAG16 *subtag;

    subtag=GWEN_Tag16_fromBuffer2(p, l, 0);
    if (subtag) {
      if (GWEN_Tag16_GetTagType(subtag)==GWEN_CRYPTMGR_TLV_CRYPTDATA) {
        pEncryptedData=GWEN_Tag16_GetTagData(subtag);
        lEncryptedData=GWEN_Tag16_GetTagLength(subtag);
      }
      p+=GWEN_Tag16_GetTagSize(subtag);
      l-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    }
  }

  /* check for all needed components */
  if (!(ch && pEncryptedData && lEncryptedData)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Encrypted object is not complete");
    GWEN_CryptHead_free(ch);
    GWEN_Tag16_free(tag);
    return GWEN_ERROR_BAD_DATA;
  }

  /* store or check peer key info */
  if (cm->localKeyName) {
    const char *s;

    /* compare peer info with expected info */
    s=GWEN_CryptHead_GetKeyName(ch);
    if (!(cm->localKeyName && s && (strcasecmp(cm->localKeyName, s)==0) &&
          (cm->localKeyNumber==GWEN_CryptHead_GetKeyNumber(ch)) &&
          (cm->localKeyVersion==GWEN_CryptHead_GetKeyVersion(ch)))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected local key information in signature");
      GWEN_CryptHead_free(ch);
      GWEN_Tag16_free(tag);

      return GWEN_ERROR_BAD_DATA;
    }
  }

  /* decrypt message key */
  tbuf=GWEN_Buffer_new(0, GWEN_CryptHead_GetKeyLen(ch), 0, 1);
  rv=GWEN_CryptMgr_DecryptKey(cm,
                              GWEN_CryptHead_GetKeyPtr(ch),
                              GWEN_CryptHead_GetKeyLen(ch),
                              tbuf);
  GWEN_CryptHead_free(ch);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Tag16_free(tag);
    return rv;
  }

  /* create message key */
  mkey=GWEN_Crypt_KeyBlowFish_fromData(GWEN_Crypt_CryptMode_Cbc,
                                       256/8,
                                       (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
                                       GWEN_Buffer_GetUsedBytes(tbuf));
  GWEN_Buffer_free(tbuf);
  if (mkey==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unable to create BLOWFISH key from received data");
    GWEN_Tag16_free(tag);
    return GWEN_ERROR_BAD_DATA;
  }


  /* decrypt data with message key */
  tbuf=GWEN_Buffer_new(0, lEncryptedData+256, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
  rv=GWEN_Crypt_Key_Decipher(mkey,
                             pEncryptedData, lEncryptedData,
                             (uint8_t*)GWEN_Buffer_GetStart(tbuf),
                             &l);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Crypt_Key_free(mkey);
    GWEN_Tag16_free(tag);
    return rv;
  }
  GWEN_Buffer_IncrementPos(tbuf, l);
  GWEN_Buffer_AdjustUsedBytes(tbuf);

  /* unpadd data */
  rv=GWEN_Padd_UnpaddWithAnsiX9_23(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Crypt_Key_free(mkey);
    GWEN_Tag16_free(tag);
    return rv;
  }

  /* store data */
  GWEN_Buffer_AppendBuffer(dbuf, tbuf);

  GWEN_Buffer_free(tbuf);
  GWEN_Crypt_Key_free(mkey);
  GWEN_Tag16_free(tag);

  return 0;
}



int GWEN_CryptMgr_Encode(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, lData, 0, 1);

  /* create signed object */
  DBG_INFO(GWEN_LOGDOMAIN, "Signing data");
  rv=GWEN_CryptMgr_Sign(cm, pData, lData, tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* create encrypted object (containing a signed object in this case) */
  DBG_INFO(GWEN_LOGDOMAIN, "Encrypting data");
  rv=GWEN_CryptMgr_Encrypt(cm,
                           (const uint8_t*)GWEN_Buffer_GetStart(tbuf),
                           GWEN_Buffer_GetUsedBytes(tbuf),
                           dbuf);
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_CryptMgr_Decode(GWEN_CRYPTMGR *cm, const uint8_t *pData, uint32_t lData, GWEN_BUFFER *dbuf) {
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, lData, 0, 1);

  /* decrypt encrypted object */
  DBG_INFO(GWEN_LOGDOMAIN, "Decrypting data");
  rv=GWEN_CryptMgr_Decrypt(cm, pData, lData, tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* verify signature, copy signed data to dbuf in the process */
  DBG_INFO(GWEN_LOGDOMAIN, "Verifying data");
  rv=GWEN_CryptMgr_Verify(cm,
                          (const uint8_t*)GWEN_Buffer_GetStart(tbuf),
                          GWEN_Buffer_GetUsedBytes(tbuf),
                          dbuf);
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}





