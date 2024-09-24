/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/


#ifndef GWEN_MSG_MSG_IPC_H
#define GWEN_MSG_MSG_IPC_H


#include <gwenhywfar/msg.h>


#define GWEN_MSGIPC_OFFS_SIZE     0   /* 4 bytes: number of all bytes including size, protoid, protover and code */
#define GWEN_MSGIPC_OFFS_PROTOID  4   /* 1 byte: protocol id (free to use) */
#define GWEN_MSGIPC_OFFS_PROTOVER 5   /* 1 byte: protocol version (free to use) */
#define GWEN_MSGIPC_OFFS_CODE     6   /* 2 bytes  msg code (meaning depends on protocol) */
#define GWEN_MSGIPC_OFFS_ID       8   /* 4 bytes  msg id */
#define GWEN_MSGIPC_OFFS_REFID    12  /* 4 bytes  reference msg id */
#define GWEN_MSGIPC_OFFS_PAYLOAD  16  /* begin of payload for a given message */



GWENHYWFAR_API GWEN_MSG *GWEN_IpcMsg_new(uint8_t protoId, uint8_t protoVer, uint16_t code,
                                         uint32_t msgId, uint32_t refMsgId,
                                         uint32_t payloadLen, const uint8_t *payload);
GWENHYWFAR_API void GWEN_IpcMsg_AdjustMsgSize(GWEN_MSG *msg);
GWENHYWFAR_API int GWEN_IpcMsg_IsMsgComplete(const GWEN_MSG *msg);
GWENHYWFAR_API uint32_t GWEN_IpcMsg_GetMsgSize(const GWEN_MSG *msg);
GWENHYWFAR_API uint8_t GWEN_IpcMsg_GetProtoId(const GWEN_MSG *msg);
GWENHYWFAR_API uint8_t GWEN_IpcMsg_GetProtoVersion(const GWEN_MSG *msg);
GWENHYWFAR_API uint16_t GWEN_IpcMsg_GetCode(const GWEN_MSG *msg);

GWENHYWFAR_API uint32_t GWEN_IpcMsg_GetMsgId(const GWEN_MSG *msg);
GWENHYWFAR_API uint32_t GWEN_IpcMsg_GetRefMsgId(const GWEN_MSG *msg);


#endif


