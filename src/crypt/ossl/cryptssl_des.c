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


#include "cryptssl_des_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/text.h>

#include <openssl/des.h>
#include <openssl/rand.h>
#include <openssl/objects.h>




GWEN_CRYPTKEY *GWEN_CryptKeyDES_new(){
  GWEN_CRYPTKEY *key;
  unsigned char *keyData;

  key=GWEN_CryptKey_new();
  keyData=(unsigned char*)malloc(16);
  assert(keyData);
  memset(keyData, 0, 16);
  GWEN_CryptKey_SetKeyData(key, keyData);

  GWEN_CryptKey_SetEncryptFn(key, GWEN_CryptKeyDES_Encrypt);
  GWEN_CryptKey_SetDecryptFn(key, GWEN_CryptKeyDES_Decrypt);
  GWEN_CryptKey_SetSignFn(key, GWEN_CryptKeyDES_Sign);
  GWEN_CryptKey_SetVerifyFn(key, GWEN_CryptKeyDES_Verify);
  GWEN_CryptKey_SetGetChunkSizeFn(key, GWEN_CryptKeyDES_GetChunkSize);
  GWEN_CryptKey_SetFromDbFn(key, GWEN_CryptKeyDES_FromDb);
  GWEN_CryptKey_SetToDbFn(key, GWEN_CryptKeyDES_ToDb);
  GWEN_CryptKey_SetGenerateKeyFn(key, GWEN_CryptKeyDES_Generate);
  GWEN_CryptKey_SetFreeKeyDataFn(key, GWEN_CryptKeyDES_FreeKeyData);
  GWEN_CryptKey_SetOpenFn(key, GWEN_CryptKeyDES_Open);
  GWEN_CryptKey_SetCloseFn(key, GWEN_CryptKeyDES_Close);
  GWEN_CryptKey_SetDupFn(key, GWEN_CryptKeyDES_dup);

  return key;
}



GWEN_CRYPTKEY *GWEN_CryptKeyDES_dup(const GWEN_CRYPTKEY *key){
  GWEN_CRYPTKEY *newKey;
  unsigned char *kd;
  unsigned char *newkd;

  kd=GWEN_CryptKey_GetKeyData(key);
  assert(kd);
  newKey=GWEN_CryptKey_new();
  newkd=(unsigned char*)malloc(16);
  assert(newkd);
  memmove(newkd, kd, 16);
  GWEN_CryptKey_SetKeyData(newKey, newkd);
  return newKey;
}



void GWEN_CryptKeyDES_FreeKeyData(GWEN_CRYPTKEY *key){
  if (key)
    free(GWEN_CryptKey_GetKeyData(key));
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Crypt(GWEN_CRYPTKEY *key,
                                      GWEN_BUFFER *src,
                                      GWEN_BUFFER *dst,
                                      int cryptMode){
  des_cblock left, right;
  unsigned char *kd;
  des_cblock iv;
  des_key_schedule key1, key2;
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
  psrc=GWEN_Buffer_GetStart(src);
  pdst=GWEN_Buffer_GetPosPointer(dst);

  memmove(left, kd, 8);
  memmove(right, kd+8, 8);

  des_set_key(&left, key1);
  des_set_key(&right, key2);
  memset(iv, 0, 8);

  DES_ede2_cbc_encrypt(psrc, pdst, srclen, &key1, &key2, &iv, cryptMode);
  GWEN_Buffer_IncrementPos(dst, srclen);
  GWEN_Buffer_AdjustUsedBytes(dst);
  return 0;
}




GWEN_ERRORCODE GWEN_CryptKeyDES_Encrypt(GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  return GWEN_CryptKeyDES_Crypt(key, src, dst, DES_ENCRYPT);
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Decrypt(GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst){
  return GWEN_CryptKeyDES_Crypt(key, src, dst, DES_DECRYPT);
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Sign(GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  return GWEN_Error_new(0,
                        GWEN_ERROR_SEVERITY_ERR,
                        GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                        GWEN_CRYPT_ERROR_UNSUPPORTED);
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Verify(GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *signature){
  return GWEN_Error_new(0,
                        GWEN_ERROR_SEVERITY_ERR,
                        GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                        GWEN_CRYPT_ERROR_UNSUPPORTED);
}



unsigned int GWEN_CryptKeyDES_GetChunkSize(const GWEN_CRYPTKEY *key){
  return 8;
}



GWEN_ERRORCODE GWEN_CryptKeyDES_FromDb(GWEN_CRYPTKEY *key,
                                       GWEN_DB_NODE *db){
  unsigned char *kd;
  const void *p;
  unsigned int size;

  assert(key);
  kd=GWEN_CryptKey_GetKeyData(key);
  assert(kd);

  p=GWEN_DB_GetBinValue(db, "keydata", 0, 0, 0, &size);
  if (!p || size!=16) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }

  memmove(kd, p, size);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyDES_ToDb(GWEN_CRYPTKEY *key,
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
                      kd, 16);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Generate(GWEN_CRYPTKEY *k,
                                         unsigned keylength){
  int result;
  des_cblock nativeDESKey;
  des_key_schedule key;
  unsigned char randomData[1024];
  unsigned char *kd;
  unsigned int i;

  kd=GWEN_CryptKey_GetKeyData(k);
  assert(kd);

  /* seed random */
  for (i=0; i<sizeof(randomData); i++)
    randomData[i] = (unsigned char) rand();
  RAND_seed(randomData, sizeof(randomData));

  /* generate left half */
  result=1;
  while (result != 0) {
    des_random_key(&nativeDESKey);
    result = des_set_key_checked(&nativeDESKey, key);
  }
  memmove(kd, nativeDESKey, 8);

  /* generate right half */
  result = 1;
  while (result != 0) {
    des_random_key(&nativeDESKey);
    result = des_set_key_checked(&nativeDESKey, key);
  }
  memmove(kd+8, nativeDESKey, 8);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Open(GWEN_CRYPTKEY *key){
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Close(GWEN_CRYPTKEY *key){
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKeyDES_Register(){
  GWEN_CRYPTKEY_PROVIDER *pr;
  GWEN_ERRORCODE err;

  pr=GWEN_CryptProvider_new();
  GWEN_CryptProvider_SetNewKeyFn(pr, GWEN_CryptKeyDES_new);
  GWEN_CryptProvider_SetName(pr, GWEN_CRYPT_DES_NAME);
  err=GWEN_Crypt_RegisterProvider(pr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_CryptProvider_free(pr);
    DBG_INFO(0, "called from here");
    return err;
  }
  return 0;
}









