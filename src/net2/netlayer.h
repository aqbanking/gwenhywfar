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


#ifndef GWEN_NETLAYER_H
#define GWEN_NETLAYER_H


#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/inetaddr.h>
#include <gwenhywfar/inetsocket.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/bufferedio.h>

#include <time.h>


typedef struct GWEN_NETLAYER GWEN_NETLAYER;

GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_NETLAYER, GWEN_NetLayer, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_NETLAYER, GWENHYWFAR_API)

/** @name Flags
 *
 * The upper 16 bits of the flags are used by this module. The lower 16 bits
 * can be used by each inheriting netlayer. These bits are considered specific
 * to the inheriting netlayers, so e.g. bit 0 might have a different meaning
 * for HTTP netlayers than it does for HBCI netlayers.
 */
/*@{*/
/** mask for netlayer specific flags */
#define GWEN_NETLAYER_FLAGS_SPECMASK   0x000000FF

#define GWEN_NETLAYER_FLAGS_EOFMET     0x80000000
#define GWEN_NETLAYER_FLAGS_BROKENPIPE 0x40000000
#define GWEN_NETLAYER_FLAGS_WANTREAD   0x20000000
#define GWEN_NETLAYER_FLAGS_WANTWRITE  0x10000000
#define GWEN_NETLAYER_FLAGS_PASSIVE    0x08000000
#define GWEN_NETLAYER_FLAGS_PKG_BASED  0x04000000
#define GWEN_NETLAYER_FLAGS_INPKG      0x02000000
#define GWEN_NETLAYER_FLAGS_OUTPKG     0x01000000
/*@}*/


#define GWEN_NETLAYER_DEF_BACKLOG 10


typedef enum {
  GWEN_NetLayerStatus_Unconnected=0,
  GWEN_NetLayerStatus_Connecting,
  GWEN_NetLayerStatus_Connected,
  GWEN_NetLayerStatus_Disconnecting,
  GWEN_NetLayerStatus_Disconnected,
  GWEN_NetLayerStatus_Listening,
  GWEN_NetLayerStatus_Disabled,
  GWEN_NetLayerStatus_Unknown=999
} GWEN_NETLAYER_STATUS;

GWENHYWFAR_API
GWEN_NETLAYER_STATUS GWEN_NetLayerStatus_fromString(const char *s);
GWENHYWFAR_API
const char *GWEN_NetLayerStatus_toString(GWEN_NETLAYER_STATUS st);


typedef enum {
  GWEN_NetLayerResult_Idle=0,
  GWEN_NetLayerResult_Changed,
  GWEN_NetLayerResult_WouldBlock,
  GWEN_NetLayerResult_Error,
  GWEN_NetLayerResult_Unknown=999
} GWEN_NETLAYER_RESULT;

GWENHYWFAR_API
GWEN_NETLAYER_RESULT GWEN_NetLayerResult_fromString(const char *s);
GWENHYWFAR_API
const char *GWEN_NetLayerResult_toString(GWEN_NETLAYER_RESULT res);


/**
 * This is the prototype for the callback function which asks the user
 * for a password.
 * @param nl GWEN_NetLayer involved
 * @param buffer destination buffer for the password
 * @param num size of the password buffer
 * @param rwflag if 1 then the password is to be created (in this case
 * the function should let the user verify the password before writing
 * it into the buffer).
 */
typedef int (*GWEN_NETLAYER_GETPASSWD_FN)(GWEN_NETLAYER *nl,
                                          char *buffer, int num,
                                          int rwflag);


typedef GWEN_NETLAYER_RESULT (*GWEN_NETLAYER_WORK_FN)(GWEN_NETLAYER *nl);


typedef int (*GWEN_NETLAYER_READ_FN)(GWEN_NETLAYER *nl,
                                     char *buffer,
                                     int *bsize);

typedef int (*GWEN_NETLAYER_WRITE_FN)(GWEN_NETLAYER *nl,
                                      const char *buffer,
                                      int *bsize);

typedef int (*GWEN_NETLAYER_CONNECT_FN)(GWEN_NETLAYER *nl);

typedef int (*GWEN_NETLAYER_DISCONNECT_FN)(GWEN_NETLAYER *nl);

typedef int (*GWEN_NETLAYER_LISTEN_FN)(GWEN_NETLAYER *nl);

typedef int (*GWEN_NETLAYER_ADDSOCKETS_FN)(GWEN_NETLAYER *nl,
                                           GWEN_SOCKETSET *readSet,
                                           GWEN_SOCKETSET *writeSet,
                                           GWEN_SOCKETSET *exSet);

typedef void (*GWEN_NETLAYER_BASESTATUS_CHG_FN)(GWEN_NETLAYER *nl,
                                                GWEN_NETLAYER_STATUS newst);

