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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ipcxmlmsglayer_p.h"
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/text.h>
#include <gwenhyfwar/hbcidialog.h>




/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCXMLMSGLAYERDATA *GWEN_IPCXMLMsgLayerData_new(GWEN_MSGENGINE *msgEngine){
  GWEN_IPCXMLMSGLAYERDATA *mcd;

  assert(msgEngine);
  GWEN_NEW_OBJECT(GWEN_IPCXMLMSGLAYERDATA, mcd);
  mcd->msgEngine=msgEngine;
  return mcd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLMsgLayerData_free(GWEN_IPCXMLMSGLAYERDATA *mcd){
  if (mcd) {
    free(mcd);
  }
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYER *GWEN_IPCXMLMsgLayer_new(GWEN_MSGENGINE *msgEngine,
                                          GWEN_IPCTRANSPORTLAYER *tl,
                                          GWEN_IPCMSGLAYER_STATE st){
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCXMLMSGLAYERDATA *mcd;

  ml=GWEN_MsgLayer_new(tl, st);
  mcd=GWEN_IPCXMLMsgLayerData_new(msgEngine);
  GWEN_MsgLayer_SetData(ml, mcd);
  GWEN_MsgLayer_SetFreeDataFn(ml, GWEN_IPCXMLMsgLayer_free);
  GWEN_MsgLayer_SetWorkFn(ml, GWEN_IPCXMLMsgLayer_Work);
  GWEN_MsgLayer_SetAcceptFn(ml, GWEN_IPCXMLMsgLayer_Accept);

  return ml;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLMsgLayer_free(GWEN_IPCMSGLAYER *ml){
  GWEN_IPCXMLMSGLAYERDATA *mcd;

  assert(ml);
  mcd=(GWEN_IPCXMLMSGLAYERDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);
  GWEN_IPCXMLMsgLayerData_free(mcd);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLMsgLayer_Work(GWEN_IPCMSGLAYER *ml,
                                        int rd){
  GWEN_IPCXMLMSGLAYERDATA *mcd;
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;
  int hasRead;
  int hasWritten;
  int canRead;

  hasRead=0;
  hasWritten=0;
  assert(ml);
  DBG_DEBUG(0, "Working on msgLayer %d", GWEN_MsgLayer_GetId(ml));
  mcd=(GWEN_IPCXMLMSGLAYERDATA*)GWEN_MsgLayer_GetData(ml);
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
                                           GWEN_IPCXMLMSGLAYER_MSGSIZE,
                                           0, 1));
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
            /* add message */
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
GWEN_ERRORCODE GWEN_IPCXMLMsgLayer_Accept(GWEN_IPCMSGLAYER *ml,
                                          GWEN_IPCTRANSPORTLAYER *tl,
                                          GWEN_IPCMSGLAYER **m){
  GWEN_IPCMSGLAYER *newml;
  GWEN_IPCXMLMSGLAYERDATA *mcd;

  assert(ml);
  assert(tl);
  mcd=(GWEN_IPCXMLMSGLAYERDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);
  newml=GWEN_IPCXMLMsgLayer_new(mcd->msgEngine, tl,
                             GWEN_IPCMsglayerStateIdle);
  DBG_INFO(0, "Created new GWEN_IPCMSGLAYER for incoming connection (%08x)",
           (unsigned int)newml);
  *m=newml;
  return 0;
}





