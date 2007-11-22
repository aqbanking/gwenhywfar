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


#ifndef GWENHYWFAR_MDIGEST_P_H
#define GWENHYWFAR_MDIGEST_P_H

#include "mdigest_be.h"


struct GWEN_MDIGEST {
  GWEN_INHERIT_ELEMENT(GWEN_MDIGEST)
  GWEN_LIST_ELEMENT(GWEN_MDIGEST)

  GWEN_CRYPT_HASHALGOID hashAlgoId;

  uint8_t *pDigest;
  unsigned int lDigest;

  GWEN_MDIGEST_BEGIN_FN beginFn;
  GWEN_MDIGEST_END_FN endFn;
  GWEN_MDIGEST_UPDATE_FN updateFn;

  int refCount;
};



#endif


