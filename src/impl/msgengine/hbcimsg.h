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

#ifndef GWENHYWFAR_HBCIMSG_H
#define GWENHYWFAR_HBCIMSG_H

#define GWEN_HBCIMSG_FLAGS_SIGN  0x0001
#define GWEN_HBCIMSG_FLAGS_CRYPT 0x0002



#include <gwenhyfwar/error.h>
#include <gwenhyfwar/buffer.h>
#include <gwenhyfwar/db.h>
#include <gwenhyfwar/msgengine.h>
#include <gwenhyfwar/keyspec.h>
#include <gwenhyfwar/hbcicryptocontext.h>
#include <gwenhyfwar/hbcidialog.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_HBCIMSG GWEN_HBCIMSG;


GWEN_KEYSPEC *GWEN_HBCIMsg_GetSigners(GWEN_HBCIMSG *hmsg);
int GWEN_HBCIMsg_AddSigner(GWEN_HBCIMSG *hmsg,
                           const GWEN_KEYSPEC *ks);
unsigned int GWEN_HBCIMsg_GetSignerCount(GWEN_HBCIMSG *hmsg);

GWEN_KEYSPEC *GWEN_HBCIMsg_GetCrypter(GWEN_HBCIMSG *hmsg);
void GWEN_HBCIMsg_SetCrypter(GWEN_HBCIMSG *hmsg,
                             const GWEN_KEYSPEC *ks);


GWEN_BUFFER *GWEN_HBCIMsg_GetBuffer(GWEN_HBCIMSG *hmsg);

/**
 * Takes the buffer away from the HBCIMsg.
 * The caller is responsible for freeing the returned buffer !
 */
GWEN_BUFFER *GWEN_HBCIMsg_TakeBuffer(GWEN_HBCIMSG *hmsg);

/**
 * Takes over ownership of the given buffer.
 */
void GWEN_HBCIMsg_SetBuffer(GWEN_HBCIMSG *hmsg,
                            GWEN_BUFFER *bf);

unsigned int GWEN_HBCIMsg_GetFlags(GWEN_HBCIMSG *hmsg);
void GWEN_HBCIMsg_SetFlags(GWEN_HBCIMSG *hmsg,
                           unsigned int f);

unsigned int GWEN_HBCIMsg_GetMsgRef(GWEN_HBCIMSG *hmsg);
void GWEN_HBCIMsg_SetMsgRef(GWEN_HBCIMSG *hmsg,
                            unsigned int i);

unsigned int GWEN_HBCIMsg_GetMsgNumber(GWEN_HBCIMSG *hmsg);
void GWEN_HBCIMsg_SetMsgNumber(GWEN_HBCIMSG *hmsg,
                               unsigned int i);

unsigned int GWEN_HBCIMsg_GetMsgLayerId(GWEN_HBCIMSG *hmsg);
void GWEN_HBCIMsg_SetMsgLayerId(GWEN_HBCIMSG *hmsg,
                                unsigned int i);

unsigned int GWEN_HBCIMsg_GetNodes(GWEN_HBCIMSG *hmsg);



GWEN_HBCIMSG *GWEN_HBCIMsg_new(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIMsg_free(GWEN_HBCIMSG *hmsg);

GWEN_HBCIMSG *GWEN_HBCIMsg_Next(GWEN_HBCIMSG *hmsg);
void GWEN_HBCIMsg_Add(GWEN_HBCIMSG *hmsg, GWEN_HBCIMSG **head);
void GWEN_HBCIMsg_Del(GWEN_HBCIMSG *hmsg, GWEN_HBCIMSG **head);

int GWEN_HBCIMsg_AddNode(GWEN_HBCIMSG *hmsg,
                         GWEN_XMLNODE *node,
                         GWEN_DB_NODE *data);

unsigned int GWEN_HBCIMsg_GetCurrentSegmentNumber(GWEN_HBCIMSG *hmsg);

int GWEN_HBCIMsg_EncodeMsg(GWEN_HBCIMSG *hmsg);
int GWEN_HBCIMsg_DecodeMsg(GWEN_HBCIMSG *hmsg,
                           GWEN_DB_NODE *gr,
                           unsigned int flags);



#ifdef __cplusplus
}
#endif



#endif



