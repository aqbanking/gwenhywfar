/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Nov 25 2003
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


#include "ipcxmlcmd.h"
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/ipc.h>
#include <string.h>
#include <assert.h>



unsigned int GWEN_IPCXMLCmd_Request_GetPubKey(GWEN_IPCXMLSERVICE *xs,
                                              unsigned int clid,
                                              unsigned int flags,
                                              const GWEN_KEYSPEC *ks) {
  GWEN_DB_NODE *cfg;
  unsigned int rqid;

  cfg=GWEN_DB_Group_new("request");

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/bankcode",
                       GWEN_IPCXMLService_GetServiceCode(xs, clid));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/userid",
                       GWEN_KeySpec_GetOwner(ks));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/keytype",
                       GWEN_KeySpec_GetKeyName(ks));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/keynum",
                      GWEN_KeySpec_GetNumber(ks));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/keyversion",
                      GWEN_KeySpec_GetVersion(ks));
  rqid=GWEN_IPCXMLService_AddRequest(xs, clid, "GetKey", 0, cfg, flags);
  if (!rqid) {
    DBG_INFO(0, "here");
  }
  return rqid;
}



GWEN_ERRORCODE GWEN_IPCXMLCmd_Handle_GetPubKey(GWEN_IPCXMLSERVICE *xs,
                                               unsigned int rqid,
                                               unsigned int flags,
                                               GWEN_DB_NODE *n) {
  GWEN_ERRORCODE err;
  GWEN_KEYSPEC *ks;
  const GWEN_CRYPTKEY *key;
  const char *s;
  unsigned int connid;
  GWEN_DB_NODE *rsp;
  GWEN_DB_NODE *keygr;
  unsigned int binsize;
  const void *binval;
  int wantSignKey;

  assert(xs);
  assert(n);

  connid=GWEN_IPCXMLService_GetRequestConnection(xs, rqid);
  if (!connid) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_REQUEST_NOT_FOUND);
  }

  ks=GWEN_KeySpec_new();

  GWEN_KeySpec_SetOwner(ks, GWEN_DB_GetCharValue(n, "key/userid", 0, ""));
  GWEN_KeySpec_SetNumber(ks, GWEN_DB_GetIntValue(n, "key/keynum", 0, 0));
  GWEN_KeySpec_SetKeyName(ks, GWEN_DB_GetCharValue(n, "key/keytype", 0, ""));
  GWEN_KeySpec_SetVersion(ks,
                          GWEN_DB_GetIntValue(n, "key/keyversion", 0, 0));
  s=GWEN_DB_GetCharValue(n, "key/keytype", 0, 0);
  if (!s) {
    DBG_ERROR(0, "no keytype");
    GWEN_KeySpec_free(ks);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_DATA);
  }
  GWEN_KeySpec_SetKeyName(ks, s);

  if (strcasecmp(GWEN_KeySpec_GetKeyName(ks), "S")==0) {
    wantSignKey=1;
    key=GWEN_IPCXMLService_GetSignKey(xs, connid);
  }
  else if (strcasecmp(GWEN_KeySpec_GetKeyName(ks), "V")==0) {
    wantSignKey=0;
    key=GWEN_IPCXMLService_GetCryptKey(xs, connid);
  }
  else {
    DBG_ERROR(0, "bad keytype");
    GWEN_KeySpec_free(ks);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_DATA);
  }

  if (!key) {
    DBG_ERROR(0, "No key");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  /* TODO: check key */

  /* build response data */
  rsp=GWEN_DB_Group_new("response");

  GWEN_DB_SetCharValue(rsp,
                       GWEN_DB_FLAGS_DEFAULT,
                       "exchangecontrol",
                       GWEN_DB_GetCharValue(n, "security/dialogid", 0, "0"));
  GWEN_DB_SetCharValue(rsp, GWEN_DB_FLAGS_DEFAULT,
                       "keyname/bankcode",
                       GWEN_IPCXMLService_GetServiceCode(xs, connid));
  GWEN_DB_SetCharValue(rsp, GWEN_DB_FLAGS_DEFAULT,
                       "keyname/userid",
                       GWEN_KeySpec_GetOwner(ks));
  GWEN_DB_SetCharValue(rsp, GWEN_DB_FLAGS_DEFAULT,
                       "keyname/keytype",
                       GWEN_KeySpec_GetKeyName(ks));
  GWEN_DB_SetIntValue(rsp, GWEN_DB_FLAGS_DEFAULT,
                      "keyname/keynum",
                      GWEN_KeySpec_GetNumber(ks));
  GWEN_DB_SetIntValue(rsp, GWEN_DB_FLAGS_DEFAULT,
                      "keyname/keyversion",
                      GWEN_KeySpec_GetVersion(ks));

  keygr=GWEN_DB_Group_new("key");
  /* get public key data */
  err=GWEN_CryptKey_ToDb(key, keygr, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_DB_Group_free(keygr);
    GWEN_DB_Group_free(rsp);
    return err;
  }

  binval=GWEN_DB_GetBinValue(keygr,
                             "data/n", 0, 0, 0, &binsize);
  if (!binval) {
    DBG_ERROR(0, "No modulus in key");
    GWEN_DB_Group_free(keygr);
    GWEN_DB_Group_free(rsp);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_DB_SetBinValue(rsp,
                      GWEN_DB_FLAGS_DEFAULT,
                      "key/modulus", binval, binsize);

  binval=GWEN_DB_GetBinValue(keygr,
                             "data/e", 0, 0, 0, &binsize);
  if (!binval) {
    DBG_ERROR(0, "No exponent in key");
    GWEN_DB_Group_free(keygr);
    GWEN_DB_Group_free(rsp);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_DB_SetBinValue(rsp,
                      GWEN_DB_FLAGS_DEFAULT,
                      "key/exponent", binval, binsize);
  GWEN_DB_SetIntValue(rsp, GWEN_DB_FLAGS_DEFAULT,
                      "keyname/purpose",
                      wantSignKey?5:6);
  GWEN_DB_Group_free(keygr);
  GWEN_KeySpec_free(ks);

  err=GWEN_IPCXMLService_AddResponse(xs, rqid, "GetKeyResponse", 0,
                                     rsp, flags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_DB_Group_free(rsp);
    return err;
  }

  GWEN_DB_Group_free(rsp);
  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLCmd_Result_GetPubKey(GWEN_IPCXMLSERVICE *xs,
                                               unsigned int rqid,
                                               GWEN_CRYPTKEY **pkey) {
  GWEN_DB_NODE *n;
  GWEN_DB_NODE *keygr;
  GWEN_CRYPTKEY *key;
  const void *binval;
  unsigned int binsize;

  n=GWEN_IPCXMLService_GetResponseData(xs, rqid);
  if (!n) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_INQUEUE_EMPTY);
  }

  keygr=GWEN_DB_Group_new("key");

  GWEN_DB_SetCharValue(keygr,
                       GWEN_DB_FLAGS_DEFAULT,
                       "type", "RSA");
  GWEN_DB_SetCharValue(keygr,
                       GWEN_DB_FLAGS_DEFAULT,
                       "name",
                       GWEN_DB_GetCharValue(n, "keyname/keytype", 0, ""));

  GWEN_DB_SetCharValue(keygr,
                       GWEN_DB_FLAGS_DEFAULT,
                       "owner",
                       GWEN_DB_GetCharValue(n, "keyname/userid", 0, ""));

  GWEN_DB_SetIntValue(keygr,
                      GWEN_DB_FLAGS_DEFAULT,
                      "number",
                      GWEN_DB_GetIntValue(n, "keyname/keynum", 0, 0));
  GWEN_DB_SetIntValue(keygr,
                      GWEN_DB_FLAGS_DEFAULT,
                      "version",
                      GWEN_DB_GetIntValue(n, "keyname/keyversion", 0, 0));
  GWEN_DB_SetIntValue(keygr,
                      GWEN_DB_FLAGS_DEFAULT,
                      "data/public", 1);

  binval=GWEN_DB_GetBinValue(n, "key/modulus", 0, 0, 0, &binsize);
  if (!binval) {
    DBG_ERROR(0, "No modulus in key");
    GWEN_DB_Group_free(keygr);
    GWEN_DB_Group_free(n);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_DATA);
  }
  GWEN_DB_SetBinValue(keygr,
                      GWEN_DB_FLAGS_DEFAULT,
                      "data/n", binval, binsize);

  binval=GWEN_DB_GetBinValue(n, "key/exponent", 0, 0, 0, &binsize);
  if (!binval) {
    DBG_ERROR(0, "No exponent in key");
    GWEN_DB_Group_free(keygr);
    GWEN_DB_Group_free(n);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_DATA);
  }
  GWEN_DB_SetBinValue(keygr,
                      GWEN_DB_FLAGS_DEFAULT,
                      "data/e", binval, binsize);

  key=GWEN_CryptKey_FromDb(keygr);
  if (!key) {
    DBG_INFO(0, "here");
    GWEN_DB_Group_free(keygr);
    GWEN_DB_Group_free(n);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_DB_Group_free(keygr);

  GWEN_DB_Group_free(n);
  *pkey=key;
  return 0;
}












