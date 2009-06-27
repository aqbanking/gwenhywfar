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


#ifndef GWEN_CRYPT_CRYPTALGO_P_H
#define GWEN_CRYPT_CRYPTALGO_P_H

#include "cryptalgo.h"


struct GWEN_CRYPT_CRYPTALGO {
  GWEN_CRYPT_CRYPTALGOID id;
  GWEN_CRYPT_CRYPTMODE mode;
  uint8_t *pInitVector;
  uint32_t lInitVector;

  int chunkSize;
  int keySizeInBits;

  uint32_t refCount;
};



#endif
