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


#ifndef GWEN_CRYPT_KEY_P_H
#define GWEN_CRYPT_KEY_P_H

#include "cryptkey_be.h"



struct GWEN_CRYPT_KEY {
  GWEN_INHERIT_ELEMENT(GWEN_CRYPT_KEY)
  GWEN_LIST_ELEMENT(GWEN_CRYPT_KEY)

  GWEN_CRYPT_CRYPTALGOID cryptAlgoId;
  int keySize;

  int keyNumber;
  int keyVersion;

  GWEN_CRYPT_KEY_SIGN_FN signFn;
  GWEN_CRYPT_KEY_VERIFY_FN verifyFn;

  GWEN_CRYPT_KEY_ENCIPHER_FN encipherFn;
  GWEN_CRYPT_KEY_DECIPHER_FN decipherFn;

  int refCount;
};



#endif

