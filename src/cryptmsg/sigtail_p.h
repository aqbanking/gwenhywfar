/***************************************************************************
    begin       : Sun Nov 30 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMSG_SIGTAIL_P_H
#define GWEN_CRYPTMSG_SIGTAIL_P_H

#include "sigtail.h"

#define GWEN_SIGTAIL_TLV_SIGNUM     0x01
#define GWEN_SIGTAIL_TLV_SIGNATURE  0x02



struct GWEN_SIGTAIL {
  GWEN_LIST_ELEMENT(GWEN_SIGTAIL)
  int signatureNumber;
  uint8_t *pSignature;
  uint32_t lSignature;
};


#endif

