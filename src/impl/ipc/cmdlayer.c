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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cmdlayer_p.h"
#include <gwenhyfwar/msglayer.h>
#include <gwenhyfwar/servicelayer.h>
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>

#include <gwenhyfwar/text.h>

#include <stdarg.h>




/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYERCMDDATA *GWEN_MsgLayerCmdData_new(GWEN_MSGENGINE *msgEngine){
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  assert(msgEngine);
  GWEN_NEW_OBJECT(GWEN_IPCMSGLAYERCMDDATA, mcd);
  mcd->msgEngine=msgEngine;
  return mcd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayerCmdData_free(GWEN_IPCMSGLAYERCMDDATA *mcd){
  if (mcd) {
    free(mcd);
  }
}






/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYERCMDDATA *
GWEN_ConnectionLayerCmdData_new(GWEN_MSGENGINE *msgEngine){
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(msgEngine);
  GWEN_NEW_OBJECT(GWEN_IPCCONNLAYERCMDDATA, ccd);
  ccd->msgEngine=msgEngine;
  return ccd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayerCmdData_free(GWEN_IPCCONNLAYERCMDDATA *ccd){
  if (ccd) {
    free(ccd);
  }
}




/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYER *GWEN_MsgLayerCmd_new(GWEN_MSGENGINE *msgEngine,
                                       GWEN_IPCTRANSPORTLAYER *tl,
                                       GWEN_IPCMSGLAYER_STATE st){
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  ml=GWEN_MsgLayer_new(tl, st);
  mcd=GWEN_MsgLayerCmdData_new(msgEngine);
  GWEN_MsgLayer_SetData(ml, mcd);
  GWEN_MsgLayer_SetFreeDataFn(ml, GWEN_MsgLayerCmd_free);
  GWEN_MsgLayer_SetWorkFn(ml, GWEN_MsgLayerCmd_Work);
  GWEN_MsgLayer_SetAcceptFn(ml, GWEN_MsgLayerCmd_Accept);

  return ml;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayerCmd_free(GWEN_IPCMSGLAYER *ml){
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  assert(ml);
  mcd=(GWEN_IPCMSGLAYERCMDDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);
  GWEN_MsgLayerCmdData_free(mcd);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayerCmd_Work(GWEN_IPCMSGLAYER *ml,
                                     int rd){
  GWEN_IPCMSGLAYERCMDDATA *mcd;
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;
  int hasRead;
  int hasWritten;
  int canRead;

  hasRead=0;
  hasWritten=0;
  assert(ml);
  DBG_INFO(0, "Working on msgLayer %d", GWEN_MsgLayer_GetId(ml));
  mcd=(GWEN_IPCMSGLAYERCMDDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);

  tl=GWEN_MsgLayer_GetTransportLayer(ml);
  assert(tl);

  if (rd) {
    for (;;) {
      /* this loop runs as long as there still is data to read */
      if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateIdle) {
        /* read mode */
        mcd->readingSize=1;
        mcd->bytesToRead=30;
        GWEN_Msg_free(mcd->currentMsg);
        mcd->currentMsg=GWEN_Msg_new();
        GWEN_Msg_SetBuffer(mcd->currentMsg,
                           GWEN_Buffer_new(0,
                                           GWEN_CMDLAYER_MAXMSGSIZE, 0, 1));
        /* go into reading mode */
        GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateReading);
      }

      if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateReading) {
        GWEN_BUFFER *buffer;

        /* in read mode */
        if (mcd->readingSize) {
          DBG_INFO(0, "Reading header (%d bytes)", mcd->bytesToRead);
          /* still reading the header to determine the message size */
          if (mcd->bytesToRead) {
            unsigned int bread;

            assert(mcd->currentMsg);
            buffer=GWEN_Msg_GetBuffer(mcd->currentMsg);

            if (!hasRead) {
              DBG_INFO(0, "Nothing read so far");
              canRead=1;
            }
            else {
              canRead=GWEN_Error_IsOk(GWEN_IPCTransportLayer_CanRead(tl));
              if (canRead) {
                DBG_INFO(0, "Still data in the queue");
              }
            }
            if (!canRead)
              break;

            /* read the rest of the header */
            bread=mcd->bytesToRead;
            err=GWEN_IPCTransportLayer_Read(tl,
                                            GWEN_Buffer_GetPosPointer(buffer),
                                            &bread);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "called from here");
              DBG_INFO(0, "Severe error, closing connection");
              GWEN_MsgLayer_Disconnect(ml);
              return err;
            }
            hasRead=1;
            GWEN_Buffer_IncrementPos(buffer, bread);
            GWEN_Buffer_SetUsedBytes(buffer,
                                     GWEN_Buffer_GetUsedBytes(buffer)+bread);
            if (bread==mcd->bytesToRead) {
              /* ok, we have the header, now get the size */
              GWEN_XMLNODE *sn;
              GWEN_DB_NODE *tmpda;
              unsigned int currPos;
              int msgsize;

              DBG_INFO(0, "Header complete");
              mcd->bytesToRead=0;
              sn=GWEN_MsgEngine_FindNodeByProperty(mcd->msgEngine,
                                                   "SEG",
                                                   "id",
                                                   0,
                                                   "MsgHeadShort");
              if (!sn) {
                DBG_ERROR(0, "Segment not found");
                DBG_INFO(0, "Severe error, closing connection");
                GWEN_MsgLayer_Disconnect(ml);
                return GWEN_Error_new(0,
                                      GWEN_ERROR_SEVERITY_ERR,
                                      GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                                      GWEN_IPC_ERROR_SEG_NOT_FOUND);
              }
              tmpda=GWEN_DB_Group_new("tmpda");
              currPos=GWEN_Buffer_GetPos(buffer);
              GWEN_Buffer_SetPos(buffer, 0);
              GWEN_Text_DumpString(GWEN_Buffer_GetStart(buffer),
                                   GWEN_Buffer_GetUsedBytes(buffer));

              if (GWEN_MsgEngine_ParseMessage(mcd->msgEngine,
                                              sn,
                                              buffer,
                                              tmpda)) {
                DBG_ERROR(0, "Bad message (pos=%d)",
                          GWEN_Buffer_GetPos(buffer));
                GWEN_DB_Group_free(tmpda);
                DBG_INFO(0, "Severe error, closing connection");
                GWEN_MsgLayer_Disconnect(ml);
                return GWEN_Error_new(0,
                                      GWEN_ERROR_SEVERITY_ERR,
                                      GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                                      GWEN_IPC_ERROR_BAD_MSG);
              }
              GWEN_Buffer_SetPos(buffer, currPos);
              msgsize=GWEN_DB_GetIntValue(tmpda, "size", 0, -1);
              if (msgsize<GWEN_Buffer_GetUsedBytes(buffer)) {
                DBG_ERROR(0, "Bad size (%d)", msgsize);
                GWEN_DB_Group_free(tmpda);
                DBG_INFO(0, "Severe error, closing connection");
                GWEN_MsgLayer_Disconnect(ml);
                return GWEN_Error_new(0,
                                      GWEN_ERROR_SEVERITY_ERR,
                                      GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                                      GWEN_IPC_ERROR_BAD_MSG);
              }
              DBG_INFO(0, "Message size is %d bytes", msgsize);
              mcd->bytesToRead=msgsize-GWEN_Buffer_GetUsedBytes(buffer);
              mcd->readingSize=0;
              GWEN_DB_Group_free(tmpda);
            } /* if header complete */
            else {
              mcd->bytesToRead-=bread;
              DBG_DEBUG(0, "Still %d bytes of header to read", mcd->bytesToRead);
            }
          }
        } /* if reading header */

        if (!mcd->readingSize) {
          /* already reading the message itself */
          DBG_INFO(0, "Reading data (%d bytes)", mcd->bytesToRead);
          if (mcd->bytesToRead) {
            if (!hasRead) {
              DBG_INFO(0, "Nothing read so far");
              canRead=1;
            }
            else {
              canRead=GWEN_Error_IsOk(GWEN_IPCTransportLayer_CanRead(tl));
              if (canRead) {
                DBG_INFO(0, "Still data in the queue");
              }
            }

            if (canRead) {
              unsigned int bread;

              assert(mcd->currentMsg);
              buffer=GWEN_Msg_GetBuffer(mcd->currentMsg);

              /* read the rest of the header */
              bread=mcd->bytesToRead;
              err=GWEN_IPCTransportLayer_Read(tl,
                                              GWEN_Buffer_GetPosPointer(buffer),
                                              &bread);
              if (!GWEN_Error_IsOk(err)) {
                DBG_INFO(0, "Severe error, closing connection");
                GWEN_MsgLayer_Disconnect(ml);
                return err;
              }
              hasRead=1;
              GWEN_Buffer_IncrementPos(buffer, bread);
              GWEN_Buffer_SetUsedBytes(buffer,
                                       GWEN_Buffer_GetUsedBytes(buffer)+bread);
              mcd->bytesToRead-=bread;
            }
          } /* if canRead */

          if (mcd->bytesToRead==0) {
            err=GWEN_MsgLayer_AddIncomingMsg(ml, mcd->currentMsg);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "called from here");
              return err;
            }
            DBG_INFO(0, "Added new incoming message");
            mcd->currentMsg=0;
            /* be idle again */
            GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
            DBG_INFO(0, "Idle again");
          }
          else {
            if (!canRead)
              break;
          }
        } /* if reading message instead of header */
      } /* if StateReading */
    } /* for */
  } /* if in read mode */
  else {
    if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateWriting) {
      /* in write mode */
      unsigned int bytes;
      GWEN_BUFFER *buffer;

      assert(mcd->currentMsg);
      buffer=GWEN_Msg_GetBuffer(mcd->currentMsg);
      bytes=GWEN_Buffer_BytesLeft(buffer);
      DBG_DEBUG(0, "Writing %d bytes", bytes);
      err=GWEN_IPCTransportLayer_Write(tl,
                                       GWEN_Buffer_GetPosPointer(buffer),
                                       &bytes);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "called from here");
        return err;
      }
      if (bytes==GWEN_Buffer_BytesLeft(buffer)) {
        /* message completed */
        DBG_INFO(0, "Message sending completed");
        GWEN_Msg_free(mcd->currentMsg);
        mcd->currentMsg=0;
        GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
      }
      else {
        /* message not finished, advance pointer */
        err=GWEN_Buffer_IncrementPos(buffer, bytes);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "called from here");
          GWEN_Msg_free(mcd->currentMsg);
          mcd->currentMsg=0;
          GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
          return err;
        }
      }
    }

    if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateConnecting) {
    }
  } /* if write mode */

  if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateWaiting) {
  }

  if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateClosed) {
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayerCmd_Accept(GWEN_IPCMSGLAYER *ml,
                                       GWEN_IPCTRANSPORTLAYER *tl,
                                       GWEN_IPCMSGLAYER **m){
  GWEN_IPCMSGLAYER *newml;
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  assert(ml);
  assert(tl);
  mcd=(GWEN_IPCMSGLAYERCMDDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);
  newml=GWEN_MsgLayerCmd_new(mcd->msgEngine, tl,
                             GWEN_IPCMsglayerStateIdle);
  DBG_INFO(0, "Created new GWEN_IPCMSGLAYER for incoming connection (%08x)",
           (unsigned int)newml);
  *m=newml;
  return 0;
}








/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_ConnectionLayerCmd_new(GWEN_MSGENGINE *msgEngine,
                                               GWEN_IPCMSGLAYER *ml,
                                               GWEN_IPCCONNLAYER_STATE st){
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  cl=GWEN_ConnectionLayer_new(ml, st);
  ccd=GWEN_ConnectionLayerCmdData_new(msgEngine);
  GWEN_ConnectionLayer_SetData(cl, ccd);
  GWEN_ConnectionLayer_SetFreeDataFn(cl, GWEN_ConnectionLayerCmd_free);
  GWEN_ConnectionLayer_SetWorkFn(cl, GWEN_ConnectionLayerCmd_Work);
  GWEN_ConnectionLayer_SetAcceptFn(cl, GWEN_ConnectionLayerCmd_Accept);
  GWEN_ConnectionLayer_SetOpenFn(cl, GWEN_ConnectionLayerCmd_Open);
  GWEN_ConnectionLayer_SetCloseFn(cl, GWEN_ConnectionLayerCmd_Close);

  return cl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayerCmd_free(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  GWEN_ConnectionLayerCmdData_free(ccd);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Work(GWEN_IPCCONNLAYER *cl, int rd){
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_IPCCONNLAYER_STATE cst;
  GWEN_IPCMSGLAYER_STATE mst;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;

  assert(cl);
  DBG_INFO(0, "Working on connection %d", GWEN_ConnectionLayer_GetId(cl));
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);
  tl=GWEN_MsgLayer_GetTransportLayer(ml);
  assert(tl);
  cst=GWEN_ConnectionLayer_GetState(cl);
  mst=GWEN_MsgLayer_GetState(ml);
  err=GWEN_MsgLayer_Work(ml, rd);

  /* let the underlying layers work */
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
  }

  switch(cst) {

  case GWEN_IPCConnectionLayerStateOpening:
    switch(ccd->securityState) {
    case GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSED:
      err=GWEN_IPCTransportLayer_StartConnect(tl);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "called from here");
        return err;
      }
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING;
      /* no break, we simply continue here to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING:
      err=GWEN_IPCTransportLayer_FinishConnect(tl);
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)!=
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
            GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
          // real error, so return that error

          DBG_INFO(0, "called from here");
          return err;
        }
        /* otherwise connecting in progress, return */
        /* TODO: timeout */
        return 0;
      }
      /* goto next state */
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED;
      /* no break, we simply continue here to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED:
      if (!(ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT)) {
        /* no encryption needed, so go directly to connected state */
        GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpen);
        ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED;
        break;
      }
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_PUBLIC_KEY_EXCHG;
      /* no break, simple fall-through to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_PUBLIC_KEY_EXCHG:
      DBG_WARN(0, "No public key exchange, encryption not implemented.");
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_SESSION_KEY_EXCHG;
      /* no break, simple fall-through to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_SESSION_KEY_EXCHG:
      DBG_WARN(0, "No session key exchange, encryption not implemented.");
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED;
      /* no break, simple fall-through to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED:
      GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpen);
      DBG_NOTICE(0, "Connection established");
      break;

    case GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSING:
      DBG_ERROR(0, "Unexpected state \"CLOSING\"");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_STATE);
    default:
      DBG_ERROR(0, "Unexpected state %d", ccd->securityState);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_STATE);
    } /* switch securityState */
    break;

  case GWEN_IPCConnectionLayerStateOpen:
    /* connection is open, nothing more to do */
    break;

  case GWEN_IPCConnectionLayerStateClosing:
    /* TODO: implement handshaking */
    break;

  case GWEN_IPCConnectionLayerStateListening:
    if (rd) {
      /* readable on listening socket, so a new connection is available */
      GWEN_IPCCONNLAYER *newcl;

      err=GWEN_ConnectionLayer_Accept(cl, &newcl);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "called from here");
        return err;
      }
      GWEN_ConnectionLayer_Append(cl, newcl);
    }
    break;

  case GWEN_IPCConnectionLayerStateClosed:
  default:
    break;
  } /* switch */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Accept(GWEN_IPCCONNLAYER *cl,
                                              GWEN_IPCMSGLAYER *ml,
                                              GWEN_IPCCONNLAYER **c){
  GWEN_IPCCONNLAYER *newcl;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_IPCCONNLAYERCMDDATA *newccd;

  assert(cl);
  assert(ml);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  newcl=GWEN_ConnectionLayerCmd_new(ccd->msgEngine, ml,
                                    GWEN_IPCConnectionLayerStateOpening);
  DBG_INFO(0, "Created new GWEN_IPCCONNLAYER for incoming connection (%08x)",
           (unsigned int)newcl);
  newccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(newcl);
  assert(newccd);
  newccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED;
  GWEN_ConnectionLayer_SetUserMark(newcl,
                                   GWEN_ConnectionLayer_GetUserMark(cl));
  GWEN_ConnectionLayer_SetFlags(newcl,
                                GWEN_ConnectionLayer_GetFlags(cl) |
                                GWEN_IPCCONNLAYER_FLAGS_PASSIVE);
  *c=newcl;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Open(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  if (GWEN_ConnectionLayer_GetState(cl)!=GWEN_IPCConnectionLayerStateClosed){
    DBG_ERROR(0, "Connection is not closed");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  }
  GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpening);
  ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Close(GWEN_IPCCONNLAYER *cl,
                                             int force){
  assert(cl);
  /* TODO: handshaking */
  GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateClosed);
  return 0;
}










