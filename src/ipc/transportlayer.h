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


#ifndef GWENHYWFAR_TRANSPORTLAYER_H
#define GWENHYWFAR_TRANSPORTLAYER_H

#include <gwenhywfar/error.h>
#include <gwenhywfar/inetsocket.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup MOD_IPC_TRANSPORTLAYER IPC Transport Layer
 * @brief Lowest IPC Layer (don't use it directly for applications or
 * libraries)
 *
 * The transport layer is the lowest layer of the IPC infra structure.
 * No application should use the function of this layer directly.
 */
/*@{*/


GWENHYWFAR_API
typedef enum {
  GWEN_TransportLayerTypeSocket=0
} GWEN_TransportLayerType;


GWENHYWFAR_API
  typedef struct GWEN_IPCTRANSPORTLAYER GWEN_IPCTRANSPORTLAYER;



GWENHYWFAR_API
const char *GWEN_IPCTransportLayer_GetAddress(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API
void GWEN_IPCTransportLayer_SetAddress(GWEN_IPCTRANSPORTLAYER *tl,
                                       const char *buffer);
GWENHYWFAR_API
int GWEN_IPCTransportLayer_GetPort(GWEN_IPCTRANSPORTLAYER *tl);
GWENHYWFAR_API
void GWEN_IPCTransportLayer_SetPort(GWEN_IPCTRANSPORTLAYER *tl, int port);
GWENHYWFAR_API
GWEN_TransportLayerType
  GWEN_IPCTransportLayer_GetType(GWEN_IPCTRANSPORTLAYER *tl);


GWENHYWFAR_API
GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayer_new();
GWENHYWFAR_API
void GWEN_IPCTransportLayer_free(GWEN_IPCTRANSPORTLAYER * tl);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_StartConnect(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_ERRORCODE
GWEN_IPCTransportLayer_FinishConnect(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Listen(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Accept(GWEN_IPCTRANSPORTLAYER *tl,
				GWEN_IPCTRANSPORTLAYER **t);
GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Disconnect(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_SOCKET*
  GWEN_IPCTransportLayer_GetReadSocket(GWEN_IPCTRANSPORTLAYER *tl);


GWENHYWFAR_API GWEN_SOCKET*
  GWEN_IPCTransportLayer_GetWriteSocket(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Read(GWEN_IPCTRANSPORTLAYER *tl,
			      char *buffer,
                              unsigned int *bsize);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Write(GWEN_IPCTRANSPORTLAYER *tl,
			       const char *buffer,
                               unsigned int *bsize);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_Flush(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_CanRead(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_CanWrite(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYWFAR_API GWEN_ERRORCODE
  GWEN_IPCTransportLayer_GetPeerAddress(GWEN_IPCTRANSPORTLAYER *tl,
                                        char *buffer,
                                        unsigned int bsize);

GWENHYWFAR_API int
  GWEN_IPCTransportLayer_GetPeerPort(GWEN_IPCTRANSPORTLAYER *tl);






GWENHYWFAR_API
  GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayerTCP_new();

GWENHYWFAR_API
  GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayerUnix_new();

/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_TRANSPORTLAYER_H */

