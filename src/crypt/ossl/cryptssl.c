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


#include "cryptssl_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>

#include <openssl/rsa.h>
#include <openssl/blowfish.h>
#include <openssl/objects.h>


/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                 RSA
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_CRYPTKEY *GWEN_CryptKeyRSA_new(){
  GWEN_CRYPTKEY *key;
  RSA *keyData;

  key=GWEN_CryptKey_new();
  keyData=RSA_new();
  assert(keyData);
  GWEN_CryptKey_SetKeyData(key, keyData);
  return key;
}



void GWEN_CryptKeyRSA_free(GWEN_CRYPTKEY *key){
  RSA_free((RSA*)GWEN_CryptKey_GetKeyData(key));
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Encrypt(GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  unsigned int srclen;
  unsigned int dstlen;
  char *psrc;
  char *pdst;

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
  if (GWEN_Buffer_RoomLeft(dst)<srclen) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BUFFER_FULL);
  }
  psrc=GWEN_Buffer_GetPosPointer(src);
  pdst=GWEN_Buffer_GetPosPointer(dst);

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



GWEN_ERRORCODE GWEN_CryptKeyRSA_Decrypt(GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  unsigned int srclen;
  unsigned int dstlen;
  char *psrc;
  char *pdst;

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
  if (GWEN_Buffer_RoomLeft(dst)<srclen) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BUFFER_FULL);
  }
  psrc=GWEN_Buffer_GetPosPointer(src);
  pdst=GWEN_Buffer_GetPosPointer(dst);

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



GWEN_ERRORCODE GWEN_CryptKeyRSA_Sign(GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  unsigned int srclen;
  unsigned int dstlen;
  char *psrc;
  char *pdst;

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
  if (GWEN_Buffer_RoomLeft(dst)<srclen) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BUFFER_FULL);
  }
  psrc=GWEN_Buffer_GetPosPointer(src);
  pdst=GWEN_Buffer_GetPosPointer(dst);

  dstlen=GWEN_Buffer_RoomLeft(dst);
  if (!RSA_sign(NID_ripemd160,
                psrc, srclen,
                pdst, &dstlen,
                GWEN_CryptKey_GetKeyData(key))) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_SIGN);
  }
  GWEN_Buffer_IncrementPos(dst, dstlen);
  GWEN_Buffer_AdjustUsedBytes(dst);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_Verify(GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *signature){
  unsigned int srclen;
  unsigned int dstlen;
  char *psrc;
  char *psig;

  assert(key);
  assert(src);
  assert(signature);

  srclen=GWEN_Buffer_GetUsedBytes(src);
  psrc=GWEN_Buffer_GetPosPointer(src);
  dstlen=GWEN_Buffer_GetUsedBytes(signature);
  psig=GWEN_Buffer_GetPosPointer(signature);
  if (!RSA_verify(NID_ripemd160,
                  psrc, srclen,
                  psig, dstlen,
                  GWEN_CryptKey_GetKeyData(key))) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_VERIFY);
  }
  return 0;
}



unsigned int GWEN_CryptKeyRSA_GetChunkSize(GWEN_CRYPTKEY *key){
  RSA *kd;

  assert(key);
  kd=(RSA*)GWEN_CryptKey_GetKeyData(key);
  assert(kd);
  return RSA_size(kd);
}



GWEN_ERRORCODE GWEN_CryptKeyRSA_FromDB(GWEN_CRYPTKEY *key,
                                       GWEN_DB_NODE *db){
  int pub;
  const void *p;
  unsigned int len;
  RSA *kd;
  BIGNUM *bn;

  kd=RSA_new();
  assert(kd);

  pub=GWEN_DB_GetIntValue(db, "public", 0, 1);

  p=GWEN_DB_GetBinValue(db, "n", 0, 0, 0, &len);
  if (p) {
    bn=BN_new();
    kd->n=BN_bin2bn((unsigned char*) p, len, bn);
  }

  p=GWEN_DB_GetBinValue(db, "e", 0, 0, 0, &len);
  if (p) {
    bn=BN_new();
    kd->e=BN_bin2bn((unsigned char*) p, len, bn);
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



GWEN_ERRORCODE GWEN_CryptKeyRSA_ToDB(GWEN_CRYPTKEY *key,
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

  l=BN_bn2bin(kd->n, (unsigned char*) &buffer);
  GWEN_DB_SetBinValue(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "n", buffer, l);
  if (pub!=0) {
    l=BN_bn2bin(kd->e, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "e", buffer, l);
  }
  else {
    l=BN_bn2bin(kd->p, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "p", buffer, l);

    l=BN_bn2bin(kd->q, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "q", buffer, l);

    l=BN_bn2bin(kd->dmp1, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "dmp1", buffer, l);

    l=BN_bn2bin(kd->dmq1, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "dmq1", buffer, l);

    l=BN_bn2bin(kd->iqmp, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "iqmp", buffer, l);

    l=BN_bn2bin(kd->d, (unsigned char*) &buffer);
    GWEN_DB_SetBinValue(db,
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "d", buffer, l);
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
















GWEN_ERRORCODE GWEN_CryptImpl_Init(){
  return 0;
}



GWEN_ERRORCODE GWEN_CryptImpl_Fini(){
  return 0;
}








