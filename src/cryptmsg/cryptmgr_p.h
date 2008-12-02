/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMSG_CRYPTMSG_P_H
#define GWEN_CRYPTMSG_CRYPTMSG_P_H


#define GWEN_CRYPTMGR_TLV_SIGHEAD    0x21
#define GWEN_CRYPTMGR_TLV_SIGTAIL    0x22
#define GWEN_CRYPTMGR_TLV_SIGDATA    0x23

#define GWEN_CRYPTMGR_TLV_CRYPTHEAD  0x21
#define GWEN_CRYPTMGR_TLV_CRYPTDATA  0x22


#include "cryptmgr_be.h"



struct GWEN_CRYPTMGR {
  GWEN_INHERIT_ELEMENT(GWEN_CRYPTMGR)

  char *localKeyName;
  int localKeyNumber;
  int localKeyVersion;

  char *peerKeyName;
  int peerKeyNumber;
  int peerKeyVersion;

  int cryptProfile;
  int signatureProfile;

  GWEN_CRYPTMGR_SIGNDATA_FN signDataFn;
  GWEN_CRYPTMGR_ENCRYPTKEY_FN encryptKeyFn;
  GWEN_CRYPTMGR_VERIFYDATA_FN verifyDataFn;
  GWEN_CRYPTMGR_DECRYPTKEY_FN decryptKeyFn;
};



#endif


