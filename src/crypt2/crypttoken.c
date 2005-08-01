/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Sep 11 2003
 copyright   : (C) 2003 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "crypttoken_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/waitcallback.h>



GWEN_LIST_FUNCTIONS(GWEN_CRYPTTOKEN_SIGNINFO,
                    GWEN_CryptToken_SignInfo)
GWEN_LIST_FUNCTIONS(GWEN_CRYPTTOKEN_CRYPTINFO,
                    GWEN_CryptToken_CryptInfo)
GWEN_LIST_FUNCTIONS(GWEN_CRYPTTOKEN_KEYINFO,
                    GWEN_CryptToken_KeyInfo)

GWEN_LIST_FUNCTIONS(GWEN_CRYPTTOKEN_CONTEXT,
                    GWEN_CryptToken_Context)

GWEN_LIST_FUNCTIONS(GWEN_CRYPTTOKEN,
                    GWEN_CryptToken)
GWEN_INHERIT_FUNCTIONS(GWEN_CRYPTTOKEN)




GWEN_CRYPTTOKEN_HASHALGO GWEN_CryptToken_HashAlgo_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_CryptToken_HashAlgo_None;
  else if (strcasecmp(s, "sha1")==0)
    return GWEN_CryptToken_HashAlgo_SHA1;
  else if (strcasecmp(s, "rmd160")==0)
    return GWEN_CryptToken_HashAlgo_RMD160;
  else if (strcasecmp(s, "md5")==0)
    return GWEN_CryptToken_HashAlgo_MD5;
  return GWEN_CryptToken_HashAlgo_Unknown;
}



const char *GWEN_CryptToken_HashAlgo_toString(GWEN_CRYPTTOKEN_HASHALGO a){
  switch(a) {
  case GWEN_CryptToken_HashAlgo_None:
    return "none";
  case GWEN_CryptToken_HashAlgo_SHA1:
    return "sha1";
  case GWEN_CryptToken_HashAlgo_RMD160:
    return "rmd160";
  case GWEN_CryptToken_HashAlgo_MD5:
    return "md5";
  default:
    return "unknown";
  }
}



GWEN_CRYPTTOKEN_PADDALGO GWEN_CryptToken_PaddAlgo_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_CryptToken_PaddAlgo_None;
  else if (strcasecmp(s, "iso9796_1")==0)
    return GWEN_CryptToken_PaddAlgo_ISO9796_1;
  else if (strcasecmp(s, "iso9796_1a4")==0)
    return GWEN_CryptToken_PaddAlgo_ISO9796_1A4;
  else if (strcasecmp(s, "iso9796_2")==0)
    return GWEN_CryptToken_PaddAlgo_ISO9796_2;
  else if (strcasecmp(s, "pkcs1_1")==0)
    return GWEN_CryptToken_PaddAlgo_PKCS1_1;
  else if (strcasecmp(s, "pkcs1_2")==0)
    return GWEN_CryptToken_PaddAlgo_PKCS1_2;
  else if (strcasecmp(s, "leftzero")==0)
    return GWEN_CryptToken_PaddAlgo_LeftZero;
  else if (strcasecmp(s, "rightzero")==0)
    return GWEN_CryptToken_PaddAlgo_RightZero;
  else if (strcasecmp(s, "ansix9_23")==0)
    return GWEN_CryptToken_PaddAlgo_ANSIX9_23;
  return GWEN_CryptToken_PaddAlgo_Unknown;
}



const char *GWEN_CryptToken_PaddAlgo_toString(GWEN_CRYPTTOKEN_PADDALGO a){
  switch(a) {
  case GWEN_CryptToken_PaddAlgo_None:
    return "none";
  case GWEN_CryptToken_PaddAlgo_ISO9796_1:
    return "iso9796_1";
  case GWEN_CryptToken_PaddAlgo_ISO9796_1A4:
    return "iso9796_1a4";
  case GWEN_CryptToken_PaddAlgo_ISO9796_2:
    return "iso9796_2";
  case GWEN_CryptToken_PaddAlgo_PKCS1_1:
    return "pkcs1_1";
  case GWEN_CryptToken_PaddAlgo_PKCS1_2:
    return "pkcs1_2";
  case GWEN_CryptToken_PaddAlgo_LeftZero:
    return "leftzero";
  case GWEN_CryptToken_PaddAlgo_RightZero:
    return "rightzero";
  case GWEN_CryptToken_PaddAlgo_ANSIX9_23:
    return "ansix9_23";
  default:
    return "unknown";
  }
}




GWEN_CRYPTTOKEN_CRYPTALGO GWEN_CryptToken_CryptAlgo_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_CryptToken_CryptAlgo_None;
  else if (strcasecmp(s, "rsa")==0)
    return GWEN_CryptToken_CryptAlgo_RSA;
  else if (strcasecmp(s, "dsa")==0)
    return GWEN_CryptToken_CryptAlgo_DSA;
  else if (strcasecmp(s, "des")==0)
    return GWEN_CryptToken_CryptAlgo_DES;
  else if (strcasecmp(s, "des_3k")==0)
    return GWEN_CryptToken_CryptAlgo_DES_3K;
  return GWEN_CryptToken_CryptAlgo_Unknown;
}



const char *GWEN_CryptToken_CryptAlgo_toString(GWEN_CRYPTTOKEN_CRYPTALGO a){
  switch(a) {
  case GWEN_CryptToken_CryptAlgo_None:
    return "none";
  case GWEN_CryptToken_CryptAlgo_RSA:
    return "rsa";
  case GWEN_CryptToken_CryptAlgo_DSA:
    return "dsa";
  case GWEN_CryptToken_CryptAlgo_DES:
    return "des";
  case GWEN_CryptToken_CryptAlgo_DES_3K:
    return "des_3k";
  default:
    return "unknown";
  }
}




GWEN_CRYPTTOKEN_DEVICE GWEN_CryptToken_Device_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_CryptToken_Device_None;
  else if (strcasecmp(s, "file")==0)
    return GWEN_CryptToken_Device_File;
  else if (strcasecmp(s, "card")==0)
    return GWEN_CryptToken_Device_Card;
  return GWEN_CryptToken_Device_Unknown;
}



const char *GWEN_CryptToken_Device_toString(GWEN_CRYPTTOKEN_DEVICE d){
  switch(d) {
  case GWEN_CryptToken_Device_None:
    return "none";
  case GWEN_CryptToken_Device_File:
    return "file";
  case GWEN_CryptToken_Device_Card:
    return "card";
  default:
    return "unknown";
  }
}




GWEN_CRYPTTOKEN_PINTYPE GWEN_CryptToken_PinType_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_CryptToken_PinType_None;
  else if (strcasecmp(s, "access")==0)
    return GWEN_CryptToken_PinType_Access;
  else if (strcasecmp(s, "manage")==0)
    return GWEN_CryptToken_PinType_Manage;
  return GWEN_CryptToken_PinType_Unknown;
}



const char *GWEN_CryptToken_PinType_toString(GWEN_CRYPTTOKEN_PINTYPE pt){
  switch(pt) {
  case GWEN_CryptToken_PinType_None:
    return "none";
  case GWEN_CryptToken_PinType_Access:
    return "access";
  case GWEN_CryptToken_PinType_Manage:
    return "manage";
  default:
    return "unknown";
  }
}




GWEN_CRYPTTOKEN_PINENCODING
GWEN_CryptToken_PinEncoding_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_CryptToken_PinEncoding_None;
  else if (strcasecmp(s, "bin")==0)
    return GWEN_CryptToken_PinEncoding_Bin;
  else if (strcasecmp(s, "bcd")==0)
    return GWEN_CryptToken_PinEncoding_BCD;
  else if (strcasecmp(s, "ascii")==0)
    return GWEN_CryptToken_PinEncoding_ASCII;
  return GWEN_CryptToken_PinEncoding_Unknown;
}



const char*
GWEN_CryptToken_PinEncoding_toString(GWEN_CRYPTTOKEN_PINENCODING pe){
  switch(pe) {
  case GWEN_CryptToken_PinEncoding_None:
    return "none";
  case GWEN_CryptToken_PinEncoding_Bin:
    return "bin";
  case GWEN_CryptToken_PinEncoding_BCD:
    return "bcd";
  case GWEN_CryptToken_PinEncoding_ASCII:
    return "ascii";
  default:
    return "unknown";
  }
}



GWEN_CRYPTTOKEN_CONTEXTTYPE
GWEN_CryptToken_ContextType_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "unknown")==0)
    return GWEN_CryptToken_ContextType_Unknown;
  else if (strcasecmp(s, "hbci")==0)
    return GWEN_CryptToken_ContextType_HBCI;
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "Unknown context type \"%s\"", s);
    return GWEN_CryptToken_ContextType_Unknown;
  }
}



const char*
GWEN_CryptToken_ContextType_toString(GWEN_CRYPTTOKEN_CONTEXTTYPE t) {
  switch(t) {
  case GWEN_CryptToken_ContextType_Unknown:
    return "unknown";
  case GWEN_CryptToken_ContextType_HBCI:
    return "hbci";

  default:
    DBG_WARN(GWEN_LOGDOMAIN, "Unhandled context type %d", t);
    return "unknown";
  }
}















/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             SignInfo functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_new(){
  GWEN_CRYPTTOKEN_SIGNINFO *si;

  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN_SIGNINFO, si);
  GWEN_LIST_INIT(GWEN_CRYPTTOKEN_SIGNINFO, si);

  return si;
}



void GWEN_CryptToken_SignInfo_free(GWEN_CRYPTTOKEN_SIGNINFO *si){
  if (si) {
    GWEN_LIST_FINI(GWEN_CRYPTTOKEN_SIGNINFO, si);
    GWEN_FREE_OBJECT(si);
  }
}



GWEN_CRYPTTOKEN_SIGNINFO*
GWEN_CryptToken_SignInfo_dup(const GWEN_CRYPTTOKEN_SIGNINFO *si){
  GWEN_CRYPTTOKEN_SIGNINFO *nsi;

  assert(si);
  nsi=GWEN_CryptToken_SignInfo_new();
  nsi->id=si->id;
  nsi->hashAlgo=si->hashAlgo;
  nsi->paddAlgo=si->paddAlgo;
  return nsi;
}



GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_fromDb(GWEN_DB_NODE *db){
  GWEN_CRYPTTOKEN_SIGNINFO *si;
  const char *s;

  si=GWEN_CryptToken_SignInfo_new();
  si->id=GWEN_DB_GetIntValue(db, "id", 0, 0);
  s=GWEN_DB_GetCharValue(db, "hashAlgo", 0, 0);
  if (s)
    si->hashAlgo=GWEN_CryptToken_HashAlgo_fromString(s);
  else
    si->hashAlgo=GWEN_CryptToken_HashAlgo_None;

  s=GWEN_DB_GetCharValue(db, "paddAlgo", 0, 0);
  if (s)
    si->paddAlgo=GWEN_CryptToken_PaddAlgo_fromString(s);
  else
    si->paddAlgo=GWEN_CryptToken_PaddAlgo_None;

  return si;
}



