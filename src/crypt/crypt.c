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

/* Define this if you are extending the "class" CryptKey */
#define GWEN_EXTEND_CRYPTKEY


#include "crypt_p.h"
#include <gwenhywfar/md.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>


#ifdef GWEN_MEMTRACE
static unsigned int GWEN_CryptKey_Count=0;
#endif


static int gwen_crypt_is_initialized=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_crypt_errorform=0;


static GWEN_CRYPTKEY_PROVIDER *gwen_crypt_providers=0;



const char *GWEN_Crypt_ErrorString(int c){
  const char *s;

  switch(c) {
  case GWEN_CRYPT_ERROR_ALREADY_REGISTERED:
    s="Crypto provider already registered";
    break;
  case GWEN_CRYPT_ERROR_NOT_REGISTERED:
    s="Crypto provider not registered";
    break;
  case GWEN_CRYPT_ERROR_BAD_SIZE:
    s="Bad size of data";
    break;
  case GWEN_CRYPT_ERROR_BUFFER_FULL:
    s="Buffer full";
    break;
  case GWEN_CRYPT_ERROR_ENCRYPT:
    s="Encryption error";
    break;
  case GWEN_CRYPT_ERROR_DECRYPT:
    s="Decryption error";
    break;
  case GWEN_CRYPT_ERROR_SIGN:
    s="Sign error";
    break;
  case GWEN_CRYPT_ERROR_VERIFY:
    s="Verification error";
    break;
  case GWEN_CRYPT_ERROR_UNSUPPORTED:
    s="Function not supported.";
    break;
  default:
    s=0;
  } /* switch */

  return s;
}



GWEN_ERRORCODE GWEN_Crypt_ModuleInit(){
  if (!gwen_crypt_is_initialized) {
    GWEN_ERRORCODE err;

    gwen_crypt_errorform=GWEN_ErrorType_new();
    GWEN_ErrorType_SetName(gwen_crypt_errorform,
                           GWEN_CRYPT_ERROR_TYPE);
    GWEN_ErrorType_SetMsgPtr(gwen_crypt_errorform,
                             GWEN_Crypt_ErrorString);
    err=GWEN_Error_RegisterType(gwen_crypt_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    err=GWEN_CryptImpl_Init();
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Error_UnregisterType(gwen_crypt_errorform);
      return err;
    }
    gwen_crypt_is_initialized=1;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_Crypt_ModuleFini(){
  if (gwen_crypt_is_initialized) {
    GWEN_ERRORCODE err;

    GWEN_Crypt_UnregisterAllProviders();
    GWEN_Md_UnregisterAllProviders();

    err=GWEN_CryptImpl_Fini();
    if (!GWEN_Error_IsOk(err))
      return err;

    err=GWEN_Error_UnregisterType(gwen_crypt_errorform);
    GWEN_ErrorType_free(gwen_crypt_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_crypt_is_initialized=0;
#ifdef GWEN_MEMTRACE
    if (GWEN_CryptKey_Count) {
      DBG_WARN(0, "Still %d CryptKeys in memory", GWEN_CryptKey_Count);
    }
#endif
  }
  return 0;
}





GWEN_CRYPTKEY *GWEN_CryptKey_new(){
  GWEN_CRYPTKEY *ck;

  GWEN_NEW_OBJECT(GWEN_CRYPTKEY, ck);
#ifdef GWEN_MEMTRACE
  GWEN_CryptKey_Count++;
  DBG_INFO(0, "New Cryptkey (now %d)", GWEN_CryptKey_Count);
#endif

  ck->keyspec=GWEN_KeySpec_new();
  return ck;
}


void GWEN_CryptKey_free(GWEN_CRYPTKEY *key){
  if (key) {
#ifdef GWEN_MEMTRACE
    assert(GWEN_CryptKey_Count);
    GWEN_CryptKey_Count--;
    DBG_INFO(0, "Free Cryptkey (now %d)", GWEN_CryptKey_Count);
#endif
    if (key->freeKeyDataFn)
      key->freeKeyDataFn(key);
    GWEN_KeySpec_free(key->keyspec);
    free(key);
  }
}



GWEN_CRYPTKEY *GWEN_CryptKey_dup(const GWEN_CRYPTKEY *key){
  GWEN_CRYPTKEY *newKey;

  assert(key);
  assert(key->dupFn);
  newKey=key->dupFn(key);
  assert(key->keyspec);
  newKey->dupFn=key->dupFn;
  newKey->encryptFn=key->encryptFn;
  newKey->decryptFn=key->decryptFn;
  newKey->signFn=key->signFn;
  newKey->verifyFn=key->verifyFn;
  newKey->getChunkSizeFn=key->getChunkSizeFn;
  newKey->fromDbFn=key->fromDbFn;
  newKey->toDbFn=key->toDbFn;
  newKey->generateKeyFn=key->generateKeyFn;
  newKey->freeKeyDataFn=key->freeKeyDataFn;
  newKey->openFn=key->openFn;
  newKey->closeFn=key->closeFn;
  DBG_INFO(0, "Freeing Keyspec");
  GWEN_KeySpec_free(newKey->keyspec);
  newKey->keyspec=GWEN_KeySpec_dup(key->keyspec);
  newKey->pub=key->pub;
  return newKey;
}



GWEN_ERRORCODE GWEN_CryptKey_Encrypt(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->encryptFn);
  return key->encryptFn(key, src, dst);
}



GWEN_ERRORCODE GWEN_CryptKey_Decrypt(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->decryptFn);
  return key->decryptFn(key, src, dst);
}



GWEN_ERRORCODE GWEN_CryptKey_Sign(const GWEN_CRYPTKEY *key,
                                  GWEN_BUFFER *src,
                                  GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->signFn);
  return key->signFn(key, src, dst);
}



