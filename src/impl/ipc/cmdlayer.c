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
  ccd->ownName=strdup("Gwenhywfar");
  ccd->ownVersion=strdup(GWENHYFWAR_VERSION_FULL_STRING);
  return ccd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayerCmdData_free(GWEN_IPCCONNLAYERCMDDATA *ccd){
  if (ccd) {
    free(ccd->peerName);
    free(ccd->peerVersion);
    free(ccd->ownName);
    free(ccd->ownVersion);
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
  DBG_DEBUG(0, "Working on msgLayer %d", GWEN_MsgLayer_GetId(ml));
  mcd=(GWEN_IPCMSGLAYERCMDDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);

  tl=GWEN_MsgLayer_GetTransportLayer(ml);
  assert(tl);

  if (rd) {
    for (;;) {
      /* this loop runs as long as there still is data to read */
      if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateIdle) {
        /* read mode */
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

        if (!GWEN_MsgLayer_CheckAddIncomingMsg(ml)) {
          DBG_INFO(0, "Could read, but inqueue is full");
          break;
        }

        DBG_INFO(0, "Starting to read header");
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

              if (GWEN_MsgEngine_ParseMessage(mcd->msgEngine,
                                              sn,
                                              buffer,
                                              tmpda,
                                              GWEN_MSGENGINE_READ_FLAGS_DEFAULT)) {
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
            break;
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
    DBG_INFO(0, "Write mode");
    if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateIdle) {
      GWEN_IPCMSG *msg;

      msg=GWEN_MsgLayer_GetOutgoingMsg(ml);
      if (msg) {
        GWEN_BUFFER *buffer;

        /* we have an outbound message, write it */
        DBG_INFO(0, "Start sending outbound message");
        assert(mcd->currentMsg==0);
        mcd->currentMsg=msg;
        buffer=GWEN_Msg_GetBuffer(mcd->currentMsg);
        GWEN_Buffer_SetPos(buffer, 0);
        GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateWriting);
      }
      else {
        DBG_DEBUG(0, "Nothing to write");
      }
    }

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
        DBG_INFO(0, "Idle again");
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

  if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateUnconnected) {
  }

  if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateClosed) {
  } /* if closed */

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
void GWEN_ConnectionLayerCmd_SetNameAndVersion(GWEN_IPCCONNLAYER *cl,
                                               const char *name,
                                               const char *version){
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  free(ccd->ownName);
  free(ccd->ownVersion);
  ccd->ownName=strdup(name);
  ccd->ownVersion=strdup(version);
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCServiceCmd_SetNameAndVersion(GWEN_IPCSERVICECMD *s,
                                          const char *name,
                                          const char *version){
  assert(s);
  free(s->ownName);
  free(s->ownVersion);
  s->ownName=strdup(name);
  s->ownVersion=strdup(version);
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_ConnectionLayerCmd_GetPeerName(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  return ccd->peerName;
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_ConnectionLayerCmd_GetPeerVersion(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  return ccd->peerVersion;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Work(GWEN_IPCCONNLAYER *cl, int rd){
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_IPCMSGLAYER_STATE mst;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;

  assert(cl);
  DBG_INFO(0, "Working on connection %d (%s mode)",
           GWEN_ConnectionLayer_GetId(cl),
           rd?"read":"write");
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);
  tl=GWEN_MsgLayer_GetTransportLayer(ml);
  assert(tl);
  mst=GWEN_MsgLayer_GetState(ml);
  err=GWEN_MsgLayer_Work(ml, rd);

  /* let the underlying layers work */
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
  }

  DBG_INFO(0, "Connecion state is %d",GWEN_ConnectionLayer_GetState(cl));
  if (GWEN_ConnectionLayer_GetState(cl)==GWEN_IPCConnectionLayerStateOpening) {
    /* =======================================================================
     * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
     *  This implements the simple IPC protocol.
     *  - Opening a connection:
     *    - exchange greeting messages
     *    - exchange public keys (if encryption/signing needed)
     *    - exchange session key (if encryption needed)
     *  - Closing a connection:
     *    - exchange closing messages
     *
     * An active connection is a connection which has been actively opened,
     * whereas a passive connection is an accepted incoming connection.
     * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
     * =======================================================================
     */
    if (ccd->securityState==GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSED) {
      err=GWEN_IPCTransportLayer_StartConnect(tl);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "called from here");
        return err;
      }
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING;
    }


    /* =======================================================================
     *                          Finish connect
     * =======================================================================
     */
    if (ccd->securityState==GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING) {
      DBG_INFO(0, "Finishing connect to %d",
               GWEN_ConnectionLayer_GetId(cl));
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
      DBG_INFO(0, "Physically connected");
      GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED;
    }

    /* =======================================================================
     *                             Connected
     *                    Passive: Await greeting message
     *                    Active:  Send greeting message
     * =======================================================================
     */
    if (ccd->securityState==GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED) {
      if (GWEN_ConnectionLayer_GetFlags(cl) &
          GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
        /* -------------------------------------------------------------------
         *                    passive, await greeting
         * -------------------------------------------------------------------
         */
        GWEN_DB_NODE *mdb;

        DBG_INFO(0, "Waiting for greeting on %d",
                 GWEN_ConnectionLayer_GetId(cl));
        mdb=GWEN_IPCCMD_ReceiveMsg(cl, "RQGreet");
        if (mdb) {
          /* evaluate the msg */
          unsigned int fl;

          fl=ccd->flags;
          if (GWEN_DB_GetIntValue(mdb, "encrypt", 0, 0))
            fl|=GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT;
          if (GWEN_DB_GetIntValue(mdb, "encrypt", 0, 0))
            fl|=GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT;
          ccd->flags=fl;
          free(ccd->peerName);
          ccd->peerName=strdup(GWEN_DB_GetCharValue(mdb,
                                                    "RQGreet/name",
                                                    0,
                                                    "[no name]"));
          free(ccd->peerVersion);
          ccd->peerVersion=strdup(GWEN_DB_GetCharValue(mdb,
                                                       "RQGreet/version",
                                                       0,
                                                       "[no version]"));
          DBG_NOTICE(0, "Greetings from \"%s\" (%s)",
                     ccd->peerName, ccd->peerVersion);
          GWEN_DB_Group_free(mdb);
          /* await greeting response */
          ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_GREETING;
        }
        else {
          /* TODO: timeout */
          return 0;
        }
      } /* if passive */
      else {
        /* -------------------------------------------------------------------
         *                    active, send greeting
         * -------------------------------------------------------------------
         */
        DBG_INFO(0, "Sending greeting on %d",
                 GWEN_ConnectionLayer_GetId(cl));
        err=GWEN_IPCCMD_SendGreetRQ(cl);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "called from here");
          return err;
        }
        /* change state */
        ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_GREETING;
        return 0;
      } /* if active */
    } /* if state "connected" */


    /* =======================================================================
     *                              Greeting
     *                 Active:  Await greeting response
     *                 Passive: ---
     * =======================================================================
     */
    if (ccd->securityState==GWEN_IPCCONNLAYERCMD_SECSTATE_GREETING) {
      if (!(GWEN_ConnectionLayer_GetFlags(cl) &
            GWEN_IPCCONNLAYER_FLAGS_PASSIVE)) {
        /* ------------------------------------------------------------------
         *             active, await greeting response
         * ------------------------------------------------------------------
         */
        GWEN_DB_NODE *mdb;

        DBG_INFO(0, "Waiting for greeting response on %d",
                 GWEN_ConnectionLayer_GetId(cl));
        mdb=GWEN_IPCCMD_ReceiveMsg(cl, "RPGreet");
        if (mdb) {
          /* TODO: evaluate the public key msg */
          ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_GREETED;
          GWEN_DB_Group_free(mdb);
        } /* if mdb */
        else {
          /* TODO: timeout */
          DBG_INFO(0, "No RPGreet message");
          return 0;
        }
      } /* if active */
      else {
        /* ------------------------------------------------------------------
         *             passive, send greeting response
         * ------------------------------------------------------------------
         */
        DBG_INFO(0, "Sending greeting response to %d",
                 GWEN_ConnectionLayer_GetId(cl));
        err=GWEN_IPCCMD_SendGreetRP(cl, ccd->lastRefId);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "called from here");
          return err;
        }
        /* change state */
        ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_GREETED;
        return 0;
      }
    } /* if state "greeting" */


    /* =======================================================================
     *                          Greeting done
     *                   Active:  Send public key
     *                   Passive: Await public key
     * =======================================================================
     */
    if (ccd->securityState==GWEN_IPCCONNLAYERCMD_SECSTATE_GREETED) {
      if (!(ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT ||
            ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_SIGN)) {
        /* no encryption needed, go directly to connected state */
        DBG_INFO(0, "No encryption needed on %d",
                 GWEN_ConnectionLayer_GetId(cl));
        GWEN_ConnectionLayer_SetState(cl,
                                      GWEN_IPCConnectionLayerStateOpen);
        ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED;
      } /* if no encryption/signing */
      else {
        /* encryption needed */
        if (GWEN_ConnectionLayer_GetFlags(cl) &
            GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
          /* ------------------------------------------------------------------
           *                 passive, await public key
           * ------------------------------------------------------------------
           */
          GWEN_DB_NODE *mdb;

          DBG_INFO(0, "Waiting for public key on %d",
                   GWEN_ConnectionLayer_GetId(cl));
          mdb=GWEN_IPCCMD_ReceiveMsg(cl, "RQPubKey");
          if (mdb) {
            /* TODO: evaluate the public key msg */

            ccd->securityState=
              GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGING_PK;
            GWEN_DB_Group_free(mdb);
          } /* if msg */
        } /* if passive */
        else {
          /* -----------------------------------------------------------------
           *                    active, send public key
           * -----------------------------------------------------------------
           */
          DBG_INFO(0, "Sending public key to %d",
                   GWEN_ConnectionLayer_GetId(cl));
          err=GWEN_IPCCMD_SendPubKeyRQ(cl);
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "called from here");
            return err;
          }
          /* change state */
          ccd->securityState=
            GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGING_PK;
        } /* if active */
      } /* if encryption/signing wanted */
    } /* if greeted */


    /* =======================================================================
     *                        Exchanging public key
     *                  Active:  Await public key response
     *                  Passive: ---
     * =======================================================================
     */
    if (ccd->securityState==
        GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGING_PK) {
      if (GWEN_ConnectionLayer_GetFlags(cl) &
          GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
        /* -------------------------------------------------------------------
         *                    passive, goto next state
         * -------------------------------------------------------------------
         */
        ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGED_PK;
      } /* if passive */
      else {
        /* -------------------------------------------------------------------
         *                  active, await public key response
         * -------------------------------------------------------------------
         */
        GWEN_DB_NODE *mdb;

        DBG_INFO(0, "Waiting for public key response on %d",
                 GWEN_ConnectionLayer_GetId(cl));
        mdb=GWEN_IPCCMD_ReceiveMsg(cl, "RPPubKey");
        if (mdb) {
          /* TODO: evaluate the msg */
          ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGED_PK;
          GWEN_DB_Group_free(mdb);
        } /* if mdb */
      } /* if active */
    }

    if (ccd->securityState==
        GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGED_PK) {
      if (GWEN_ConnectionLayer_GetFlags(cl) &
          GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
        /* -----------------------------------------------------------------
         *                   passive, await session key request
         * -----------------------------------------------------------------
         */
        GWEN_DB_NODE *mdb;

        DBG_INFO(0, "Waiting for session key request on %d",
                 GWEN_ConnectionLayer_GetId(cl));
        mdb=GWEN_IPCCMD_ReceiveMsg(cl, "RQSessKey");
        if (mdb) {
          /* TODO: handle request */
          ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGING_SK;
          GWEN_DB_Group_free(mdb);
        }
      } /* if passive */
      else {
        /* -----------------------------------------------------------------
         *                   active, send session key request
         * -----------------------------------------------------------------
         */
        DBG_INFO(0, "Sending session key to %d",
                 GWEN_ConnectionLayer_GetId(cl));
        err=GWEN_IPCCMD_SendSessionKeyRQ(cl);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "called from here");
          return err;
        }
        /* change state */
        ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGING_SK;
      } /* if active */
    } /* if exchanged public key */


    /* =======================================================================
     *                          Session key exchange
     * =======================================================================
     */
    if (ccd->securityState==
        GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGING_SK){
      DBG_WARN(0, "No session key exchange, encryption not implemented.");
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGED_SK;
    }


    /* =======================================================================
     *                          Session key exchanged
     * =======================================================================
     */
    if (ccd->securityState==
        GWEN_IPCCONNLAYERCMD_SECSTATE_EXCHANGED_SK){
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED;
    }


    /* =======================================================================
     *                          Connection Established
     *           Move from messages from ConnectionLayer to MsgLayer
     * =======================================================================
     */
    if (ccd->securityState==GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED) {
      GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpen);
      DBG_NOTICE(0, "Connection %d established",
                 GWEN_ConnectionLayer_GetId(cl));
    }


    /* =======================================================================
     *              Closing Connection (unexpected while opening)
     * =======================================================================
     */
    if (ccd->securityState==GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSING) {
      DBG_ERROR(0, "Unexpected state \"CLOSING\"");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_STATE);
    }
  }

  if (GWEN_ConnectionLayer_GetState(cl)==GWEN_IPCConnectionLayerStateOpen) {
    /* connection is open, fill queues of next lower level */
    GWEN_IPCMSG *msg;

    /* fill msgLayer's outqueue */
    while (GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
      msg=GWEN_ConnectionLayer_GetOutgoingMsg(cl);
      if (msg) {
        DBG_INFO(0, "Passing outbound message to msgLayer");
        err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR(0,"Adding a message should be possible at this point");
          GWEN_ConnectionLayer_Close(cl, 1);
          GWEN_Msg_free(msg);
          return err;
        }
      }
      else
        break;
    } /* while room for outgoing msgs */

    /* read msgs from msgLayer's inqueue */
    while (1) {
      msg=GWEN_MsgLayer_GetIncomingMsg(ml);
      if (!msg) {
        DBG_INFO(0, "No incoming message on msgLayer");
        break;
      }

      /* TODO: decrypt message */

      DBG_INFO(0, "Got an inbound message");
      err=GWEN_ConnectionLayer_AddIncomingMsg(cl, msg);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR(0,"Adding an inbound message should always be possible");
        GWEN_ConnectionLayer_Close(cl, 1);
        GWEN_Msg_free(msg);
        return err;
      }
    } /* while */
  }

  if (GWEN_ConnectionLayer_GetState(cl)==GWEN_IPCConnectionLayerStateClosing) {
    /* TODO: implement handshaking */
  }

  if (GWEN_ConnectionLayer_GetState(cl)==GWEN_IPCConnectionLayerStateListening) {
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
  }

  if (GWEN_ConnectionLayer_GetState(cl)==GWEN_IPCConnectionLayerStateUnconnected) {
  }

  if (GWEN_ConnectionLayer_GetState(cl)==GWEN_IPCConnectionLayerStateClosed) {
    if (GWEN_ConnectionLayer_GetFlags(cl) &
        GWEN_IPCCONNLAYER_FLAGS_PERSISTENT) {
      /* closed, but in persistent mode, so reset to "unconnected"
       * to allow reconnect when needed */
      GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateUnconnected);
      GWEN_ConnectionLayer_SetState(cl,
                                    GWEN_IPCConnectionLayerStateUnconnected);
    }
  }

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
  *c=newcl;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Open(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_ERRORCODE err;
  GWEN_IPCMSGLAYER *ml;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  if (GWEN_ConnectionLayer_GetState(cl)!=
      GWEN_IPCConnectionLayerStateUnconnected){
    DBG_ERROR(0, "Connection %d is not closed (%d)",
              GWEN_ConnectionLayer_GetId(cl),
              GWEN_ConnectionLayer_GetState(cl));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  }

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);

  DBG_INFO(0, "Starting to connect %d",
           GWEN_ConnectionLayer_GetId(cl));
  err=GWEN_MsgLayer_Connect(ml);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
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
GWEN_IPCMSG *GWEN_ConnectionLayerCmd_CreateMsg(GWEN_IPCCONNLAYER *cl,
                                               unsigned int refId,
                                               const char *name,
                                               unsigned int version,
                                               GWEN_DB_NODE *da) {
  GWEN_XMLNODE *n;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_BUFFER *gbuf;
  GWEN_IPCMSG *msg;
  unsigned int msgId;
  GWEN_DB_NODE *mhda;
  GWEN_BUFFER *mhbuf;
  GWEN_BUFFER *resbuf;

  assert(name);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  /* find node */
  n=GWEN_MsgEngine_FindNodeByProperty(ccd->msgEngine,
                                      "SEG",
                                      "id",
                                      version,
                                      name);
  if (!n) {
    DBG_ERROR(0, "Command \"%s\" not found", name);
    return 0;
  }

  /* preset SegmentNumber */
  GWEN_MsgEngine_SetIntValue(ccd->msgEngine,
                             "SegmentNumber",
                             2);
  /* now create the message */
  gbuf=GWEN_Buffer_new(0, GWEN_CMDLAYER_MAXMSGSIZE, 0, 1);
  if (GWEN_MsgEngine_CreateMessageFromNode(ccd->msgEngine,
                                           n,
                                           gbuf,
                                           da)) {
    DBG_ERROR(0, "Error creating message \"%s\"", name);
    return 0;
  }

  /* TODO: encrypt buffer */

  /* create msg header */
  DBG_INFO(0, "Creating message head");
  GWEN_MsgEngine_SetIntValue(ccd->msgEngine,
                             "SegmentNumber",
                             1);
  n=GWEN_MsgEngine_FindNodeByProperty(ccd->msgEngine,
                                      "SEG",
                                      "id",
                                      version,
                                      "MsgHead");
  if (!n) {
    DBG_ERROR(0, "MsgHead not found");
    return 0;
  }
  mhda=GWEN_DB_Group_new("msghead");
  GWEN_DB_SetIntValue(mhda,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "size", 0);
  GWEN_DB_SetIntValue(mhda,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "version",
                      GWEN_MsgEngine_GetIntValue(ccd->msgEngine,
                                                 "Version", 0));
  GWEN_DB_SetCharValue(mhda,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "dialogId",
                       GWEN_MsgEngine_GetValue(ccd->msgEngine,
                                               "DialogId", "0"));

  mhbuf=GWEN_Buffer_new(0, GWEN_CMDLAYER_MAXMSGSIZE, 0, 1);
  if (GWEN_MsgEngine_CreateMessageFromNode(ccd->msgEngine,
                                           n,
                                           mhbuf,
                                           mhda)) {
    DBG_ERROR(0, "Error creating message head");
    GWEN_Buffer_free(mhbuf);
    GWEN_Buffer_free(gbuf);
    GWEN_DB_Group_free(mhda);
    return 0;
  }

  /* set real size */
  GWEN_DB_SetIntValue(mhda,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "size",
                      GWEN_Buffer_GetUsedBytes(mhbuf)+
                      GWEN_Buffer_GetUsedBytes(gbuf));

  /* rebuild message head with correct size */
  GWEN_MsgEngine_SetIntValue(ccd->msgEngine,
                             "SegmentNumber",
                             1);
  GWEN_Buffer_SetPos(mhbuf, 0);
  GWEN_Buffer_SetUsedBytes(mhbuf, 0);
  if (GWEN_MsgEngine_CreateMessageFromNode(ccd->msgEngine,
                                           n,
                                           mhbuf,
                                           mhda)) {
    DBG_ERROR(0, "Error creating message head");
    GWEN_Buffer_free(mhbuf);
    GWEN_Buffer_free(gbuf);
    GWEN_DB_Group_free(mhda);
    return 0;
  }

  /* copy all buffers together */
  resbuf=GWEN_Buffer_new(0,
                         GWEN_Buffer_GetUsedBytes(mhbuf)+
                         GWEN_Buffer_GetUsedBytes(gbuf),
                         0, 1);
  GWEN_Buffer_AppendBuffer(resbuf, mhbuf);
  GWEN_Buffer_AppendBuffer(resbuf, gbuf);
  GWEN_Buffer_free(mhbuf);
  GWEN_Buffer_free(gbuf);
  GWEN_DB_Group_free(mhda);
  gbuf=resbuf;

  /* create msg */
  /*fprintf(stderr, "Message is:\n");
  GWEN_Buffer_Dump(gbuf, stderr, 1);*/

  msg=GWEN_Msg_new();
  msgId=GWEN_Msg_GetMsgId(msg);
  GWEN_Msg_SetBuffer(msg, gbuf);
  GWEN_Msg_SetMsgLayerId(msg, GWEN_ConnectionLayer_GetId(cl));
  GWEN_Msg_SetReferenceId(msg, refId);

  /* return msg */
  return msg;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_IPCServiceCmd_ParseMsg(GWEN_IPCCONNLAYER *cl,
                                          GWEN_IPCMSG *msg) {
  GWEN_DB_NODE *db;
  int rv;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_BUFFER *buffer;

  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  buffer=GWEN_Msg_GetBuffer(msg);
  assert(buffer);
  db=GWEN_DB_Group_new("msgdata");
  rv=GWEN_MsgEngine_ReadMessage(ccd->msgEngine,
                                "SEG",
                                buffer,
                                db,
                                GWEN_MSGENGINE_READ_FLAGS_DEFAULT);
  if (rv==-1) {
    DBG_INFO(0, "called from here");
    GWEN_DB_Group_free(db);
    return 0;
  }

  ccd->lastRefId=GWEN_DB_GetIntValue(db, "head/seq", 0, 0);

  /* TODO: Decrypt message */

  /* return data */
  return db;
}






/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCSERVICECMD *GWEN_IPCServiceCmd_new(){
  GWEN_IPCSERVICECMD *s;

  GWEN_NEW_OBJECT(GWEN_IPCSERVICECMD, s);
  s->ownName=strdup("Gwenhywfar");
  s->ownVersion=strdup(GWENHYFWAR_VERSION_FULL_STRING);
  return s;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCServiceCmd_free(GWEN_IPCSERVICECMD *s){
  if (s) {
    GWEN_MsgEngine_free(s->msgEngine);
    GWEN_ServiceLayer_free(s->serviceLayer);
    free(s->ownName);
    free(s->ownVersion);
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
  if (GWEN_XML_ReadFile(n, xmlfile, GWEN_XML_FLAGS_DEFAULT)) {
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
  GWEN_ConnectionLayerCmd_SetNameAndVersion(cl, s->ownName, s->ownVersion);
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
  ml=GWEN_MsgLayerCmd_new(s->msgEngine, tl,
                          GWEN_IPCMsglayerStateUnconnected);
  cl=GWEN_ConnectionLayerCmd_new(s->msgEngine,
                                 ml,
                                 GWEN_IPCConnectionLayerStateUnconnected);
  GWEN_ConnectionLayerCmd_SetNameAndVersion(cl, s->ownName, s->ownVersion);
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
GWEN_IPCCONNLAYER*
GWEN_IPCServiceCmd_FindConnection(GWEN_IPCSERVICECMD *s,
                                  unsigned int id,
                                  unsigned int userMark){
  assert(s);
  assert(s->serviceLayer);
  return GWEN_ServiceLayer_FindConnection(s->serviceLayer, id, userMark);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IPCServiceCmd_CreateMsg(GWEN_IPCSERVICECMD *s,
                                          unsigned int id,
                                          unsigned int refId,
                                          const char *name,
                                          unsigned int version,
                                          GWEN_DB_NODE *da){
  GWEN_IPCCONNLAYER *cl;

  assert(s);
  assert(s->serviceLayer);
  cl=GWEN_ServiceLayer_FindConnection(s->serviceLayer, id, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection %d not found", id);
    return 0;
  }
  return GWEN_ConnectionLayerCmd_CreateMsg(cl,
                                           refId,
                                           name,
                                           version,
                                           da);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IPCServiceCmd_FindMsgReply(GWEN_IPCSERVICECMD *s,
                                             unsigned int refId){
  assert(s);
  assert(s->serviceLayer);
  return GWEN_ServiceLayer_FindMsgReply(s->serviceLayer, refId);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IPCServiceCmd_GetRequest(GWEN_IPCSERVICECMD *s) {
  assert(s);
  assert(s->serviceLayer);
  return GWEN_ServiceLayer_GetRequest(s->serviceLayer);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCServiceCmd_SendMessage(GWEN_IPCSERVICECMD *s,
                                              GWEN_IPCMSG *msg){
  assert(s);
  assert(s->serviceLayer);
  return GWEN_ServiceLayer_SendMessage(s->serviceLayer, msg);
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCServiceCmd_GetPeerName(GWEN_IPCSERVICECMD *s,
                                           unsigned int id){
  GWEN_IPCCONNLAYER *cl;

  assert(s);
  assert(s->serviceLayer);
  cl=GWEN_ServiceLayer_FindConnection(s->serviceLayer, id, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection %d not found", id);
    return 0;
  }
  return GWEN_ConnectionLayerCmd_GetPeerName(cl);
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCServiceCmd_GetPeerVersion(GWEN_IPCSERVICECMD *s,
                                              unsigned int id){
  GWEN_IPCCONNLAYER *cl;

  assert(s);
  assert(s->serviceLayer);
  cl=GWEN_ServiceLayer_FindConnection(s->serviceLayer, id, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection %d not found", id);
    return 0;
  }
  return GWEN_ConnectionLayerCmd_GetPeerVersion(cl);
}










/* --------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_IPCCMD_ReceiveMsg(GWEN_IPCCONNLAYER *cl,
                                     const char *name) {

  GWEN_IPCMSG *msg;
  GWEN_IPCMSGLAYER *ml;

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  msg=GWEN_MsgLayer_GetIncomingMsg(ml);
  if (msg) {
    /* we have a message, parse it */
    GWEN_DB_NODE *mdb;
    GWEN_DB_NODE *wdb;

    GWEN_Buffer_SetPos(GWEN_Msg_GetBuffer(msg), 0);
    mdb=GWEN_IPCServiceCmd_ParseMsg(cl,
                                    msg);
    if (!mdb) {
      DBG_INFO(0, "called from here");
      return 0;
    }
    wdb=GWEN_DB_GetGroup(mdb,
                         GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         name);
    if (!wdb) {
      DBG_ERROR(0, "Message \"%s\" expected.", name);
      GWEN_DB_Group_free(mdb);
      return 0;
    }
    fprintf(stderr, "Received this message:\n");
    GWEN_DB_Dump(mdb, stderr, 1);
    return mdb;
  } /* if msg */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCCMD_SendGreetRQ(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_ERRORCODE err;

  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  if (GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
    GWEN_IPCMSG *msg;
    GWEN_DB_NODE *db;

    /* create msg */
    db=GWEN_DB_Group_new("params");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "name", ccd->ownName);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "version", ccd->ownVersion);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "sign",
                        ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_SIGN);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "encrypt",
                        ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT);

    msg=GWEN_ConnectionLayerCmd_CreateMsg(cl, 0, "RQGreet", 0, db);
    GWEN_DB_Group_free(db);
    if (!msg) {
      DBG_ERROR(0, "Could not create msg");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    }

    /* add msg to outgoing queue (directly to the msgLayer!) */
    err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Msg_free(msg);
      DBG_WARN(0,
               "Adding a msg should be possible at this point :-(");
      GWEN_ConnectionLayer_Close(cl, 1);
      return err;
    }
  } /* if room for outgoing msg */
  else {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCCMD_SendGreetRP(GWEN_IPCCONNLAYER *cl,
                                       unsigned int refId) {
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_ERRORCODE err;

  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  if (GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
    GWEN_IPCMSG *msg;
    GWEN_DB_NODE *db;

    /* create msg */
    db=GWEN_DB_Group_new("params");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "name", ccd->ownName);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "version", ccd->ownVersion);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "sign",
                        ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_SIGN);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "encrypt",
                        ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT);

    msg=GWEN_ConnectionLayerCmd_CreateMsg(cl, 0, "RPGreet", 0, db);
    GWEN_DB_Group_free(db);
    if (!msg) {
      DBG_ERROR(0, "Could not create msg");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    }

    /* add msg to outgoing queue */
    err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Msg_free(msg);
      DBG_WARN(0,
               "Adding a msg should be possible at this point :-(");
      GWEN_ConnectionLayer_Close(cl, 1);
      return err;
    }
  } /* if room for outgoing msg */
  else {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCCMD_SendPubKeyRQ(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_ERRORCODE err;

  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  if (GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
    GWEN_IPCMSG *msg;
    GWEN_DB_NODE *db;

    /* create msg */
    db=GWEN_DB_Group_new("params");
    msg=GWEN_ConnectionLayerCmd_CreateMsg(cl, 0, "RQPubKey", 0, db);
    GWEN_DB_Group_free(db);
    if (!msg) {
      DBG_ERROR(0, "Could not create msg");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    }

    /* add msg to outgoing queue */
    err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Msg_free(msg);
      DBG_WARN(0,
               "Adding a msg should be possible at this point :-(");
      GWEN_ConnectionLayer_Close(cl, 1);
      return err;
    }
  } /* if room for outgoing msg */
  else {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCCMD_SendPubKeyRP(GWEN_IPCCONNLAYER *cl,
                                        unsigned int refId) {
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_ERRORCODE err;

  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  if (GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
    GWEN_IPCMSG *msg;
    GWEN_DB_NODE *db;

    /* create msg */
    /* TODO: set parameters */
    db=GWEN_DB_Group_new("params");
    msg=GWEN_ConnectionLayerCmd_CreateMsg(cl, 0, "RPPubKey", 0, db);
    GWEN_DB_Group_free(db);
    if (!msg) {
      DBG_ERROR(0, "Could not create msg");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    }

    /* add msg to outgoing queue */
    err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Msg_free(msg);
      DBG_WARN(0,
               "Adding a msg should be possible at this point :-(");
      GWEN_ConnectionLayer_Close(cl, 1);
      return err;
    }
  } /* if room for outgoing msg */
  else {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCCMD_SendSessionKeyRQ(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_ERRORCODE err;

  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  if (GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
    GWEN_IPCMSG *msg;
    GWEN_DB_NODE *db;

    /* create msg */
    /* TODO: set parameters */
    db=GWEN_DB_Group_new("params");
    msg=GWEN_ConnectionLayerCmd_CreateMsg(cl, 0, "RQSessKey", 0, db);
    GWEN_DB_Group_free(db);
    if (!msg) {
      DBG_ERROR(0, "Could not create msg");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    }

    /* add msg to outgoing queue */
    err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Msg_free(msg);
      DBG_WARN(0,
               "Adding a msg should be possible at this point :-(");
      GWEN_ConnectionLayer_Close(cl, 1);
      return err;
    }
  } /* if room for outgoing msg */
  else {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCCMD_SendSessionKeyRP(GWEN_IPCCONNLAYER *cl,
                                            unsigned int refId) {
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_ERRORCODE err;

  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  if (GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
    GWEN_IPCMSG *msg;
    GWEN_DB_NODE *db;

    /* create msg */
    /* TODO: set parameters */
    db=GWEN_DB_Group_new("params");
    msg=GWEN_ConnectionLayerCmd_CreateMsg(cl, 0, "RPSessKey", 0, db);
    GWEN_DB_Group_free(db);
    if (!msg) {
      DBG_ERROR(0, "Could not create msg");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    }

    /* add msg to outgoing queue */
    err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Msg_free(msg);
      DBG_WARN(0,
               "Adding a msg should be possible at this point :-(");
      GWEN_ConnectionLayer_Close(cl, 1);
      return err;
    }
  } /* if room for outgoing msg */
  else {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }
  return 0;
}