int GWEN_CryptToken_SignInfo_toDb(const GWEN_CRYPTTOKEN_SIGNINFO *si,
                                  GWEN_DB_NODE *db){
  assert(si);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "id", si->id);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "hashAlgo",
		       GWEN_CryptToken_HashAlgo_toString(si->hashAlgo));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "paddAlgo",
                       GWEN_CryptToken_PaddAlgo_toString(si->paddAlgo));
  return 0;
}



GWEN_CRYPTTOKEN_SIGNINFO *GWEN_CryptToken_SignInfo_fromXml(GWEN_XMLNODE *n){
  GWEN_CRYPTTOKEN_SIGNINFO *si;
  const char *s;

  si=GWEN_CryptToken_SignInfo_new();
  si->id=GWEN_XMLNode_GetIntValue(n, "id", 0);
  s=GWEN_XMLNode_GetCharValue(n, "hashAlgo", 0);
  if (s)
    si->hashAlgo=GWEN_CryptToken_HashAlgo_fromString(s);
  else
    si->hashAlgo=GWEN_CryptToken_HashAlgo_None;

  s=GWEN_XMLNode_GetCharValue(n, "paddAlgo", 0);
  if (s)
    si->paddAlgo=GWEN_CryptToken_PaddAlgo_fromString(s);
  else
    si->paddAlgo=GWEN_CryptToken_PaddAlgo_None;

  return si;
}



GWEN_TYPE_UINT32
GWEN_CryptToken_SignInfo_GetId(const GWEN_CRYPTTOKEN_SIGNINFO *si){
  assert(si);
  return si->id;
}



void GWEN_CryptToken_SignInfo_SetId(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                    GWEN_TYPE_UINT32 id){
  assert(si);
  si->id=id;
}



GWEN_CRYPTTOKEN_HASHALGO
GWEN_CryptToken_SignInfo_GetHashAlgo(const GWEN_CRYPTTOKEN_SIGNINFO *si){
  assert(si);
  return si->hashAlgo;
}



void GWEN_CryptToken_SignInfo_SetHashAlgo(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                          GWEN_CRYPTTOKEN_HASHALGO a){
  assert(si);
  si->hashAlgo=a;
}



GWEN_CRYPTTOKEN_PADDALGO
GWEN_CryptToken_SignInfo_GetPaddAlgo(const GWEN_CRYPTTOKEN_SIGNINFO *si){
  assert(si);
  return si->paddAlgo;
}



void GWEN_CryptToken_SignInfo_SetPaddAlgo(GWEN_CRYPTTOKEN_SIGNINFO *si,
                                          GWEN_CRYPTTOKEN_PADDALGO a){
  assert(si);
  si->paddAlgo=a;
}





/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             CryptInfo functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_new(){
  GWEN_CRYPTTOKEN_CRYPTINFO *ci;

  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN_CRYPTINFO, ci);
  GWEN_LIST_INIT(GWEN_CRYPTTOKEN_CRYPTINFO, ci);

  return ci;
}



void GWEN_CryptToken_CryptInfo_free(GWEN_CRYPTTOKEN_CRYPTINFO *ci){
  if (ci) {
    GWEN_LIST_FINI(GWEN_CRYPTTOKEN_CRYPTINFO, ci);
    GWEN_FREE_OBJECT(ci);
  }
}



GWEN_CRYPTTOKEN_CRYPTINFO*
GWEN_CryptToken_CryptInfo_dup(const GWEN_CRYPTTOKEN_CRYPTINFO *ci){
  GWEN_CRYPTTOKEN_CRYPTINFO *nci;

  assert(ci);
  nci=GWEN_CryptToken_CryptInfo_new();
  nci->id=ci->id;
  nci->cryptAlgo=ci->cryptAlgo;
  nci->paddAlgo=ci->paddAlgo;

  return nci;
}



GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_fromDb(GWEN_DB_NODE *db){
  GWEN_CRYPTTOKEN_CRYPTINFO *ci;
  const char *s;

  ci=GWEN_CryptToken_CryptInfo_new();
  ci->id=GWEN_DB_GetIntValue(db, "id", 0, 0);
  s=GWEN_DB_GetCharValue(db, "cryptAlgo", 0, 0);
  if (s)
    ci->cryptAlgo=GWEN_CryptToken_CryptAlgo_fromString(s);
  else
    ci->cryptAlgo=GWEN_CryptToken_CryptAlgo_None;

  s=GWEN_DB_GetCharValue(db, "paddAlgo", 0, 0);
  if (s)
    ci->paddAlgo=GWEN_CryptToken_PaddAlgo_fromString(s);
  else
    ci->paddAlgo=GWEN_CryptToken_PaddAlgo_None;

  return ci;
}



void GWEN_CryptToken_CryptInfo_toDb(const GWEN_CRYPTTOKEN_CRYPTINFO *ci,
				   GWEN_DB_NODE *db){
  assert(ci);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "id", ci->id);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "cryptAlgo",
		       GWEN_CryptToken_CryptAlgo_toString(ci->cryptAlgo));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "paddAlgo",
		       GWEN_CryptToken_PaddAlgo_toString(ci->paddAlgo));
}



GWEN_CRYPTTOKEN_CRYPTINFO *GWEN_CryptToken_CryptInfo_fromXml(GWEN_XMLNODE *n){
  GWEN_CRYPTTOKEN_CRYPTINFO *ci;
  const char *s;

  ci=GWEN_CryptToken_CryptInfo_new();
  ci->id=GWEN_XMLNode_GetIntValue(n, "id", 0);
  s=GWEN_XMLNode_GetCharValue(n, "cryptAlgo", 0);
  if (s)
    ci->cryptAlgo=GWEN_CryptToken_CryptAlgo_fromString(s);
  else
    ci->cryptAlgo=GWEN_CryptToken_CryptAlgo_None;

  s=GWEN_XMLNode_GetCharValue(n, "paddAlgo", 0);
  if (s)
    ci->paddAlgo=GWEN_CryptToken_PaddAlgo_fromString(s);
  else
    ci->paddAlgo=GWEN_CryptToken_PaddAlgo_None;

  return ci;
}



GWEN_TYPE_UINT32
GWEN_CryptToken_CryptInfo_GetId(const GWEN_CRYPTTOKEN_CRYPTINFO *ci){
  assert(ci);
  return ci->id;
}



void GWEN_CryptToken_CryptInfo_SetId(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                     GWEN_TYPE_UINT32 id){
  assert(ci);
  ci->id=id;
}



GWEN_CRYPTTOKEN_CRYPTALGO
GWEN_CryptToken_CryptInfo_GetCryptAlgo(const GWEN_CRYPTTOKEN_CRYPTINFO *ci){
  assert(ci);
  return ci->cryptAlgo;
}



void GWEN_CryptToken_CryptInfo_SetCryptAlgo(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                            GWEN_CRYPTTOKEN_CRYPTALGO a){
  assert(ci);
  ci->cryptAlgo=a;
}



GWEN_CRYPTTOKEN_PADDALGO
GWEN_CryptToken_CryptInfo_GetPaddAlgo(const GWEN_CRYPTTOKEN_CRYPTINFO *ci){
  assert(ci);
  return ci->paddAlgo;
}



void GWEN_CryptToken_CryptInfo_SetPaddAlgo(GWEN_CRYPTTOKEN_CRYPTINFO *ci,
                                           GWEN_CRYPTTOKEN_PADDALGO a){
  assert(ci);
  ci->paddAlgo=a;
}







/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             KeyInfo functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_new(){
  GWEN_CRYPTTOKEN_KEYINFO *ki;

  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN_KEYINFO, ki);
  GWEN_LIST_INIT(GWEN_CRYPTTOKEN_KEYINFO, ki);

  return ki;
}



void GWEN_CryptToken_KeyInfo_free(GWEN_CRYPTTOKEN_KEYINFO *ki){
  if (ki) {
    GWEN_LIST_FINI(GWEN_CRYPTTOKEN_KEYINFO, ki);
    free(ki->keyDescription);
    GWEN_FREE_OBJECT(ki);
  }
}



GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_fromDb(GWEN_DB_NODE *db){
  GWEN_CRYPTTOKEN_KEYINFO *ki;
  const char *s;
  int i;

  ki=GWEN_CryptToken_KeyInfo_new();
  ki->keyId=GWEN_DB_GetIntValue(db, "keyId", 0, 0);
  ki->keySize=GWEN_DB_GetIntValue(db, "keySize", 0, 0);
  ki->minKeySize=GWEN_DB_GetIntValue(db, "minKeySize", 0, 0);
  ki->maxKeySize=GWEN_DB_GetIntValue(db, "maxKeySize", 0, 0);
  ki->chunkSize=GWEN_DB_GetIntValue(db, "chunkSize", 0, 0);
  s=GWEN_DB_GetCharValue(db, "cryptAlgo", 0, 0);
  if (s)
    ki->cryptAlgo=GWEN_CryptToken_CryptAlgo_fromString(s);
  else
    ki->cryptAlgo=GWEN_CryptToken_CryptAlgo_None;
  s=GWEN_DB_GetCharValue(db, "keyDescription", 0, 0);
  if (s)
    ki->keyDescription=strdup(s);
  for (i=0; ; i++) {
    s=GWEN_DB_GetCharValue(db, "keyFlags", i, 0);
    if (!s)
      break;
    if (strcasecmp(s, "canSign")==0)
      ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_SIGN;
    else if (strcasecmp(s, "canVerify")==0)
      ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_VERIFY;
    else if (strcasecmp(s, "canEncrypt")==0)
      ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_ENCRYPT;
    else if (strcasecmp(s, "canDecrypt")==0)
      ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_DECRYPT;
    else if (strcasecmp(s, "readable")==0)
      ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_READABLE;
    else if (strcasecmp(s, "writeable")==0)
      ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_WRITEABLE;
    else if (strcasecmp(s, "hasSignSeq")==0)
      ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_HAS_SIGNSEQ;
    else {
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown flag \"%s\"", s);
    }
  } /* for */

  return ki;
}



int GWEN_CryptToken_KeyInfo_toDb(const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                 GWEN_DB_NODE *db){
  assert(ki);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keyId", ki->keyId);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keySize", ki->keySize);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "minKeySize", ki->minKeySize);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "maxKeySize", ki->maxKeySize);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "chunkSize", ki->chunkSize);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "cryptAlgo",
		       GWEN_CryptToken_CryptAlgo_toString(ki->cryptAlgo));
  if (ki->keyDescription)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "keyDescription",
			 ki->keyDescription);
  GWEN_DB_DeleteVar(db, "keyFlags");

  if (ki->keyFlags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_SIGN)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
			 "keyFlags", "canSign");
  if (ki->keyFlags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_VERIFY)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
			 "keyFlags", "canVerify");
  if (ki->keyFlags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_ENCRYPT)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
			 "keyFlags", "canEncrypt");
  if (ki->keyFlags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_DECRYPT)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
			 "keyFlags", "canDecrypt");
  if (ki->keyFlags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_READABLE)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
			 "keyFlags", "readable");
  if (ki->keyFlags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_WRITEABLE)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
			 "keyFlags", "writeable");
  if (ki->keyFlags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_HAS_SIGNSEQ)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                         "keyFlags", "hasSignSeq");
  return 0;
}



