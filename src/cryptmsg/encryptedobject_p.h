/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_ENCRYPTEDOBJECT_P_H
#define GWEN_CRYPTMGR_ENCRYPTEDOBJECT_P_H

#include "encryptedobject.h"

#define GWEN_ENCRYPTEDOBJECT_TLV_CRYPTHEAD    0x21
#define GWEN_ENCRYPTEDOBJECT_TLV_CRYPTDATA    0x22



struct GWEN_ENCRYPTED_OBJECT {
  uint8_t *pData;
  uint32_t lData;
  int dataOwned;

  GWEN_CRYPTHEAD *cryptHead;
};


#endif

