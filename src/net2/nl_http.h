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


#ifndef GWEN_NL_HTTP_H
#define GWEN_NL_HTTP_H

#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/inetsocket.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/url.h>


#define GWEN_NL_HTTP_NAME "Http"


/*
 * This module handles the HTTP protocol.
 * When reading the body of an HTTP message the transfer encoding type
 * "chunked" is automatically detected and handled transparently.
 */


/**
 * Use a connection for IPC purposes.
 */
#define GWEN_NL_HTTP_FLAGS_IPC   0x00000001


typedef enum {
  GWEN_NetLayerHttpVersion_1_0=0,
  GWEN_NetLayerHttpVersion_1_1
} GWEN_NETLAYER_HTTP_VERSION;


GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayerHttp_new(GWEN_NETLAYER *baseLayer);



GWENHYWFAR_API
void GWEN_NetLayerHttp_SetOutCommand(GWEN_NETLAYER *nl,
                                     const char *command,
                                     const GWEN_URL *url);

GWENHYWFAR_API
void GWEN_NetLayerHttp_SetOutStatus(GWEN_NETLAYER *nl,
                                    int code,
                                    const char *text);

GWENHYWFAR_API
GWEN_DB_NODE *GWEN_NetLayerHttp_GetOutHeader(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
GWEN_DB_NODE *GWEN_NetLayerHttp_GetInHeader(const GWEN_NETLAYER *nl);

GWENHYWFAR_API
int GWEN_NetLayerHttp_GetInStatusCode(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
const char *GWEN_NetLayerHttp_GetInStatusText(const GWEN_NETLAYER *nl);


GWENHYWFAR_API
int GWEN_NetLayerHttp_Request(GWEN_NETLAYER *nl,
                              const char *command,
                              const GWEN_URL *url,
                              GWEN_DB_NODE *dbHeader,
                              const char *pBody,
                              int lBody,
                              GWEN_BUFFEREDIO *bio);

#endif /* GWEN_NL_HTTP_H */


