/***************************************************************************
    begin       : Sun Nov 30 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_SIGHEAD_P_H
#define GWEN_CRYPTMGR_SIGHEAD_P_H

#include "sighead.h"

#define GWEN_SIGHEAD_TLV_KEYNAME    0x01
#define GWEN_SIGHEAD_TLV_KEYNUM     0x02
#define GWEN_SIGHEAD_TLV_KEYVER     0x03
#define GWEN_SIGHEAD_TLV_DATETIME   0x04
#define GWEN_SIGHEAD_TLV_SIGPROFILE 0x05
#define GWEN_SIGHEAD_TLV_SIGNUM     0x06



struct GWEN_SIGHEAD {
  GWEN_LIST_ELEMENT(GWEN_SIGHEAD)
  char *keyName;
  int keyNumber;
  int keyVersion;
  GWEN_TIME *dateTime;
  int signatureProfile;
  int signatureNumber;
};


#endif

