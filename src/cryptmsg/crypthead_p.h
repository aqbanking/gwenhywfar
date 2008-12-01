/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_CRYPTHEAD_P_H
#define GWEN_CRYPTMGR_CRYPTHEAD_P_H

#include "crypthead.h"

#define GWEN_CRYPTHEAD_TLV_KEYNAME      0x01
#define GWEN_CRYPTHEAD_TLV_KEYNUM       0x02
#define GWEN_CRYPTHEAD_TLV_KEYVER       0x03
#define GWEN_CRYPTHEAD_TLV_CRYPTPROFILE 0x05
#define GWEN_CRYPTHEAD_TLV_KEY          0x06



struct GWEN_CRYPTHEAD {
  GWEN_LIST_ELEMENT(GWEN_CRYPTHEAD)
  char *keyName;
  int keyNumber;
  int keyVersion;
  int cryptProfile;
  uint8_t *pKey;
  uint32_t lKey;
};


#endif