GWEN_ERRORCODE GWEN_CryptKey_Verify(const GWEN_CRYPTKEY *key,
                                    GWEN_BUFFER *src,
                                    GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->verifyFn);
  return key->verifyFn(key, src, dst);
}



unsigned int GWEN_CryptKey_GetChunkSize(const GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->getChunkSizeFn);
  return key->getChunkSizeFn(key);
}



GWEN_CRYPTKEY *GWEN_CryptKey_FromDb(GWEN_DB_NODE *db){
  GWEN_DB_NODE *gr;
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;

  /*DBG_ERROR(0, "Reading key from here:");
  GWEN_DB_Dump(db, stderr, 2);*/
  key=GWEN_CryptKey_Factory(GWEN_DB_GetCharValue(db, "type", 0, ""));
  assert(key);
  GWEN_KeySpec_SetKeyName(key->keyspec,
                          GWEN_DB_GetCharValue(db, "name", 0, ""));
  GWEN_KeySpec_SetOwner(key->keyspec,
                        GWEN_DB_GetCharValue(db, "owner", 0, ""));
  GWEN_KeySpec_SetNumber(key->keyspec,
                         GWEN_DB_GetIntValue(db, "number", 0, 0));
  GWEN_KeySpec_SetVersion(key->keyspec,
                          GWEN_DB_GetIntValue(db, "version", 0, 0));
  gr=GWEN_DB_GetGroup(db,
                      GWEN_DB_FLAGS_DEFAULT,
                      "data");
  assert(key->fromDbFn);
  err=key->fromDbFn(key, gr);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_CryptKey_free(key);
    return 0;
  }
  return key;
}



GWEN_ERRORCODE GWEN_CryptKey_ToDb(const GWEN_CRYPTKEY *key,
                                  GWEN_DB_NODE *db,
                                  int pub){
  GWEN_DB_NODE *gr;

  assert(key);
  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "type", GWEN_KeySpec_GetKeyType(key->keyspec));
  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "name", GWEN_KeySpec_GetKeyName(key->keyspec));
  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "owner", GWEN_KeySpec_GetOwner(key->keyspec));
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "number", GWEN_KeySpec_GetNumber(key->keyspec));
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "version", GWEN_KeySpec_GetVersion(key->keyspec));

  gr=GWEN_DB_GetGroup(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                      "data");
  assert(key->toDbFn);
  return key->toDbFn(key, gr, pub);
}



