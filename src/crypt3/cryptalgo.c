/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#define DISABLE_DEBUGLOG


#include "cryptalgo_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_LIST2_FUNCTIONS(GWEN_CRYPT_CRYPTALGO, GWEN_Crypt_CryptAlgo)



GWEN_CRYPT_CRYPTALGOID GWEN_Crypt_CryptAlgoId_fromString(const char *s)
{
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_Crypt_CryptAlgoId_None;
  else if (strcasecmp(s, "rsa")==0)
    return GWEN_Crypt_CryptAlgoId_Rsa;
  else if (strcasecmp(s, "dsa")==0)
    return GWEN_Crypt_CryptAlgoId_Dsa;
  else if (strcasecmp(s, "des")==0)
    return GWEN_Crypt_CryptAlgoId_Des;
  else if (strcasecmp(s, "des_3k")==0 ||
           strcasecmp(s, "des3k")==0)
    return GWEN_Crypt_CryptAlgoId_Des3K;
  else if (strcasecmp(s, "blowfish")==0)
    return GWEN_Crypt_CryptAlgoId_BlowFish;
  else if (strcasecmp(s, "aes128")==0)
    return GWEN_Crypt_CryptAlgoId_Aes128;
  else if (strcasecmp(s, "any")==0)
    return GWEN_Crypt_CryptAlgoId_Any;
  return GWEN_Crypt_CryptAlgoId_Unknown;
}



const char *GWEN_Crypt_CryptAlgoId_toString(GWEN_CRYPT_CRYPTALGOID a)
{
  switch (a) {
  case GWEN_Crypt_CryptAlgoId_None:
    return "none";
  case GWEN_Crypt_CryptAlgoId_Rsa:
    return "rsa";
  case GWEN_Crypt_CryptAlgoId_Dsa:
    return "dsa";
  case GWEN_Crypt_CryptAlgoId_Des:
    return "des";
  case GWEN_Crypt_CryptAlgoId_Des3K:
    return "des_3k";
  case GWEN_Crypt_CryptAlgoId_BlowFish:
    return "blowfish";
  case GWEN_Crypt_CryptAlgoId_Aes128:
    return "aes128";
  case GWEN_Crypt_CryptAlgoId_Any:
    return "any";
  default:
    return "unknown";
  }
}



GWEN_CRYPT_CRYPTMODE GWEN_Crypt_CryptMode_fromString(const char *s)
{
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_Crypt_CryptMode_None;
  else if (strcasecmp(s, "ecb")==0)
    return GWEN_Crypt_CryptMode_Ecb;
  else if (strcasecmp(s, "cfb")==0)
    return GWEN_Crypt_CryptMode_Cfb;
  else if (strcasecmp(s, "cbc")==0)
    return GWEN_Crypt_CryptMode_Cbc;
  return GWEN_Crypt_CryptMode_Unknown;
}



const char *GWEN_Crypt_CryptMode_toString(GWEN_CRYPT_CRYPTMODE m)
{
  switch (m) {
  case GWEN_Crypt_CryptMode_None:
    return "none";
  case GWEN_Crypt_CryptMode_Ecb:
    return "ecb";
  case GWEN_Crypt_CryptMode_Cfb:
    return "cfb";
  case GWEN_Crypt_CryptMode_Cbc:
    return "cbc";
  default:
    return "unknown";
  }
}



GWEN_CRYPT_CRYPTALGO *GWEN_Crypt_CryptAlgo_new(GWEN_CRYPT_CRYPTALGOID id,
                                               GWEN_CRYPT_CRYPTMODE m)
{
  GWEN_CRYPT_CRYPTALGO *a;

  GWEN_NEW_OBJECT(GWEN_CRYPT_CRYPTALGO, a);
  a->refCount=1;

  a->id=id;
  a->mode=m;

  return a;
}



void GWEN_Crypt_CryptAlgo_Attach(GWEN_CRYPT_CRYPTALGO *a)
{
  assert(a);
  assert(a->refCount);
  a->refCount++;
}



GWEN_CRYPT_CRYPTALGO *GWEN_Crypt_CryptAlgo_fromDb(GWEN_DB_NODE *db)
{
  const char *s;

  assert(db);
  s=GWEN_DB_GetCharValue(db, "id", 0, NULL);
  if (s) {
    GWEN_CRYPT_CRYPTALGO *a;
    GWEN_CRYPT_CRYPTALGOID id;
    GWEN_CRYPT_CRYPTMODE m;
    const void *p;
    unsigned int len;

    id=GWEN_Crypt_CryptAlgoId_fromString(s);
    if (id==GWEN_Crypt_CryptAlgoId_Unknown) {
      DBG_INFO(GWEN_LOGDOMAIN, "Unknown cryptalgo id [%s]", s);
      return NULL;
    }

    s=GWEN_DB_GetCharValue(db, "mode", 0, NULL);
    if (s)
      m=GWEN_Crypt_CryptMode_fromString(s);
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Missing crypt mode");
      return NULL;
    }

    a=GWEN_Crypt_CryptAlgo_new(id, m);
    assert(a);
    p=GWEN_DB_GetBinValue(db, "initVector", 0, NULL, 0, &len);
    if (p && len)
      GWEN_Crypt_CryptAlgo_SetInitVector(a, p, len);

    a->chunkSize=GWEN_DB_GetIntValue(db, "chunkSize", 0, 0);
    a->keySizeInBits=GWEN_DB_GetIntValue(db, "keySizeInBits", 0, 0);

    return a;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Missing cryptalgo id");
    return NULL;
  }
}



