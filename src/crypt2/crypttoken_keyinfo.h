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


#ifndef GWEN_CRYPTTOKEN_KEYINFO_H
#define GWEN_CRYPTTOKEN_KEYINFO_H

#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/plugin.h>


#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_SIGN      0x00000001
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_VERIFY    0x00000002
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_ENCRYPT   0x00000004
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_DECRYPT   0x00000008
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_READABLE      0x00000010
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_WRITEABLE     0x00000020
#define GWEN_CRYPTTOKEN_KEYINFO_FLAGS_HAS_SIGNSEQ   0x00000040

typedef struct GWEN_CRYPTTOKEN_KEYINFO GWEN_CRYPTTOKEN_KEYINFO;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_KEYINFO,
                            GWEN_CryptToken_KeyInfo,
                            GWENHYWFAR_API)


/** @name KeyInfo
 *
 */
/*@{*/
GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_new();
void GWEN_CryptToken_KeyInfo_free(GWEN_CRYPTTOKEN_KEYINFO *ki);
GWEN_CRYPTTOKEN_KEYINFO*
  GWEN_CryptToken_KeyInfo_dup(const GWEN_CRYPTTOKEN_KEYINFO *ki);

GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_fromDb(GWEN_DB_NODE *db);
int GWEN_CryptToken_KeyInfo_toDb(const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                 GWEN_DB_NODE *db);

int GWEN_CryptToken_KeyInfo_GetKeyId(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetKeyId(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                      int id);

int GWEN_CryptToken_KeyInfo_GetKeySize(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetKeySize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                        int i);

int GWEN_CryptToken_KeyInfo_GetMinKeySize(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetMinKeySize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                           int i);

int GWEN_CryptToken_KeyInfo_GetMaxKeySize(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetMaxKeySize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                           int i);

int GWEN_CryptToken_KeyInfo_GetChunkSize(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetChunkSize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                          int i);

GWEN_CRYPTTOKEN_CRYPTALGO
  GWEN_CryptToken_KeyInfo_GetCryptAlgo(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetCryptAlgo(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                          GWEN_CRYPTTOKEN_CRYPTALGO a);

GWEN_TYPE_UINT32
  GWEN_CryptToken_KeyInfo_GetKeyFlags(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl);
void GWEN_CryptToken_KeyInfo_AddKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl);
void GWEN_CryptToken_KeyInfo_SubKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl);

const char*
  GWEN_CryptToken_KeyInfo_GetDescription(const GWEN_CRYPTTOKEN_KEYINFO *ki);
void GWEN_CryptToken_KeyInfo_SetDescription(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                            const char *s);

/*@}*/


#endif /* GWEN_CRYPTTOKEN_KEYINFO_H */


