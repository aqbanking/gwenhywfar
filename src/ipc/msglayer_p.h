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


#ifndef GWENHYFWAR_MSGLAYER_P_H
#define GWENHYFWAR_MSGLAYER_P_H

#define GWEN_IPCMSGLAYER_MAXINCOMING_MSGS 32
#define GWEN_IPCMSGLAYER_MAXOUTGOING_MSGS 1

#include <gwenhyfwar/msglayer.h>
#include <gwenhyfwar/transportlayer.h>
#include <gwenhyfwar/error.h>



struct GWEN_IPCMSG {
  GWEN_IPCMSG *next;
  GWEN_BUFFER *buffer;
  unsigned int msgLayerId;
  unsigned int msgId;
  unsigned int refId;
  void *data;
  GWEN_IPCMSG_FREE freeDataFn;
};



struct GWEN_IPCMSGLAYER {
  GWEN_IPCMSGLAYER *next;
  GWEN_IPCTRANSPORTLAYER *transportLayer;
  unsigned int id;
  GWEN_IPCMSGLAYER_STATE state;
  GWEN_IPCMSG *incomingMsgs;
  unsigned int nIncomingMsgs;
  unsigned int maxIncomingMsgs;
  GWEN_IPCMSG *outgoingMsgs;
  unsigned int nOutgoingMsgs;
  unsigned int maxOutgoingMsgs;
  void *data;
  GWEN_IPCMSGLAYER_WORK workFn;
  GWEN_IPCMSGLAYER_FREE freeDataFn;
  GWEN_IPCMSGLAYER_ACCEPT acceptFn;
};




#endif /* GWENHYFWAR_MSGLAYER_P_H */



