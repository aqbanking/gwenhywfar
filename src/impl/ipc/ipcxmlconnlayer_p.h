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


#ifndef GWEN_IPCXMLCONNLAYER_P_H
#define GWEN_IPCXMLCONNLAYER_P_H

#include "ipcxmlconnlayer.h"

#include <gwenhyfwar/connlayer.h>
#include <gwenhyfwar/transportlayer.h>
#include <gwenhyfwar/msgengine.h>
#include <gwenhyfwar/crypt.h>


#define GWEN_IPCXMLCONNLAYER_FLAGS_MUST_SIGN  0x0001
#define GWEN_IPCXMLCONNLAYER_FLAGS_MUST_CRYPT 0x0002

#define GWEN_IPCXMLCONNLAYER_SECSTATE_HAVE_ID    0x0001
#define GWEN_IPCXMLCONNLAYER_SECSTATE_HAVE_PK    0x0002
#define GWEN_IPCXMLCONNLAYER_SECSTATE_HAVE_SK    0x0004

#define GWEN_IPCXMLCONNLAYER_MSGSIZE 1024
#define GWEN_IPCXMLCONNLAYER_TYPE 0xbeef

#include <gwenhyfwar/hbcimsg.h>


struct GWEN_IPCXMLCONNLAYERDATA {
  GWEN_MSGENGINE *msgEngine; /* not owned ! */
  unsigned int securityState;
  unsigned int flags;
  GWEN_CRYPTKEY *localKey;
  GWEN_CRYPTKEY *remoteKey;
  GWEN_CRYPTKEY *sessionKey;
  char *peerName;
  char *peerVersion;
  char *ownName;
  char *ownVersion;
  unsigned int msgNumber;
  GWEN_HBCIMSG *incomingMsgs;
  GWEN_HBCIMSG *outgoingMsgs;
  GWEN_IPCXMLCONNLAYER_ENCODE_FN encodeFn;
  GWEN_IPCXMLCONNLAYER_DECODE_FN decodeFn;
};


void GWEN_IPCXMLConnLayer_free(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Work(GWEN_IPCCONNLAYER *cl, int rd);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Accept(GWEN_IPCCONNLAYER *cl,
                                           GWEN_IPCMSGLAYER *ml,
                                           GWEN_IPCCONNLAYER **c);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Open(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Close(GWEN_IPCCONNLAYER *cl,
                                          int force);

void GWEN_IPCXMLConnLayer_AddIncomingMsg(GWEN_IPCCONNLAYER *cl,
                                         GWEN_HBCIMSG *m);
void GWEN_IPCXMLConnLayer_AddOutgoingMsg(GWEN_IPCCONNLAYER *cl,
                                         GWEN_HBCIMSG *m);


GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_NextIncomingMsg(GWEN_IPCCONNLAYER *cl);
GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_FindIncomingMsg(GWEN_IPCCONNLAYER *cl,
                                                      unsigned int refid);
void GWEN_IPCXMLConnLayer_UnlinkIncomingMsg(GWEN_IPCCONNLAYER *cl,
                                            GWEN_HBCIMSG *m);


#endif /* GWEN_IPCXMLCONNLAYER_P_H */



