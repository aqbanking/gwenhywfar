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


#include "mdigest_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




GWEN_INHERIT_FUNCTIONS(GWEN_MDIGEST)
GWEN_LIST_FUNCTIONS(GWEN_MDIGEST, GWEN_MDigest)
GWEN_LIST2_FUNCTIONS(GWEN_MDIGEST, GWEN_MDigest)





GWEN_MDIGEST *GWEN_MDigest_new(GWEN_CRYPT_HASHALGOID a) {
  GWEN_MDIGEST *md;

  GWEN_NEW_OBJECT(GWEN_MDIGEST, md)
  md->refCount=1;
  GWEN_INHERIT_INIT(GWEN_MDIGEST, md)
  GWEN_LIST_INIT(GWEN_MDIGEST, md)

  md->hashAlgoId=a;
  return md;
}



void GWEN_MDigest_free(GWEN_MDIGEST *md) {
  if (md) {
    assert(md->refCount);
    if (md->refCount==1) {
      free(md->pDigest);
      md->refCount=0;
      GWEN_FREE_OBJECT(md);
    }
    else
      md->refCount--;
  }
}



GWEN_CRYPT_HASHALGOID GWEN_MDigest_GetHashAlgoId(const GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  return md->hashAlgoId;
}



uint8_t *GWEN_MDigest_GetDigestPtr(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  return md->pDigest;
}



unsigned int GWEN_MDigest_GetDigestSize(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  return md->lDigest;
}



void GWEN_MDigest_SetDigestBuffer(GWEN_MDIGEST *md, uint8_t *buf, unsigned int l) {
  assert(md);
  assert(md->refCount);

  if (l) {
    assert(buf);
  }

  if (md->pDigest && md->lDigest)
    free(md->pDigest);
  md->pDigest=buf;
  md->lDigest=l;
}



void GWEN_MDigest_SetDigestLen(GWEN_MDIGEST *md, unsigned int l) {
  assert(md);
  assert(md->refCount);

  if (md->pDigest && md->lDigest)
    free(md->pDigest);
  md->pDigest=NULL;
  md->lDigest=l;
}



int GWEN_MDigest_Begin(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  if (md->beginFn)
    return md->beginFn(md);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_MDigest_End(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  if (md->endFn)
    return md->endFn(md);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_MDigest_Update(GWEN_MDIGEST *md, const uint8_t *buf, unsigned int l) {
  assert(md);
  assert(md->refCount);
  if (md->updateFn)
    return md->updateFn(md, buf, l);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_MDIGEST_BEGIN_FN GWEN_MDigest_SetBeginFn(GWEN_MDIGEST *md, GWEN_MDIGEST_BEGIN_FN f) {
  GWEN_MDIGEST_BEGIN_FN of;

  assert(md);
  assert(md->refCount);
  of=md->beginFn;
  md->beginFn=f;

  return of;
}



GWEN_MDIGEST_END_FN GWEN_MDigest_SetEndFn(GWEN_MDIGEST *md, GWEN_MDIGEST_END_FN f) {
  GWEN_MDIGEST_END_FN of;

  assert(md);
  assert(md->refCount);
  of=md->endFn;
  md->endFn=f;

  return of;
}



GWEN_MDIGEST_UPDATE_FN GWEN_MDigest_SetUpdateFn(GWEN_MDIGEST *md, GWEN_MDIGEST_UPDATE_FN f) {
  GWEN_MDIGEST_UPDATE_FN of;

  assert(md);
  assert(md->refCount);
  of=md->updateFn;
  md->updateFn=f;

  return of;
}



