/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DISABLE_DEBUGLOG*/


#include "./endpoint2_msgio_p.h"

#include <gwenhywfar/debug.h>


#define GWEN_ENDPOINT_MSGIO_BUFFERSIZE 1024



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);

static void _addSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_UNUSED GWEN_SOCKETSET *xSet);
static void _checkSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);
static int _writeCurrentMessage(GWEN_MSG_ENDPOINT2 *ep);
static int _readCurrentMessage(GWEN_MSG_ENDPOINT2 *ep);
static int _distributeBufferContent(GWEN_MSG_ENDPOINT2 *ep, const uint8_t *bufferPtr, int bufferLen);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_MSGIO)



void GWEN_MsgIoEndpoint2_Extend(GWEN_MSG_ENDPOINT2 *ep)
{
  GWEN_ENDPOINT2_MSGIO *xep;

  GWEN_NEW_OBJECT(GWEN_ENDPOINT2_MSGIO, xep);
  GWEN_INHERIT_SETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_MSGIO, ep, xep, _freeData);

  xep->addSocketsFn=GWEN_MsgEndpoint2_SetAddSocketsFn(ep, _addSockets);
  xep->checkSocketsFn=GWEN_MsgEndpoint2_SetCheckSocketsFn(ep, _checkSockets);
}



void _freeData(void *bp, void *p)
{
  GWEN_MSG_ENDPOINT2 *ep;
  GWEN_ENDPOINT2_MSGIO *xep;

  ep=(GWEN_MSG_ENDPOINT2*) bp;
  xep=(GWEN_ENDPOINT2_MSGIO*) p;
  GWEN_MsgEndpoint2_SetCheckSocketsFn(ep, xep->checkSocketsFn);
  GWEN_FREE_OBJECT(xep);
}



void GWEN_MsgIoEndpoint2_SetGetNeededBytesFn(GWEN_MSG_ENDPOINT2 *ep, GWEN_ENDPOINT2_MSGIO_GETBYTESNEEDED_FN f)
{
  if (ep) {
    GWEN_ENDPOINT2_MSGIO *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_MSGIO, ep);
    if (xep)
      xep->getBytesNeededFn=f;
  }
}



void _addSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_UNUSED GWEN_SOCKETSET *xSet)
{
  if (ep) {
    GWEN_ENDPOINT2_MSGIO *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_MSGIO, ep);
    if (xep) {
      if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTED) {
        GWEN_SOCKET *sk;

        sk=GWEN_MsgEndpoint2_GetSocket(ep);
        if (sk) {
          DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Adding socket to read set", GWEN_MsgEndpoint2_GetName(ep));
          GWEN_SocketSet_AddSocket(readSet, sk);
          if (GWEN_MsgEndpoint2_HaveMessageToSend(ep)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Adding socket to write set", GWEN_MsgEndpoint2_GetName(ep));
            GWEN_SocketSet_AddSocket(writeSet, sk);
          }
        } /* if socket */
      }
      else if (xep->addSocketsFn) {
        DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Not connected, calling base function", GWEN_MsgEndpoint2_GetName(ep));
        xep->addSocketsFn(ep, readSet, writeSet, xSet);
      }
    } /* if (xep) */
  } /* if (ep) */
}



void _checkSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  if (ep) {
    GWEN_ENDPOINT2_MSGIO *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_MSGIO, ep);
    if (xep) {
      int rv;

      if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTED) {
        GWEN_SOCKET *sk;
      
        sk=GWEN_MsgEndpoint2_GetSocket(ep);
        if (sk) {
          if (GWEN_SocketSet_HasSocket(writeSet, sk)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Has socket in write set", GWEN_MsgEndpoint2_GetName(ep));
            rv=_writeCurrentMessage(ep);
            if (rv<0 && rv!=GWEN_ERROR_TIMEOUT) {
              DBG_INFO(GWEN_LOGDOMAIN,
                       "Endpoint %s: Error writing current message (%d), disconnecting",
                       GWEN_MsgEndpoint2_GetName(ep),
                       rv);
              GWEN_MsgEndpoint2_Disconnect(ep);
              return;
            }
          }

          if (GWEN_SocketSet_HasSocket(readSet, sk)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Has socket in read set", GWEN_MsgEndpoint2_GetName(ep));
            rv=_readCurrentMessage(ep);
            if (rv<0 && rv!=GWEN_ERROR_TIMEOUT) {
              DBG_INFO(GWEN_LOGDOMAIN,
                       "Endpoint %s: Error reading current message (%d), disconnecting",
                       GWEN_MsgEndpoint2_GetName(ep),
                       rv);
              GWEN_MsgEndpoint2_Disconnect(ep);
              return;
            }
          }
        }
      } /* if connected */
      else if (xep->checkSocketsFn) {
        DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Not connected, calling base function", GWEN_MsgEndpoint2_GetName(ep));
        xep->checkSocketsFn(ep, readSet, writeSet, xSet);
      }
    }
  }
}



