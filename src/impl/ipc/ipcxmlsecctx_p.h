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

#include <gwenhyfwar/ipcxmlsecctx.h>


struct GWEN_IPCXMLSECCTXDATA {
  GWEN_KEYMANAGER *keyManager;
  GWEN_CRYPTKEY *sessionKey;

  char *serviceCode;
  char *securityId;
};

GWEN_IPCXMLSECCTXDATA *GWEN_IPCXMLSecCtxData_new();
void GWEN_IPCXMLSecCtxData_free(GWEN_IPCXMLSECCTXDATA *d);


int GWEN_IPCXMLSecCtx_PrepareCTX(GWEN_SECCTX *sc,
                                 GWEN_HBCICRYPTOCONTEXT *ctx,
                                 int crypt);
int GWEN_IPCXMLSecCtx_Sign(GWEN_SECCTX *sc,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_IPCXMLSecCtx_Verify(GWEN_SECCTX *sc,
                             GWEN_BUFFER *msgbuf,
                             GWEN_BUFFER *signbuf,
                             GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_IPCXMLSecCtx_Encrypt(GWEN_SECCTX *sc,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *cryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_IPCXMLSecCtx_Decrypt(GWEN_SECCTX *sc,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *decryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx);

void GWEN_IPCXMLSecCtx_FreeData(GWEN_SECCTX *sc);


#endif


