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


#ifndef GWENHYWFAR_IPCXMLDIALOG_P_H
#define GWENHYWFAR_IPCXMLDIALOG_P_H

#include <gwenhyfwar/ipcxmldialog.h>


struct GWEN_IPCXMLDIALOGDATA {
  GWEN_CRYPTKEY *localKey;
  unsigned int localSignSeq;
  GWEN_CRYPTKEY *remoteKey;
  unsigned int remoteSignSeq;
  GWEN_CRYPTKEY *sessionKey;
  unsigned int flags;

  char *serviceCode;
  char *securityId;
};

GWEN_IPCXMLDIALOGDATA *GWEN_IPCXMLDialogData_new();
void GWEN_IPCXMLDialogData_free(GWEN_IPCXMLDIALOGDATA *d);


int GWEN_IPCXMLDialog_PrepareCTX(GWEN_HBCIDIALOG *hdlg,
                                 GWEN_HBCICRYPTOCONTEXT *ctx,
                                 int crypt);
int GWEN_IPCXMLDialog_Sign(GWEN_HBCIDIALOG *hdlg,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_IPCXMLDialog_Verify(GWEN_HBCIDIALOG *hdlg,
                             GWEN_BUFFER *msgbuf,
                             GWEN_BUFFER *signbuf,
                             GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_IPCXMLDialog_Encrypt(GWEN_HBCIDIALOG *hdlg,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *cryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_IPCXMLDialog_Decrypt(GWEN_HBCIDIALOG *hdlg,
                              GWEN_BUFFER *msgbuf,
                              GWEN_BUFFER *decryptbuf,
                              GWEN_HBCICRYPTOCONTEXT *ctx);

void GWEN_IPCXMLDialog_FreeData(GWEN_HBCIDIALOG *hdlg);



#endif


