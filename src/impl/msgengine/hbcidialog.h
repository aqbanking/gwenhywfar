/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 08 2003
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

#ifndef GWENHYWFAR_HBCIDLG_H
#define GWENHYWFAR_HBCIDLG_H

#include <gwenhyfwar/hbcicryptocontext.h>
#include <gwenhyfwar/buffer.h>
#include <gwenhyfwar/msgengine.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_HBCIDIALOG GWEN_HBCIDIALOG;


typedef int
  (*GWEN_HBCIDLG_PREPARECTX_FN)(GWEN_HBCIDIALOG *hdlg,
                                GWEN_HBCICRYPTOCONTEXT *ctx,
                                int crypt);


typedef int
  (*GWEN_HBCIDLG_SIGN_FN)(GWEN_HBCIDIALOG *hdlg,
                          GWEN_BUFFER *msgbuf,
                          GWEN_BUFFER *signbuf,
                          GWEN_HBCICRYPTOCONTEXT *ctx);

typedef int
  (*GWEN_HBCIDLG_VERIFY_FN)(GWEN_HBCIDIALOG *hdlg,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *signbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx);

typedef int
  (*GWEN_HBCIDLG_ENCRYPT_FN)(GWEN_HBCIDIALOG *hdlg,
                             GWEN_BUFFER *msgbuf,
                             GWEN_BUFFER *cryptbuf,
                             GWEN_HBCICRYPTOCONTEXT *ctx);

typedef int
  (*GWEN_HBCIDLG_DECRYPT_FN)(GWEN_HBCIDIALOG *hdlg,
                             GWEN_BUFFER *msgbuf,
                             GWEN_BUFFER *decryptbuf,
                             GWEN_HBCICRYPTOCONTEXT *ctx);

typedef void
  (*GWEN_HBCIDLG_FREEDATA_FN)(GWEN_HBCIDIALOG *hdlg);



void GWEN_HBCIDialog_SetPrepareCtxFn(GWEN_HBCIDIALOG *hdlg,
                                  GWEN_HBCIDLG_PREPARECTX_FN fn);
void GWEN_HBCIDialog_SetSignFn(GWEN_HBCIDIALOG *hdlg,
                            GWEN_HBCIDLG_SIGN_FN signFn);
void GWEN_HBCIDialog_SetVerifyFn(GWEN_HBCIDIALOG *hdlg,
                              GWEN_HBCIDLG_VERIFY_FN verifyFn);
void GWEN_HBCIDialog_SetEncryptFn(GWEN_HBCIDIALOG *hdlg,
                               GWEN_HBCIDLG_ENCRYPT_FN encryptFn);
void GWEN_HBCIDialog_SetDecrpytFn(GWEN_HBCIDIALOG *hdlg,
                               GWEN_HBCIDLG_DECRYPT_FN decryptFn);
void GWEN_HBCIDialog_SetFreeDataFn(GWEN_HBCIDIALOG *hdlg,
                                GWEN_HBCIDLG_FREEDATA_FN fn);
void GWEN_HBCIDialog_SetInheritorData(GWEN_HBCIDIALOG *hdlg,
                                   void *data);

GWEN_MSGENGINE *GWEN_HBCIDialog_GetMsgEngine(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetMsgEngine(GWEN_HBCIDIALOG *hdlg,
                                  GWEN_MSGENGINE *e);

const char *GWEN_HBCIDialog_GetDialogId(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetDialogId(GWEN_HBCIDIALOG *hdlg,
                                 const char *s);

unsigned int GWEN_HBCIDialog_GetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg,
                                           unsigned int i);

unsigned int GWEN_HBCIDialog_GetNextMsgNum(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetNextMsgNum(GWEN_HBCIDIALOG *hdlg,
                                   unsigned int i);




int GWEN_HBCIDialog_PrepareContext(GWEN_HBCIDIALOG *hdlg,
                                   GWEN_HBCICRYPTOCONTEXT *ctx,
                                   int crypt);


int GWEN_HBCIDialog_Sign(GWEN_HBCIDIALOG *hdlg,
                         GWEN_BUFFER *msgbuf,
                         GWEN_BUFFER *signbuf,
                         GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_HBCIDialog_Verify(GWEN_HBCIDIALOG *hdlg,
                           GWEN_BUFFER *msgbuf,
                           GWEN_BUFFER *signbuf,
                           GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_HBCIDialog_Encrypt(GWEN_HBCIDIALOG *hdlg,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *cryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx);

int GWEN_HBCIDialog_Decrypt(GWEN_HBCIDIALOG *hdlg,
                            GWEN_BUFFER *msgbuf,
                            GWEN_BUFFER *decryptbuf,
                            GWEN_HBCICRYPTOCONTEXT *ctx);

GWEN_HBCIDIALOG *GWEN_HBCIDialog_new(GWEN_MSGENGINE *e);
void GWEN_HBCIDialog_free(GWEN_HBCIDIALOG *hdlg);


/**
 * The original code (in C++) has been written by Fabian Kaiser for OpenHBCI
 * (file rsakey.cpp). Moved to C by me (Martin Preuss)
 */
int GWEN_HBCIDialog_PaddWithISO9796(GWEN_BUFFER *src);


#ifdef __cplusplus
}
#endif


#endif



