/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#define DISABLE_DEBUGLOG


#include "paddalgo_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_LIST2_FUNCTIONS(GWEN_CRYPT_PADDALGO, GWEN_Crypt_PaddAlgo)



GWEN_CRYPT_PADDALGOID GWEN_Crypt_PaddAlgoId_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_Crypt_PaddAlgoId_None;
  else if (strcasecmp(s, "iso9796_1")==0)
    return GWEN_Crypt_PaddAlgoId_Iso9796_1;
  else if (strcasecmp(s, "iso9796_1a4")==0)
    return GWEN_Crypt_PaddAlgoId_Iso9796_1A4;
  else if (strcasecmp(s, "iso9796_2")==0)
    return GWEN_Crypt_PaddAlgoId_Iso9796_2;
  else if (strcasecmp(s, "pkcs1_1")==0)
    return GWEN_Crypt_PaddAlgoId_Pkcs1_1;
  else if (strcasecmp(s, "pkcs1_2")==0)
    return GWEN_Crypt_PaddAlgoId_Pkcs1_2;
  else if (strcasecmp(s, "leftzero")==0)
    return GWEN_Crypt_PaddAlgoId_LeftZero;
  else if (strcasecmp(s, "rightzero")==0)
    return GWEN_Crypt_PaddAlgoId_RightZero;
  else if (strcasecmp(s, "ansix9_23")==0)
    return GWEN_Crypt_PaddAlgoId_AnsiX9_23;
  else if (strcasecmp(s, "pkcs1_pss_sha256")==0)
    return GWEN_Crypt_PaddAlgoId_Pkcs1_Pss_Sha256;
  else if (strcasecmp(s, "any")==0)
    return GWEN_Crypt_PaddAlgoId_Any;
  return GWEN_Crypt_PaddAlgoId_Unknown;
}



const char *GWEN_Crypt_PaddAlgoId_toString(GWEN_CRYPT_PADDALGOID a) {
  switch(a) {
  case GWEN_Crypt_PaddAlgoId_None:
    return "none";
  case GWEN_Crypt_PaddAlgoId_Iso9796_1:
    return "iso9796_1";
  case GWEN_Crypt_PaddAlgoId_Iso9796_1A4:
    return "iso9796_1a4";
  case GWEN_Crypt_PaddAlgoId_Iso9796_2:
    return "iso9796_2";
  case GWEN_Crypt_PaddAlgoId_Pkcs1_1:
    return "pkcs1_1";
  case GWEN_Crypt_PaddAlgoId_Pkcs1_2:
    return "pkcs1_2";
  case GWEN_Crypt_PaddAlgoId_LeftZero:
    return "leftzero";
  case GWEN_Crypt_PaddAlgoId_RightZero:
    return "rightzero";
  case GWEN_Crypt_PaddAlgoId_AnsiX9_23:
    return "ansix9_23";
  case GWEN_Crypt_PaddAlgoId_Pkcs1_Pss_Sha256:
    return "pkcs1_pss_sha256";
  case GWEN_Crypt_PaddAlgoId_Any:
    return "any";
  default:
    return "unknown";
  }
}



GWEN_CRYPT_PADDALGO *GWEN_Crypt_PaddAlgo_new(GWEN_CRYPT_PADDALGOID id) {
  GWEN_CRYPT_PADDALGO *a;

  GWEN_NEW_OBJECT(GWEN_CRYPT_PADDALGO, a);
  a->refCount=1;

  a->id=id;

  return a;
}



void GWEN_Crypt_PaddAlgo_Attach(GWEN_CRYPT_PADDALGO *a) {
  assert(a);
  assert(a->refCount);
  a->refCount++;
}



GWEN_CRYPT_PADDALGO *GWEN_Crypt_PaddAlgo_fromDb(GWEN_DB_NODE *db) {
  const char *s;

  assert(db);
  s=GWEN_DB_GetCharValue(db, "id", 0, NULL);
  if (s) {
    GWEN_CRYPT_PADDALGO *a;
    GWEN_CRYPT_PADDALGOID id;

    id=GWEN_Crypt_PaddAlgoId_fromString(s);
    if (id==GWEN_Crypt_PaddAlgoId_Unknown) {
      DBG_INFO(GWEN_LOGDOMAIN, "Unknown paddalgo id [%s]", s);
      return NULL;
    }
    a=GWEN_Crypt_PaddAlgo_new(id);
    assert(a);

    a->paddSize=GWEN_DB_GetIntValue(db, "paddSize", 0, 0);

    return a;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Missing paddalgo id");
    return NULL;
  }
}



int GWEN_Crypt_PaddAlgo_toDb(const GWEN_CRYPT_PADDALGO *a, GWEN_DB_NODE *db) {
  assert(a);
  assert(a->refCount);

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "id",
		       GWEN_Crypt_PaddAlgoId_toString(a->id));
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "paddSize", a->paddSize);
  return 0;
}



GWEN_CRYPT_PADDALGO *GWEN_Crypt_PaddAlgo_dup(const GWEN_CRYPT_PADDALGO *na) {
  GWEN_CRYPT_PADDALGO *a;

  assert(na);
  a=GWEN_Crypt_PaddAlgo_new(na->id);
  a->paddSize=na->paddSize;
  return a;
}



void GWEN_Crypt_PaddAlgo_free(GWEN_CRYPT_PADDALGO *a) {
  if (a) {
    assert(a->refCount);
    if (a->refCount==1) {
      a->refCount--;
      GWEN_FREE_OBJECT(a);
    }
    else {
      a->refCount--;
    }
  }
}



GWEN_CRYPT_PADDALGOID GWEN_Crypt_PaddAlgo_GetId(const GWEN_CRYPT_PADDALGO *a){
  assert(a);
  assert(a->refCount);
  return a->id;
}



int GWEN_Crypt_PaddAlgo_GetPaddSize(const GWEN_CRYPT_PADDALGO *a){
  assert(a);
  assert(a->refCount);
  return a->paddSize;
}



void GWEN_Crypt_PaddAlgo_SetPaddSize(GWEN_CRYPT_PADDALGO *a, int s){
  assert(a);
  assert(a->refCount);
  a->paddSize=s;
}