int GWEN_Crypt_CryptAlgo_toDb(const GWEN_CRYPT_CRYPTALGO *a, GWEN_DB_NODE *db)
{
  assert(a);
  assert(a->refCount);

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "id",
                       GWEN_Crypt_CryptAlgoId_toString(a->id));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "mode",
                       GWEN_Crypt_CryptMode_toString(a->mode));
  if (a->pInitVector && a->lInitVector)
    GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "initVector",
                        a->pInitVector, a->lInitVector);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "chunkSize",
                      a->chunkSize);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keySizeInBits",
                      a->keySizeInBits);

  return 0;
}



GWEN_CRYPT_CRYPTALGO *GWEN_Crypt_CryptAlgo_dup(const GWEN_CRYPT_CRYPTALGO *na)
{
  GWEN_CRYPT_CRYPTALGO *a;

  assert(na);
  a=GWEN_Crypt_CryptAlgo_new(na->id, na->mode);
  if (na->pInitVector && na->lInitVector) {
    a->pInitVector=(uint8_t *) malloc(na->lInitVector);
    if (a->pInitVector==NULL) {
      GWEN_Crypt_CryptAlgo_free(a);
      return NULL;
    }
    else
      memmove(a->pInitVector, na->pInitVector, na->lInitVector);
    a->lInitVector=na->lInitVector;
  }
  a->chunkSize=na->chunkSize;
  a->keySizeInBits=na->keySizeInBits;
  return a;
}



void GWEN_Crypt_CryptAlgo_free(GWEN_CRYPT_CRYPTALGO *a)
{
  if (a) {
    assert(a->refCount);
    if (a->refCount==1) {
      if (a->pInitVector) {
        free(a->pInitVector);
        a->pInitVector=NULL;
      }
      a->refCount--;
      GWEN_FREE_OBJECT(a);
    }
    else {
      a->refCount--;
    }
  }
}



GWEN_CRYPT_CRYPTALGOID GWEN_Crypt_CryptAlgo_GetId(const GWEN_CRYPT_CRYPTALGO *a)
{
  assert(a);
  assert(a->refCount);
  return a->id;
}



GWEN_CRYPT_CRYPTMODE GWEN_Crypt_CryptAlgo_GetMode(const GWEN_CRYPT_CRYPTALGO *a)
{
  assert(a);
  assert(a->refCount);
  return a->mode;
}



uint8_t *GWEN_Crypt_CryptAlgo_GetInitVectorPtr(const GWEN_CRYPT_CRYPTALGO *a)
{
  assert(a);
  assert(a->refCount);
  return a->pInitVector;
}



uint32_t GWEN_Crypt_CryptAlgo_GetInitVectorLen(const GWEN_CRYPT_CRYPTALGO *a)
{
  assert(a);
  assert(a->refCount);
  return a->lInitVector;
}



int GWEN_Crypt_CryptAlgo_SetInitVector(GWEN_CRYPT_CRYPTALGO *a,
                                       const uint8_t *pv,
                                       uint32_t lv)
{
  uint8_t *nv=NULL;

  assert(a);
  assert(a->refCount);

  if (pv && lv) {
    nv=(uint8_t *) malloc(lv);
    if (nv==NULL)
      return GWEN_ERROR_MEMORY_FULL;
    memmove(nv, pv, lv);
  }

  if (a->pInitVector && a->lInitVector)
    free(a->pInitVector);

  a->pInitVector=nv;
  a->lInitVector=(nv!=NULL)?lv:0;

  return 0;
}



int GWEN_Crypt_CryptAlgo_GetChunkSize(const GWEN_CRYPT_CRYPTALGO *a)
{
  assert(a);
  assert(a->refCount);

  return a->chunkSize;
}



void GWEN_Crypt_CryptAlgo_SetChunkSize(GWEN_CRYPT_CRYPTALGO *a, int s)
{
  assert(a);
  assert(a->refCount);

  a->chunkSize=s;
}



int GWEN_Crypt_CryptAlgo_GetKeySizeInBits(const GWEN_CRYPT_CRYPTALGO *a)
{
  assert(a);
  assert(a->refCount);

  return a->keySizeInBits;
}



void GWEN_Crypt_CryptAlgo_SetKeySizeInBits(GWEN_CRYPT_CRYPTALGO *a, int s)
{
  assert(a);
  assert(a->refCount);

  a->keySizeInBits=s;
}