typedef void (*GWEN_NETLAYER_STATUS_CHG_FN)(GWEN_NETLAYER *nl,
                                            GWEN_NETLAYER_STATUS newst);

typedef int (*GWEN_NETLAYER_BEGIN_OUT_PACKET_FN)(GWEN_NETLAYER *nl,
                                                  int totalSize);

typedef int (*GWEN_NETLAYER_END_OUT_PACKET_FN)(GWEN_NETLAYER *nl);

typedef int (*GWEN_NETLAYER_BEGIN_IN_PACKET_FN)(GWEN_NETLAYER *nl);
typedef int (*GWEN_NETLAYER_CHECK_IN_PACKET_FN)(GWEN_NETLAYER *nl);


GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayer_new(const char *typeName);
GWENHYWFAR_API
void GWEN_NetLayer_free(GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_Attach(GWEN_NETLAYER *nl);

GWENHYWFAR_API
const char *GWEN_NetLayer_GetTypeName(const GWEN_NETLAYER *nl);

GWENHYWFAR_API
GWEN_NETLAYER_STATUS GWEN_NetLayer_GetStatus(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetStatus(GWEN_NETLAYER *nl, GWEN_NETLAYER_STATUS st);
GWENHYWFAR_API
time_t GWEN_NetLayer_GetLastStatusChangeTime(const GWEN_NETLAYER *nl);

GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_NetLayer_GetFlags(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetFlags(GWEN_NETLAYER *nl, GWEN_TYPE_UINT32 fl);
GWENHYWFAR_API
void GWEN_NetLayer_AddFlags(GWEN_NETLAYER *nl, GWEN_TYPE_UINT32 fl);
GWENHYWFAR_API
void GWEN_NetLayer_SubFlags(GWEN_NETLAYER *nl, GWEN_TYPE_UINT32 fl);

GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayer_GetBaseLayer(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetBaseLayer(GWEN_NETLAYER *nl, GWEN_NETLAYER *baseLayer);

GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayer_GetParentLayer(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetParentLayer(GWEN_NETLAYER *nl, GWEN_NETLAYER *pLayer);

GWENHYWFAR_API
void GWEN_NetLayer_AddIncomingLayer(GWEN_NETLAYER *nl, GWEN_NETLAYER *newnl);
GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayer_PeekIncomingLayer(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayer_GetIncomingLayer(GWEN_NETLAYER *nl);
GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayer_GetIncomingLayer_Wait(GWEN_NETLAYER *nl,
                                                   int timeout);
GWENHYWFAR_API
int GWEN_NetLayer_GetIncomingLayerCount(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_DelIncomingLayers(GWEN_NETLAYER *nl);


GWENHYWFAR_API
const GWEN_INETADDRESS *GWEN_NetLayer_GetLocalAddr(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetLocalAddr(GWEN_NETLAYER *nl,
                                const GWEN_INETADDRESS *addr);

GWENHYWFAR_API
const GWEN_INETADDRESS *GWEN_NetLayer_GetPeerAddr(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetPeerAddr(GWEN_NETLAYER *nl,
                               const GWEN_INETADDRESS *addr);

GWENHYWFAR_API
int GWEN_NetLayer_GetBackLog(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetBackLog(GWEN_NETLAYER *nl, int i);

GWENHYWFAR_API
int GWEN_NetLayer_GetInBodySize(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetInBodySize(GWEN_NETLAYER *nl, int i);

GWENHYWFAR_API
int GWEN_NetLayer_GetOutBodySize(const GWEN_NETLAYER *nl);
GWENHYWFAR_API
void GWEN_NetLayer_SetOutBodySize(GWEN_NETLAYER *nl, int i);


GWENHYWFAR_API
void GWEN_NetLayer_SetGetPasswordFn(GWEN_NETLAYER *nl,
                                    GWEN_NETLAYER_GETPASSWD_FN f);

GWENHYWFAR_API
void GWEN_NetLayer_SetWorkFn(GWEN_NETLAYER *nl, GWEN_NETLAYER_WORK_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetReadFn(GWEN_NETLAYER *nl, GWEN_NETLAYER_READ_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetWriteFn(GWEN_NETLAYER *nl, GWEN_NETLAYER_WRITE_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetConnectFn(GWEN_NETLAYER *nl,
                                GWEN_NETLAYER_CONNECT_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetDisconnectFn(GWEN_NETLAYER *nl,
                                   GWEN_NETLAYER_DISCONNECT_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetListenFn(GWEN_NETLAYER *nl,
                               GWEN_NETLAYER_LISTEN_FN f);

GWENHYWFAR_API
void GWEN_NetLayer_SetAddSocketsFn(GWEN_NETLAYER *nl,
                                   GWEN_NETLAYER_ADDSOCKETS_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetBaseStatusChangeFn(GWEN_NETLAYER *nl,
                                         GWEN_NETLAYER_BASESTATUS_CHG_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetStatusChangeFn(GWEN_NETLAYER *nl,
                                     GWEN_NETLAYER_STATUS_CHG_FN f);

GWENHYWFAR_API
void GWEN_NetLayer_SetBeginOutPacketFn(GWEN_NETLAYER *nl,
                                        GWEN_NETLAYER_BEGIN_OUT_PACKET_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetEndOutPacketFn(GWEN_NETLAYER *nl,
                                      GWEN_NETLAYER_END_OUT_PACKET_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetBeginInPacketFn(GWEN_NETLAYER *nl,
                                       GWEN_NETLAYER_BEGIN_IN_PACKET_FN f);
GWENHYWFAR_API
void GWEN_NetLayer_SetCheckInPacketFn(GWEN_NETLAYER *nl,
                                       GWEN_NETLAYER_CHECK_IN_PACKET_FN f);



/** @name Virtual IO functions
 *
 * All functions in this group - except @ref GWEN_NetLayer_Work - return
 * a negative value on error, 0 on success and 1 if the operation would
 * block (and thus needs to wait for IO to occurr on this object).
 */
/*@{*/

GWENHYWFAR_API
int GWEN_NetLayer_GetPassword(GWEN_NETLAYER *nl,
                              char *buffer, int num,
                              int rwflag);


GWENHYWFAR_API
GWEN_NETLAYER_RESULT GWEN_NetLayer_Work(GWEN_NETLAYER *nl);

GWENHYWFAR_API
int GWEN_NetLayer_Read(GWEN_NETLAYER *nl,
                       char *buffer,
                       int *bsize);
GWENHYWFAR_API
int GWEN_NetLayer_Read_Wait(GWEN_NETLAYER *nl,
                            char *buffer, int *bsize,
                            int timeout);

GWENHYWFAR_API
int GWEN_NetLayer_Write(GWEN_NETLAYER *nl,
                        const char *buffer,
                        int *bsize);
GWENHYWFAR_API
int GWEN_NetLayer_Write_Wait(GWEN_NETLAYER *nl,
                             const char *buffer, int *bsize,
                             int timeout);

GWENHYWFAR_API
int GWEN_NetLayer_Connect(GWEN_NETLAYER *nl);
GWENHYWFAR_API
int GWEN_NetLayer_Connect_Wait(GWEN_NETLAYER *nl, int timeout);

GWENHYWFAR_API
int GWEN_NetLayer_Disconnect(GWEN_NETLAYER *nl);
GWENHYWFAR_API
int GWEN_NetLayer_Disconnect_Wait(GWEN_NETLAYER *nl, int timeout);

GWENHYWFAR_API
int GWEN_NetLayer_Listen(GWEN_NETLAYER *nl);

GWENHYWFAR_API
int GWEN_NetLayer_AddSockets(GWEN_NETLAYER *nl,
                             GWEN_SOCKETSET *readSet,
                             GWEN_SOCKETSET *writeSet,
                             GWEN_SOCKETSET *exSet);

/**
 * This function is called just before the status of the base layer changes.
 */
GWENHYWFAR_API
void GWEN_NetLayer_BaseStatusChange(GWEN_NETLAYER *nl,
                                    GWEN_NETLAYER_STATUS newst);
/*@}*/


/** @name Virtual Packet IO functions
 *
 */
/*@{*/

GWENHYWFAR_API
int GWEN_NetLayer_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize);
GWENHYWFAR_API
int GWEN_NetLayer_EndOutPacket(GWEN_NETLAYER *nl);
GWENHYWFAR_API
int GWEN_NetLayer_EndOutPacket_Wait(GWEN_NETLAYER *nl, int timeout);

GWENHYWFAR_API
int GWEN_NetLayer_BeginInPacket(GWEN_NETLAYER *nl);
/**
 * @return <0 on error, 1 if there still is body data, 0 if not
 */
GWENHYWFAR_API
int GWEN_NetLayer_CheckInPacket(GWEN_NETLAYER *nl);
/*@}*/


GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayer_FindBaseLayer(const GWEN_NETLAYER *nl,
                                           const char *tname);


GWENHYWFAR_API
int GWEN_NetLayer_RecvPacket(GWEN_NETLAYER *nl, GWEN_BUFFER *mbuf,
                             int timeout);
GWENHYWFAR_API
int GWEN_NetLayer_RecvPacketBio(GWEN_NETLAYER *nl,
                                GWEN_BUFFEREDIO *bio,
                                int timeout);

GWENHYWFAR_API
int GWEN_NetLayer_SendPacket(GWEN_NETLAYER *nl,
                             const char *dPtr, int dLen,
                             int timeout);
GWENHYWFAR_API
int GWEN_NetLayer_SendPacketBio(GWEN_NETLAYER *nl,
                                GWEN_BUFFEREDIO *bio,
                                int timeout);


#endif /* GWEN_NETLAYER_H */