GWEN_CRYPTTOKEN_KEYINFO *GWEN_CryptToken_KeyInfo_fromXml(GWEN_XMLNODE *n){
  GWEN_CRYPTTOKEN_KEYINFO *ki;
  GWEN_XMLNODE *nn;
  const char *s;

  ki=GWEN_CryptToken_KeyInfo_new();
  ki->keyId=GWEN_XMLNode_GetIntValue(n, "keyId", 0);
  ki->keySize=GWEN_XMLNode_GetIntValue(n, "keySize", 0);
  ki->minKeySize=GWEN_XMLNode_GetIntValue(n, "minKeySize", 0);
  ki->maxKeySize=GWEN_XMLNode_GetIntValue(n, "maxKeySize", 0);
  ki->chunkSize=GWEN_XMLNode_GetIntValue(n, "chunkSize", 0);
  s=GWEN_XMLNode_GetCharValue(n, "cryptAlgo", 0);
  if (s)
    ki->cryptAlgo=GWEN_CryptToken_CryptAlgo_fromString(s);
  else
    ki->cryptAlgo=GWEN_CryptToken_CryptAlgo_None;
  s=GWEN_XMLNode_GetCharValue(n, "keyDescription", 0);
  if (s)
    ki->keyDescription=strdup(s);

  nn=GWEN_XMLNode_FindFirstTag(n, "keyFlags", 0, 0);
  while(nn) {
    GWEN_XMLNODE *nnn;

    nnn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nnn) {
      GWEN_XMLNODE *nd;

      nd=GWEN_XMLNode_GetFirstData(nnn);
      if (nd) {
        s=GWEN_XMLNode_GetData(nd);
        assert(s);
        if (strcasecmp(s, "canSign")==0)
          ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_SIGN;
        else if (strcasecmp(s, "canVerify")==0)
          ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_VERIFY;
        else if (strcasecmp(s, "canEncrypt")==0)
          ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_ENCRYPT;
        else if (strcasecmp(s, "canDecrypt")==0)
          ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_DECRYPT;
        else if (strcasecmp(s, "readable")==0)
          ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_READABLE;
        else if (strcasecmp(s, "writeable")==0)
          ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_WRITEABLE;
        else if (strcasecmp(s, "hasSignSeq")==0)
          ki->keyFlags|=GWEN_CRYPTTOKEN_KEYINFO_FLAGS_HAS_SIGNSEQ;
        else {
          DBG_WARN(GWEN_LOGDOMAIN, "Unknown flag \"%s\"", s);
        }
      }
      nnn=GWEN_XMLNode_FindNextTag(nnn, "flag", 0, 0);
    } /* while */
    nn=GWEN_XMLNode_FindNextTag(nn, "keyFlags", 0, 0);
  } /* while */

  return ki;
}



GWEN_CRYPTTOKEN_KEYINFO*
GWEN_CryptToken_KeyInfo_dup(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  GWEN_CRYPTTOKEN_KEYINFO *nki;

  assert(ki);
  nki=GWEN_CryptToken_KeyInfo_new();
  nki->keyId=ki->keyId;
  nki->keySize=ki->keySize;
  nki->minKeySize=ki->minKeySize;
  nki->maxKeySize=ki->maxKeySize;
  nki->chunkSize=ki->chunkSize;
  nki->cryptAlgo=ki->cryptAlgo;
  nki->keyFlags=ki->keyFlags;
  if (ki->keyDescription)
    nki->keyDescription=strdup(ki->keyDescription);

  return nki;
}



int GWEN_CryptToken_KeyInfo_GetKeyId(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->keyId;
}



void GWEN_CryptToken_KeyInfo_SetKeyId(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                      int id){
  assert(ki);
  ki->keyId=id;
}



int GWEN_CryptToken_KeyInfo_GetKeySize(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->keySize;
}



void GWEN_CryptToken_KeyInfo_SetKeySize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                        int i){
  assert(ki);
  ki->keySize=i;
}



int GWEN_CryptToken_KeyInfo_GetMinKeySize(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->minKeySize;
}



void GWEN_CryptToken_KeyInfo_SetMinKeySize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                           int i) {
  assert(ki);
  ki->minKeySize=i;
}



int GWEN_CryptToken_KeyInfo_GetMaxKeySize(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->maxKeySize;
}



void GWEN_CryptToken_KeyInfo_SetMaxKeySize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                           int i) {
  assert(ki);
  ki->maxKeySize=i;
}



int GWEN_CryptToken_KeyInfo_GetChunkSize(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->chunkSize;
}



void GWEN_CryptToken_KeyInfo_SetChunkSize(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                          int i){
  assert(ki);
  ki->chunkSize=i;
}



GWEN_CRYPTTOKEN_CRYPTALGO
GWEN_CryptToken_KeyInfo_GetCryptAlgo(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->cryptAlgo;
}



void GWEN_CryptToken_KeyInfo_SetCryptAlgo(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                          GWEN_CRYPTTOKEN_CRYPTALGO a){
  assert(ki);
  ki->cryptAlgo=a;
}



GWEN_TYPE_UINT32
GWEN_CryptToken_KeyInfo_GetKeyFlags(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->keyFlags;
}



void GWEN_CryptToken_KeyInfo_SetKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl){
  assert(ki);
  ki->keyFlags=fl;
}



void GWEN_CryptToken_KeyInfo_AddKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl){
  assert(ki);
  ki->keyFlags|=fl;
}



void GWEN_CryptToken_KeyInfo_SubKeyFlags(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                         GWEN_TYPE_UINT32 fl){
  assert(ki);
  ki->keyFlags&=~fl;
}



const char*
GWEN_CryptToken_KeyInfo_GetDescription(const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ki);
  return ki->keyDescription;
}



void GWEN_CryptToken_KeyInfo_SetDescription(GWEN_CRYPTTOKEN_KEYINFO *ki,
                                            const char *s){
  assert(ki);
  free(ki->keyDescription);
  if (s)
    ki->keyDescription=strdup(s);
  else
    ki->keyDescription=0;
}







/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             Context functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_new(){
  GWEN_CRYPTTOKEN_CONTEXT *ctx;

  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN_CONTEXT, ctx);
  GWEN_LIST_INIT(GWEN_CRYPTTOKEN_CONTEXT, ctx);

  return ctx;
}



void GWEN_CryptToken_Context_free(GWEN_CRYPTTOKEN_CONTEXT *ctx){
  if (ctx) {
    GWEN_CryptToken_KeyInfo_free(ctx->signKeyInfo);
    GWEN_CryptToken_KeyInfo_free(ctx->verifyKeyInfo);
    GWEN_CryptToken_KeyInfo_free(ctx->encryptKeyInfo);
    GWEN_CryptToken_KeyInfo_free(ctx->decryptKeyInfo);
    GWEN_CryptToken_CryptInfo_free(ctx->cryptInfo);
    GWEN_CryptToken_SignInfo_free(ctx->signInfo);
    free(ctx->description);
    GWEN_FREE_OBJECT(ctx);
  }
}



GWEN_CRYPTTOKEN_CONTEXT*
GWEN_CryptToken_Context_dup(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  GWEN_CRYPTTOKEN_CONTEXT *nctx;

  assert(ctx);
  nctx=GWEN_CryptToken_Context_new();
  nctx->id=ctx->id;

  if (ctx->signKeyInfo)
    nctx->signKeyInfo=GWEN_CryptToken_KeyInfo_dup(ctx->signKeyInfo);
  if (ctx->verifyKeyInfo)
    nctx->verifyKeyInfo=GWEN_CryptToken_KeyInfo_dup(ctx->verifyKeyInfo);
  if (ctx->signInfo)
    nctx->signInfo=GWEN_CryptToken_SignInfo_dup(ctx->signInfo);

  if (ctx->encryptKeyInfo)
    nctx->encryptKeyInfo=GWEN_CryptToken_KeyInfo_dup(ctx->encryptKeyInfo);
  if (ctx->decryptKeyInfo)
    nctx->decryptKeyInfo=GWEN_CryptToken_KeyInfo_dup(ctx->decryptKeyInfo);
  if (ctx->cryptInfo)
    nctx->cryptInfo=GWEN_CryptToken_CryptInfo_dup(ctx->cryptInfo);

  if (ctx->description)
    nctx->description=strdup(ctx->description);

  nctx->contextType=ctx->contextType;

  return nctx;
}



GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_fromDb(GWEN_DB_NODE *db){
  GWEN_CRYPTTOKEN_CONTEXT *ctx;
  GWEN_DB_NODE *dbT;
  const char *s;

  assert(db);

  ctx=GWEN_CryptToken_Context_new();
  ctx->id=GWEN_DB_GetIntValue(db, "id", 0, 0);
  s=GWEN_DB_GetCharValue(db, "contextType", 0, 0);
  if (s)
    ctx->contextType=GWEN_CryptToken_ContextType_fromString(s);
  else
    ctx->contextType=GWEN_CryptToken_ContextType_Unknown;
  s=GWEN_DB_GetCharValue(db, "description", 0, 0);
  if (s)
    ctx->description=strdup(s);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "signKeyInfo");
  if (dbT)
    ctx->signKeyInfo=GWEN_CryptToken_KeyInfo_fromDb(dbT);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "verifyKeyInfo");
  if (dbT)
    ctx->verifyKeyInfo=GWEN_CryptToken_KeyInfo_fromDb(dbT);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "signInfo");
  if (dbT)
    ctx->signInfo=GWEN_CryptToken_SignInfo_fromDb(dbT);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "encryptKeyInfo");
  if (dbT)
    ctx->encryptKeyInfo=GWEN_CryptToken_KeyInfo_fromDb(dbT);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "decryptKeyInfo");
  if (dbT)
    ctx->decryptKeyInfo=GWEN_CryptToken_KeyInfo_fromDb(dbT);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "cryptInfo");
  if (dbT)
    ctx->cryptInfo=GWEN_CryptToken_CryptInfo_fromDb(dbT);

  return ctx;
}



int GWEN_CryptToken_Context_toDb(const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                 GWEN_DB_NODE *db){
  GWEN_DB_NODE *dbT;

  assert(ctx);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "id", ctx->id);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "contextType",
                       GWEN_CryptToken_ContextType_toString(ctx->contextType));
  if (ctx->description)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "description", ctx->description);
  if (ctx->signKeyInfo) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "signKeyInfo");
    assert(dbT);
    GWEN_CryptToken_KeyInfo_toDb(ctx->signKeyInfo, dbT);
  }
  if (ctx->verifyKeyInfo) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "verifyKeyInfo");
    assert(dbT);
    GWEN_CryptToken_KeyInfo_toDb(ctx->verifyKeyInfo, dbT);
  }
  if (ctx->signInfo) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "signInfo");
    assert(dbT);
    GWEN_CryptToken_SignInfo_toDb(ctx->signInfo, dbT);
  }

  if (ctx->encryptKeyInfo) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "encryptKeyInfo");
    assert(dbT);
    GWEN_CryptToken_KeyInfo_toDb(ctx->encryptKeyInfo, dbT);
  }
  if (ctx->decryptKeyInfo) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "decryptKeyInfo");
    assert(dbT);
    GWEN_CryptToken_KeyInfo_toDb(ctx->decryptKeyInfo, dbT);
  }
  if (ctx->cryptInfo) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "cryptInfo");
    assert(dbT);
    GWEN_CryptToken_CryptInfo_toDb(ctx->cryptInfo, dbT);
  }

  return 0;
}



