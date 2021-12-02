/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2021 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_IPC_PROTO_H
#define GWEN_IPC_PROTO_H


#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/list1.h>

#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif



#define GWEN_IPC_PROTOCOL_FIELD_IPC          250
#define GWEN_IPC_PROTOCOL_FIELD_PROTOID      251
#define GWEN_IPC_PROTOCOL_FIELD_PROTOVER     252
#define GWEN_IPC_PROTOCOL_FIELD_PROTOMSGTYPE 253
#define GWEN_IPC_PROTOCOL_FIELD_PROTOMSGID   254



/**
 * IPC messages are PDU32 objects (see @ref GWEN_PduItem_new and others) containing multiple
 * PDU16 sub-objects in a specific order.
 * - protocol id (int)
 * - protocol version (int)
 * - message type (int)
 * - message id (int)
 * - more PDU16 according to message type
 *
 *
 */
typedef struct GWEN_IPC_PROTOCOL GWEN_IPC_PROTOCOL;
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_IPC_PROTOCOL, GWEN_IpcProtocol, GWENHYWFAR_API)

typedef int (*GWEN_IPC_PROTOCOL_HANDLE_FN)(GWEN_IPC_PROTOCOL *proto,
                                           const uint8_t *bufferPointer, uint32_t bufferLength,
                                           GWEN_BUFFER *responseBuffer);


GWENHYWFAR_API
GWEN_IPC_PROTOCOL *GWEN_IpcProtocol_new(uint32_t protocolId, uint32_t protocolVersion);

GWENHYWFAR_API
void GWEN_IpcProtocol_free(GWEN_IPC_PROTOCOL *proto);


/**
 * Handle an IPC request and prepare a response.
 *
 * The incoming message is in the buffer given by bufferPointer with a remaining length given in bufferLength.
 * The buffer pointer points to the first PDU after protocol id and protocol version thus pointing to message type.
 * To return data in the response buffer you need to call GWEN_IpcProtocol_PrepareMessage(), add some response APU16
 * to the buffer and call @ref GWEN_PduItem_AdjustItemSizeInBuffer32(buf, 0) on that buffer.
 * If there is no response just leave the buffer empty.
 */
GWENHYWFAR_API 
int GWEN_IpcProtocol_Handle(GWEN_IPC_PROTOCOL *proto,
                            const uint8_t *bufferPointer, uint32_t bufferLength,
                            GWEN_BUFFER *responseBuffer);


/**
 * Prepare a PDU32 message and write PDU16 items with protocol id and protocol version to the given buffer.
 * To finish the message inside the buffer you need to call @ref GWEN_PduItem_AdjustItemSizeInBuffer32(buf, 0).
 */
GWENHYWFAR_API 
void GWEN_IpcProtocol_PrepareMessage(const GWEN_IPC_PROTOCOL *proto, GWEN_BUFFER *buf);


GWENHYWFAR_API
GWEN_IPC_PROTOCOL_HANDLE_FN GWEN_IpcProtocol_SetHandleFn(GWEN_IPC_PROTOCOL *proto, GWEN_IPC_PROTOCOL_HANDLE_FN fn);


GWENHYWFAR_API 
GWEN_IPC_PROTOCOL *GWEN_IpcProtocol_List_FindByIdAndVersion(const GWEN_IPC_PROTOCOL_LIST *protoList,
                                                            uint32_t protocolId, uint32_t protocolVersion);

#ifdef __cplusplus
}
#endif




#endif
