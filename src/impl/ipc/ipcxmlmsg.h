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


#ifndef GWENHYFWAR_IPCXMLMSG_H
#define GWENHYFWAR_IPCXMLMSG_H

#include <gwenhyfwar/db.h>



typedef struct GWEN_IPCXML_MSG GWEN_IPCXML_MSG;




GWEN_IPCXML_MSG *GWEN_IPCXML_Msg_new();
void GWEN_IPCXML_Msg_free(GWEN_IPCXML_MSG *m);
GWEN_IPCXML_MSG *GWEN_IPCXML_Msg_dup(GWEN_IPCXML_MSG *m);


const char *GWEN_IPCXML_Msg_GetName(GWEN_IPCXML_MSG *m);
void GWEN_IPCXML_Msg_SetName(GWEN_IPCXML_MSG *m,
                             const char *name);
unsigned int GWEN_IPCXML_Msg_GetVersion(GWEN_IPCXML_MSG *m);
void GWEN_IPCXML_Msg_SetVersion(GWEN_IPCXML_MSG *m,
                                unsigned int version);

unsigned int GWEN_IPCXML_Msg_GetId(GWEN_IPCXML_MSG *m);
void GWEN_IPCXML_Msg_SetId(GWEN_IPCXML_MSG *m,
                           unsigned int id);

unsigned int GWEN_IPCXML_Msg_GetRefId(GWEN_IPCXML_MSG *m);
void GWEN_IPCXML_Msg_SetRefId(GWEN_IPCXML_MSG *m,
                              unsigned int id);


unsigned int GWEN_IPCXML_Msg_GetPeerId(GWEN_IPCXML_MSG *m);
void GWEN_IPCXML_Msg_SetPeerId(GWEN_IPCXML_MSG *m,
                               unsigned int peerid);

unsigned int GWEN_IPCXML_Msg_GetSignerCount(GWEN_IPCXML_MSG *m);


GWEN_DB_NODE *GWEN_IPCXML_Msg_GetData(GWEN_IPCXML_MSG *m);
void GWEN_IPCXML_Msg_SetData(GWEN_IPCXML_MSG *m,
                             GWEN_DB_NODE *d);

void GWEN_IPCXML_Msg_Add(GWEN_IPCXML_MSG *m,
                         GWEN_IPCXML_MSG **head);

void GWEN_IPCXML_Msg_Del(GWEN_IPCXML_MSG *m,
                         GWEN_IPCXML_MSG **head);

GWEN_IPCXML_MSG *GWEN_IPCXML_Msg_Next(GWEN_IPCXML_MSG *m);


#endif