GWEN_CRYPTTOKEN_CONTEXT *GWEN_CryptToken_Context_fromXml(GWEN_XMLNODE *n){
  GWEN_CRYPTTOKEN_CONTEXT *ctx;
  GWEN_XMLNODE *nn;
  const char *s;

  assert(n);

  ctx=GWEN_CryptToken_Context_new();
  ctx->id=GWEN_XMLNode_GetIntValue(n, "id", 0);
  s=GWEN_XMLNode_GetCharValue(n, "contextType", 0);
  if (s)
    ctx->contextType=GWEN_CryptToken_ContextType_fromString(s);
  else
    ctx->contextType=GWEN_CryptToken_ContextType_Unknown;
  s=GWEN_XMLNode_GetCharValue(n, "description", 0);
  if (s)
    ctx->description=strdup(s);
  nn=GWEN_XMLNode_FindFirstTag(n, "signKeyInfo", 0, 0);
  if (nn)
    ctx->signKeyInfo=GWEN_CryptToken_KeyInfo_fromXml(nn);
  nn=GWEN_XMLNode_FindFirstTag(n, "verifyKeyInfo", 0, 0);
  if (nn)
    ctx->verifyKeyInfo=GWEN_CryptToken_KeyInfo_fromXml(nn);
  nn=GWEN_XMLNode_FindFirstTag(n, "signInfo", 0, 0);
  if (nn)
    ctx->signInfo=GWEN_CryptToken_SignInfo_fromXml(nn);

  nn=GWEN_XMLNode_FindFirstTag(n, "encryptKeyInfo", 0, 0);
  if (nn)
    ctx->encryptKeyInfo=GWEN_CryptToken_KeyInfo_fromXml(nn);
  nn=GWEN_XMLNode_FindFirstTag(n, "decryptKeyInfo", 0, 0);
  if (nn)
    ctx->decryptKeyInfo=GWEN_CryptToken_KeyInfo_fromXml(nn);
  nn=GWEN_XMLNode_FindFirstTag(n, "cryptInfo", 0, 0);
  if (nn)
    ctx->cryptInfo=GWEN_CryptToken_CryptInfo_fromXml(nn);

  return ctx;
}





GWEN_TYPE_UINT32
GWEN_CryptToken_Context_GetId(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ctx);
  return ctx->id;
}



void GWEN_CryptToken_Context_SetId(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                   GWEN_TYPE_UINT32 id){
  assert(ctx);
  ctx->id=id;
}



GWEN_CRYPTTOKEN_CONTEXTTYPE
GWEN_CryptToken_Context_GetContextType(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ctx);
  return ctx->contextType;
}



void GWEN_CryptToken_Context_SetContextType(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                            GWEN_CRYPTTOKEN_CONTEXTTYPE t){
  assert(ctx);
  ctx->contextType=t;
}



const GWEN_CRYPTTOKEN_KEYINFO*
GWEN_CryptToken_Context_GetSignKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ctx);
  return ctx->signKeyInfo;
}



void
GWEN_CryptToken_Context_SetSignKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                       const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ctx);
  GWEN_CryptToken_KeyInfo_free(ctx->signKeyInfo);
  if (ki)
    ctx->signKeyInfo=GWEN_CryptToken_KeyInfo_dup(ki);
  else
    ctx->signKeyInfo=0;
}



const GWEN_CRYPTTOKEN_KEYINFO*
GWEN_CryptToken_Context_GetVerifyKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ctx);
  return ctx->verifyKeyInfo;
}



void
GWEN_CryptToken_Context_SetVerifyKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ctx);
  GWEN_CryptToken_KeyInfo_free(ctx->verifyKeyInfo);
  if (ki)
    ctx->verifyKeyInfo=GWEN_CryptToken_KeyInfo_dup(ki);
  else
    ctx->verifyKeyInfo=0;
}



const GWEN_CRYPTTOKEN_KEYINFO*
GWEN_CryptToken_Context_GetEncryptKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ctx);
  return ctx->encryptKeyInfo;
}



void
GWEN_CryptToken_Context_SetEncryptKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ctx);
  GWEN_CryptToken_KeyInfo_free(ctx->encryptKeyInfo);
  if (ki)
    ctx->encryptKeyInfo=GWEN_CryptToken_KeyInfo_dup(ki);
  else
    ctx->encryptKeyInfo=0;
}



const GWEN_CRYPTTOKEN_KEYINFO*
GWEN_CryptToken_Context_GetDecryptKeyInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ctx);
  return ctx->decryptKeyInfo;
}



void
GWEN_CryptToken_Context_SetDecryptKeyInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                          const GWEN_CRYPTTOKEN_KEYINFO *ki){
  assert(ctx);
  GWEN_CryptToken_KeyInfo_free(ctx->decryptKeyInfo);
  if (ki)
    ctx->decryptKeyInfo=GWEN_CryptToken_KeyInfo_dup(ki);
  else
    ctx->decryptKeyInfo=0;
}



const char*
GWEN_CryptToken_Context_GetDescription(const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ctx);
  return ctx->description;
}



void GWEN_CryptToken_Context_SetDescription(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                            const char *s){
  assert(ctx);
  free(ctx->description);
  if (s)
    ctx->description=strdup(s);
  else
    ctx->description=0;
}



const GWEN_CRYPTTOKEN_SIGNINFO*
GWEN_CryptToken_Context_GetSignInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx) {
  assert(ctx);
  return ctx->signInfo;
}



void GWEN_CryptToken_Context_SetSignInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                         const GWEN_CRYPTTOKEN_SIGNINFO *si){
  assert(ctx);
  GWEN_CryptToken_SignInfo_free(ctx->signInfo);
  if (si) ctx->signInfo=GWEN_CryptToken_SignInfo_dup(si);
  else ctx->signInfo=0;
}



const GWEN_CRYPTTOKEN_CRYPTINFO*
GWEN_CryptToken_Context_GetCryptInfo(const GWEN_CRYPTTOKEN_CONTEXT *ctx) {
  assert(ctx);
  return ctx->cryptInfo;
}



void
GWEN_CryptToken_Context_SetCryptInfo(GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                     const GWEN_CRYPTTOKEN_CRYPTINFO *ci) {
  assert(ctx);
  GWEN_CryptToken_CryptInfo_free(ctx->cryptInfo);
  if (ci) ctx->cryptInfo=GWEN_CryptToken_CryptInfo_dup(ci);
  else ctx->cryptInfo=0;
}









/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             Token functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_CRYPTTOKEN *GWEN_CryptToken_new(GWEN_PLUGIN_MANAGER *pm,
                                     GWEN_CRYPTTOKEN_DEVICE devType,
                                     const char *typeName,
                                     const char *subTypeName,
                                     const char *name){
  GWEN_CRYPTTOKEN *ct;

  assert(pm);
  assert(typeName);
  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN, ct);
  GWEN_LIST_INIT(GWEN_CRYPTTOKEN, ct);
  GWEN_INHERIT_INIT(GWEN_CRYPTTOKEN, ct);

  ct->cryptManager=pm;
  ct->deviceType=devType;
  if (name)
    ct->tokenName=strdup(name);
  ct->tokenType=strdup(typeName);
  if (subTypeName)
    ct->tokenSubType=strdup(subTypeName);

  ct->contextList=GWEN_CryptToken_Context_List_new();
  ct->signInfoList=GWEN_CryptToken_SignInfo_List_new();
  ct->cryptInfoList=GWEN_CryptToken_CryptInfo_List_new();
  ct->keyInfoList=GWEN_CryptToken_KeyInfo_List_new();
  ct->userList=GWEN_CryptToken_User_List_new();

  return ct;
}



void GWEN_CryptToken_free(GWEN_CRYPTTOKEN *ct){
  if (ct) {
    GWEN_INHERIT_FINI(GWEN_CRYPTTOKEN, ct);
    GWEN_LIST_FINI(GWEN_CRYPTTOKEN, ct);
    free(ct->tokenName);
    free(ct->tokenSubType);
    free(ct->tokenType);
    free(ct->descriptiveName);
    GWEN_CryptToken_User_List_free(ct->userList);
    GWEN_CryptToken_Context_List_free(ct->contextList);
    GWEN_CryptToken_SignInfo_List_free(ct->signInfoList);
    GWEN_CryptToken_CryptInfo_List_free(ct->cryptInfoList);
    GWEN_CryptToken_KeyInfo_List_free(ct->keyInfoList);
    GWEN_FREE_OBJECT(ct);
  }
}



GWEN_CRYPTTOKEN *GWEN_CryptToken_fromXml(GWEN_PLUGIN_MANAGER *pm,
                                         GWEN_CRYPTTOKEN_DEVICE devType,
					 GWEN_XMLNODE *n) {
  GWEN_CRYPTTOKEN *ct;
  const char *typeName;
  const char *subTypeName;
  const char *name;

  typeName=GWEN_XMLNode_GetProperty(n, "typeName", 0);
  subTypeName=GWEN_XMLNode_GetProperty(n, "subTypeName", 0);
  name=GWEN_XMLNode_GetProperty(n, "name", 0);
  if (!typeName || !*typeName || !name || !*name) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing name or typeName");
    return 0;
  }

  ct=GWEN_CryptToken_new(pm, devType, typeName, subTypeName, name);
  if (GWEN_CryptToken_ReadXml(ct, n)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_CryptToken_free(ct);
    return 0;
  }

  return ct;
}



