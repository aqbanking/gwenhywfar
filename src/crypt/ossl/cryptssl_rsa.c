/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 06 2003
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


#include "cryptssl_rsa_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>

#include <openssl/rsa.h>
#include <openssl/objects.h>

#include "cryptssl_p.h"


void GWEN_CryptKeyRSA_FreeKeyData(GWEN_CRYPTKEY *key){
  RSA *kd;

  kd=(RSA*)GWEN_CryptKey_GetKeyData(key);
  if (kd)
    RSA_free(kd);
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Encrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  unsigned int srclen;
  unsigned int dstlen;
  unsigned char *psrc;
  unsigned char *pdst;

  assert(key);
  assert(src);
  assert(dst);

  srclen=GWEN_Buffer_GetUsedBytes(src);
  if (srclen!=GWEN_CryptKey_GetChunkSize(key)) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }
  if (GWEN_Buffer_AllocRoom(dst, srclen)) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BUFFER_FULL);
  }
  psrc=(unsigned char*)GWEN_Buffer_GetStart(src);
  pdst=(unsigned char*)GWEN_Buffer_GetPosPointer(dst);

  dstlen=RSA_public_encrypt(srclen, psrc, pdst,
                            GWEN_CryptKey_GetKeyData(key),
                            RSA_NO_PADDING);
  if (dstlen!=srclen) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_ENCRYPT);
  }
  GWEN_Buffer_IncrementPos(dst, dstlen);
  GWEN_Buffer_AdjustUsedBytes(dst);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Decrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  unsigned int srclen;
  unsigned int dstlen;
  unsigned char *psrc;
  unsigned char *pdst;

  assert(key);
  assert(src);
  assert(dst);

  srclen=GWEN_Buffer_GetUsedBytes(src);
  if (srclen!=GWEN_CryptKey_GetChunkSize(key)) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }
  if (GWEN_Buffer_AllocRoom(dst, srclen)) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BUFFER_FULL);
  }
  psrc=(unsigned char*)GWEN_Buffer_GetStart(src);
  pdst=(unsigned char*)GWEN_Buffer_GetPosPointer(dst);

  dstlen=RSA_private_decrypt(srclen, psrc, pdst,
                             GWEN_CryptKey_GetKeyData(key),
                             RSA_NO_PADDING);
  if (dstlen!=srclen) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_ENCRYPT);
  }
  GWEN_Buffer_IncrementPos(dst, dstlen);
  GWEN_Buffer_AdjustUsedBytes(dst);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_SignBigNum(const GWEN_CRYPTKEY *key,
                                           GWEN_BUFFER *src,
                                           BIGNUM *bnresult){
  unsigned int srclen;
  unsigned char *psrc;
  RSA *kd;

  int res;
  BIGNUM *bnhash;
  BIGNUM *bnresult2;
  BN_CTX *bnctx;

  assert(key);
  assert(src);
  assert(bnresult);

  kd=(RSA*)GWEN_CryptKey_GetKeyData(key);
  assert(kd);

  srclen=GWEN_Buffer_GetUsedBytes(src);
  if (srclen!=GWEN_CryptKey_GetChunkSize(key)) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }
  psrc=(unsigned char*)GWEN_Buffer_GetStart(src);

  bnhash = BN_new();
  bnresult2 = BN_new();
  bnctx = BN_CTX_new();

  bnhash = BN_bin2bn(psrc, srclen, bnhash);

  BN_CTX_start(bnctx);
  res=BN_mod_exp(bnresult, bnhash, kd->d, kd->n, bnctx);

  /* FIXME: The description is quite the opposite of the code, while the
   * code seems to be working (at least when verifying my own signatures)
   * the iso9796-appendix is as follows:
   * if (the calculated signature - the modulus) < (the calculated signature)
   * use (the calculated signature - the modulus) as signature
   */

  if (BN_cmp(bnresult, kd->n) < 0) {
    if (!BN_sub(bnresult2, kd->n, bnresult)) {
      DBG_ERROR(0, "Math error");
      BN_free(bnresult2);
      BN_free(bnhash);
      return -1;
    }

    if (BN_cmp(bnresult2, bnresult) < 0) {
      DBG_ERROR(0, "Using smaller signature");
      BN_copy(bnresult, bnresult2);
    }
  }

  BN_free(bnresult2);
  BN_free(bnhash);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Sign(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  unsigned int srclen;
  unsigned char *pdst;
  RSA *kd;
  GWEN_ERRORCODE err;
  BIGNUM *bnresult;

  assert(key);
  assert(src);
  assert(dst);

  kd=(RSA*)GWEN_CryptKey_GetKeyData(key);
  assert(kd);

  srclen=GWEN_Buffer_GetUsedBytes(src);
  if (srclen!=GWEN_CryptKey_GetChunkSize(key)) {
    DBG_INFO(0, "Bad size of source data (%d!=%d)",
             srclen, GWEN_CryptKey_GetChunkSize(key));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }
  if (GWEN_Buffer_AllocRoom(dst, srclen)) {
    DBG_INFO(0, "Could not allocate room for %d bytes", srclen);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BUFFER_FULL);
  }
  pdst=(unsigned char*)GWEN_Buffer_GetPosPointer(dst);

  bnresult = BN_new();

  err=GWEN_CryptKeyRSA_SignBigNum(key, src, bnresult);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    BN_free(bnresult);
    return err;
  }

  BN_bn2bin(bnresult, pdst);
  BN_free(bnresult);

  GWEN_Buffer_IncrementPos(dst, srclen);
  GWEN_Buffer_AdjustUsedBytes(dst);

  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Verify(const GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *signature){
  unsigned int srclen;
  unsigned int siglen;
  unsigned char *psrc;
  unsigned char *psig;
  RSA *kd;

  BIGNUM *bnsig;
  BIGNUM *bndecsig;
  BIGNUM *bnhash;
  BN_CTX *bnctx;

  assert(key);
  assert(src);
  assert(signature);

  kd=(RSA*)GWEN_CryptKey_GetKeyData(key);
  assert(kd);

  srclen=GWEN_Buffer_GetUsedBytes(src);
  psrc=(unsigned char*)GWEN_Buffer_GetStart(src);
  siglen=GWEN_Buffer_GetUsedBytes(signature);
  psig=(unsigned char*)GWEN_Buffer_GetStart(signature);

  bnsig=BN_new();
  bndecsig=BN_new();
  bnhash=BN_new();
  bnctx=BN_CTX_new();

  /* decrypt the institutes signature */
  bnsig = BN_bin2bn(psig, siglen, bnsig);
  BN_CTX_start(bnctx);
  BN_mod_exp(bndecsig, bnsig, kd->e, kd->n, bnctx);

  bnhash = BN_bin2bn(psrc, srclen, bnhash);

  if (BN_cmp(bndecsig, bnhash)!=0) {
    DBG_ERROR(0, "Trying other signature variant");
    BN_sub(bnhash, kd->n, bnhash);
    if (BN_cmp(bndecsig, bnhash)!=0) {
      DBG_ERROR(0, "Signature does not match");
      BN_free(bnsig);
      BN_free(bndecsig);
      BN_free(bnhash);
      BN_CTX_free(bnctx);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                            GWEN_CRYPT_ERROR_VERIFY);
    }
  }

  BN_free(bnsig);
  BN_free(bndecsig);
  BN_free(bnhash);
  BN_CTX_free(bnctx);

  return 0;
}



