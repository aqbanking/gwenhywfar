/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "hashalgo_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_LIST2_FUNCTIONS(GWEN_CRYPT_HASHALGO, GWEN_Crypt_HashAlgo)



GWEN_CRYPT_HASHALGOID GWEN_Crypt_HashAlgoId_fromString(const char *s) {
    assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_Crypt_HashAlgoId_None;
  else if (strcasecmp(s, "sha1")==0)
    return GWEN_Crypt_HashAlgoId_Sha1;
  else if (strcasecmp(s, "rmd160")==0)
    return GWEN_Crypt_HashAlgoId_Rmd160;
  else if (strcasecmp(s, "md5")==0)
    return GWEN_Crypt_HashAlgoId_Md5;
  else if (strcasecmp(s, "any")==0)
    return GWEN_Crypt_HashAlgoId_Any;
  return GWEN_Crypt_HashAlgoId_Unknown;
}



const char *GWEN_Crypt_HashAlgoId_toString(GWEN_CRYPT_HASHALGOID a) {
  switch(a) {
  case GWEN_Crypt_HashAlgoId_None:
    return "none";
  case GWEN_Crypt_HashAlgoId_Sha1:
    return "sha1";
  case GWEN_Crypt_HashAlgoId_Rmd160:
    return "rmd160";
  case GWEN_Crypt_HashAlgoId_Md5:
    return "md5";
  case GWEN_Crypt_HashAlgoId_Any:
    return "any";
  default:
    return "unknown";
  }
}



GWEN_CRYPT_HASHALGO *GWEN_Crypt_HashAlgo_new(GWEN_CRYPT_HASHALGOID id) {
  GWEN_CRYPT_HASHALGO *a;

  GWEN_NEW_OBJECT(GWEN_CRYPT_HASHALGO, a);
  a->refCount=1;

  a->id=id;

  return a;
}



void GWEN_Crypt_HashAlgo_Attach(GWEN_CRYPT_HASHALGO *a) {
  assert(a);
  assert(a->refCount);
  a->refCount++;
}



GWEN_CRYPT_HASHALGO *GWEN_Crypt_HashAlgo_fromDb(GWEN_DB_NODE *db) {
  const char *s;

  assert(db);
  s=GWEN_DB_GetCharValue(db, "id", 0, NULL);
  if (s) {
    GWEN_CRYPT_HASHALGO *a;
    GWEN_CRYPT_HASHALGOID id;
    const void *p;
    unsigned int len;

    id=GWEN_Crypt_HashAlgoId_fromString(s);
    if (id==GWEN_Crypt_HashAlgoId_Unknown) {
      DBG_INFO(GWEN_LOGDOMAIN, "Unknown hashalgo id [%s]", s);
      return NULL;
    }
    a=GWEN_Crypt_HashAlgo_new(id);
    assert(a);
    p=GWEN_DB_GetBinValue(db, "initVector", 0, NULL, 0, &len);
    if (p && len)
      GWEN_Crypt_HashAlgo_SetInitVector(a, p, len);

    return a;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Missing hashalgo id");
    return NULL;
  }
}



int GWEN_Crypt_HashAlgo_toDb(const GWEN_CRYPT_HASHALGO *a, GWEN_DB_NODE *db) {
  assert(a);
  assert(a->refCount);

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "id",
		       GWEN_Crypt_HashAlgoId_toString(a->id));
  if (a->pInitVector && a->lInitVector)
    GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"initVector",
			a->pInitVector, a->lInitVector);

  return 0;
}



GWEN_CRYPT_HASHALGO *GWEN_Crypt_HashAlgo_dup(const GWEN_CRYPT_HASHALGO *na) {
  GWEN_CRYPT_HASHALGO *a;

  assert(na);
  a=GWEN_Crypt_HashAlgo_new(na->id);
  if (na->pInitVector && na->lInitVector) {
    a->pInitVector=(uint8_t*) malloc(na->lInitVector);
    if (a->pInitVector==NULL) {
      GWEN_Crypt_HashAlgo_free(a);
      return NULL;
    }
    else
      memmove(a->pInitVector, na->pInitVector, na->lInitVector);
    a->lInitVector=na->lInitVector;
  }

  return a;
}



void GWEN_Crypt_HashAlgo_free(GWEN_CRYPT_HASHALGO *a) {
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



GWEN_CRYPT_HASHALGOID GWEN_Crypt_HashAlgo_GetId(const GWEN_CRYPT_HASHALGO *a){
  assert(a);
  assert(a->refCount);
  return a->id;
}



uint8_t *GWEN_Crypt_HashAlgo_GetInitVectorPtr(const GWEN_CRYPT_HASHALGO *a){
  assert(a);
  assert(a->refCount);
  return a->pInitVector;
}



uint32_t GWEN_Crypt_HashAlgo_GetInitVectorLen(const GWEN_CRYPT_HASHALGO *a){
  assert(a);
  assert(a->refCount);
  return a->lInitVector;
}



int GWEN_Crypt_HashAlgo_SetInitVector(GWEN_CRYPT_HASHALGO *a,
				      const uint8_t *pv,
				      uint32_t lv) {
  uint8_t *nv=NULL;

  assert(a);
  assert(a->refCount);

  if (pv && lv) {
    nv=(uint8_t*) malloc(lv);
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




