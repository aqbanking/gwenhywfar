/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Sep 14 2003
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


#ifndef GWENHYFWAR_TRANSPORTLAYER_H
#define GWENHYFWAR_TRANSPORTLAYER_H

#include <gwenhywfar/error.h>
#include <gwenhywfar/inetsocket.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
  GWEN_TransportLayerTypeSocket=0
} GWEN_TransportLayerType;


GWENHYFWAR_API
  typedef struct GWEN_IPCTRANSPORTLAYER GWEN_IPCTRANSPORTLAYER;



const char *GWEN_IPCTransportLayer_GetAddress(GWEN_IPCTRANSPORTLAYER *tl);

void GWEN_IPCTransportLayer_SetAddress(GWEN_IPCTRANSPORTLAYER *tl,
                                       const char *buffer);
int GWEN_IPCTransportLayer_GetPort(GWEN_IPCTRANSPORTLAYER *tl);
void GWEN_IPCTransportLayer_SetPort(GWEN_IPCTRANSPORTLAYER *tl, int port);
GWEN_TransportLayerType
  GWEN_IPCTransportLayer_GetType(GWEN_IPCTRANSPORTLAYER *tl);


GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayer_new();
void GWEN_IPCTransportLayer_free(GWEN_IPCTRANSPORTLAYER * tl);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_StartConnect(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_ERRORCODE
GWEN_IPCTransportLayer_FinishConnect(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Listen(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Accept(GWEN_IPCTRANSPORTLAYER *tl,
				GWEN_IPCTRANSPORTLAYER **t);
GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Disconnect(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_SOCKET*
  GWEN_IPCTransportLayer_GetReadSocket(GWEN_IPCTRANSPORTLAYER *tl);


GWENHYFWAR_API GWEN_SOCKET*
  GWEN_IPCTransportLayer_GetWriteSocket(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Read(GWEN_IPCTRANSPORTLAYER *tl,
			      char *buffer,
                              unsigned int *bsize);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Write(GWEN_IPCTRANSPORTLAYER *tl,
			       const char *buffer,
                               unsigned int *bsize);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Flush(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_CanRead(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_CanWrite(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_GetPeerAddress(GWEN_IPCTRANSPORTLAYER *tl,
                                        char *buffer,
                                        unsigned int bsize);

GWENHYFWAR_API int
  GWEN_IPCTransportLayer_GetPeerPort(GWEN_IPCTRANSPORTLAYER *tl);






GWENHYFWAR_API
  GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayerTCP_new();

GWENHYFWAR_API
  GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayerUnix_new();


#ifdef __cplusplus
}
#endif


#endif /* GWENHYFWAR_TRANSPORTLAYER_H */