int GWEN_CryptToken_ReadXml(GWEN_CRYPTTOKEN *ct, GWEN_XMLNODE *n) {
  GWEN_XMLNODE *nn;
  const char *s;

  assert(ct);
  GWEN_CryptToken_Context_List_Clear(ct->contextList);
  GWEN_CryptToken_SignInfo_List_Clear(ct->signInfoList);
  GWEN_CryptToken_CryptInfo_List_Clear(ct->cryptInfoList);
  GWEN_CryptToken_KeyInfo_List_Clear(ct->keyInfoList);
  ct->flags=0;

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  while(nn) {
    GWEN_XMLNODE *nnn;

    nnn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nnn) {
      GWEN_XMLNODE *nd;

      nd=GWEN_XMLNode_GetFirstData(nnn);
      if (nd) {
        s=GWEN_XMLNode_GetData(nd);
        assert(s);
	if (strcasecmp(s, "context_management")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_CONTEXT_MANAGEMENT;
	else if (strcasecmp(s, "context_allow_add")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_CONTEXT_ALLOW_ADD;
	else if (strcasecmp(s, "context_ro_keys")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_KEYS;
	else if (strcasecmp(s, "context_ro_descr")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_DESCR;
	else if (strcasecmp(s, "context_ro_signdescr")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_SIGNDESCR;
	else if (strcasecmp(s, "context_ro_cryptdescr")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_CRYPTDESCR;
	else if (strcasecmp(s, "context_ro_keydescrs")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_CONTEXT_RO_KEYDESCRS;
	else if (strcasecmp(s, "manages_signseq")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_MANAGES_SIGNSEQ;
	else if (strcasecmp(s, "predef_context_only")==0)
	  ct->flags|=GWEN_CRYPTTOKEN_FLAGS_PREDEF_CONTEXT_ONLY;
	else {
	  DBG_WARN(GWEN_LOGDOMAIN, "Unknown flag \"%s\"", s);
	}
      }
      nnn=GWEN_XMLNode_FindNextTag(nnn, "flag", 0, 0);
    } /* while */
    nn=GWEN_XMLNode_FindNextTag(nn, "flags", 0, 0);
  } /* while */

  /* read signinfos */
  nn=GWEN_XMLNode_FindFirstTag(n, "signinfos", 0, 0);
  while(nn) {
    GWEN_XMLNODE *nnn;

    nnn=GWEN_XMLNode_FindFirstTag(nn, "signinfo", 0, 0);
    while(nnn) {
      GWEN_CRYPTTOKEN_SIGNINFO *si;

      si=GWEN_CryptToken_SignInfo_fromXml(nnn);
      assert(si);
      GWEN_CryptToken_SignInfo_List_Add(si, ct->signInfoList);
      nnn=GWEN_XMLNode_FindNextTag(nnn, "signinfo", 0, 0);
    }

    nn=GWEN_XMLNode_FindNextTag(nn, "signinfos", 0, 0);
  } /* while */

  /* read cryptinfos */
  nn=GWEN_XMLNode_FindFirstTag(n, "cryptinfos", 0, 0);
  while(nn) {
    GWEN_XMLNODE *nnn;

    nnn=GWEN_XMLNode_FindFirstTag(nn, "cryptinfo", 0, 0);
    while(nnn) {
      GWEN_CRYPTTOKEN_CRYPTINFO *ci;

      ci=GWEN_CryptToken_CryptInfo_fromXml(nnn);
      assert(ci);
      GWEN_CryptToken_CryptInfo_List_Add(ci, ct->cryptInfoList);
      nnn=GWEN_XMLNode_FindNextTag(nnn, "cryptinfo", 0, 0);
    }

    nn=GWEN_XMLNode_FindNextTag(nn, "cryptinfos", 0, 0);
  } /* while */

  /* read key infos */
  nn=GWEN_XMLNode_FindFirstTag(n, "keyinfos", 0, 0);
  while(nn) {
    GWEN_XMLNODE *nnn;

    nnn=GWEN_XMLNode_FindFirstTag(nn, "keyinfo", 0, 0);
    while(nnn) {
      GWEN_CRYPTTOKEN_KEYINFO *ki;

      ki=GWEN_CryptToken_KeyInfo_fromXml(nnn);
      assert(ki);
      GWEN_CryptToken_KeyInfo_List_Add(ki, ct->keyInfoList);
      nnn=GWEN_XMLNode_FindNextTag(nnn, "keyinfo", 0, 0);
    }

    nn=GWEN_XMLNode_FindNextTag(nn, "keyinfos", 0, 0);
  } /* while */

  /* read contexts */
  nn=GWEN_XMLNode_FindFirstTag(n, "contexts", 0, 0);
  while(nn) {
    GWEN_XMLNODE *nnn;

    nnn=GWEN_XMLNode_FindFirstTag(nn, "context", 0, 0);
    while(nnn) {
      GWEN_CRYPTTOKEN_CONTEXT *ctx;

      ctx=GWEN_CryptToken_Context_fromXml(nnn);
      assert(ctx);
      GWEN_CryptToken_Context_List_Add(ctx, ct->contextList);
      nnn=GWEN_XMLNode_FindNextTag(nnn, "context", 0, 0);
    }

    nn=GWEN_XMLNode_FindNextTag(nn, "contexts", 0, 0);
  } /* while */

  return 0;
}




GWEN_PLUGIN_MANAGER*
GWEN_CryptToken_GetCryptManager(const GWEN_CRYPTTOKEN *ct){
  assert(ct);
  return ct->cryptManager;
}



const char *GWEN_CryptToken_GetTokenName(const GWEN_CRYPTTOKEN *ct){
  assert(ct);
  return ct->tokenName;
}



void GWEN_CryptToken_SetTokenName(GWEN_CRYPTTOKEN *ct, const char *s){
  assert(ct);
  free(ct->tokenName);
  if (s) ct->tokenName=strdup(s);
  else ct->tokenName=0;
}



const char *GWEN_CryptToken_GetTokenType(const GWEN_CRYPTTOKEN *ct){
  assert(ct);
  return ct->tokenType;
}



const char *GWEN_CryptToken_GetTokenSubType(const GWEN_CRYPTTOKEN *ct){
  assert(ct);
  return ct->tokenSubType;
}



GWEN_CRYPTTOKEN_DEVICE
GWEN_CryptToken_GetDeviceType(const GWEN_CRYPTTOKEN *ct){
  assert(ct);
  return ct->deviceType;
}



const char *GWEN_CryptToken_GetDescriptiveName(const GWEN_CRYPTTOKEN *ct) {
  assert(ct);
  return ct->descriptiveName;
}



void GWEN_CryptToken_SetDescriptiveName(GWEN_CRYPTTOKEN *ct, const char *s) {
  assert(ct);
  free(ct->descriptiveName);
  if (s) ct->descriptiveName=strdup(s);
  else ct->descriptiveName=0;
}



GWEN_TYPE_UINT32 GWEN_CryptToken_GetFlags(const GWEN_CRYPTTOKEN *ct) {
  assert(ct);
  return ct->flags;
}



void GWEN_CryptToken_SetFlags(GWEN_CRYPTTOKEN *ct, GWEN_TYPE_UINT32 fl) {
  assert(ct);
  ct->flags=fl;
}



void GWEN_CryptToken_AddFlags(GWEN_CRYPTTOKEN *ct, GWEN_TYPE_UINT32 fl) {
  assert(ct);
  ct->flags|=fl;
}



void GWEN_CryptToken_SubFlags(GWEN_CRYPTTOKEN *ct, GWEN_TYPE_UINT32 fl) {
  assert(ct);
  ct->flags&=~fl;
}



void GWEN_CryptToken_SetOpenFn(GWEN_CRYPTTOKEN *ct,
                               GWEN_CRYPTTOKEN_OPEN_FN fn){
  assert(ct);
  ct->openFn=fn;
}



GWEN_CRYPTTOKEN_OPEN_FN
GWEN_CryptToken_GetOpenFn(const GWEN_CRYPTTOKEN *ct) {
  assert(ct);
  return ct->openFn;
}



void GWEN_CryptToken_SetCreateFn(GWEN_CRYPTTOKEN *ct,
                                 GWEN_CRYPTTOKEN_CREATE_FN fn){
  assert(ct);
  ct->createFn=fn;
}



GWEN_CRYPTTOKEN_CREATE_FN
GWEN_CryptToken_GetCreateFn(const GWEN_CRYPTTOKEN *ct) {
  assert(ct);
  return ct->createFn;
}



void GWEN_CryptToken_SetCloseFn(GWEN_CRYPTTOKEN *ct,
                                GWEN_CRYPTTOKEN_CLOSE_FN fn) {
  assert(ct);
  ct->closeFn=fn;
}



GWEN_CRYPTTOKEN_CLOSE_FN
GWEN_CryptToken_GetCloseFn(const GWEN_CRYPTTOKEN *ct) {
  assert(ct);
  return ct->closeFn;
}



void GWEN_CryptToken_SetChangePinFn(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_CHANGEPIN_FN fn){
  assert(ct);
  ct->changePinFn=fn;
}



void GWEN_CryptToken_SetSignFn(GWEN_CRYPTTOKEN *ct,
                               GWEN_CRYPTTOKEN_SIGN_FN fn){
  assert(ct);
  ct->signFn=fn;
}



void GWEN_CryptToken_SetVerifyFn(GWEN_CRYPTTOKEN *ct,
                                 GWEN_CRYPTTOKEN_VERIFY_FN fn){
  assert(ct);
  ct->verifyFn=fn;
}



void GWEN_CryptToken_SetEncryptFn(GWEN_CRYPTTOKEN *ct,
                                  GWEN_CRYPTTOKEN_ENCRYPT_FN fn){
  assert(ct);
  ct->encryptFn=fn;
}



void GWEN_CryptToken_SetDecryptFn(GWEN_CRYPTTOKEN *ct,
                                  GWEN_CRYPTTOKEN_DECRYPT_FN fn){
  assert(ct);
  ct->decryptFn=fn;
}



void GWEN_CryptToken_SetReadKeyFn(GWEN_CRYPTTOKEN *ct,
                                  GWEN_CRYPTTOKEN_READKEY_FN fn){
  assert(ct);
  ct->readKeyFn=fn;
}



void GWEN_CryptToken_SetWriteKeyFn(GWEN_CRYPTTOKEN *ct,
                                   GWEN_CRYPTTOKEN_WRITEKEY_FN fn){
  assert(ct);
  ct->writeKeyFn=fn;
}



void GWEN_CryptToken_SetReadKeySpecFn(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_READKEYSPEC_FN fn){
  assert(ct);
  ct->readKeySpecFn=fn;
}



void GWEN_CryptToken_SetWriteKeySpecFn(GWEN_CRYPTTOKEN *ct,
                                       GWEN_CRYPTTOKEN_WRITEKEYSPEC_FN fn){
  assert(ct);
  ct->writeKeySpecFn=fn;
}



void GWEN_CryptToken_SetGenerateKeyFn(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_GENERATEKEY_FN fn){
  assert(ct);
  ct->generateKeyFn=fn;
}



void GWEN_CryptToken_SetFillContextListFn(GWEN_CRYPTTOKEN *ct,
                                          GWEN_CRYPTTOKEN_FILLCONTEXTLIST_FN fn){
  assert(ct);
  ct->fillContextListFn=fn;
}



void GWEN_CryptToken_SetFillSignInfoListFn(GWEN_CRYPTTOKEN *ct,
                                           GWEN_CRYPTTOKEN_FILLSIGNINFOLIST_FN fn){
  assert(ct);
  ct->fillSignInfoListFn=fn;
}



void GWEN_CryptToken_SetFillCryptInfoListFn(GWEN_CRYPTTOKEN *ct,
                                            GWEN_CRYPTTOKEN_FILLCRYPTINFOLIST_FN fn){
  assert(ct);
  ct->fillCryptInfoListFn=fn;
}



void GWEN_CryptToken_SetFillKeyInfoListFn(GWEN_CRYPTTOKEN *ct,
                                          GWEN_CRYPTTOKEN_FILLKEYINFOLIST_FN fn){
  assert(ct);
  ct->fillKeyInfoListFn=fn;
}



void GWEN_CryptToken_SetFillUserListFn(GWEN_CRYPTTOKEN *ct,
                                       GWEN_CRYPTTOKEN_FILLUSERLIST_FN fn){
  assert(ct);
  ct->fillUserListFn=fn;
}



void GWEN_CryptToken_SetGetSignSeqFn(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_GETSIGNSEQ_FN fn){
  assert(ct);
  ct->getSignSeqFn=fn;
}



void GWEN_CryptToken_SetModifyUserFn(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_MODIFYUSER_FN fn) {
  assert(ct);
  ct->modifyUserFn=fn;
}




int GWEN_CryptToken_Open(GWEN_CRYPTTOKEN *ct, int manage){
  int rv;

  assert(ct);
  if (ct->isOpen) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Already open");
    return GWEN_ERROR_OPEN;
  }
  if (ct->openFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  rv=ct->openFn(ct, manage);
  if (rv)
    return rv;
  ct->isOpen=1;
  return 0;
}



int GWEN_CryptToken_Create(GWEN_CRYPTTOKEN *ct){
  int rv;

  assert(ct);
  if (ct->isOpen) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Already open");
    return GWEN_ERROR_OPEN;
  }
  if (ct->createFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  rv=ct->createFn(ct);
  if (rv)
    return rv;
  ct->isOpen=1;
  return 0;
}



int GWEN_CryptToken_Close(GWEN_CRYPTTOKEN *ct){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->closeFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->closeFn(ct);
}



int GWEN_CryptToken_ChangePin(GWEN_CRYPTTOKEN *ct){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->changePinFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->changePinFn(ct);
}



int GWEN_CryptToken_Sign(GWEN_CRYPTTOKEN *ct,
                         const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                         GWEN_BUFFER *src,
                         GWEN_BUFFER *dst){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->signFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->signFn(ct, ctx, src, dst);
}



int GWEN_CryptToken_Verify(GWEN_CRYPTTOKEN *ct,
                           const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                           GWEN_BUFFER *src,
                           GWEN_BUFFER *dst){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->verifyFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->verifyFn(ct, ctx, src, dst);
}



int GWEN_CryptToken_Encrypt(GWEN_CRYPTTOKEN *ct,
                            const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                            GWEN_BUFFER *src,
                            GWEN_BUFFER *dst){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->encryptFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->encryptFn(ct, ctx, src, dst);
}



int GWEN_CryptToken_Decrypt(GWEN_CRYPTTOKEN *ct,
                            const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                            GWEN_BUFFER *src,
                            GWEN_BUFFER *dst){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->decryptFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->decryptFn(ct, ctx, src, dst);
}



int GWEN_CryptToken_ReadKey(GWEN_CRYPTTOKEN *ct,
                            GWEN_TYPE_UINT32 kid,
                            GWEN_CRYPTKEY **key){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->readKeyFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->readKeyFn(ct, kid, key);
}



int GWEN_CryptToken_WriteKey(GWEN_CRYPTTOKEN *ct,
                             GWEN_TYPE_UINT32 kid,
                             const GWEN_CRYPTKEY *key){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->writeKeyFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->writeKeyFn(ct, kid, key);
}



int GWEN_CryptToken_ReadKeySpec(GWEN_CRYPTTOKEN *ct,
                                GWEN_TYPE_UINT32 kid,
                                GWEN_KEYSPEC **ks) {
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->readKeySpecFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->readKeySpecFn(ct, kid, ks);
}



int GWEN_CryptToken_WriteKeySpec(GWEN_CRYPTTOKEN *ct,
                                 GWEN_TYPE_UINT32 kid,
                                 const GWEN_KEYSPEC *ks) {
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->writeKeySpecFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->writeKeySpecFn(ct, kid, ks);
}



int GWEN_CryptToken_GetSignSeq(GWEN_CRYPTTOKEN *ct,
                               GWEN_TYPE_UINT32 kid,
                               GWEN_TYPE_UINT32 *signSeq){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->getSignSeqFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->getSignSeqFn(ct, kid, signSeq);
}



int GWEN_CryptToken_AddContext(GWEN_CRYPTTOKEN *ct,
                               const GWEN_CRYPTTOKEN_CONTEXT *ctx){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  GWEN_CryptToken_Context_List_Add(GWEN_CryptToken_Context_dup(ctx),
                                   ct->contextList);
  return 0;
}



int GWEN_CryptToken_FillContextList(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_CONTEXT_LIST *l){
  GWEN_CRYPTTOKEN_CONTEXT *ctx;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }

  if (GWEN_CryptToken_Context_List_GetCount(ct->contextList)==0) {
    int rv;

    if (ct->fillContextListFn==0)
      return GWEN_ERROR_UNSUPPORTED;
    rv=ct->fillContextListFn(ct, ct->contextList);
    if (rv)
      return rv;
  }

  ctx=GWEN_CryptToken_Context_List_First(ct->contextList);
  while(ctx) {
    GWEN_CryptToken_Context_List_Add(GWEN_CryptToken_Context_dup(ctx), l);
    ctx=GWEN_CryptToken_Context_List_Next(ctx);
  }

  return 0;
}



int GWEN_CryptToken_FillSignInfoList(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_SIGNINFO_LIST *l){
  GWEN_CRYPTTOKEN_SIGNINFO *si;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->fillSignInfoListFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  if (GWEN_CryptToken_SignInfo_List_GetCount(ct->signInfoList)==0) {
    int rv;

    rv=ct->fillSignInfoListFn(ct, ct->signInfoList);
    if (rv)
      return rv;
  }

  si=GWEN_CryptToken_SignInfo_List_First(ct->signInfoList);
  while(si) {
    GWEN_CryptToken_SignInfo_List_Add(GWEN_CryptToken_SignInfo_dup(si), l);
    si=GWEN_CryptToken_SignInfo_List_Next(si);
  }

  return 0;
}



int GWEN_CryptToken_FillCryptInfoList(GWEN_CRYPTTOKEN *ct,
                                      GWEN_CRYPTTOKEN_CRYPTINFO_LIST *l){
  GWEN_CRYPTTOKEN_CRYPTINFO *ci;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->fillCryptInfoListFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  if (GWEN_CryptToken_CryptInfo_List_GetCount(ct->cryptInfoList)==0) {
    int rv;

    rv=ct->fillCryptInfoListFn(ct, ct->cryptInfoList);
    if (rv)
      return rv;
  }

  ci=GWEN_CryptToken_CryptInfo_List_First(ct->cryptInfoList);
  while(ci) {
    GWEN_CryptToken_CryptInfo_List_Add(GWEN_CryptToken_CryptInfo_dup(ci), l);
    ci=GWEN_CryptToken_CryptInfo_List_Next(ci);
  }

  return 0;
}



int GWEN_CryptToken_FillKeyInfoList(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_KEYINFO_LIST *l){
  GWEN_CRYPTTOKEN_KEYINFO *ki;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->fillKeyInfoListFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  if (GWEN_CryptToken_KeyInfo_List_GetCount(ct->keyInfoList)==0) {
    int rv;

    rv=ct->fillKeyInfoListFn(ct, ct->keyInfoList);
    if (rv)
      return rv;
  }

  ki=GWEN_CryptToken_KeyInfo_List_First(ct->keyInfoList);
  while(ki) {
    GWEN_CryptToken_KeyInfo_List_Add(GWEN_CryptToken_KeyInfo_dup(ki), l);
    ki=GWEN_CryptToken_KeyInfo_List_Next(ki);
  }

  return 0;
}



int GWEN_CryptToken_GenerateKey(GWEN_CRYPTTOKEN *ct,
                                const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                GWEN_CRYPTKEY **key){
  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  if (ct->generateKeyFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return ct->generateKeyFn(ct, ki, key);
}




const GWEN_CRYPTTOKEN_SIGNINFO*
GWEN_CryptToken_GetSignInfoByAlgos(GWEN_CRYPTTOKEN *ct,
                                   GWEN_CRYPTTOKEN_HASHALGO hashAlgo,
                                   GWEN_CRYPTTOKEN_PADDALGO paddAlgo){
  GWEN_CRYPTTOKEN_SIGNINFO *si;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return 0;
  }

  if (GWEN_CryptToken_SignInfo_List_GetCount(ct->signInfoList)==0) {
    if (ct->fillSignInfoListFn==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "fillSignInfoListFn not set");
      return 0;
    }
    else {
      int rv;

      rv=ct->fillSignInfoListFn(ct, ct->signInfoList);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return 0;
      }
    }
  }

  si=GWEN_CryptToken_SignInfo_List_First(ct->signInfoList);
  while(si) {
    if ((hashAlgo==0 || hashAlgo==GWEN_CryptToken_SignInfo_GetHashAlgo(si)) &&
        (paddAlgo==0 || paddAlgo==GWEN_CryptToken_SignInfo_GetPaddAlgo(si)))
      return si;
    si=GWEN_CryptToken_SignInfo_List_Next(si);
  }

  return 0;
}



const GWEN_CRYPTTOKEN_CRYPTINFO*
GWEN_CryptToken_GetCryptInfoByAlgos(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_CRYPTALGO cryptAlgo,
                                    GWEN_CRYPTTOKEN_PADDALGO paddAlgo){
  GWEN_CRYPTTOKEN_CRYPTINFO *ci;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return 0;
  }

  if (GWEN_CryptToken_CryptInfo_List_GetCount(ct->cryptInfoList)==0) {
    if (ct->fillCryptInfoListFn==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "fillCryptInfoListFn not set");
      return 0;
    }
    else {
      int rv;

      rv=ct->fillCryptInfoListFn(ct, ct->cryptInfoList);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return 0;
      }
    }
  }

  ci=GWEN_CryptToken_CryptInfo_List_First(ct->cryptInfoList);
  while(ci) {
    if ((cryptAlgo==0 || cryptAlgo==GWEN_CryptToken_CryptInfo_GetCryptAlgo(ci))
        &&
        (paddAlgo==0 || paddAlgo==GWEN_CryptToken_CryptInfo_GetPaddAlgo(ci)))
      return ci;
    ci=GWEN_CryptToken_CryptInfo_List_Next(ci);
  }

  return 0;
}



const GWEN_CRYPTTOKEN_KEYINFO*
GWEN_CryptToken_GetKeyInfoById(GWEN_CRYPTTOKEN *ct,
                               GWEN_TYPE_UINT32 kid){
  GWEN_CRYPTTOKEN_KEYINFO *ki;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return 0;
  }

  if (GWEN_CryptToken_KeyInfo_List_GetCount(ct->keyInfoList)==0) {
    if (ct->fillKeyInfoListFn==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "fillKeyInfoListFn not set");
      return 0;
    }
    else {
      int rv;

      rv=ct->fillKeyInfoListFn(ct, ct->keyInfoList);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return 0;
      }
    }
  }

  ki=GWEN_CryptToken_KeyInfo_List_First(ct->keyInfoList);
  while(ki) {
    if (kid==0 || kid==GWEN_CryptToken_KeyInfo_GetKeyId(ki))
      return ki;
    ki=GWEN_CryptToken_KeyInfo_List_Next(ki);
  }

  return 0;
}



