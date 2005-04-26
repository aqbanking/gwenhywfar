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
#include <time.h>


static int gwen_crypt_is_initialized=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_crypt_errorform=0;


static GWEN_CRYPTKEY_PROVIDER *gwen_crypt_providers=0;

#define GWEN_RANDSTATE_BUFSIZE 256
#ifdef HAVE_RANDOM
static char gwen_random_state[GWEN_RANDSTATE_BUFSIZE];
#endif


GWEN_INHERIT_FUNCTIONS(GWEN_CRYPTKEY)
GWEN_LIST2_FUNCTIONS(GWEN_CRYPTKEY, GWEN_CryptKey)


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
  case GWEN_CRYPT_ERROR_GENERIC:
    s="Generic crypto error.";
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

    /* seed random number generator*/
#ifdef HAVE_SRANDOM
    {
      char* prev_randstate = initstate((unsigned int)time(0),
				       gwen_random_state,
				       GWEN_RANDSTATE_BUFSIZE);
      setstate(prev_randstate);
    }
#else
    srand((unsigned int)time(0));
#endif

    gwen_crypt_is_initialized=1;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_Crypt_ModuleFini(){
  if (gwen_crypt_is_initialized) {
    GWEN_ERRORCODE err;

    GWEN_Crypt_UnregisterAllProviders();
    GWEN_MD_UnregisterAllProviders();

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
      DBG_WARN(GWEN_LOGDOMAIN, "Still %d CryptKeys in memory", GWEN_CryptKey_Count);
    }
#endif
  }
  return 0;
}





GWEN_CRYPTKEY *GWEN_CryptKey_new(){
  GWEN_CRYPTKEY *ck;

  GWEN_NEW_OBJECT(GWEN_CRYPTKEY, ck);
  GWEN_INHERIT_INIT(GWEN_CRYPTKEY, ck);
  DBG_MEM_INC("GWEN_CRYPTKEY", 0);

  ck->keyspec=GWEN_KeySpec_new();
  return ck;
}


void GWEN_CryptKey_free(GWEN_CRYPTKEY *key){
  if (key) {
    DBG_MEM_DEC("GWEN_CRYPTKEY");
    GWEN_INHERIT_FINI(GWEN_CRYPTKEY, key);
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
  DBG_INFO(GWEN_LOGDOMAIN, "Freeing Keyspec");
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

  key=GWEN_CryptKey_Factory(GWEN_DB_GetCharValue(db, "type", 0, ""));
  if (!key) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not create key");
    return 0;
  }
  if (GWEN_KeySpec_FromDb(key->keyspec, db)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create keyspec from DB");
    GWEN_CryptKey_free(key);
    return 0;
  }

  key->flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);

  gr=GWEN_DB_GetGroup(db,
                      GWEN_DB_FLAGS_DEFAULT,
                      "data");
  assert(key->fromDbFn);
  err=key->fromDbFn(key, gr);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
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
  if (GWEN_KeySpec_ToDb(key->keyspec, db)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not store keyspec in DB");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_GENERIC);
  }
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "flags", key->flags);
  /* save key specific data */
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
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
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
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    GWEN_DB_Group_free(n);
    return err;
  }

  p=GWEN_DB_GetBinValue(n, "keydata", 0, 0, 0, &size);
  if (!p) {
    DBG_INFO(GWEN_LOGDOMAIN, "No key data");
    GWEN_DB_Group_free(n);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_BAD_SIZE);
  }
  if (size>*bsize) {
    DBG_INFO(GWEN_LOGDOMAIN, "Buffer too small");
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
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad size (%d)", bsize);
    return -1;
  } /* switch */

  nsize=bsize;
  if (GWEN_MD_Hash(algo,
                   password,
                   strlen(password),
                   /* GCC4 pointer-signedness fix: */ (char*) buffer,
                   &nsize)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
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



int GWEN_CryptKey_GetStatus(const GWEN_CRYPTKEY *key){
  assert(key);
  return GWEN_KeySpec_GetStatus(key->keyspec);
}



void GWEN_CryptKey_SetStatus(GWEN_CRYPTKEY *key, int i){
  assert(key);
  GWEN_KeySpec_SetStatus(key->keyspec, i);
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
  assert(key->keyspec);
  return GWEN_KeySpec_GetOwner(key->keyspec);
}



void GWEN_CryptKey_SetOwner(GWEN_CRYPTKEY *key,
                            const char *s){
  assert(key);
  assert(s);
  assert(key->keyspec);
  GWEN_KeySpec_SetOwner(key->keyspec, s);
}



unsigned int GWEN_CryptKey_GetNumber(const GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->keyspec);
  return GWEN_KeySpec_GetNumber(key->keyspec);
}



void GWEN_CryptKey_SetNumber(GWEN_CRYPTKEY *key,
                             unsigned int i){
  assert(key);
  assert(key->keyspec);
  GWEN_KeySpec_SetNumber(key->keyspec, i);
}



unsigned int GWEN_CryptKey_GetVersion(const GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->keyspec);
  return GWEN_KeySpec_GetVersion(key->keyspec);
}



void GWEN_CryptKey_SetVersion(GWEN_CRYPTKEY *key,
                              unsigned int i){
  assert(key);
  assert(key->keyspec);
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
    DBG_WARN(GWEN_LOGDOMAIN, "OpenCount already 0");
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
    DBG_INFO(GWEN_LOGDOMAIN, "Service \"%s\" already registered", pr->name);
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
    DBG_INFO(GWEN_LOGDOMAIN, "Service \"%s\" not registered", pr->name);
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
    DBG_ERROR(GWEN_LOGDOMAIN, "No crypt provider for \"%s\" found", t);
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



GWEN_CRYPTKEY *GWEN_CryptKey_List2__freeAll_cb(GWEN_CRYPTKEY *st, void *user_data) {
  GWEN_CryptKey_free(st);
return 0;
}


void GWEN_CryptKey_List2_freeAll(GWEN_CRYPTKEY_LIST2 *stl) {
  if (stl) {
    GWEN_CryptKey_List2_ForEach(stl, GWEN_CryptKey_List2__freeAll_cb, 0);
    GWEN_CryptKey_List2_free(stl); 
  }
}



GWEN_TYPE_UINT32 GWEN_CryptKey_GetFlags(const GWEN_CRYPTKEY *key){
  assert(key);
  return key->flags;
}



void GWEN_CryptKey_SetFlags(GWEN_CRYPTKEY *key, GWEN_TYPE_UINT32 fl){
  assert(key);
  key->flags=fl;
}



void GWEN_CryptKey_AddFlags(GWEN_CRYPTKEY *key, GWEN_TYPE_UINT32 fl){
  assert(key);
  key->flags|=fl;
}



void GWEN_CryptKey_SubFlags(GWEN_CRYPTKEY *key, GWEN_TYPE_UINT32 fl){
  assert(key);
  key->flags&=~fl;
}


long int GWEN_Random()
{
  long int result;
#ifdef HAVE_RANDOM
  char* prev_randstate = setstate(gwen_random_state);
  result = random();
  setstate(prev_randstate);
#else
  result = rand();
#endif
  return result;
}