unsigned int GWEN_CryptKeyRSA_GetChunkSize(const GWEN_CRYPTKEY *key){
  RSA *kd;

  assert(key);
  kd=(RSA*)GWEN_CryptKey_GetKeyData(key);
  assert(kd);
  return RSA_size(kd);
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_FromDb(GWEN_CRYPTKEY *key,
                                       GWEN_DB_NODE *db){
  int pub;
  const void *p;
  unsigned int len;
  RSA *kd;
  BIGNUM *bn;

  kd=RSA_new();
  assert(kd);

  DBG_INFO(0, "Reading this key:");
  GWEN_DB_Dump(db, stderr, 2);

  pub=GWEN_DB_GetIntValue(db, "public", 0, 1);

  if (pub) {
    p=GWEN_DB_GetBinValue(db, "e", 0, 0, 0, &len);
    if (p) {
      bn=BN_new();
      kd->e=BN_bin2bn((unsigned char*) p, len, bn);
    }
  }
  else {
    /* OpenSSL might be using blinding, so we MUST set e even if it is not
     * otherwise used for private keys */
    kd->e=BN_new();
    BN_set_word(kd->e, GWEN_CRYPT_RSA_DEFAULT_EXPONENT);
  }

  p=GWEN_DB_GetBinValue(db, "n", 0, 0, 0, &len);
  if (p) {
    bn=BN_new();
    kd->n=BN_bin2bn((unsigned char*) p, len, bn);
  }

  if (!pub) {
    p=GWEN_DB_GetBinValue(db, "p", 0, 0, 0, &len);
    if (p) {
      bn=BN_new();
      kd->p=BN_bin2bn((unsigned char*) p, len, bn);
    }

    p=GWEN_DB_GetBinValue(db, "q", 0, 0, 0, &len);
    if (p) {
      bn=BN_new();
      kd->q=BN_bin2bn((unsigned char*) p, len, bn);
    }

    p=GWEN_DB_GetBinValue(db, "dmp1", 0, 0, 0, &len);
    if (p) {
      bn=BN_new();
      kd->dmp1=BN_bin2bn((unsigned char*) p, len, bn);
    }

    p=GWEN_DB_GetBinValue(db, "dmq1", 0, 0, 0, &len);
    if (p) {
      bn=BN_new();
      kd->dmq1=BN_bin2bn((unsigned char*) p, len, bn);
    }

    p=GWEN_DB_GetBinValue(db, "iqmp", 0, 0, 0, &len);
    if (p) {
      bn=BN_new();
      kd->iqmp=BN_bin2bn((unsigned char*) p, len, bn);
    }

    p=GWEN_DB_GetBinValue(db, "d", 0, 0, 0, &len);
    if (p) {
      bn=BN_new();
      kd->d=BN_bin2bn((unsigned char*) p, len, bn);
    }
  }

  GWEN_CryptKey_SetKeyData(key, kd);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_ToDb(const GWEN_CRYPTKEY *key,
                                     GWEN_DB_NODE *db,
                                     int pub){
  char buffer[GWEN_CRYPT_RSA_MAX_KEY_LENGTH/8];
  int l;
  RSA *kd;

  kd=GWEN_CryptKey_GetKeyData(key);
  assert(kd);
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "public", pub);
  if (kd->n) {
    l=BN_bn2bin(kd->n, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "n", buffer, l);
  }
  if (pub!=0) {
    if (kd->e) {
      l=BN_bn2bin(kd->e, (unsigned char*) &buffer);
      GWEN_DB_SetBinValue(db,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "e", buffer, l);
    }
  }
  else {
    if (kd->p) {
      l=BN_bn2bin(kd->p, (unsigned char*) &buffer);
      GWEN_DB_SetBinValue(db,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "p", buffer, l);
    }

    if (kd->q) {
      l=BN_bn2bin(kd->q, (unsigned char*) &buffer);
      GWEN_DB_SetBinValue(db,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "q", buffer, l);
    }

    if (kd->dmp1) {
      l=BN_bn2bin(kd->dmp1, (unsigned char*) &buffer);
      GWEN_DB_SetBinValue(db,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "dmp1", buffer, l);
    }

    if (kd->dmq1) {
      l=BN_bn2bin(kd->dmq1, (unsigned char*) &buffer);
      GWEN_DB_SetBinValue(db,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "dmq1", buffer, l);
    }

    if (kd->iqmp) {
      l=BN_bn2bin(kd->iqmp, (unsigned char*) &buffer);
      GWEN_DB_SetBinValue(db,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "iqmp", buffer, l);
    }

    if (kd->d) {
      l=BN_bn2bin(kd->d, (unsigned char*) &buffer);
      GWEN_DB_SetBinValue(db,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "d", buffer, l);
    }
  }
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Generate(GWEN_CRYPTKEY *key,
                                         unsigned keylength){
  RSA *newKey;

  assert(key);
  if (keylength==0)
    keylength=GWEN_CRYPT_RSA_DEFAULT_KEY_LENGTH;

  newKey=RSA_generate_key(keylength, GWEN_CRYPT_RSA_DEFAULT_EXPONENT,
                          NULL, NULL);
  assert(newKey);
  GWEN_CryptKey_SetKeyData(key, newKey);

  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Open(GWEN_CRYPTKEY *key){
  GWEN_CryptKey_IncrementOpenCount(key);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Close(GWEN_CRYPTKEY *key){
  GWEN_CryptKey_DecrementOpenCount(key);
  return 0;
}



GWEN_CRYPTKEY *GWEN_CryptKeyRSA_new(){
  GWEN_CRYPTKEY *key;
  RSA *keyData;

  key=GWEN_CryptKey_new();
  keyData=RSA_new();
  assert(keyData);
  GWEN_CryptKey_SetKeyData(key, keyData);

  GWEN_CryptKey_SetEncryptFn(key, GWEN_CryptKeyRSA_Encrypt);
  GWEN_CryptKey_SetDecryptFn(key, GWEN_CryptKeyRSA_Decrypt);
  GWEN_CryptKey_SetSignFn(key, GWEN_CryptKeyRSA_Sign);
  GWEN_CryptKey_SetVerifyFn(key, GWEN_CryptKeyRSA_Verify);
  GWEN_CryptKey_SetGetChunkSizeFn(key, GWEN_CryptKeyRSA_GetChunkSize);
  GWEN_CryptKey_SetFromDbFn(key, GWEN_CryptKeyRSA_FromDb);
  GWEN_CryptKey_SetToDbFn(key, GWEN_CryptKeyRSA_ToDb);
  GWEN_CryptKey_SetGenerateKeyFn(key, GWEN_CryptKeyRSA_Generate);
  GWEN_CryptKey_SetFreeKeyDataFn(key, GWEN_CryptKeyRSA_FreeKeyData);
  GWEN_CryptKey_SetOpenFn(key, GWEN_CryptKeyRSA_Open);
  GWEN_CryptKey_SetCloseFn(key, GWEN_CryptKeyRSA_Close);
  GWEN_CryptKey_SetDupFn(key, GWEN_CryptKeyRSA_dup);
  return key;
}



GWEN_CRYPTKEY *GWEN_CryptKeyRSA_dup(const GWEN_CRYPTKEY *key){
  GWEN_CRYPTKEY *newKey;
  RSA *keyData;
  RSA *newKeyData;

  keyData=(RSA*)GWEN_CryptKey_GetKeyData(key);
  assert(keyData);
  newKey=GWEN_CryptKey_new();
  newKeyData=RSA_new();
  assert(newKeyData);
  newKeyData->n=BN_dup(keyData->n);
  newKeyData->e=BN_dup(keyData->e);
  newKeyData->p=BN_dup(keyData->p);
  newKeyData->q=BN_dup(keyData->q);
  newKeyData->dmp1=BN_dup(keyData->dmp1);
  newKeyData->dmq1=BN_dup(keyData->dmq1);
  newKeyData->iqmp=BN_dup(keyData->iqmp);
  newKeyData->d=BN_dup(keyData->d);
  GWEN_CryptKey_SetKeyData(newKey, newKeyData);
  return newKey;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Register(){
  GWEN_CRYPTKEY_PROVIDER *pr;
  GWEN_ERRORCODE err;

  pr=GWEN_CryptProvider_new();
  GWEN_CryptProvider_SetNewKeyFn(pr, GWEN_CryptKeyRSA_new);
  GWEN_CryptProvider_SetName(pr, GWEN_CRYPT_RSA_NAME);
  err=GWEN_Crypt_RegisterProvider(pr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_CryptProvider_free(pr);
    DBG_INFO(0, "called from here");
    return err;
  }
  return 0;
}






