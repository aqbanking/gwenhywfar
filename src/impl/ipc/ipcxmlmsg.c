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


#include "ipcxmlmsg_p.h"
#include <gwenhyfwar/misc.h>


GWEN_IPCXML_MSG *GWEN_IPCXML_Msg_new(){
  GWEN_IPCXML_MSG *m;

  GWEN_NEW_OBJECT(GWEN_IPCXML_MSG, m);
  return m;
}



void GWEN_IPCXML_Msg_free(GWEN_IPCXML_MSG *m){
  if (m) {
    free(m->name);
    GWEN_DB_Group_free(m->data);
    GWEN_IPCXML_KeySpec_Clear(&(m->signers));
    GWEN_IPCXML_KeySpec_free(m->crypter);
    free(m);
  }
}



GWEN_IPCXML_MSG *GWEN_IPCXML_Msg_dup(GWEN_IPCXML_MSG *m){
  GWEN_IPCXML_MSG *newM;
  GWEN_IPCXMLKEYSPEC *ks;

  assert(m);
  newM=GWEN_IPCXML_Msg_new();
  if (m->name)
    newM->name=strdup(m->name);
  newM->firstSegment=m->firstSegment;
  newM->lastSegment=m->lastSegment;
  newM->version=m->version;
  newM->peerId=m->peerId;
  newM->msgId=m->msgId;
  newM->refId=m->refId;
  newM->nSigners=m->nSigners;
  if (m->data)
    newM->data=GWEN_DB_Group_dup(m->data);

  ks=m->signers;
  while(ks) {
    GWEN_IPCXML_KeySpec_Add(GWEN_IPCXML_KeySpec_dup(ks), &(newM->signers));
    ks=GWEN_IPCXML_KeySpec_Next(ks);
  } /* while */

  if (m->crypter)
    newM->crypter=GWEN_IPCXML_KeySpec_dup(m->crypter);

  return newM;
}



const char *GWEN_IPCXML_Msg_GetName(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->name;
}



void GWEN_IPCXML_Msg_SetName(GWEN_IPCXML_MSG *m,
                             const char *name){
  assert(m);
  free(m->name);
  m->name=strdup(name);
}



unsigned int GWEN_IPCXML_Msg_GetVersion(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->version;
}



void GWEN_IPCXML_Msg_SetVersion(GWEN_IPCXML_MSG *m,
                                unsigned int version){
  assert(m);
  m->version=version;
}



unsigned int GWEN_IPCXML_Msg_GetPeerId(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->peerId;
}



void GWEN_IPCXML_Msg_SetPeerId(GWEN_IPCXML_MSG *m,
                               unsigned int peerid){
  assert(m);
  m->peerId=peerid;
}



unsigned int GWEN_IPCXML_Msg_GetSignerCount(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->nSigners;
}



GWEN_DB_NODE *GWEN_IPCXML_Msg_GetData(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->data;
}



void GWEN_IPCXML_Msg_SetData(GWEN_IPCXML_MSG *m,
                             GWEN_DB_NODE *d){
  assert(m);
  GWEN_DB_Group_free(m->data);
  m->data=d;
}



unsigned int GWEN_IPCXML_Msg_GetId(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->msgId;
}



void GWEN_IPCXML_Msg_SetId(GWEN_IPCXML_MSG *m,
                           unsigned int id){
  assert(m);
  m->msgId=id;
}



unsigned int GWEN_IPCXML_Msg_GetRefId(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->refId;
}



void GWEN_IPCXML_Msg_SetRefId(GWEN_IPCXML_MSG *m,
                              unsigned int id){
  assert(m);
  m->refId=id;
}



void GWEN_IPCXML_Msg_Add(GWEN_IPCXML_MSG *m,
                         GWEN_IPCXML_MSG **head){
  assert(m);
  assert(head);
  GWEN_LIST_ADD(GWEN_IPCXML_MSG, m, head);
}



void GWEN_IPCXML_Msg_Del(GWEN_IPCXML_MSG *m,
                         GWEN_IPCXML_MSG **head){
  assert(m);
  assert(head);
  GWEN_LIST_DEL(GWEN_IPCXML_MSG, m, head);
}



GWEN_IPCXML_MSG *GWEN_IPCXML_Msg_Next(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->next;
}



GWEN_IPCXMLKEYSPEC *GWEN_IPCXML_Msg_GetSigners(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->signers;
}



void GWEN_IPCXML_Msg_AddSigner(GWEN_IPCXML_MSG *m,
                               GWEN_IPCXMLKEYSPEC *ks){
  assert(m);
  GWEN_IPCXML_KeySpec_Add(ks, &(m->signers));
  m->nSigners++;
}



GWEN_IPCXMLKEYSPEC *GWEN_IPCXML_Msg_GetCrypter(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->crypter;
}



void GWEN_IPCXML_Msg_SetCrypter(GWEN_IPCXML_MSG *m,
                                GWEN_IPCXMLKEYSPEC *ks){
  assert(m);

  GWEN_IPCXML_KeySpec_free(m->crypter);
  m->crypter=ks;
}



unsigned int GWEN_IPCXML_Msg_GetFirstSegment(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->firstSegment;
}



void GWEN_IPCXML_Msg_SetFirstSegment(GWEN_IPCXML_MSG *m,
                                     unsigned int i){
  assert(m);
  m->firstSegment=i;
}



unsigned int GWEN_IPCXML_Msg_GetLastSegment(GWEN_IPCXML_MSG *m){
  assert(m);
  return m->lastSegment;
}



void GWEN_IPCXML_Msg_SetLastSegment(GWEN_IPCXML_MSG *m,
                                    unsigned int i){
  assert(m);
  m->lastSegment=i;
}