/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IPCCMD_CreateMsg(GWEN_SERVICELAYER *sl,
                                   unsigned int connId,
                                   unsigned int refId,
                                   const char *name,
                                   unsigned int version,
                                   ...) {
  va_list arguments;
  char *arg;
  char *value;
  GWEN_DB_NODE *da;
  GWEN_XMLNODE *n;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_BUFFER *gbuf;
  GWEN_IPCMSG *msg;
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;
  unsigned int msgId;

  assert(sl);
  assert(name);
  cl=GWEN_ServiceLayer_FindConnection(sl, connId, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection %d not found", connId);
    return 0;
  }
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  /* find node */
  n=GWEN_MsgEngine_FindNodeByProperty(ccd->msgEngine,
                                      "SEG",
                                      "code",
                                      version,
                                      name);
  if (!n) {
    DBG_ERROR(0, "Command \"%s\" not found", name);
    return 0;
  }

  /* set parameters */
  da=GWEN_DB_Group_new("Data");
  va_start(arguments, version);
  for(;;) {
    arg=va_arg(arguments, char*);
    if (!arg)
      break;
    value=va_arg(arguments, char*);
    if (!value) {
      DBG_ERROR(0, "missing value for argument \"%s\" in call", arg);
      GWEN_DB_Group_free(da);
      va_end(arguments);
      return 0;
    }
    if (GWEN_DB_SetCharValue(da,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             arg,
                             value)) {
      DBG_ERROR(0, "Could not set value for argument \"%s\"", arg);
      GWEN_DB_Group_free(da);
      va_end(arguments);
      return 0;
    }
  } /* for */
  va_end(arguments);

  /* now create the message */
  gbuf=GWEN_Buffer_new(0, GWEN_CMDLAYER_MAXMSGSIZE, 0, 1);
  if (GWEN_MsgEngine_CreateMessageFromNode(ccd->msgEngine,
                                           n,
                                           gbuf,
                                           da)) {
    DBG_ERROR(0, "Error creating message \"%s\"", name);
    GWEN_DB_Group_free(da);
    return 0;
  }
  GWEN_DB_Group_free(da);

  /* TODO: encrypt buffer */

  /* TODO: create msg header */

  /* create msg */
  msg=GWEN_Msg_new();
  msgId=GWEN_Msg_GetMsgId(msg);
  GWEN_Msg_SetBuffer(msg, gbuf);
  GWEN_Msg_SetMsgLayerId(msg, connId);
  GWEN_Msg_SetReferenceId(msg, refId);

  /* return msg */
  return msg;
}






