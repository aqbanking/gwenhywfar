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


#ifndef GWEN_CRYPTTOKEN_SIGNINFO_H
#define GWEN_CRYPTTOKEN_SIGNINFO_H

#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/plugin.h>


typedef struct GWEN_CRYPTTOKEN_SIGNINFO GWEN_CRYPTTOKEN_SIGNINFO;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTTOKEN_SIGNINFO,
                            GWEN_CryptToken_SignInfo,
                            GWENHYWFAR_API)

/** @name SignInfo
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_new();

GWENHYWFAR_API
void GWEN_CryptToken_SignInfo_free(GWEN_CRYPTTOKEN_SIGNINFO *si);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_SIGNINFO*
  GWEN_CryptToken_SignInfo_dup(const GWEN_CRYPTTOKEN_SIGNINFO *si);

GWENHYWFAR_API
GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_fromDb(GWEN_DB_NODE *db);

GWENHYWFAR_API
int GWEN_CryptToken_SignInfo_toDb(const GWEN_CRYPTTOKEN_SIGNINFO *si,
                                  GWEN_DB_NODE *db);


GWENHYWFAR_API
GWEN_TYPE_UINT32
  GWEN_CryptToken_SignInfo_GetId(const GWEN_CRYPTTOKEN_SIGNINFO *si);

GWENHYWFAR_API
void GWEN_CryptToken_SignInfo_SetId(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                    GWEN_TYPE_UINT32 id);


GWENHYWFAR_API
GWEN_CRYPTTOKEN_HASHALGO
  GWEN_CryptToken_SignInfo_GetHashAlgo(const GWEN_CRYPTTOKEN_SIGNINFO *si);

GWENHYWFAR_API
void GWEN_CryptToken_SignInfo_SetHashAlgo(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                          GWEN_CRYPTTOKEN_HASHALGO a);


GWENHYWFAR_API
GWEN_CRYPTTOKEN_PADDALGO
  GWEN_CryptToken_SignInfo_GetPaddAlgo(const GWEN_CRYPTTOKEN_SIGNINFO *si);

GWENHYWFAR_API
void GWEN_CryptToken_SignInfo_SetPaddAlgo(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                          GWEN_CRYPTTOKEN_PADDALGO a);

/*@}*/






#endif /* GWEN_CRYPTTOKEN_SIGNINFO_H */


