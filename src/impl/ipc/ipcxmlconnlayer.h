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


#include <gwenhywfar/connlayer.h>
#include <gwenhywfar/transportlayer.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/keymanager.h>
#include <gwenhywfar/hbcimsg.h>

#include <gwenhywfar/ipcxml.h>


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
                                            const char *localName,
                                            GWEN_SECCTX_MANAGER *scm,
                                            GWEN_IPCMSGLAYER *ml,
                                            int active);

GWEN_MSGENGINE *GWEN_IPCXMLConnLayer_GetMsgEngine(GWEN_IPCCONNLAYER *cl);
unsigned int GWEN_IPCXMLConnLayer_GetFlags(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetFlags(GWEN_IPCCONNLAYER *cl,
                                   unsigned int f);


GWEN_ERRORCODE
  GWEN_IPCXMLConnLayer_SetSecurityEnv(GWEN_IPCCONNLAYER *cl,
                                      const GWEN_KEYSPEC *signer,
                                      const GWEN_KEYSPEC *crypter);

GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Flush(GWEN_IPCCONNLAYER *cl);

GWEN_IPCXMLREQUEST *GWEN_IPCXMLConnLayer_AddRequest(GWEN_IPCCONNLAYER *cl,
                                                    GWEN_XMLNODE *node,
                                                    GWEN_DB_NODE *db,
                                                    int flush);

GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_IPC2HBCI(GWEN_IPCCONNLAYER *cl,
                                            GWEN_IPCMSG *msg);

const char *GWEN_IPCXMLConnLayer_GetLocalName(GWEN_IPCCONNLAYER *cl);

const char *GWEN_IPCXMLConnLayer_GetRemoteName(GWEN_IPCCONNLAYER *cl);
void GWEN_IPCXMLConnLayer_SetRemoteName(GWEN_IPCCONNLAYER *cl,
                                        const char *s);

#endif /* GWEN_IPCXMLCONNLAYER_H */



