/***************************************************************************
    begin       : Sun Jan 13 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPT_TOKEN_TEST_P_H
#define GWEN_CRYPT_TOKEN_TEST_P_H


#include "cttest.h"

#include <gwenhywfar/fslock.h>
#include <gwenhywfar/cryptkey.h>

#include <sys/stat.h>


#define GWEN_CRYPT_TOKEN_TEST_MAX_CONTEXTS 10
#define GWEN_CRYPT_TOKEN_TEST_MAX_KEYS     70



typedef struct GWEN_CRYPT_TOKEN_TEST GWEN_CRYPT_TOKEN_TEST;
struct GWEN_CRYPT_TOKEN_TEST {
  int maximumContexts;
  int maximumKeys;
  GWEN_CRYPT_TOKEN_CONTEXT_LIST *contextList;
  GWEN_CRYPT_KEY_LIST *keyList;
};

static GWENHYWFAR_CB void GWEN_Crypt_TokenTest_freeData(void *bp, void *p);



#endif