const GWEN_CRYPTTOKEN_CONTEXT*
GWEN_CryptToken_GetContextById(GWEN_CRYPTTOKEN *ct,
                               GWEN_TYPE_UINT32 id){
  GWEN_CRYPTTOKEN_CONTEXT *ctx;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return 0;
  }

  if (GWEN_CryptToken_Context_List_GetCount(ct->contextList)==0) {
    if (ct->fillContextListFn==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "fillKeyInfoListFn not set");
      return 0;
    }
    else {
      int rv;

      rv=ct->fillContextListFn(ct, ct->contextList);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return 0;
      }
    }
  }

  ctx=GWEN_CryptToken_Context_List_First(ct->contextList);
  while(ctx) {
    if (id==0 || id==GWEN_CryptToken_Context_GetId(ctx))
      return ctx;
    ctx=GWEN_CryptToken_Context_List_Next(ctx);
  }

  return 0;

}



int GWEN_CryptToken_FillUserList(GWEN_CRYPTTOKEN *ct,
                                 GWEN_CRYPTTOKEN_USER_LIST *ul) {
  GWEN_CRYPTTOKEN_USER *u;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return GWEN_ERROR_NOT_OPEN;
  }

  if (GWEN_CryptToken_User_List_GetCount(ct->userList)==0) {
    int rv;

    if (ct->fillUserListFn==0)
      return GWEN_ERROR_UNSUPPORTED;
    rv=ct->fillUserListFn(ct, ct->userList);
    if (rv)
      return rv;
  }

  if (ul) {
    u=GWEN_CryptToken_User_List_First(ct->userList);
    while(u) {
      GWEN_CryptToken_User_List_Add(GWEN_CryptToken_User_dup(u), ul);
      u=GWEN_CryptToken_User_List_Next(u);
    }
  }

  return 0;
}



