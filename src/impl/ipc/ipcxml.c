/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
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

#include "ipcxml_p.h"
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>


static unsigned int GWEN_IPCXMLRequest_LastId=0;


GWEN_IPCXMLREQUEST *GWEN_IPCXMLRequest_new(){
  GWEN_IPCXMLREQUEST *r;

  GWEN_NEW_OBJECT(GWEN_IPCXMLREQUEST, r);
  r->id=--GWEN_IPCXMLRequest_LastId;
  return r;
}



void GWEN_IPCXMLRequest_free(GWEN_IPCXMLREQUEST *r){
  if (r) {
    GWEN_DB_Group_free(r->db);
    free(r);
  }
}



unsigned int GWEN_IPCXMLRequest_GetId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->id;
}



unsigned int GWEN_IPCXMLRequest_GetMsgLayerId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->msgLayerId;
}



void GWEN_IPCXMLRequest_SetMsgLayerId(GWEN_IPCXMLREQUEST *r,
                                      unsigned int i){
  assert(r);
  r->msgLayerId=i;
}


unsigned int GWEN_IPCXMLRequest_GetDialogId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->dialogId;
}



void GWEN_IPCXMLRequest_SetDialogId(GWEN_IPCXMLREQUEST *r,
                                    unsigned int i){
  assert(r);
  r->dialogId=i;
}



unsigned int GWEN_IPCXMLRequest_GetMessageNumber(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->messageNumber;
}



void GWEN_IPCXMLRequest_SetMessageNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i){
  assert(r);
  r->messageNumber=i;
}



unsigned int GWEN_IPCXMLRequest_GetSegmentNumber(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->segmentNumber;
}



void GWEN_IPCXMLRequest_SetSegmentNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i){
  assert(r);
  r->segmentNumber=i;
}



GWEN_DB_NODE *GWEN_IPCXMLRequest_GetDb(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->db;
}



void GWEN_IPCXMLRequest_SetDb(GWEN_IPCXMLREQUEST *r,
                              GWEN_DB_NODE *db){
  assert(r);
  GWEN_DB_Group_free(r->db);
  r->db=db;
}






