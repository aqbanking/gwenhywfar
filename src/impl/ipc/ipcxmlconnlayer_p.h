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

#include <gwenhywfar/ipcxmlconnlayer.h>

#include <gwenhywfar/connlayer.h>
#include <gwenhywfar/transportlayer.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/crypt.h>


#define GWEN_IPCXMLCONNLAYER_FLAGS_MUST_SIGN  0x0001
#define GWEN_IPCXMLCONNLAYER_FLAGS_MUST_CRYPT 0x0002

#define GWEN_IPCXMLCONNLAYER_MSGSIZE 1024
#define GWEN_IPCXMLCONNLAYER_TYPE 0xbeef

#include <gwenhywfar/hbcimsg.h>
#include <gwenhywfar/hbcidialog.h>
#include <gwenhywfar/db.h>


struct GWEN_IPCXMLCONNLAYERDATA {
  GWEN_MSGENGINE *msgEngine; /* not owned ! */
  unsigned int flags;

  unsigned int msgFlags;

  GWEN_HBCIDIALOG *dialog;
  GWEN_SECCTX_MANAGER *securityManager;
  unsigned int dialogId;

  unsigned int connected;
  GWEN_HBCIMSG *currentMsg;
};


void GWEN_IPCXMLConnLayer_free(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Work(GWEN_IPCCONNLAYER *cl, int rd);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Accept(GWEN_IPCCONNLAYER *cl,
                                           GWEN_IPCMSGLAYER *ml,
                                           GWEN_IPCCONNLAYER **c);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Open(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Close(GWEN_IPCCONNLAYER *cl,
                                          int force);
void GWEN_IPCXMLConnLayer_Down(GWEN_IPCCONNLAYER *cl);

GWEN_IPCMSG *GWEN_IPCXMLConnLayer_HBCI2IPC(GWEN_IPCCONNLAYER *cl,
                                           GWEN_HBCIMSG *hmsg);

void GWEN_IPCXMLConnLayer_Down(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Up(GWEN_IPCCONNLAYER *cl);

GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_MsgFactory(GWEN_IPCCONNLAYER *cl);


#endif /* GWEN_IPCXMLCONNLAYER_P_H */



