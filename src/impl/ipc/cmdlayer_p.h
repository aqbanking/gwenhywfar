/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 19 2003
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


#ifndef GWENHYFWAR_CMDLAYER_P_H
#define GWENHYFWAR_CMDLAYER_P_H

#include <gwenhyfwar/cmdlayer.h>
#include <gwenhyfwar/transportlayer.h>
#include <gwenhyfwar/msgengine.h>


#define GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSED            0
#define GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING        1
#define GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED         2
#define GWEN_IPCCONNLAYERCMD_SECSTATE_GREETING          3
#define GWEN_IPCCONNLAYERCMD_SECSTATE_PUBLIC_KEY_EXCHG  4
#define GWEN_IPCCONNLAYERCMD_SECSTATE_SESSION_KEY_EXCHG 5
#define GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED       6
#define GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSING           7


#define GWEN_IPCCONNLAYERCMD_FLAGS_MUST_SIGN  0x0001
#define GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT 0x0002


typedef struct GWEN_IPCMSGLAYERCMDDATA GWEN_IPCMSGLAYERCMDDATA;

/**
 * This is the module specific data for the msgLayer.
 */
struct GWEN_IPCMSGLAYERCMDDATA {
  GWEN_MSGENGINE *msgEngine; /* not owned ! */
  GWEN_IPCMSG *currentMsg;
  int readingSize; /* if !=0 then we are still reading the size */
  unsigned int bytesToRead; /* bytes still to read */

};
GWEN_IPCMSGLAYERCMDDATA *GWEN_MsgLayerCmdData_new(GWEN_MSGENGINE *msgEngine);
void GWEN_MsgLayerCmdData_free(GWEN_IPCMSGLAYERCMDDATA *mcd);



typedef struct GWEN_IPCCONNLAYERCMDDATA GWEN_IPCCONNLAYERCMDDATA;

/**
 * This is the module specific data for the connectionLayer.
 */
struct GWEN_IPCCONNLAYERCMDDATA {
  GWEN_MSGENGINE *msgEngine; /* not owned ! */
  unsigned int securityState;
  unsigned int flags;
};
GWEN_IPCCONNLAYERCMDDATA *
GWEN_ConnectionLayerCmdData_new(GWEN_MSGENGINE *msgEngine);
void GWEN_ConnectionLayerCmdData_free(GWEN_IPCCONNLAYERCMDDATA *ccd);



GWEN_IPCMSGLAYER *GWEN_MsgLayerCmd_new(GWEN_MSGENGINE *msgEngine,
                                       GWEN_IPCTRANSPORTLAYER *tl,
                                       GWEN_IPCMSGLAYER_STATE st);
void GWEN_MsgLayerCmd_free(GWEN_IPCMSGLAYER *ml);
GWEN_ERRORCODE GWEN_MsgLayerCmd_Work(GWEN_IPCMSGLAYER *ml, int rd);
GWEN_ERRORCODE GWEN_MsgLayerCmd_Accept(GWEN_IPCMSGLAYER *ml,
                                       GWEN_IPCTRANSPORTLAYER *tl,
                                       GWEN_IPCMSGLAYER **m);



GWEN_IPCCONNLAYER *GWEN_ConnectionLayerCmd_new(GWEN_MSGENGINE *msgEngine,
                                               GWEN_IPCMSGLAYER *ml,
                                               GWEN_IPCMSGLAYER_STATE st);
void GWEN_ConnectionLayerCmd_free(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Work(GWEN_IPCCONNLAYER *cl, int rd);
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Accept(GWEN_IPCCONNLAYER *cl,
                                              GWEN_IPCMSGLAYER *ml,
                                              GWEN_IPCCONNLAYER **c);
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Open(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Close(GWEN_IPCCONNLAYER *cl,
                                             int force);



struct GWEN_IPCSERVICECMD {
  GWEN_MSGENGINE *msgEngine;
  GWEN_SERVICELAYER *serviceLayer;
};



#endif /* GWENHYFWAR_CMDLAYER_P_H */



