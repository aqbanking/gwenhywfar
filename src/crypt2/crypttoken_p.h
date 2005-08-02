/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTTOKEN_P_H
#define GWEN_CRYPTTOKEN_P_H

#include <gwenhywfar/crypttoken.h>
#include "crypttoken_user_l.h"



struct GWEN_CRYPTTOKEN_SIGNINFO {
  GWEN_LIST_ELEMENT(GWEN_CRYPTTOKEN_SIGNINFO)
  GWEN_TYPE_UINT32 id;
  GWEN_CRYPTTOKEN_HASHALGO hashAlgo;
  GWEN_CRYPTTOKEN_PADDALGO paddAlgo;
};


struct GWEN_CRYPTTOKEN_CRYPTINFO {
  GWEN_LIST_ELEMENT(GWEN_CRYPTTOKEN_CRYPTINFO)
  GWEN_TYPE_UINT32 id;
  GWEN_CRYPTTOKEN_CRYPTALGO cryptAlgo;
  GWEN_CRYPTTOKEN_PADDALGO paddAlgo;
};


struct GWEN_CRYPTTOKEN_KEYINFO {
  GWEN_LIST_ELEMENT(GWEN_CRYPTTOKEN_KEYINFO)
  int keyId;
  int keySize;      /** key size in bits */
  int minKeySize;   /** minimum key size in bits */
  int maxKeySize;   /** maximum key size in bits */
  int chunkSize;    /** chunk size in bytes on which this key operates */
  GWEN_CRYPTTOKEN_CRYPTALGO cryptAlgo;
  char *keyDescription;
  GWEN_TYPE_UINT32 keyFlags;
};



struct GWEN_CRYPTTOKEN_CONTEXT {
  GWEN_LIST_ELEMENT(GWEN_CRYPTTOKEN_CONTEXT)
  GWEN_TYPE_UINT32 id;

  GWEN_CRYPTTOKEN_CONTEXTTYPE contextType;
  char *description;

  GWEN_CRYPTTOKEN_KEYINFO *signKeyInfo;
  GWEN_CRYPTTOKEN_KEYINFO *verifyKeyInfo;
  GWEN_CRYPTTOKEN_SIGNINFO *signInfo;

  GWEN_CRYPTTOKEN_KEYINFO *encryptKeyInfo;
  GWEN_CRYPTTOKEN_KEYINFO *decryptKeyInfo;
  GWEN_CRYPTTOKEN_CRYPTINFO *cryptInfo;
};



struct GWEN_CRYPTTOKEN {
  GWEN_INHERIT_ELEMENT(GWEN_CRYPTTOKEN)
  GWEN_LIST_ELEMENT(GWEN_CRYPTTOKEN)
  int isOpen;
  GWEN_PLUGIN_MANAGER *cryptManager;
  char *tokenType;
  char *tokenSubType;
  char *tokenName;
  char *descriptiveName;
  GWEN_CRYPTTOKEN_DEVICE deviceType;
  GWEN_TYPE_UINT32 flags;

  GWEN_CRYPTTOKEN_OPEN_FN openFn;
  GWEN_CRYPTTOKEN_CREATE_FN createFn;
  GWEN_CRYPTTOKEN_CLOSE_FN closeFn;

  GWEN_CRYPTTOKEN_CHANGEPIN_FN changePinFn;

  GWEN_CRYPTTOKEN_SIGN_FN signFn;
  GWEN_CRYPTTOKEN_VERIFY_FN verifyFn;
  GWEN_CRYPTTOKEN_ENCRYPT_FN encryptFn;
  GWEN_CRYPTTOKEN_DECRYPT_FN decryptFn;
  GWEN_CRYPTTOKEN_READKEY_FN readKeyFn;
  GWEN_CRYPTTOKEN_WRITEKEY_FN writeKeyFn;
  GWEN_CRYPTTOKEN_READKEYSPEC_FN readKeySpecFn;
  GWEN_CRYPTTOKEN_WRITEKEYSPEC_FN writeKeySpecFn;
  GWEN_CRYPTTOKEN_GENERATEKEY_FN generateKeyFn;
  GWEN_CRYPTTOKEN_FILLCONTEXTLIST_FN fillContextListFn;
  GWEN_CRYPTTOKEN_FILLSIGNINFOLIST_FN fillSignInfoListFn;
  GWEN_CRYPTTOKEN_FILLCRYPTINFOLIST_FN fillCryptInfoListFn;
  GWEN_CRYPTTOKEN_FILLKEYINFOLIST_FN fillKeyInfoListFn;
  GWEN_CRYPTTOKEN_FILLUSERLIST_FN fillUserListFn;
  GWEN_CRYPTTOKEN_GETSIGNSEQ_FN getSignSeqFn;
  GWEN_CRYPTTOKEN_MODIFYUSER_FN modifyUserFn;

  GWEN_CRYPTTOKEN_GETTOKENIDDATA_FN getTokenIdDataFn;

  /* runtime data */
  GWEN_CRYPTTOKEN_CONTEXT_LIST *contextList;
  GWEN_CRYPTTOKEN_SIGNINFO_LIST *signInfoList;
  GWEN_CRYPTTOKEN_CRYPTINFO_LIST *cryptInfoList;
  GWEN_CRYPTTOKEN_KEYINFO_LIST *keyInfoList;

  GWEN_CRYPTTOKEN_USER_LIST *userList;
};



typedef struct GWEN_CRYPTTOKEN_PLUGIN GWEN_CRYPTTOKEN_PLUGIN;
struct GWEN_CRYPTTOKEN_PLUGIN {
  GWEN_CRYPTTOKEN_DEVICE devType;
  GWEN_CRYPTTOKEN_PLUGIN_CREATETOKEN_FN createTokenFn;
  GWEN_CRYPTTOKEN_PLUGIN_CHECKTOKEN_FN checkTokenFn;
};
void GWEN_CryptToken_Plugin_FreeData(void *bp, void *p);



typedef struct GWEN_CRYPTMANAGER GWEN_CRYPTMANAGER;
struct GWEN_CRYPTMANAGER {
  GWEN_CRYPTMANAGER_GETPIN_FN getPinFn;
  GWEN_CRYPTMANAGER_BEGIN_ENTER_PIN_FN beginEnterPinFn;
  GWEN_CRYPTMANAGER_END_ENTER_PIN_FN endEnterPinFn;
  GWEN_CRYPTMANAGER_INSERT_TOKEN_FN insertTokenFn;
  GWEN_CRYPTMANAGER_INSERT_CORRECT_TOKEN_FN insertCorrectTokenFn;
  GWEN_CRYPTMANAGER_SHOW_MESSAGE_FN showMessageFn;
};
void GWEN_CryptManager_FreeData(void *bp, void *p);





#endif /* GWEN_CRYPTTOKEN_P_H */

