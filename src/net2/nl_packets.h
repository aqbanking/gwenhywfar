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


#ifndef GWEN_NL_PACKETS_H
#define GWEN_NL_PACKETS_H

#include <gwenhywfar/netlayer.h>

#include <gwenhywfar/buffer.h>

#define GWEN_NL_PACKETS_NAME "Packets"

/*
 * This module sends and receives packets over any other NetLayer.
 */

typedef enum {
  GWEN_NL_PacketStatus_New=0,
  GWEN_NL_PacketStatus_Enqueued,
  GWEN_NL_PacketStatus_StartReadMsg,
  GWEN_NL_PacketStatus_ReadMsg,
  GWEN_NL_PacketStatus_StartWriteMsg,
  GWEN_NL_PacketStatus_WriteMsg,
  GWEN_NL_PacketStatus_EndWriteMsg,
  GWEN_NL_PacketStatus_Finished,
  GWEN_NL_PacketStatus_Aborted
} GWEN_NL_PACKET_STATUS;

typedef struct GWEN_NL_PACKET GWEN_NL_PACKET;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_NL_PACKET, GWENHYWFAR_API)


GWENHYWFAR_API
GWEN_NL_PACKET *GWEN_NL_Packet_new();

GWENHYWFAR_API
void GWEN_NL_Packet_free(GWEN_NL_PACKET *pk);

GWENHYWFAR_API
void GWEN_NL_Packet_Attach(GWEN_NL_PACKET *pk);

GWENHYWFAR_API
GWEN_NL_PACKET_STATUS GWEN_NL_Packet_GetStatus(const GWEN_NL_PACKET *pk);

GWENHYWFAR_API
GWEN_BUFFER *GWEN_NL_Packet_GetBuffer(const GWEN_NL_PACKET *pk);

GWENHYWFAR_API
GWEN_BUFFER *GWEN_NL_Packet_TakeBuffer(GWEN_NL_PACKET *pk);

GWENHYWFAR_API
void GWEN_NL_Packet_SetBuffer(GWEN_NL_PACKET *pk, GWEN_BUFFER *buf);


GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayerPackets_new(GWEN_NETLAYER *baseLayer);

GWENHYWFAR_API
unsigned int GWEN_NetLayerPackets_GetFixedPacketSize(const GWEN_NETLAYER *nl);

GWENHYWFAR_API
void GWEN_NetLayerPackets_SetFixedPacketSize(GWEN_NETLAYER *nl,
                                             unsigned int i);

GWENHYWFAR_API
int GWEN_NetLayerPackets_SendPacket(GWEN_NETLAYER *nl,
                                    GWEN_NL_PACKET *packet);

GWENHYWFAR_API
int GWEN_NetLayerPackets_Flush(GWEN_NETLAYER *nl, int timeout);

GWENHYWFAR_API
int GWEN_NetLayerPackets_HasNextPacket(const GWEN_NETLAYER *nl);

GWENHYWFAR_API
GWEN_NL_PACKET *GWEN_NetLayerPackets_GetNextPacket(GWEN_NETLAYER *nl);

GWENHYWFAR_API
GWEN_NL_PACKET *GWEN_NetLayerPackets_GetNextPacket_Wait(GWEN_NETLAYER *nl,
                                                        int timeout);

#endif /* GWEN_NL_PACKETS_P_H */


