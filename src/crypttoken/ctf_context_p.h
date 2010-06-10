/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CTF_CONTEXT_P_H
#define GWEN_CTF_CONTEXT_P_H


#include "ctf_context_be.h"



typedef struct GWEN_CTF_CONTEXT GWEN_CTF_CONTEXT;
struct GWEN_CTF_CONTEXT {
  GWEN_CRYPT_KEY *localSignKey;
  GWEN_CRYPT_TOKEN_KEYINFO *localSignKeyInfo;

  GWEN_CRYPT_KEY *localCryptKey;
  GWEN_CRYPT_TOKEN_KEYINFO *localCryptKeyInfo;

  GWEN_CRYPT_KEY *remoteSignKey;
  GWEN_CRYPT_TOKEN_KEYINFO *remoteSignKeyInfo;

  GWEN_CRYPT_KEY *remoteCryptKey;
  GWEN_CRYPT_TOKEN_KEYINFO *remoteCryptKeyInfo;

  GWEN_CRYPT_KEY *localAuthKey;
  GWEN_CRYPT_TOKEN_KEYINFO *localAuthKeyInfo;

  GWEN_CRYPT_KEY *remoteAuthKey;
  GWEN_CRYPT_TOKEN_KEYINFO *remoteAuthKeyInfo;

  GWEN_CRYPT_KEY *tempLocalSignKey;
  GWEN_CRYPT_TOKEN_KEYINFO *tempLocalSignKeyInfo;

};



static GWENHYWFAR_CB void GWEN_CTF_Context_freeData(void *bp, void *p);




#endif


