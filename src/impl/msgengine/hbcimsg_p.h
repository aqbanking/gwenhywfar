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

#ifndef GWENHYWFAR_HBCIMSG_P_H
#define GWENHYWFAR_HBCIMSG_P_H

#define GWEN_HBCIMSG_DEFAULTSIZE 256

#include <gwenhywfar/hbcimsg.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/msgengine.h>


struct GWEN_HBCIMSG {
  GWEN_HBCIMSG *next;
  GWEN_HBCIDIALOG *dialog;
  GWEN_BUFFER *buffer;
  GWEN_BUFFER *origbuffer;

  GWEN_KEYSPEC *crypter;
  GWEN_KEYSPEC *signers;
  unsigned int nSigners;

  unsigned int msgLayerId;

  unsigned int nodes;
  unsigned int dialogNumber;
  unsigned int msgNum;
  unsigned int refMsgNum;
  unsigned int firstSegment;
  unsigned int lastSegment;
  unsigned int flags;
};



int GWEN_HBCIMsg_PrepareCryptoSeg(GWEN_HBCIMSG *hmsg,
                                  GWEN_HBCICRYPTOCONTEXT *ctx,
                                  GWEN_DB_NODE *cfg,
                                  int crypt,
                                  int createCtrlRef);


int GWEN_HBCIMsg_SignMsg(GWEN_HBCIMSG *hmsg,
                         GWEN_BUFFER *rawBuf,
                         GWEN_KEYSPEC *ks);

int GWEN_HBCIMsg_EncryptMsg(GWEN_HBCIMSG *hmsg);

/**
 * This function verifies all signatures and adds th corrsponding keyspec
 * to the list of signers.
 * If a signature can not be verified (because there is no remote sign key
 * or our remote sign key differs from that used signing the data) then a
 * "?" is prepended to the owner of the keyspec added.
 * If the signature could be verified and turned out to be invalid a "!" is
 * prepended to the owner's name.
 * Thus you can later check for correct signatures by checking for the first
 * character of the keyspec's owner.
 */
int GWEN_HBCIMsg_Verify(GWEN_HBCIMSG *hmsg,
                        GWEN_DB_NODE *gr,
                        unsigned int flags);

int GWEN_HBCIMsg_AddMsgHead(GWEN_HBCIMSG *hmsg);
int GWEN_HBCIMsg_AddMsgTail(GWEN_HBCIMSG *hmsg);

/* return -1 on error (with group "seg/error" set) or -2 if the message is
 * faulty */
int GWEN_HBCIMsg_ReadSegment(GWEN_MSGENGINE *e,
                             const char *gtype,
                             GWEN_BUFFER *mbuf,
                             GWEN_DB_NODE *gr,
                             unsigned int flags);

int GWEN_HBCIMsg_ReadMessage(GWEN_MSGENGINE *e,
                             const char *gtype,
                             GWEN_BUFFER *mbuf,
                             GWEN_DB_NODE *gr,
                             unsigned int flags);

int GWEN_HBCIMsg_PrepareCryptoSegDec(GWEN_HBCIMSG *hmsg,
                                     GWEN_HBCICRYPTOCONTEXT *ctx,
                                     GWEN_DB_NODE *n,
                                     int crypt);

int GWEN_HBCIMsg_Decrypt(GWEN_HBCIMSG *hmsg, GWEN_DB_NODE *gr);

int GWEN_HBCIMsg_SequenceCheck(GWEN_DB_NODE *gr);




#endif