int _writeCurrentMessage(GWEN_MSG_ENDPOINT2 *ep)
{
  GWEN_MSG *msg;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Writing to endpoint %s", GWEN_MsgEndpoint2_GetName(ep));
  msg=GWEN_MsgEndpoint2_GetFirstSendMessage(ep);
  if (msg) {
    uint8_t pos;
    int remaining;
    int rv;

    pos=GWEN_Msg_GetCurrentPos(msg);
    remaining=GWEN_Msg_GetRemainingBytes(msg);
    if (remaining>0) {
      const uint8_t *buf;

      /* start new message */
      buf=GWEN_Msg_GetBuffer(msg)+pos;
      rv=GWEN_MsgEndpoint2_WriteToSocket(ep, buf, remaining);
      if (rv<0) {
        if (rv==GWEN_ERROR_TIMEOUT)
          return rv;
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on write() (%d)", rv);
        return rv;
      }
      GWEN_Msg_IncCurrentPos(msg, rv);
      if (rv==remaining) {
        DBG_INFO(GWEN_LOGDOMAIN, "Message completely sent");
        /* end current message */
        GWEN_Msg_List_Del(msg);
        GWEN_Msg_free(msg);
      }
    }
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Nothing to send");
  }
  return 0;
}




int _readCurrentMessage(GWEN_MSG_ENDPOINT2 *ep)
{
  int rv;
  uint8_t buffer[GWEN_ENDPOINT_MSGIO_BUFFERSIZE];

  DBG_INFO(GWEN_LOGDOMAIN, "Reading from endpoint %s", GWEN_MsgEndpoint2_GetName(ep));
  rv=GWEN_MsgEndpoint2_ReadFromSocket(ep, buffer, sizeof(buffer));
  if (rv<0 && rv!=GWEN_ERROR_TIMEOUT) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else if (rv==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met on read()");
    return GWEN_ERROR_IO;
  }

  rv=_distributeBufferContent(ep, buffer, rv);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _distributeBufferContent(GWEN_MSG_ENDPOINT2 *ep, const uint8_t *bufferPtr, int bufferLen)
{
  if (ep) {
    GWEN_ENDPOINT2_MSGIO *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_MSGIO, ep);
    if (xep) {
      if (xep->getBytesNeededFn) {
	GWEN_MSG *msg;

        DBG_INFO(GWEN_LOGDOMAIN, "Distributing %d received bytes", bufferLen);
	msg=GWEN_MsgEndpoint2_GetCurrentlyReceivedMsg(ep);
	while(bufferLen) {
	  int bytesNeeded;

          DBG_INFO(GWEN_LOGDOMAIN, "%d remaining bytes in buffer", bufferLen);
	  if (msg==NULL) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Creating new message");
	    msg=GWEN_Msg_new(GWEN_MsgEndpoint2_GetDefaultMessageSize(ep));
	    GWEN_Msg_SetGroupId(msg, GWEN_MsgEndpoint2_GetGroupId(ep));
	    GWEN_MsgEndpoint2_SetCurrentlyReceivedMsg(ep, msg);
	  }
      
	  bytesNeeded=xep->getBytesNeededFn(ep, msg);
          DBG_INFO(GWEN_LOGDOMAIN, "current message still needs %d bytes", bytesNeeded);
          if (bytesNeeded==0) {
            /* message finished already before adding bytes?? */
            DBG_ERROR(GWEN_LOGDOMAIN, "Incoming message complete, SNH!");
          }
	  else {
	    int rv;
      
	    /* add bytes to message */
	    if (bytesNeeded>bufferLen)
	      bytesNeeded=bufferLen;
            DBG_INFO(GWEN_LOGDOMAIN, "adding %d bytes to current message", bytesNeeded);
	    rv=GWEN_Msg_AddBytes(msg, bufferPtr, bytesNeeded);
	    if (rv<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      return rv;
	    }
            if (xep->getBytesNeededFn(ep, msg)==0) {
              /* message finished */
              DBG_INFO(GWEN_LOGDOMAIN, "Incoming message complete");
              GWEN_Msg_Attach(msg);
              GWEN_MsgEndpoint2_SetCurrentlyReceivedMsg(ep, NULL);
              GWEN_MsgEndpoint2_AddReceivedMessage(ep, msg);
              msg=NULL;
            }
            bufferPtr+=bytesNeeded;
            bufferLen-=bytesNeeded;
          }
	} /* while */
    
	return 0;
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Function \"getBytesNeeded\" not set", GWEN_MsgEndpoint2_GetName(ep));
      }
    } /* if (xep) */
  } /* if (ep) */
  return GWEN_ERROR_GENERIC;
}



