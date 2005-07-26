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

/* Define this if you are extending the "class" CryptKey */
#define GWEN_EXTEND_CRYPTKEY


#include "cryptssl_bf_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include "crypt_l.h"

#include <openssl/blowfish.h>
#include <openssl/rand.h>
#include <openssl/objects.h>



GWEN_CRYPTKEY *GWEN_CryptKeyBF_new(){
  GWEN_CRYPTKEY *key;
  unsigned char *keyData;

  key=GWEN_CryptKey_new();
  keyData=(unsigned char*)malloc(GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES);
  assert(keyData);
  memset(keyData, 0, GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES);
  GWEN_CryptKey_SetKeyData(key, keyData);

  GWEN_CryptKey_SetEncryptFn(key, GWEN_CryptKeyBF_Encrypt);
  GWEN_CryptKey_SetDecryptFn(key, GWEN_CryptKeyBF_Decrypt);
  GWEN_CryptKey_SetSignFn(key, GWEN_CryptKeyBF_Sign);
  GWEN_CryptKey_SetVerifyFn(key, GWEN_CryptKeyBF_Verify);
  GWEN_CryptKey_SetGetChunkSizeFn(key, GWEN_CryptKeyBF_GetChunkSize);
  GWEN_CryptKey_SetFromDbFn(key, GWEN_CryptKeyBF_FromDb);
  GWEN_CryptKey_SetToDbFn(key, GWEN_CryptKeyBF_ToDb);
  GWEN_CryptKey_SetGenerateKeyFn(key, GWEN_CryptKeyBF_Generate);
  GWEN_CryptKey_SetFreeKeyDataFn(key, GWEN_CryptKeyBF_FreeKeyData);
  GWEN_CryptKey_SetOpenFn(key, GWEN_CryptKeyBF_Open);
  GWEN_CryptKey_SetCloseFn(key, GWEN_CryptKeyBF_Close);
  GWEN_CryptKey_SetDupFn(key, GWEN_CryptKeyBF_dup);

  return key;
}



GWEN_CRYPTKEY *GWEN_CryptKeyBF_dup(const GWEN_CRYPTKEY *key){
  GWEN_CRYPTKEY *newKey;
  unsigned char *kd;
  unsigned char *newkd;

  kd=GWEN_CryptKey_GetKeyData(key);
  assert(kd);
  newKey=GWEN_CryptKey_new();
  newkd=(unsigned char*)malloc(GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES);
  assert(newkd);
  memmove(newkd, kd, GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES);
  GWEN_CryptKey_SetKeyData(newKey, newkd);
  return newKey;
}



void GWEN_CryptKeyBF_FreeKeyData(GWEN_CRYPTKEY *key){
  if (key)
    free(GWEN_CryptKey_GetKeyData(key));
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Crypt(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst,
                                     int cryptMode){
  BF_KEY bk;
  unsigned char *kd;
  unsigned char ivec[8];
  unsigned int srclen;
  unsigned char *psrc;
  unsigned char *pdst;

  assert(key);
  assert(src);
  assert(dst);

  kd=GWEN_CryptKey_GetKeyData(key);
  assert(kd);

  srclen=GWEN_Buffer_GetUsedBytes(src);
  if (srclen % GWEN_CryptKey_GetChunkSize(key)) {
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

  BF_set_key(&bk, GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES, kd);
  memset(ivec, 0, sizeof(ivec));

  BF_cbc_encrypt(psrc, pdst, srclen, &bk, ivec, cryptMode);

  GWEN_Buffer_IncrementPos(dst, srclen);
  GWEN_Buffer_AdjustUsedBytes(dst);
  return 0;
}




GWEN_ERRORCODE GWEN_CryptKeyBF_Encrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  return GWEN_CryptKeyBF_Crypt(key, src, dst, BF_ENCRYPT);
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Decrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  return GWEN_CryptKeyBF_Crypt(key, src, dst, BF_DECRYPT);
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Sign(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  return GWEN_Error_new(0,
                        GWEN_ERROR_SEVERITY_ERR,
                        GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                        GWEN_CRYPT_ERROR_UNSUPPORTED);
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Verify(const GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *signature){
  return GWEN_Error_new(0,
                        GWEN_ERROR_SEVERITY_ERR,
                        GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                        GWEN_CRYPT_ERROR_UNSUPPORTED);
}



unsigned int GWEN_CryptKeyBF_GetChunkSize(const GWEN_CRYPTKEY *key){
  return 8;
}



GWEN_ERRORCODE GWEN_CryptKeyBF_FromDb(GWEN_CRYPTKEY *key,
                                       GWEN_DB_NODE *db){
  unsigned char *kd;
  const void *p;
  unsigned int size;

  assert(key);
  kd=GWEN_CryptKey_GetKeyData(key);
  assert(kd);

  p=GWEN_DB_GetBinValue(db, "keydata", 0, 0, 0, &size);
  if (!p || size!=GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }

  memmove(kd, p, size);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyBF_ToDb(const GWEN_CRYPTKEY *key,
                                     GWEN_DB_NODE *db,
                                     int pub){
  unsigned char *kd;

  assert(key);
  kd=GWEN_CryptKey_GetKeyData(key);
  assert(kd);

  if (pub)
    return 0;
  GWEN_DB_SetBinValue(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keydata",
                      kd, GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Generate(GWEN_CRYPTKEY *k,
                                        unsigned int keylength){
  unsigned char *kd;
  unsigned int i;

  if ((keylength/8)!=GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Only keylength %d currently implemented",
              GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES*8);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }

  kd=GWEN_CryptKey_GetKeyData(k);
  assert(kd);

  /* seed random */
  for (i=0; i<GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES; i++)
    kd[i] = (unsigned char) GWEN_Random();

  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Open(GWEN_CRYPTKEY *key){
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Close(GWEN_CRYPTKEY *key){
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyBF_Register(){
  GWEN_CRYPTKEY_PROVIDER *pr;
  GWEN_ERRORCODE err;

  pr=GWEN_CryptProvider_new();
  GWEN_CryptProvider_SetNewKeyFn(pr, GWEN_CryptKeyBF_new);
  GWEN_CryptProvider_SetName(pr, GWEN_CRYPT_BF_NAME);
  err=GWEN_Crypt_RegisterProvider(pr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_CryptProvider_free(pr);
    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
    return err;
  }
  return 0;
}









