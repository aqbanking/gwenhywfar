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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "netmsg_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>


GWEN_LIST_FUNCTIONS(GWEN_NETMSG, GWEN_NetMsg);



GWEN_NETMSG *GWEN_NetMsg_new(GWEN_TYPE_UINT32 bufferSize){
  GWEN_NETMSG *msg;

  GWEN_NEW_OBJECT(GWEN_NETMSG, msg);
  GWEN_LIST_INIT(GWEN_NETMSG, msg);
  if (bufferSize)
    msg->buffer=GWEN_Buffer_new(0, bufferSize, 0, 1);
  msg->node=GWEN_DB_Group_new("NetMsg");

  msg->usage=1;
  return msg;
}



void GWEN_NetMsg_free(GWEN_NETMSG *msg){
  if (msg) {
    assert(msg->usage);
    if (--(msg->usage)==0) {
      GWEN_DB_Group_free(msg->node);
      GWEN_Buffer_free(msg->buffer);

      if (msg->bio) {
        GWEN_BufferedIO_Abandon(msg->bio);
        GWEN_BufferedIO_free(msg->bio);
      }

      GWEN_LIST_FINI(GWEN_NETMSG, msg);
      free(msg);
    }
  }
}



void GWEN_NetMsg_Attach(GWEN_NETMSG *msg){
  assert(msg);
  msg->usage++;
}



GWEN_BUFFER *GWEN_NetMsg_GetBuffer(const GWEN_NETMSG *msg){
  assert(msg);
  return msg->buffer;
}



GWEN_BUFFER *GWEN_NetMsg_TakeBuffer(GWEN_NETMSG *msg){
  GWEN_BUFFER *buf;

  assert(msg);
  buf=msg->buffer;
  msg->buffer=0;
  return buf;
}



void GWEN_NetMsg_SetBuffer(GWEN_NETMSG *msg,
                           GWEN_BUFFER *buf){
  assert(msg);
  assert(buf);
  GWEN_Buffer_free(msg->buffer);
  msg->buffer=buf;
}



GWEN_TYPE_UINT32 GWEN_NetMsg_GetSize(const GWEN_NETMSG *msg){
  assert(msg);
  return msg->size;
}



void GWEN_NetMsg_SetSize(GWEN_NETMSG *msg,
                         GWEN_TYPE_UINT32 size){
  assert(msg);
  msg->size=size;
}



void GWEN_NetMsg_DecrementSize(GWEN_NETMSG *msg,
                               GWEN_TYPE_UINT32 offs){
  assert(msg);
  if (msg->size>=offs)
    msg->size-=offs;
}



void GWEN_NetMsg_IncrementSize(GWEN_NETMSG *msg,
                               GWEN_TYPE_UINT32 offs){
  assert(msg);
  msg->size+=offs;
}



GWEN_DB_NODE *GWEN_NetMsg_GetDB(const GWEN_NETMSG *msg){
  assert(msg);
  return msg->node;
}



GWEN_BUFFEREDIO *GWEN_NetMsg_GetBufferedIO(const GWEN_NETMSG *msg){
  assert(msg);
  return msg->bio;
}



GWEN_BUFFEREDIO *GWEN_NetMsg_TakeBufferedIO(GWEN_NETMSG *msg){
  GWEN_BUFFEREDIO *bio;

  assert(msg);
  bio=msg->bio;
  msg->bio=0;
  return bio;
}



void GWEN_NetMsg_SetBufferedIO(GWEN_NETMSG *msg, GWEN_BUFFEREDIO *bio){
  assert(msg);
  assert(bio);
  if (msg->bio) {
    GWEN_BufferedIO_Abandon(msg->bio);
    GWEN_BufferedIO_free(msg->bio);
  }
  msg->bio=bio;
}



int GWEN_NetMsg_GetProtocolMajorVersion(const GWEN_NETMSG *msg){
  assert(msg);
  return msg->pmajor;
}



int GWEN_NetMsg_GetProtocolMinorVersion(const GWEN_NETMSG *msg){
  assert(msg);
  return msg->pminor;
}



void GWEN_NetMsg_SetProtocolVersion(GWEN_NETMSG *msg,
                                    int pmajor, int pminor){
  assert(msg);
  msg->pmajor=pmajor;
  msg->pminor=pminor;
}











