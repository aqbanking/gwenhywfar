/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
    copyright   : (C) 2004 by Martin Preuss
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


#ifndef GWEN_NETCONNECTION_P_H
#define GWEN_NETCONNECTION_P_H

/** ringbuffer size */
#define GWEN_NETCONNECTION_BUFFERSIZE 512
/** sleep 200 ms in wait function when there are no sockets to select */
#define GWEN_NETCONNECTION_CPU_TIMEOUT 200

#include <gwenhywfar/netconnection.h>
#include <gwenhywfar/netmsg.h>


#define GWEN_NETCONNECTION_NOTIFIED_UP   0x0001
#define GWEN_NETCONNECTION_NOTIFIED_DOWN 0x0002


struct GWEN_NETCONNECTION {
  GWEN_LIST_ELEMENT(GWEN_NETCONNECTION);
  GWEN_INHERIT_ELEMENT(GWEN_NETCONNECTION);

  GWEN_TYPE_UINT32 usage;

  int downAfterSend;

  GWEN_RINGBUFFER *readBuffer;
  GWEN_RINGBUFFER *writeBuffer;
  GWEN_NETTRANSPORT_RESULT lastResult;
  GWEN_NETTRANSPORT *transportLayer;
  int takeTransport;
  GWEN_NETCONNECTION_WORKFN workFn;
  GWEN_NETCONNECTION_UPFN upFn;
  GWEN_NETCONNECTION_DOWNFN downFn;

  GWEN_TYPE_UINT32 notified;

  GWEN_NETMSG_LIST *inMsgs;
  GWEN_NETMSG_LIST *outMsgs;

  GWEN_TYPE_UINT32 libraryMark;
  GWEN_TYPE_UINT32 userMark;
};


/**
 * Waits up to given amount of milliseconds for activity/availability.
 *
 * @return 0 if there is activity on any socket, -1 on error and 1 on timeout
 * @param timeout timeout in seconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 * @param waitFlags see @ref GWEN_NETCONNECTION_WAIT_READ
 */
int GWEN_NetConnection_Wait(GWEN_NETCONNECTION *conn,
                            int timeout,
                            GWEN_TYPE_UINT32 waitFlags);

/**
 * Lets a list of connections work.
 */
GWEN_NETCONNECTION_WORKRESULT
  GWEN_NetConnection__Walk(GWEN_NETCONNECTION_LIST *connList,
                           int timeout);


#endif
