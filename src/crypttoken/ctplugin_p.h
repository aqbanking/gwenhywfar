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


#ifndef GWEN_CRYPT_TOKEN_PLUGIN_P_H
#define GWEN_CRYPT_TOKEN_PLUGIN_P_H

#include "ctplugin_be.h"



typedef struct GWEN_CRYPT_TOKEN_PLUGIN GWEN_CRYPT_TOKEN_PLUGIN;
struct GWEN_CRYPT_TOKEN_PLUGIN {
  GWEN_CRYPT_TOKEN_DEVICE devType;
  GWEN_CRYPT_TOKEN_PLUGIN_CREATETOKEN_FN createTokenFn;
  GWEN_CRYPT_TOKEN_PLUGIN_CHECKTOKEN_FN checkTokenFn;
};
static void GWENHYWFAR_CB GWEN_Crypt_Token_Plugin_FreeData(void *bp, void *p);




#endif

