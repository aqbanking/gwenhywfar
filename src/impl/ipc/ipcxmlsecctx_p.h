/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
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


#ifndef GWENHYWFAR_IPCXMLSECCTX_P_H
#define GWENHYWFAR_IPCXMLSECCTX_P_H

#include <gwenhywfar/ipcxmlsecctx.h>
#include <gwenhywfar/list.h>


struct GWEN_IPCXMLSECCTXDATA {
  GWEN_CRYPTKEY *sessionKey;
  unsigned int localSignSeq;
  unsigned int remoteSignSeq;

  GWEN_CRYPTKEY *localSignKey;
  GWEN_CRYPTKEY *localCryptKey;

  GWEN_CRYPTKEY *remoteSignKey;
  GWEN_CRYPTKEY *remoteCryptKey;

  char *serviceCode;
  char *securityId;
};

GWEN_IPCXMLSECCTXDATA *GWEN_IPCXMLSecCtxData_new();
void GWEN_IPCXMLSecCtxData_free(GWEN_IPCXMLSECCTXDATA *d);


GWEN_SECCTX_RETVAL GWEN_IPCXMLSecCtx_PrepareCTX(GWEN_SECCTX *sc,
                                                GWEN_HBCICRYPTOCONTEXT *ctx,
                                                int crypt);
GWEN_SECCTX_RETVAL GWEN_IPCXMLSecCtx_Sign(GWEN_SECCTX *sc,
                                          GWEN_BUFFER *msgbuf,
                                          GWEN_BUFFER *signbuf,
                                          GWEN_HBCICRYPTOCONTEXT *ctx);

GWEN_SECCTX_RETVAL GWEN_IPCXMLSecCtx_Verify(GWEN_SECCTX *sc,
                                            GWEN_BUFFER *msgbuf,
                                            GWEN_BUFFER *signbuf,
                                            GWEN_HBCICRYPTOCONTEXT *ctx);

GWEN_SECCTX_RETVAL GWEN_IPCXMLSecCtx_Encrypt(GWEN_SECCTX *sc,
                                             GWEN_BUFFER *msgbuf,
                                             GWEN_BUFFER *cryptbuf,
                                             GWEN_HBCICRYPTOCONTEXT *ctx);

GWEN_SECCTX_RETVAL GWEN_IPCXMLSecCtx_Decrypt(GWEN_SECCTX *sc,
                                             GWEN_BUFFER *msgbuf,
                                             GWEN_BUFFER *decryptbuf,
                                             GWEN_HBCICRYPTOCONTEXT *ctx);

void GWEN_IPCXMLSecCtx_FreeData(GWEN_SECCTX *sc);


GWEN_SECCTX_RETVAL GWEN_IPCXMLSecCtx_FromDB(GWEN_SECCTX *sc, GWEN_DB_NODE *db);
GWEN_SECCTX_RETVAL GWEN_IPCXMLSecCtx_ToDB(GWEN_SECCTX *sc,
                                          GWEN_DB_NODE *db);

const char *GWEN_IPCXMLSecCtx_GetSecurityId(GWEN_SECCTX *d);
void GWEN_IPCXMLSecCtx_SetSecurityId(GWEN_SECCTX *d,
                                     const char *s);

const GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetLocalSignKey(GWEN_SECCTX *sc);
const GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetLocalCryptKey(GWEN_SECCTX *sc);



typedef struct GWEN_IPCXMLSECCTXMGRDATA GWEN_IPCXMLSECCTXMGRDATA;

struct GWEN_IPCXMLSECCTXMGRDATA {
  GWEN_LIST *contextList; /* for temporary contexts */
  GWEN_CRYPTKEY *localSignKey;
  GWEN_CRYPTKEY *localCryptKey;
  unsigned int flags;
  char *dir;

};


GWEN_IPCXMLSECCTXMGRDATA *GWEN_IPCXMLSecCtxMgrData_new();
void GWEN_IPCXMLSecCtxMgrData_free(GWEN_SECCTX_MANAGER *scm);



int GWEN_IPCXMLSecCtxMgr_LockFile(const char *path);
int GWEN_IPCXMLSecCtxMgr_UnlockFile(int fid);

GWEN_SECCTX *GWEN_IPCXMLSecCtxtMgr_FindContext(GWEN_SECCTX_MANAGER *scm,
                                               const char *localName,
                                               const char *remoteName);


GWEN_SECCTX *GWEN_IPCXMLSecCtxMgr_GetContext(GWEN_SECCTX_MANAGER *scm,
                                             const char *localName,
                                             const char *remoteName);
int GWEN_IPCXMLSecCtxMgr_AddContext(GWEN_SECCTX_MANAGER *scm,
                                    GWEN_SECCTX *sc);
int GWEN_IPCXMLSecCtxMgr_DelContext(GWEN_SECCTX_MANAGER *scm,
                                    GWEN_SECCTX *sc);

int GWEN_IPCXMLSecCtxMgr_ReleaseContext(GWEN_SECCTX_MANAGER *scm,
                                        GWEN_SECCTX *sc,
                                        int aban);





#endif