/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCSERVICECMD *GWEN_IPCServiceCmd_new(){
  GWEN_IPCSERVICECMD *s;

  GWEN_NEW_OBJECT(GWEN_IPCSERVICECMD, s);
  return s;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCServiceCmd_free(GWEN_IPCSERVICECMD *s){
  if (s) {
    GWEN_MsgEngine_free(s->msgEngine);
    GWEN_ServiceLayer_free(s->serviceLayer);
    free(s);
  }
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCServiceCmd_Init(GWEN_IPCSERVICECMD *s,
                                       const char *xmlfile){
  GWEN_XMLNODE *n;

  DBG_INFO(0, "Initializing IPCServiceCmd");
  assert(s);
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  if (GWEN_XML_ReadFile(n, xmlfile)) {
    DBG_ERROR(0, "Error reading XML file \"%s\".\n", xmlfile);
    GWEN_XMLNode_free(n);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_XMLFILE);
  }

  s->msgEngine=GWEN_MsgEngine_new();
  GWEN_MsgEngine_SetDefinitions(s->msgEngine, n);

  s->serviceLayer=GWEN_ServiceLayer_new();
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCServiceCmd_Fini(GWEN_IPCSERVICECMD *s){
  assert(s);
  assert(s->msgEngine);
  assert(s->serviceLayer);

  DBG_INFO(0, "Deinitializing IPCServiceCmd");
  GWEN_ServiceLayer_Close(s->serviceLayer, 0, 0, 1);
  return 0;

}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCServiceCmd_Work(GWEN_IPCSERVICECMD *s,
                                       int timeout){
  GWEN_ERRORCODE err;

  assert(s);
  assert(s->serviceLayer);
  err=GWEN_ServiceLayer_Work(s->serviceLayer, timeout);
  return err;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_IPCServiceCmd_AddListener(GWEN_IPCSERVICECMD *s,
                                            const char *addr,
                                            int port,
                                            unsigned int mark){
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;
  unsigned int cid;

  if (port==-1) {
    tl=GWEN_IPCTransportLayerUnix_new();
  }
  else {
    tl=GWEN_IPCTransportLayerTCP_new();
    GWEN_IPCTransportLayer_SetPort(tl, port);
  }
  GWEN_IPCTransportLayer_SetAddress(tl, addr);

  /* start listening */
  err=GWEN_IPCTransportLayer_Listen(tl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_IPCTransportLayer_free(tl);
    return 0;
  }

  /* create higher layers */
  ml=GWEN_MsgLayerCmd_new(s->msgEngine, tl, GWEN_IPCMsglayerStateListening);
  cl=GWEN_ConnectionLayerCmd_new(s->msgEngine,
                                 ml,
                                 GWEN_IPCConnectionLayerStateListening);
  cid=GWEN_ConnectionLayer_GetId(cl);
  GWEN_ConnectionLayer_SetUserMark(cl, mark);
  err=GWEN_ServiceLayer_AddConnection(s->serviceLayer, cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_ConnectionLayer_free(cl);
    return 0;
  }

  return cid;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_IPCServiceCmd_AddPeer(GWEN_IPCSERVICECMD *s,
                                        const char *addr,
                                        int port,
                                        unsigned int mark){
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;
  unsigned int cid;

  if (port==-1) {
    tl=GWEN_IPCTransportLayerUnix_new();
  }
  else {
    tl=GWEN_IPCTransportLayerTCP_new();
    GWEN_IPCTransportLayer_SetPort(tl, port);
  }
  GWEN_IPCTransportLayer_SetAddress(tl, addr);

  /* create higher layers */
  ml=GWEN_MsgLayer_new(tl, GWEN_IPCMsglayerStateListening);
  cl=GWEN_ConnectionLayerCmd_new(s->msgEngine,
                                 ml,
                                 GWEN_IPCConnectionLayerStateListening);
  cid=GWEN_ConnectionLayer_GetId(cl);
  GWEN_ConnectionLayer_SetUserMark(cl, mark);
  err=GWEN_ServiceLayer_AddConnection(s->serviceLayer, cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_ConnectionLayer_free(cl);
    return 0;
  }

  return cid;
}







