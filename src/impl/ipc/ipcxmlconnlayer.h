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


#ifndef GWEN_IPCXMLCONNLAYER_H
#define GWEN_IPCXMLCONNLAYER_H


#include <gwenhyfwar/connlayer.h>
#include <gwenhyfwar/transportlayer.h>
#include <gwenhyfwar/msgengine.h>
#include <gwenhyfwar/crypt.h>


typedef struct GWEN_IPCXMLCONNLAYERDATA GWEN_IPCXMLCONNLAYERDATA;


typedef GWEN_ERRORCODE
  (*GWEN_IPCXMLCONNLAYER_ENCODE_FN)(GWEN_IPCCONNLAYER *cl,
                                    GWEN_BUFFER **buffer,
                                    int crypt, int sign);

typedef GWEN_ERRORCODE
  (*GWEN_IPCXMLCONNLAYER_DECODE_FN)(GWEN_IPCCONNLAYER *cl,
                                    GWEN_DB_NODE *dn);



GWEN_IPCXMLCONNLAYERDATA *GWEN_IPCXMLConnLayerData_new();
void GWEN_IPCXMLConnLayerData_free(GWEN_IPCXMLCONNLAYERDATA *ccd);


GWEN_IPCCONNLAYER *GWEN_IPCXMLConnLayer_new(GWEN_MSGENGINE *msgEngine,
                                            GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCMSGLAYER_STATE st);

GWEN_MSGENGINE *GWEN_IPCXMLConnLayer_GetMsgEngine(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetMsgEngine(GWEN_IPCCONNLAYER *cl,
                                       GWEN_MSGENGINE *e);
unsigned int GWEN_IPCXMLConnLayer_GetSecurityState(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetSecurityState(GWEN_IPCCONNLAYER *cl,
                                           unsigned int s);
unsigned int GWEN_IPCXMLConnLayer_GetFlags(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetFlags(GWEN_IPCCONNLAYER *cl,
                                   unsigned int f);
GWEN_CRYPTKEY *GWEN_IPCXMLConnLayer_GetLocalKey(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetLocalKey(GWEN_IPCCONNLAYER *cl,
                                      GWEN_CRYPTKEY *k);
GWEN_CRYPTKEY *GWEN_IPCXMLConnLayer_GetRemoteKey(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetRemoteKey(GWEN_IPCCONNLAYER *cl,
                                       GWEN_CRYPTKEY *k);
GWEN_CRYPTKEY *GWEN_IPCXMLConnLayer_GetSessionKey(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetSessionKey(GWEN_IPCCONNLAYER *cl,
                                        GWEN_CRYPTKEY *k);

const char *GWEN_IPCXMLConnLayer_GetPeerName(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetPeerName(GWEN_IPCCONNLAYER *cl,
                                      const char *s);
const char *GWEN_IPCXMLConnLayer_GetPeerVersion(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetPeerVersion(GWEN_IPCCONNLAYER *cl,
                                         const char *s);

const char *GWEN_IPCXMLConnLayer_GetOwnName(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetOwnName(GWEN_IPCCONNLAYER *cl,
                                     const char *s);
const char *GWEN_IPCXMLConnLayer_GetOwnVersion(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetOwnVersion(GWEN_IPCCONNLAYER *cl,
                                        const char *s);







#endif /* GWEN_IPCXMLCONNLAYER_H */



