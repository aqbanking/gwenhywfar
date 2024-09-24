/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DISABLE_DEBUGLOG*/


#include "msgio/msg_p.h"
#include "msgio/msg_ipc.h"




GWEN_MSG *GWEN_IpcMsg_new(uint8_t protoId, uint8_t protoVer, uint16_t code,
                          uint32_t msgId, uint32_t refMsgId,
                          uint32_t payloadLen, const uint8_t *payload)
{
  GWEN_MSG *msg;
  uint32_t len;

  len=GWEN_MSGIPC_OFFS_PAYLOAD+payloadLen;
  msg=GWEN_Msg_new(len);
  if (msg==NULL)
    return NULL;

  msg->maxSize=len;

  GWEN_Msg_AddUint32(msg, len);
  GWEN_Msg_AddUint8(msg, protoId);
  GWEN_Msg_AddUint8(msg, protoVer);
  GWEN_Msg_AddUint16(msg, code);
  GWEN_Msg_AddUint32(msg, msgId);
  GWEN_Msg_AddUint32(msg, refMsgId);

  if (payloadLen && payload)
    GWEN_Msg_AddBytes(msg, payload, payloadLen);
  return msg;
}



void GWEN_IpcMsg_AdjustMsgSize(GWEN_MSG *msg)
{
  if (msg) {
    GWEN_Msg_WriteUint32At(msg, GWEN_MSGIPC_OFFS_SIZE, msg->bytesInBuffer);
  }
}


int GWEN_IpcMsg_IsMsgComplete(const GWEN_MSG *msg)
{
  if (msg && msg->bytesInBuffer>4) {
    uint32_t len;

    len=GWEN_Msg_GetUint32At(msg, GWEN_MSGIPC_OFFS_SIZE, 0);
    if (len>msg->maxSize)
      return -1;
    else if (msg->bytesInBuffer>=len)
      return 1;
  }
  return 0;
}



uint32_t GWEN_IpcMsg_GetMsgSize(const GWEN_MSG *msg)
{
  return GWEN_Msg_GetUint32At(msg, GWEN_MSGIPC_OFFS_SIZE, 0);
}



uint8_t GWEN_IpcMsg_GetProtoId(const GWEN_MSG *msg)
{
  return GWEN_Msg_GetUint8At(msg, GWEN_MSGIPC_OFFS_PROTOID, 0);
}



uint8_t GWEN_IpcMsg_GetProtoVersion(const GWEN_MSG *msg)
{
  return GWEN_Msg_GetUint8At(msg, GWEN_MSGIPC_OFFS_PROTOVER, 0);
}



uint16_t GWEN_IpcMsg_GetCode(const GWEN_MSG *msg)
{
  return GWEN_Msg_GetUint16At(msg, GWEN_MSGIPC_OFFS_CODE, 0);
}



uint32_t GWEN_IpcMsg_GetMsgId(const GWEN_MSG *msg)
{
  return GWEN_Msg_GetUint32At(msg, GWEN_MSGIPC_OFFS_ID, 0);
}



uint32_t GWEN_IpcMsg_GetRefMsgId(const GWEN_MSG *msg)
{
  return GWEN_Msg_GetUint32At(msg, GWEN_MSGIPC_OFFS_REFID, 0);
}







