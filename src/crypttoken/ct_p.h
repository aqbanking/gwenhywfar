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


#ifndef GWEN_CRYPT_CRYPTTOKEN_P_H
#define GWEN_CRYPT_CRYPTTOKEN_P_H


#include <gwenhywfar/ct_be.h>



struct GWEN_CRYPT_TOKEN {
  GWEN_INHERIT_ELEMENT(GWEN_CRYPT_TOKEN)
  GWEN_LIST_ELEMENT(GWEN_CRYPT_TOKEN)

  GWEN_CRYPT_TOKEN_DEVICE device;
  char *typeName;
  char *tokenName;

  char *friendlyName;

  uint32_t modes;
  uint32_t flags;

  GWEN_CRYPT_TOKEN_OPEN_FN openFn;
  GWEN_CRYPT_TOKEN_CREATE_FN createFn;
  GWEN_CRYPT_TOKEN_CLOSE_FN closeFn;

  GWEN_CRYPT_TOKEN_GETKEYIDLIST_FN getKeyIdListFn;
  GWEN_CRYPT_TOKEN_GETKEYINFO_FN getKeyInfoFn;
  GWEN_CRYPT_TOKEN_SETKEYINFO_FN setKeyInfoFn;

  GWEN_CRYPT_TOKEN_GETCONTEXTIDLIST_FN getContextIdListFn;
  GWEN_CRYPT_TOKEN_GETCONTEXT_FN getContextFn;
  GWEN_CRYPT_TOKEN_SETCONTEXT_FN setContextFn;

  GWEN_CRYPT_TOKEN_SIGN_FN signFn;
  GWEN_CRYPT_TOKEN_VERIFY_FN verifyFn;
  GWEN_CRYPT_TOKEN_ENCIPHER_FN encipherFn;
  GWEN_CRYPT_TOKEN_DECIPHER_FN decipherFn;

  GWEN_CRYPT_TOKEN_CHANGEPIN_FN changePinFn;

  GWEN_CRYPT_TOKEN_GENERATEKEY_FN generateKeyFn;

  GWEN_CRYPT_TOKEN_ACTIVATEKEY_FN activateKeyFn;

  int openCount;

  int refCount;
};





#endif


