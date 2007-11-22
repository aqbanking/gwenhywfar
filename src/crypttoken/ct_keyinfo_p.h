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


#ifndef GWEN_CRYPT_TOKEN_KEYINFO_P_H
#define GWEN_CRYPT_TOKEN_KEYINFO_P_H


#include "ct_keyinfo.h"



struct GWEN_CRYPT_TOKEN_KEYINFO {
  GWEN_LIST_ELEMENT(GWEN_CRYPT_TOKEN_KEYINFO)

  uint32_t keyId;
  GWEN_CRYPT_CRYPTALGOID cryptAlgoId;
  int keySize;
  uint32_t flags;

  char *keyDescr;

  uint8_t *modulusData;
  uint32_t modulusLen;

  uint8_t *exponentData;
  uint32_t exponentLen;

  uint32_t keyNumber;
  uint32_t keyVersion;
  uint32_t signCounter;

  int refCount;
};



#endif



