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

#include <gwenhyfwar/hbcimsg.h>
#include <gwenhyfwar/error.h>
#include <gwenhyfwar/buffer.h>
#include <gwenhyfwar/db.h>
#include <gwenhyfwar/msgengine.h>


struct GWEN_HBCIMSG {
  GWEN_HBCIMSG *next;
  GWEN_HBCIDIALOG *dialog;
  GWEN_BUFFER *buffer;

  GWEN_KEYSPEC *crypter;
  GWEN_KEYSPEC *signers;
  unsigned int nSigners;

  unsigned int nodes;
  unsigned int msgNum;
  unsigned int refMsgNum;
  unsigned int firstSegment;
  unsigned int lastSegment;
  unsigned int flags;
};



int GWEN_HBCIMsg_PrepareCryptoSeg(GWEN_HBCIMSG *hmsg,
                                  GWEN_HBCICRYPTOCONTEXT *ctx,
                                  GWEN_DB_NODE *cfg,
                                  int createCtrlRef);


int GWEN_HBCIMsg_SignMsg(GWEN_HBCIMSG *hmsg,
                         GWEN_BUFFER *rawBuf,
                         GWEN_KEYSPEC *ks);

int GWEN_HBCIMsg_EncryptMsg(GWEN_HBCIMSG *hmsg);

int GWEN_HBCIMsg_AddMsgHead(GWEN_HBCIMSG *hmsg);
int GWEN_HBCIMsg_AddMsgTail(GWEN_HBCIMSG *hmsg);





#endif



