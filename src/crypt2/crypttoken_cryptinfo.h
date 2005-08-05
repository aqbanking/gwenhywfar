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


#ifndef GWEN_CRYPTTOKEN_CRYPTINFO_H
#define GWEN_CRYPTTOKEN_CRYPTINFO_H

#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/plugin.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_CRYPTTOKEN_CRYPTINFO GWEN_CRYPTTOKEN_CRYPTINFO;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_CRYPTINFO,
                            GWEN_CryptToken_CryptInfo,
                            GWENHYWFAR_API)


/** @name CryptInfo
 *
 */
/*@{*/
GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_new();
void GWEN_CryptToken_CryptInfo_free(GWEN_CRYPTTOKEN_CRYPTINFO *ci);
GWEN_CRYPTTOKEN_CRYPTINFO*
  GWEN_CryptToken_CryptInfo_dup(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);

GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_fromDb(GWEN_DB_NODE *db);
void GWEN_CryptToken_CryptInfo_toDb(const GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                    GWEN_DB_NODE *db);


GWEN_TYPE_UINT32
  GWEN_CryptToken_CryptInfo_GetId(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
void GWEN_CryptToken_CryptInfo_SetId(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                     GWEN_TYPE_UINT32 id);

GWEN_CRYPTTOKEN_CRYPTALGO
  GWEN_CryptToken_CryptInfo_GetCryptAlgo(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
void GWEN_CryptToken_CryptInfo_SetCryptAlgo(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                            GWEN_CRYPTTOKEN_CRYPTALGO a);

GWEN_CRYPTTOKEN_PADDALGO
  GWEN_CryptToken_CryptInfo_GetPaddAlgo(const GWEN_CRYPTTOKEN_CRYPTINFO *ci);
void GWEN_CryptToken_CryptInfo_SetPaddAlgo(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                           GWEN_CRYPTTOKEN_PADDALGO a);
/*@}*/


#ifdef __cplusplus
}
#endif



#endif /* GWEN_CRYPTTOKEN_CRYPTINFO_H */