int GWEN_CryptToken_ModifyUser(GWEN_CRYPTTOKEN *ct,
                               const GWEN_CRYPTTOKEN_USER *u) {
  int rv;
  GWEN_TYPE_UINT32 id;
  GWEN_CRYPTTOKEN_USER *ou;
  const char *s;
  int i;

  assert(ct);
  if (ct->isOpen==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not open");
    return 0;
  }
  assert(u);

  id=GWEN_CryptToken_User_GetId(u);
  if (id==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid id");
    return GWEN_ERROR_INVALID;
  }

  /* make sure we have a valid list of users */
  GWEN_CryptToken_FillUserList(ct, 0);

  ou=GWEN_CryptToken_User_List_First(ct->userList);
  while(ou) {
    if (id==GWEN_CryptToken_User_GetId(ou))
      break;
  }
  if (!ou) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown user");
    return GWEN_ERROR_INVALID;
  }

  if (ct->modifyUserFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  rv=ct->modifyUserFn(ct, u);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  /* copy data to stored user */
  s=GWEN_CryptToken_User_GetUserId(u);
  GWEN_CryptToken_User_SetUserId(ou, s);

  s=GWEN_CryptToken_User_GetUserName(u);
  GWEN_CryptToken_User_SetUserName(ou, s);

  s=GWEN_CryptToken_User_GetPeerId(u);
  GWEN_CryptToken_User_SetPeerId(ou, s);

  s=GWEN_CryptToken_User_GetPeerName(u);
  GWEN_CryptToken_User_SetPeerName(ou, s);

  s=GWEN_CryptToken_User_GetAddress(u);
  GWEN_CryptToken_User_SetAddress(ou, s);

  i=GWEN_CryptToken_User_GetPort(u);
  GWEN_CryptToken_User_SetPort(ou, i);

  id=GWEN_CryptToken_User_GetContextId(u);
  GWEN_CryptToken_User_SetContextId(ou, id);

  return 0;
}



int GWEN_CryptToken_Hash(GWEN_CRYPTTOKEN_HASHALGO algo,
                         const char *src,
                         unsigned int slen,
                         GWEN_BUFFER *dstBuf) {
  int rv;

  assert(src);
  assert(slen);

  DBG_ERROR(GWEN_LOGDOMAIN, "Hashing with algo \"%s\"",
            GWEN_CryptToken_HashAlgo_toString(algo));

  switch(algo) {
  case GWEN_CryptToken_HashAlgo_None:
    GWEN_Buffer_AppendBytes(dstBuf, src, slen);
    rv=0;
    break;

  case GWEN_CryptToken_HashAlgo_SHA1:
    rv=GWEN_MD_HashToBuffer("sha1", src, slen, dstBuf);
    break;

  case GWEN_CryptToken_HashAlgo_RMD160:
    rv=GWEN_MD_HashToBuffer("rmd160", src, slen, dstBuf);
    break;

  case GWEN_CryptToken_HashAlgo_MD5:
    rv=GWEN_MD_HashToBuffer("md5", src, slen, dstBuf);
    break;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Algo-Type %d (%s) not supported",
              algo, GWEN_CryptToken_HashAlgo_toString(algo));
    return GWEN_ERROR_NOT_AVAILABLE;
  }

  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error hashing with algo %d (%s)",
              algo, GWEN_CryptToken_HashAlgo_toString(algo));
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int GWEN_CryptToken_Padd(GWEN_CRYPTTOKEN_PADDALGO algo,
                         unsigned int chunkSize,
                         GWEN_BUFFER *buf) {
  int rv;
  unsigned int diff;
  unsigned bsize;
  unsigned dstSize;

  DBG_ERROR(GWEN_LOGDOMAIN, "Padding with algo \"%s\"",
            GWEN_CryptToken_PaddAlgo_toString(algo));

  assert(buf);
  bsize=GWEN_Buffer_GetUsedBytes(buf);
  dstSize=bsize+(chunkSize-1);
  dstSize=(dstSize/chunkSize)*chunkSize;
  diff=dstSize-GWEN_Buffer_GetUsedBytes(buf);

  switch(algo) {
  case GWEN_CryptToken_PaddAlgo_None:
    rv=0;
    break;

  case GWEN_CryptToken_PaddAlgo_ISO9796_1A4:
    if (dstSize>96) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Padding size must be <=96 bytes (is %d)",
		dstSize);
      return GWEN_ERROR_INVALID;
    }
    rv=GWEN_Padd_PaddWithISO9796(buf);
    break;

  case GWEN_CryptToken_PaddAlgo_LeftZero:
    rv=GWEN_Buffer_FillLeftWithBytes(buf, 0, diff);
    break;

  case GWEN_CryptToken_PaddAlgo_RightZero:
    rv=GWEN_Buffer_FillWithBytes(buf, 0, diff);
    break;

  case GWEN_CryptToken_PaddAlgo_ANSIX9_23:
    if (dstSize % 8) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Padding size must be multiple of 8 (is %d)", dstSize);
      return GWEN_ERROR_INVALID;
    }
    rv=GWEN_Padd_PaddWithANSIX9_23(buf);
    break;

  case GWEN_CryptToken_PaddAlgo_PKCS1_1:
    rv=GWEN_Padd_PaddWithPkcs1Bt1(buf, dstSize);
    break;

  case GWEN_CryptToken_PaddAlgo_PKCS1_2:
    rv=GWEN_Padd_PaddWithPkcs1Bt2(buf, dstSize);
    break;

  case GWEN_CryptToken_PaddAlgo_ISO9796_1:
  case GWEN_CryptToken_PaddAlgo_ISO9796_2:
  default:
    DBG_INFO(GWEN_LOGDOMAIN, "Algo-Type %d (%s) not supported",
             algo, GWEN_CryptToken_PaddAlgo_toString(algo));
    return GWEN_ERROR_NOT_AVAILABLE;
  }

  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error padding with algo %d (%s)",
              algo, GWEN_CryptToken_PaddAlgo_toString(algo));
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int GWEN_CryptToken_Unpadd(GWEN_CRYPTTOKEN_PADDALGO algo,
                           GWEN_BUFFER *buf) {
  int rv;

  assert(buf);

  switch(algo) {
  case GWEN_CryptToken_PaddAlgo_None:
    rv=0;
    break;

  case GWEN_CryptToken_PaddAlgo_ISO9796_1A4:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unpadding with ISO9796_1A4 not possible");
    return GWEN_ERROR_INVALID;

  case GWEN_CryptToken_PaddAlgo_LeftZero:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unpadding with LeftZero not possible");
    return GWEN_ERROR_INVALID;

  case GWEN_CryptToken_PaddAlgo_RightZero:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unpadding with RightZero not possible");
    return GWEN_ERROR_INVALID;

  case GWEN_CryptToken_PaddAlgo_ANSIX9_23:
    rv=GWEN_Padd_UnpaddWithANSIX9_23(buf);
    break;

  case GWEN_CryptToken_PaddAlgo_PKCS1_1:
    rv=GWEN_Padd_UnpaddWithPkcs1Bt1(buf);
    break;

  case GWEN_CryptToken_PaddAlgo_PKCS1_2:
    rv=GWEN_Padd_UnpaddWithPkcs1Bt2(buf);
    break;

  case GWEN_CryptToken_PaddAlgo_ISO9796_1:
  case GWEN_CryptToken_PaddAlgo_ISO9796_2:
  default:
    DBG_INFO(GWEN_LOGDOMAIN, "Algo-Type %d (%s) not supported",
             algo, GWEN_CryptToken_PaddAlgo_toString(algo));
    return GWEN_ERROR_NOT_AVAILABLE;
  }

  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error unpadding with algo %d (%s)",
              algo, GWEN_CryptToken_PaddAlgo_toString(algo));
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}







/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                           CryptToken_Plugin functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_INHERIT(GWEN_PLUGIN, GWEN_CRYPTTOKEN_PLUGIN)


GWEN_PLUGIN *GWEN_CryptToken_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                                        GWEN_CRYPTTOKEN_DEVICE devType,
                                        const char *typeName,
					const char *fileName) {
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN_PLUGIN *ctp;

  assert(pm);
  pl=GWEN_Plugin_new(pm, typeName, fileName);
  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN_PLUGIN, ctp);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, GWEN_CRYPTTOKEN_PLUGIN, pl, ctp,
		       GWEN_CryptToken_Plugin_FreeData);
  ctp->devType=devType;
  return pl;
}



void GWEN_CryptToken_Plugin_FreeData(void *bp, void *p){
  GWEN_CRYPTTOKEN_PLUGIN *ctp;

  ctp=(GWEN_CRYPTTOKEN_PLUGIN*)p;
  GWEN_FREE_OBJECT(ctp);
}



GWEN_CRYPTTOKEN_DEVICE
GWEN_CryptToken_Plugin_GetDeviceType(const GWEN_PLUGIN *pl){
  GWEN_CRYPTTOKEN_PLUGIN *ctp;

  assert(pl);
  ctp=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPTTOKEN_PLUGIN, pl);
  assert(ctp);

  return ctp->devType;
}



GWEN_CRYPTTOKEN*
GWEN_CryptToken_Plugin_CreateToken(GWEN_PLUGIN *pl,
                                   const char *subTypeName,
				   const char *name){
  GWEN_CRYPTTOKEN_PLUGIN *ctp;

  assert(pl);
  ctp=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPTTOKEN_PLUGIN, pl);
  assert(ctp);

  if (ctp->createTokenFn==0) {
    DBG_WARN(GWEN_LOGDOMAIN, "No createToken function set");
    return 0;
  }
  return ctp->createTokenFn(pl, subTypeName, name);
}



int GWEN_CryptToken_Plugin_CheckToken(GWEN_PLUGIN *pl,
				      GWEN_BUFFER *subTypeName,
				      GWEN_BUFFER *name){
  GWEN_CRYPTTOKEN_PLUGIN *ctp;

  assert(pl);
  ctp=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPTTOKEN_PLUGIN, pl);
  assert(ctp);

  if (ctp->checkTokenFn==0) {
    DBG_WARN(GWEN_LOGDOMAIN, "No checkToken function set");
    return GWEN_ERROR_CT_NOT_IMPLEMENTED;
  }
  return ctp->checkTokenFn(pl, subTypeName, name);
}



