/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Sep 16 2003
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


#ifndef GWENHYFWAR_CONNLAYER_H
#define GWENHYFWAR_CONNLAYER_H

#include <gwenhyfwar/msglayer.h>
#include <gwenhyfwar/error.h>

typedef enum {
  GWEN_IPCConnectionLayerStateClosed=0,
  GWEN_IPCConnectionLayerStateOpening,
  GWEN_IPCConnectionLayerStateOpen,
  GWEN_IPCConnectionLayerStateClosing,
  GWEN_IPCConnectionLayerStateListening
} GWEN_IPCCONNLAYER_STATE;


typedef struct GWEN_IPCCONNLAYER GWEN_IPCCONNLAYER;

typedef void (*GWEN_IPCCONNLAYER_FREE)(GWEN_IPCCONNLAYER *cl);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_WORK)(GWEN_IPCCONNLAYER *cl);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_ACCEPT)(GWEN_IPCCONNLAYER *cl,
                                                   GWEN_IPCCONNLAYER **c);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_OPEN)(GWEN_IPCCONNLAYER *cl);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_CLOSE)(GWEN_IPCCONNLAYER *cl,
                                                  int force);



GWEN_IPCMSG *GWEN_ConnectionLayer_GetIncomingMsg(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayer_AddOutgoingMsg(GWEN_IPCCONNLAYER *cl,
                                                   GWEN_IPCMSG *msg);
GWEN_ERRORCODE GWEN_ConnectionLayer_Work(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayer_Open(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayer_Close(GWEN_IPCCONNLAYER *cl,
                                          int force);
GWEN_IPCCONNLAYER_STATE GWEN_ConnectionLayer_GetState(GWEN_IPCCONNLAYER *cl);
unsigned int GWEN_ConnLayer_GetId(GWEN_IPCCONNLAYER *cl);
const char *GWEN_ConnectionLayer_GetInfo(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetInfo(GWEN_IPCCONNLAYER *cl, const char *s);



GWEN_IPCCONNLAYER *GWEN_ConnectionLayer_new(GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCCONNLAYER_STATE st);
void GWEN_ConnectionLayer_free(GWEN_IPCCONNLAYER *cl);
void *GWEN_ConnectionLayer_GetData(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetData(GWEN_IPCCONNLAYER *cl, void *d);

GWEN_SOCKET *GWEN_IPCConnectionLayer_GetReadSocket(GWEN_IPCCONNLAYER *cl);
GWEN_SOCKET *GWEN_IPCConnectionLayer_GetWriteSocket(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayer_Accept(GWEN_IPCCONNLAYER *cl,
                                           GWEN_IPCCONNLAYER **c);

void GWEN_ConnectionLayer_SetFreeDataFn(GWEN_IPCCONNLAYER *cl,
                                        GWEN_IPCCONNLAYER_FREE f);
void GWEN_ConnectionLayer_SetWorkFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_WORK f);
void GWEN_ConnectionLayer_SetAcceptFn(GWEN_IPCCONNLAYER *cl,
                                      GWEN_IPCCONNLAYER_ACCEPT f);
void GWEN_ConnectionLayer_SetOpenFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_OPEN f);
void GWEN_ConnectionLayer_SetCloseFn(GWEN_IPCCONNLAYER *cl,
                                     GWEN_IPCCONNLAYER_CLOSE f);
GWEN_IPCMSGLAYER *GWEN_ConnectionLayer_GetMsgLayer(GWEN_IPCCONNLAYER *cl);







#endif /* GWENHYFWAR_CONNLAYER_H */

