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


#ifndef GWENHYWFAR_IPCXML_H
#define GWENHYWFAR_IPCXML_H

#include <gwenhywfar/db.h>
#include <gwenhywfar/servicelayer.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/keymanager.h>
#include <gwenhywfar/ipcxmlsecctx.h>


typedef struct GWEN_IPCXMLREQUEST GWEN_IPCXMLREQUEST;
typedef struct GWEN_IPCXMLSERVICE GWEN_IPCXMLSERVICE;


typedef enum {
  GWEN_IPCXMLServiceTypeUnknown=0,
  GWEN_IPCXMLServiceTypeTCP,
  GWEN_IPCXMLServiceTypeUnix
} GWEN_IPCXMLSERVICE_TYPE;


GWEN_IPCXMLREQUEST *GWEN_IPCXMLRequest_new();
void GWEN_IPCXMLRequest_free(GWEN_IPCXMLREQUEST *r);

unsigned int GWEN_IPCXMLRequest_GetId(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetId(GWEN_IPCXMLREQUEST *r,
                              unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetMsgLayerId(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetMsgLayerId(GWEN_IPCXMLREQUEST *r,
                                      unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetDialogId(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetDialogId(GWEN_IPCXMLREQUEST *r,
                                    unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetMessageNumber(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetMessageNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetSegmentNumber(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetSegmentNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i);

GWEN_DB_NODE *GWEN_IPCXMLRequest_GetDb(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetDb(GWEN_IPCXMLREQUEST *r,
                              GWEN_DB_NODE *db);





GWEN_IPCXMLSERVICE *GWEN_IPCXMLService_new(GWEN_MSGENGINE *msgEngine,
                                           GWEN_SECCTX_MANAGER *scm);

void GWEN_IPCXMLService_free(GWEN_IPCXMLSERVICE *xs);

/**
 * @param localContext name of the local context. This is the name of the
 * owner of the local keys. These keys are looked up in the service's
 * context manager.
 */
unsigned int GWEN_IPCXMLService_AddServer(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          const char *localContext,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags);

unsigned int GWEN_IPCXMLService_AddClient(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          const char *localContext,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags);

unsigned int GWEN_IPCXMLService_AddRequest(GWEN_IPCXMLSERVICE *xs,
                                           unsigned int clid,
                                           const char *requestName,
                                           unsigned int requestVersion,
                                           GWEN_DB_NODE *db,
                                           int flush);

GWEN_ERRORCODE GWEN_IPCXMLService_SetSecurityEnv(GWEN_IPCXMLSERVICE *xs,
                                                 unsigned int clid,
                                                 const GWEN_KEYSPEC *signer,
                                                 const GWEN_KEYSPEC *crypter);

const char *GWEN_IPCXMLService_GetLocalName(GWEN_IPCXMLSERVICE *xs,
                                            unsigned int clid);
const char *GWEN_IPCXMLService_GetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid);
void GWEN_IPCXMLService_SetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                      unsigned int clid,
                                      const char *s);


GWEN_ERRORCODE GWEN_IPCXMLService_Work(GWEN_IPCXMLSERVICE *xs,
                                       int timeout);

GWEN_ERRORCODE GWEN_IPCXMLService_HandleMsgs(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int userMark,
                                             int maxmsgs);




#endif