GWEN_ERRORCODE GWEN_CryptKey_SetData(GWEN_CRYPTKEY *key,
                                     const char *buffer,
                                     unsigned int bsize) {
  GWEN_DB_NODE *n;
  GWEN_ERRORCODE err;

  assert(key);
  assert(buffer);
  assert(bsize);
  n=GWEN_DB_Group_new("data");
  GWEN_DB_SetBinValue(n,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keydata",
                      buffer, bsize);
  assert(key->fromDbFn);
  err=key->fromDbFn(key, n);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_DB_Group_free(n);
    return err;
  }
  GWEN_DB_Group_free(n);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKey_GetData(GWEN_CRYPTKEY *key,
                                     char *buffer,
                                     unsigned int *bsize) {
  GWEN_DB_NODE *n;
  GWEN_ERRORCODE err;
  const void *p;
  unsigned int size;

  assert(key);
  assert(buffer);
  assert(*bsize);
  assert(key->toDbFn);
  n=GWEN_DB_Group_new("data");
  err=key->toDbFn(key, n, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_DB_Group_free(n);
    return err;
  }

  p=GWEN_DB_GetBinValue(n, "keydata", 0, 0, 0, &size);
  if (!p) {
    DBG_INFO(0, "No key data");
    GWEN_DB_Group_free(n);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }
  if (size>*bsize) {
    DBG_INFO(0, "Buffer too small");
    GWEN_DB_Group_free(n);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }

  memmove(buffer, p, size);
  *bsize=size;
  GWEN_DB_Group_free(n);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKey_Generate(GWEN_CRYPTKEY *key,
                                      unsigned keylength){
  assert(key);
  assert(key->generateKeyFn);
  return key->generateKeyFn(key, keylength);
}



int GWEN_CryptKey_FromPassword(const char *password,
                               unsigned char *buffer,
                               unsigned int bsize){
  const char *algo;
  unsigned int nsize;

  switch (bsize) {
  case 16: algo="MD5"; break;
  case 20: algo="RMD160"; break;
  default:
    DBG_ERROR(0, "Bad size (%d)", bsize);
    return -1;
  } /* switch */

  nsize=bsize;
  if (GWEN_Md_Hash(algo,
                   password,
                   strlen(password),
                   buffer,
                   &nsize)) {
    DBG_INFO(0, "here");
    return -1;
  }

  assert(nsize==bsize);
  return 0;
}



GWEN_ERRORCODE GWEN_CryptKey_Open(GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->openFn);
  return key->openFn(key);
}



GWEN_ERRORCODE GWEN_CryptKey_Close(GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->closeFn);
  return key->closeFn(key);
}



const char *GWEN_CryptKey_GetKeyType(const GWEN_CRYPTKEY *key){
  assert(key);
  return GWEN_KeySpec_GetKeyType(key->keyspec);
}



void GWEN_CryptKey_SetKeyType(GWEN_CRYPTKEY *key,
                              const char *s){
  assert(key);
  assert(s);
  GWEN_KeySpec_SetKeyType(key->keyspec, s);
}



const char *GWEN_CryptKey_GetKeyName(const GWEN_CRYPTKEY *key){
  assert(key);
  return GWEN_KeySpec_GetKeyName(key->keyspec);
}



void GWEN_CryptKey_SetKeyName(GWEN_CRYPTKEY *key,
                              const char *s){
  assert(key);
  assert(s);
  GWEN_KeySpec_SetKeyName(key->keyspec, s);
}



const char *GWEN_CryptKey_GetOwner(const GWEN_CRYPTKEY *key){
  assert(key);
  return GWEN_KeySpec_GetOwner(key->keyspec);
}



void GWEN_CryptKey_SetOwner(GWEN_CRYPTKEY *key,
                            const char *s){
  assert(key);
  assert(s);
  GWEN_KeySpec_SetOwner(key->keyspec, s);
}



unsigned int GWEN_CryptKey_GetNumber(const GWEN_CRYPTKEY *key){
  assert(key);
  return GWEN_KeySpec_GetNumber(key->keyspec);
}



