/***************************************************************************
    begin       : Sun Nov 30 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_SIGNEDOBJECT_P_H
#define GWEN_CRYPTMGR_SIGNEDOBJECT_P_H

#include "signedobject.h"

#define GWEN_SIGNEDOBJECT_TLV_SIGHEAD    0x21
#define GWEN_SIGNEDOBJECT_TLV_SIGTAIL    0x22
#define GWEN_SIGNEDOBJECT_TLV_SIGDATA    0x23



struct GWEN_SIGNED_OBJECT {
  uint8_t *pData;
  uint32_t lData;
  int dataOwned;

  GWEN_SIGHEAD_LIST *sigHeadList;
  GWEN_SIGTAIL_LIST *sigTailList;
};


#endif

