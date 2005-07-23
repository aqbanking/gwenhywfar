/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GCT_TOOL_CRYPTMANAGER_P_H
#define GCT_TOOL_CRYPTMANAGER_P_H

#include "cryptmanager_l.h"

#define CON_CRYPTMANAGER_CHAR_ABORT  27
#define CON_CRYPTMANAGER_CHAR_DELETE 127
#define CON_CRYPTMANAGER_CHAR_ENTER  10


typedef struct CON_CRYPTMANAGER CON_CRYPTMANAGER;
struct CON_CRYPTMANAGER {
};
void CON_CryptManager_FreeData(void *bp, void *p);


char CON_CryptManager___readCharFromStdin(int waitFor);

int CON_CryptManager__input(GWEN_TYPE_UINT32 flags,
                            char *buffer,
                            int minLen,
                            int maxLen);


int CON_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                           GWEN_CRYPTTOKEN *token,
                           GWEN_CRYPTTOKEN_PINTYPE pt,
                           GWEN_CRYPTTOKEN_PINENCODING pe,
                           GWEN_TYPE_UINT32 flags,
                           unsigned char *buffer,
                           unsigned int minLength,
                           unsigned int maxLength,
                           unsigned int *pinLength);

int CON_CryptManager_BeginEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token,
                                  GWEN_CRYPTTOKEN_PINTYPE pt);
int CON_CryptManager_EndEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                GWEN_CRYPTTOKEN *token,
                                GWEN_CRYPTTOKEN_PINTYPE pt,
                                int ok);
int CON_CryptManager_InsertToken(GWEN_PLUGIN_MANAGER *cm,
                                GWEN_CRYPTTOKEN *token);
int CON_CryptManager_InsertCorrectToken(GWEN_PLUGIN_MANAGER *cm,
                                       GWEN_CRYPTTOKEN *token);

int CON_CryptManager_ShowMessage(GWEN_PLUGIN_MANAGER *cm,
                                 GWEN_CRYPTTOKEN *token,
                                 const char *title,
                                 const char *msg);




#endif /* GCT_TOOL_CRYPTMANAGER_P_H */