void GWEN_CryptKey_SetNumber(GWEN_CRYPTKEY *key,
                             unsigned int i){
  assert(key);
  GWEN_KeySpec_SetNumber(key->keyspec, i);
}



unsigned int GWEN_CryptKey_GetVersion(const GWEN_CRYPTKEY *key){
  assert(key);
  return GWEN_KeySpec_GetVersion(key->keyspec);
}



void GWEN_CryptKey_SetVersion(GWEN_CRYPTKEY *key,
                              unsigned int i){
  assert(key);
  GWEN_KeySpec_SetVersion(key->keyspec, i);
}



void *GWEN_CryptKey_GetKeyData(const GWEN_CRYPTKEY *key){
  assert(key);
  return key->keyData;
}



void GWEN_CryptKey_SetKeyData(GWEN_CRYPTKEY *key,
                              void *kd){
  assert(key);
  assert(kd);

  if (key->keyData && key->freeKeyDataFn)
    key->freeKeyDataFn(key);
  key->keyData=kd;
}



const GWEN_KEYSPEC *GWEN_CryptKey_GetKeySpec(const GWEN_CRYPTKEY *key){
  assert(key);
  return key->keyspec;
}


int GWEN_CryptKey_GetOpenCount(const GWEN_CRYPTKEY *key){
  assert(key);
  return key->openCount;
}



void GWEN_CryptKey_IncrementOpenCount(GWEN_CRYPTKEY *key){
  assert(key);
  key->openCount++;
}



void GWEN_CryptKey_DecrementOpenCount(GWEN_CRYPTKEY *key){
  assert(key);
  if (key->openCount>0)
    key->openCount--;
  else {
    DBG_WARN(0, "OpenCount already 0");
  }
}


void GWEN_CryptKey_SetEncryptFn(GWEN_CRYPTKEY *key,
                                GWEN_CRYPTKEY_ENCRYPT_FN encryptFn){
  assert(key);
  key->encryptFn=encryptFn;
}



void GWEN_CryptKey_SetDecryptFn(GWEN_CRYPTKEY *key,
                                GWEN_CRYPTKEY_DECRYPT_FN decryptFn){
  assert(key);
  key->decryptFn=decryptFn;
}



void GWEN_CryptKey_SetSignFn(GWEN_CRYPTKEY *key,
                             GWEN_CRYPTKEY_SIGN_FN signFn){
  assert(key);
  key->signFn=signFn;
}



void GWEN_CryptKey_SetVerifyFn(GWEN_CRYPTKEY *key,
                               GWEN_CRYPTKEY_VERIFY_FN verifyFn){
  assert(key);
  key->verifyFn=verifyFn;
}



void GWEN_CryptKey_SetGetChunkSizeFn(GWEN_CRYPTKEY *key,
                                     GWEN_CRYPTKEY_GETCHUNKSIZE_FN getChunkSizeFn){
  assert(key);
  key->getChunkSizeFn=getChunkSizeFn;
}



void GWEN_CryptKey_SetFromDbFn(GWEN_CRYPTKEY *key,
                               GWEN_CRYPTKEY_FROMDB_FN fromDbFn){
  assert(key);
  key->fromDbFn=fromDbFn;
}



void GWEN_CryptKey_SetToDbFn(GWEN_CRYPTKEY *key,
                             GWEN_CRYPTKEY_TODB_FN toDbFn){
  assert(key);
  key->toDbFn=toDbFn;
}



void GWEN_CryptKey_SetGenerateKeyFn(GWEN_CRYPTKEY *key,
                                    GWEN_CRYPTKEY_GENERATEKEY_FN generateKeyFn){
  assert(key);
  key->generateKeyFn=generateKeyFn;
}



void GWEN_CryptKey_SetFreeKeyDataFn(GWEN_CRYPTKEY *key,
                                    GWEN_CRYPTKEY_FREEKEYDATA_FN freeKeyDataFn){
  assert(key);
  key->freeKeyDataFn=freeKeyDataFn;
}



void GWEN_CryptKey_SetOpenFn(GWEN_CRYPTKEY *key,
                             GWEN_CRYPTKEY_OPEN_FN openFn){
  assert(key);
  key->openFn=openFn;
}



