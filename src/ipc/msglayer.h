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


#ifndef GWENHYFWAR_MSGLAYER_H
#define GWENHYFWAR_MSGLAYER_H


#include <gwenhywfar/transportlayer.h>
#include <gwenhywfar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GWEN_IPCMSG GWEN_IPCMSG;
typedef struct GWEN_IPCMSGLAYER GWEN_IPCMSGLAYER;

typedef void (*GWEN_IPCMSG_FREE)(GWEN_IPCMSG *m);


typedef enum {
  GWEN_IPCMsglayerStateUnconnected=0,
  GWEN_IPCMsglayerStateIdle,
  GWEN_IPCMsglayerStateReading,
  GWEN_IPCMsglayerStateWriting,
  GWEN_IPCMsglayerStateConnecting,
  GWEN_IPCMsglayerStateListening,
  GWEN_IPCMsglayerStateWaiting,
  GWEN_IPCMsglayerStateClosed
} GWEN_IPCMSGLAYER_STATE;


GWEN_IPCMSG *GWEN_Msg_new();
void GWEN_Msg_free(GWEN_IPCMSG *m);
GWEN_IPCMSG *GWEN_Msg_GetNext(GWEN_IPCMSG *m);

GWEN_BUFFER *GWEN_Msg_GetBuffer(GWEN_IPCMSG *m);
/**
 * Takes the buffer from the message.
 * This makes the caller responsible for freeing the returned buffer !
 */
GWEN_BUFFER *GWEN_Msg_TakeBuffer(GWEN_IPCMSG *m);

void GWEN_Msg_SetBuffer(GWEN_IPCMSG *m, GWEN_BUFFER *buf);

unsigned int GWEN_Msg_GetMsgLayerId(GWEN_IPCMSG *m);
void GWEN_Msg_SetMsgLayerId(GWEN_IPCMSG *m,
                            unsigned int i);



typedef void (*GWEN_IPCMSGLAYER_FREE)(GWEN_IPCMSGLAYER *ml);
typedef GWEN_ERRORCODE (*GWEN_IPCMSGLAYER_WORK)(GWEN_IPCMSGLAYER *ml,
                                                int rd);
typedef GWEN_ERRORCODE (*GWEN_IPCMSGLAYER_ACCEPT)(GWEN_IPCMSGLAYER *ml,
                                                  GWEN_IPCTRANSPORTLAYER *tl,
                                                  GWEN_IPCMSGLAYER **m);


GWEN_ERRORCODE GWEN_MsgLayer_Work(GWEN_IPCMSGLAYER *ml,
                                  int rd);

GWEN_IPCMSG *GWEN_MsgLayer_GetIncomingMsg(GWEN_IPCMSGLAYER *ml);
GWEN_ERRORCODE GWEN_MsgLayer_AddOutgoingMsg(GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCMSG *msg);
int GWEN_MsgLayer_CheckAddOutgoingMsg(GWEN_IPCMSGLAYER *ml);
int GWEN_MsgLayer_CheckAddIncomingMsg(GWEN_IPCMSGLAYER *ml);

unsigned int GWEN_MsgLayer_GetId(GWEN_IPCMSGLAYER *ml);

GWEN_IPCMSGLAYER_STATE GWEN_MsgLayer_GetState(GWEN_IPCMSGLAYER *ml);

GWEN_ERRORCODE GWEN_MsgLayer_Connect(GWEN_IPCMSGLAYER *ml);
GWEN_ERRORCODE GWEN_MsgLayer_Disconnect(GWEN_IPCMSGLAYER *ml);
GWEN_ERRORCODE GWEN_MsgLayer_Accept(GWEN_IPCMSGLAYER *ml,
                                    GWEN_IPCMSGLAYER **m);



/**
 * @name Internal functions not to be used by applications
 */
/*@{*/
/**
 * This function takes over ownership of the transportlayer !
 */
GWEN_IPCMSGLAYER *GWEN_MsgLayer_new(GWEN_IPCTRANSPORTLAYER *tl,
                                    GWEN_IPCMSGLAYER_STATE st);
void GWEN_MsgLayer_free(GWEN_IPCMSGLAYER *ml);

GWEN_IPCMSG *GWEN_MsgLayer_GetOutgoingMsg(GWEN_IPCMSGLAYER *ml);
unsigned int GWEN_MsgLayer_OutgoingMsgs(GWEN_IPCMSGLAYER *ml);
GWEN_ERRORCODE GWEN_MsgLayer_AddIncomingMsg(GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCMSG *msg);
void GWEN_MsgLayer_SetState(GWEN_IPCMSGLAYER *ml,
                            GWEN_IPCMSGLAYER_STATE st);
GWEN_IPCTRANSPORTLAYER *GWEN_MsgLayer_GetTransportLayer(GWEN_IPCMSGLAYER *ml);
GWEN_SOCKET *GWEN_IPCMsgLayer_GetReadSocket(GWEN_IPCMSGLAYER *ml);
GWEN_SOCKET *GWEN_IPCMsgLayer_GetWriteSocket(GWEN_IPCMSGLAYER *ml);
void *GWEN_MsgLayer_GetData(GWEN_IPCMSGLAYER *ml);
void GWEN_MsgLayer_SetData(GWEN_IPCMSGLAYER *ml, void *d);


void GWEN_MsgLayer_SetFreeDataFn(GWEN_IPCMSGLAYER *ml,
                                 GWEN_IPCMSGLAYER_FREE f);
void GWEN_MsgLayer_SetWorkFn(GWEN_IPCMSGLAYER *ml, GWEN_IPCMSGLAYER_WORK f);
void GWEN_MsgLayer_SetAcceptFn(GWEN_IPCMSGLAYER *ml,
                               GWEN_IPCMSGLAYER_ACCEPT f);
const char *GWEN_MsgLayer_GetStateString(GWEN_IPCMSGLAYER_STATE st);

/*@}*/


#ifdef __cplusplus
}
#endif

#endif /* GWENHYFWAR_MSGLAYER_H */