void GWEN_CryptToken_Plugin_SetCreateTokenFn(GWEN_PLUGIN *pl,
					     GWEN_CRYPTTOKEN_PLUGIN_CREATETOKEN_FN fn){
  GWEN_CRYPTTOKEN_PLUGIN *ctp;

  assert(pl);
  ctp=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPTTOKEN_PLUGIN, pl);
  assert(ctp);

  ctp->createTokenFn=fn;
}



void GWEN_CryptToken_Plugin_SetCheckTokenFn(GWEN_PLUGIN *pl,
					    GWEN_CRYPTTOKEN_PLUGIN_CHECKTOKEN_FN fn){
  GWEN_CRYPTTOKEN_PLUGIN *ctp;

  assert(pl);
  ctp=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPTTOKEN_PLUGIN, pl);
  assert(ctp);

  ctp->checkTokenFn=fn;
}

















/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                           CryptManager functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_INHERIT(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER)


GWEN_PLUGIN_MANAGER *GWEN_CryptManager_new(){
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_CRYPTMANAGER *cm;

  pm=GWEN_PluginManager_new(GWEN_CRYPTMANAGER_NAME);
  GWEN_NEW_OBJECT(GWEN_CRYPTMANAGER, cm);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm, cm,
		       GWEN_CryptManager_FreeData);

  return pm;
}



void GWEN_CryptManager_FreeData(void *bp, void *p) {
  GWEN_CRYPTMANAGER *cm;

  cm=(GWEN_CRYPTMANAGER*)p;
  GWEN_FREE_OBJECT(cm);
}



void GWEN_CryptManager_SetGetPinFn(GWEN_PLUGIN_MANAGER *pm,
				   GWEN_CRYPTMANAGER_GETPIN_FN fn){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);
  cm->getPinFn=fn;
}




void GWEN_CryptManager_SetBeginEnterPinFn(GWEN_PLUGIN_MANAGER *pm,
                                          GWEN_CRYPTMANAGER_BEGIN_ENTER_PIN_FN fn){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  cm->beginEnterPinFn=fn;
}



void GWEN_CryptManager_SetEndEnterPinFn(GWEN_PLUGIN_MANAGER *pm,
                                         GWEN_CRYPTMANAGER_END_ENTER_PIN_FN fn){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  cm->endEnterPinFn=fn;
}



void GWEN_CryptManager_SetInsertTokenFn(GWEN_PLUGIN_MANAGER *pm,
                                        GWEN_CRYPTMANAGER_INSERT_TOKEN_FN fn){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  cm->insertTokenFn=fn;
}



void GWEN_CryptManager_SetInsertCorrectTokenFn(GWEN_PLUGIN_MANAGER *pm,
                                               GWEN_CRYPTMANAGER_INSERT_CORRECT_TOKEN_FN fn){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  cm->insertCorrectTokenFn=fn;
}



void GWEN_CryptManager_SetShowMessageFn(GWEN_PLUGIN_MANAGER *pm,
                                        GWEN_CRYPTMANAGER_SHOW_MESSAGE_FN fn){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  cm->showMessageFn=fn;
}



int GWEN_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *pm,
                             GWEN_CRYPTTOKEN *token,
			     GWEN_CRYPTTOKEN_PINTYPE pt,
                             GWEN_CRYPTTOKEN_PINENCODING pe,
			     GWEN_TYPE_UINT32 flags,
                             unsigned char *buffer,
                             unsigned int minLength,
                             unsigned int maxLength,
                             unsigned int *pinLength){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  if (cm->getPinFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return cm->getPinFn(pm, token, pt, pe, flags, buffer,
                      minLength, maxLength, pinLength);
}




int GWEN_CryptManager_BeginEnterPin(GWEN_PLUGIN_MANAGER *pm,
                                    GWEN_CRYPTTOKEN *token,
                                    GWEN_CRYPTTOKEN_PINTYPE pt){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  if (cm->beginEnterPinFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return cm->beginEnterPinFn(pm, token, pt);
}



int GWEN_CryptManager_EndEnterPin(GWEN_PLUGIN_MANAGER *pm,
                                  GWEN_CRYPTTOKEN *token,
                                  GWEN_CRYPTTOKEN_PINTYPE pt,
                                   int ok){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  if (cm->endEnterPinFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return cm->endEnterPinFn(pm, token, pt, ok);
}



int GWEN_CryptManager_InsertToken(GWEN_PLUGIN_MANAGER *pm,
                                  GWEN_CRYPTTOKEN *token){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  if (cm->insertTokenFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return cm->insertTokenFn(pm, token);
}



int GWEN_CryptManager_InsertCorrectToken(GWEN_PLUGIN_MANAGER *pm,
                                         GWEN_CRYPTTOKEN *token){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  if (cm->insertCorrectTokenFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return cm->insertCorrectTokenFn(pm, token);
}



int GWEN_CryptManager_ShowMessage(GWEN_PLUGIN_MANAGER *pm,
                                  GWEN_CRYPTTOKEN *token,
				  const char *title,
                                  const char *msg){
  GWEN_CRYPTMANAGER *cm;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  if (cm->showMessageFn==0)
    return GWEN_ERROR_UNSUPPORTED;
  return cm->showMessageFn(pm, token, title, msg);
}



int GWEN_CryptManager_CheckToken(GWEN_PLUGIN_MANAGER *pm,
                                 GWEN_CRYPTTOKEN_DEVICE devt,
                                 GWEN_BUFFER *typeName,
                                 GWEN_BUFFER *subTypeName,
                                 GWEN_BUFFER *tokenName) {
  GWEN_CRYPTMANAGER *cm;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;

  assert(pm);
  cm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, GWEN_CRYPTMANAGER, pm);
  assert(cm);

  pdl=GWEN_PluginManager_GetPluginDescrs(pm);
  if (pdl==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No plugin descriptions found");
    return GWEN_ERROR_NOT_FOUND;
  }
  else {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;

    pit=GWEN_PluginDescription_List2_First(pdl);
    if (pit) {
      GWEN_PLUGIN_DESCRIPTION *pd;

      pd=GWEN_PluginDescription_List2Iterator_Data(pit);
      assert(pd);
      while(pd) {
        GWEN_XMLNODE *n;
        const char *p;
        GWEN_CRYPTTOKEN_DEVICE currentDevt;

        n=GWEN_PluginDescription_GetXmlNode(pd);
        assert(n);
        p=GWEN_XMLNode_GetProperty(n, "device", "file");
        if (strcasecmp(p, "file")==0)
          currentDevt=GWEN_CryptToken_Device_File;
        else if (strcasecmp(p, "card")==0)
          currentDevt=GWEN_CryptToken_Device_Card;
        else if (strcasecmp(p, "none")==0)
          currentDevt=GWEN_CryptToken_Device_None;
        else
          currentDevt=GWEN_CryptToken_Device_Unknown;

        if (currentDevt==devt) {
          GWEN_PLUGIN *pl;
          char logbuffer[256];

          snprintf(logbuffer, sizeof(logbuffer)-1,
                   I18N("Loading plugin \"%s\""),
                   GWEN_PluginDescription_GetName(pd));
          logbuffer[sizeof(logbuffer)-1]=0;
          GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo, logbuffer);

          /* device type matches, check this plugin */
          pl=GWEN_PluginManager_GetPlugin(pm,
                                          GWEN_PluginDescription_GetName(pd));
          if (pl) {
            GWEN_BUFFER *lSubTypeName;
            GWEN_BUFFER *lTokenName;
            int rv;

            lSubTypeName=GWEN_Buffer_dup(subTypeName);
            lTokenName=GWEN_Buffer_dup(tokenName);

            snprintf(logbuffer, sizeof(logbuffer)-1,
                     I18N("Checking plugin \"%s\""),
                     GWEN_Plugin_GetName(pl));
            logbuffer[sizeof(logbuffer)-1]=0;
            GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice, logbuffer);

            DBG_ERROR(GWEN_LOGDOMAIN,
                      "Checking plugin \"%s\"",
                      GWEN_Plugin_GetName(pl));

            rv=GWEN_CryptToken_Plugin_CheckToken(pl,
                                                 lSubTypeName,
                                                 lTokenName);
            switch(rv) {
            case 0:
              /* responsive plugin found */
              snprintf(logbuffer, sizeof(logbuffer)-1,
                       I18N("Plugin \"%s\" supports this token"),
                       GWEN_Plugin_GetName(pl));
              logbuffer[sizeof(logbuffer)-1]=0;
              GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice, logbuffer);

              GWEN_Buffer_Reset(typeName);
              GWEN_Buffer_AppendString(typeName, GWEN_Plugin_GetName(pl));
              GWEN_Buffer_Reset(subTypeName);
              GWEN_Buffer_AppendBuffer(subTypeName, lSubTypeName);
              GWEN_Buffer_Reset(tokenName);
              GWEN_Buffer_AppendBuffer(tokenName, lTokenName);
              GWEN_Buffer_free(lTokenName);
              GWEN_Buffer_free(lSubTypeName);
              GWEN_PluginDescription_List2Iterator_free(pit);
              GWEN_PluginDescription_List2_freeAll(pdl);
              return 0;

            case GWEN_ERROR_CT_NOT_IMPLEMENTED:
              snprintf(logbuffer, sizeof(logbuffer)-1,
                       I18N("Plugin \"%s\": Function not implemented"),
                       GWEN_Plugin_GetName(pl));
              logbuffer[sizeof(logbuffer)-1]=0;
              GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice, logbuffer);
              break;

            case GWEN_ERROR_CT_NOT_SUPPORTED:
              snprintf(logbuffer, sizeof(logbuffer)-1,
                       I18N("Plugin \"%s\" does not support this token"),
                       GWEN_Plugin_GetName(pl));
              logbuffer[sizeof(logbuffer)-1]=0;
              GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo, logbuffer);
              break;

            case GWEN_ERROR_CT_BAD_NAME:
              snprintf(logbuffer, sizeof(logbuffer)-1,
                       I18N("Plugin \"%s\" supports this token, but the name"
                            "did not match"),
                       GWEN_Plugin_GetName(pl));
              logbuffer[sizeof(logbuffer)-1]=0;
              GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo, logbuffer);
              break;

            default:
              snprintf(logbuffer, sizeof(logbuffer)-1,
                       I18N("Plugin \"%s\": Unexpected error (%d)"),
                       GWEN_Plugin_GetName(pl), rv);
              logbuffer[sizeof(logbuffer)-1]=0;
              GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo, logbuffer);
              break;
            } /* switch */
          } /* if plugin loaded */
          else {
            snprintf(logbuffer, sizeof(logbuffer)-1,
                     I18N("Could not load plugin \"%s\""),
                     GWEN_PluginDescription_GetName(pd));
            logbuffer[sizeof(logbuffer)-1]=0;
            GWEN_WaitCallback_Log(GWEN_LoggerLevelWarning, logbuffer);
          }
        }

        pd=GWEN_PluginDescription_List2Iterator_Next(pit);
      }

      GWEN_PluginDescription_List2Iterator_free(pit);
    }
    GWEN_PluginDescription_List2_freeAll(pdl);
  }

  return GWEN_ERROR_CT_NOT_SUPPORTED;
}