void GWEN_CryptKey_SetCloseFn(GWEN_CRYPTKEY *key,
                              GWEN_CRYPTKEY_CLOSE_FN closeKeyFn){
  assert(key);
  key->closeFn=closeKeyFn;
}



void GWEN_CryptKey_SetDupFn(GWEN_CRYPTKEY *key,
                            GWEN_CRYPTKEY_DUP_FN dupFn){
  assert(key);
  key->dupFn=dupFn;
}



int GWEN_CryptKey_IsPublic(const GWEN_CRYPTKEY *key){
  assert(key);
  return key->pub;
}



void GWEN_CryptKey_SetPublic(GWEN_CRYPTKEY *key, int i){
  assert(key);
  key->pub=i;
}





GWEN_CRYPTKEY_PROVIDER *GWEN_CryptProvider_new(){
  GWEN_CRYPTKEY_PROVIDER *pr;

  GWEN_NEW_OBJECT(GWEN_CRYPTKEY_PROVIDER, pr);
  return pr;
}



void GWEN_CryptProvider_free(GWEN_CRYPTKEY_PROVIDER *pr){
  if (pr) {
    free(pr->name);
    free(pr);
  }
}



void GWEN_CryptProvider_SetNewKeyFn(GWEN_CRYPTKEY_PROVIDER *pr,
                                    GWEN_CRYPTPROVIDER_NEWKEY_FN newKeyFn){
  assert(pr);
  pr->newKeyFn=newKeyFn;
}



GWEN_CRYPTKEY_PROVIDER *GWEN_Crypt_FindProvider(const char *name){
  GWEN_CRYPTKEY_PROVIDER *pr;

  pr=gwen_crypt_providers;
  while(pr) {
    if (strcasecmp(pr->name, name)==0)
      return pr;
    pr=pr->next;
  } /* while */

  return 0;
}



GWEN_ERRORCODE GWEN_Crypt_RegisterProvider(GWEN_CRYPTKEY_PROVIDER *pr){
  assert(pr);

  if (GWEN_Crypt_FindProvider(pr->name)){
    DBG_INFO(0, "Service \"%s\" already registered", pr->name);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_ALREADY_REGISTERED);
  }

  GWEN_LIST_ADD(GWEN_CRYPTKEY_PROVIDER, pr, &gwen_crypt_providers);
  return 0;
}



GWEN_ERRORCODE GWEN_Crypt_UnregisterProvider(GWEN_CRYPTKEY_PROVIDER *pr){
  assert(pr);

  if (!GWEN_Crypt_FindProvider(pr->name)){
    DBG_INFO(0, "Service \"%s\" not registered", pr->name);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_NOT_REGISTERED);
  }

  GWEN_LIST_DEL(GWEN_CRYPTKEY_PROVIDER, pr, &gwen_crypt_providers);
  return 0;
}


void GWEN_CryptProvider_SetName(GWEN_CRYPTKEY_PROVIDER *pr,
                                const char *name){
  assert(pr);
  assert(name);
  free(pr->name);
  pr->name=strdup(name);
}



GWEN_CRYPTKEY *GWEN_CryptKey_Factory(const char *t){
  GWEN_CRYPTKEY_PROVIDER *pr;
  GWEN_CRYPTKEY *key;

  pr=GWEN_Crypt_FindProvider(t);
  if (!pr) {
    DBG_ERROR(0, "No crypt provider for \"%s\" found", t);
    return 0;
  }

  key=pr->newKeyFn(pr);
  if (key)
    GWEN_CryptKey_SetKeyType(key, t);
  return key;
}



void GWEN_Crypt_UnregisterAllProviders(){
  GWEN_CRYPTKEY_PROVIDER *pr;

  pr=gwen_crypt_providers;
  while(pr) {
    GWEN_CRYPTKEY_PROVIDER *npr;

    npr=pr->next;
    GWEN_Crypt_UnregisterProvider(pr);
    GWEN_CryptProvider_free(pr);
    pr=npr;
  } /* while */
}







