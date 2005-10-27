/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Jan 24 2004
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


#ifndef GWEN_NETLAYER_P_H
#define GWEN_NETLAYER_P_H

#include "netlayer_l.h"

#define GWEN_NETLAYER_CPU_TIMEOUT 200


struct GWEN_NETLAYER {
  GWEN_INHERIT_ELEMENT(GWEN_NETLAYER)
  GWEN_LIST_ELEMENT(GWEN_NETLAYER)

  char *typeName;
  GWEN_NETLAYER_STATUS status;
  time_t lastStatusChange;

  GWEN_TYPE_UINT32 flags;

  GWEN_NETLAYER_LIST *incomingLayers;

  GWEN_NETLAYER *baseLayer;
  GWEN_NETLAYER *parentLayer;

  GWEN_NETLAYER_WORK_FN workFn;
  GWEN_NETLAYER_READ_FN readFn;
  GWEN_NETLAYER_WRITE_FN writeFn;

  GWEN_NETLAYER_CONNECT_FN connectFn;
  GWEN_NETLAYER_DISCONNECT_FN disconnectFn;
  GWEN_NETLAYER_LISTEN_FN listenFn;

  GWEN_NETLAYER_ADDSOCKETS_FN addSocketsFn;
  GWEN_NETLAYER_BASESTATUS_CHG_FN baseStatusChangeFn;

  GWEN_NETLAYER_BEGIN_OUT_PACKET_FN beginOutPacketFn;
  GWEN_NETLAYER_END_OUT_PACKET_FN endOutPacketFn;
  GWEN_NETLAYER_BEGIN_IN_PACKET_FN beginInPacketFn;
  GWEN_NETLAYER_CHECK_IN_PACKET_FN checkInPacketFn;

  GWEN_INETADDRESS *localAddr;
  GWEN_INETADDRESS *peerAddr;

  int backLog;

  int usage;
};


GWEN_NETLAYER_RESULT GWEN_NetLayer__Wait(GWEN_NETLAYER_LIST *nll,
                                         int timeout);

GWEN_NETLAYER_RESULT GWEN_NetLayer__WorkAll(GWEN_NETLAYER_LIST *nll);



#endif /* GWEN_NETLAYER_P_H */


