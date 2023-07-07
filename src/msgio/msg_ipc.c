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




GWEN_MSG *GWEN_IpcMsg_new(uint8_t protoId, uint8_t protoVer, uint16_t code, uint32_t payloadLen, const uint8_t *payload)
{
  GWEN_MSG *msg;
  uint32_t len;

  len=GWEN_MSGIPC_OFFS_PAYLOAD+payloadLen;
  msg=GWEN_Msg_new(len);
  if (msg==NULL)
    return NULL;

  msg->maxSize=len;

  msg->buffer[GWEN_MSGIPC_OFFS_SIZE+0]=len & 0xff;
  msg->buffer[GWEN_MSGIPC_OFFS_SIZE+1]=(len>>8) & 0xff;
  msg->buffer[GWEN_MSGIPC_OFFS_SIZE+2]=(len>>16) & 0xff;
  msg->buffer[GWEN_MSGIPC_OFFS_SIZE+3]=(len>>24) & 0xff;

  msg->buffer[GWEN_MSGIPC_OFFS_PROTOID]=protoId;
  msg->buffer[GWEN_MSGIPC_OFFS_PROTOVER]=protoVer;

  msg->buffer[GWEN_MSGIPC_OFFS_CODE+0]=code & 0xff;
  msg->buffer[GWEN_MSGIPC_OFFS_CODE+1]=(code>>8) & 0xff;

  if (payloadLen && payload)
    memmove(msg->buffer+GWEN_MSGIPC_OFFS_PAYLOAD, payload, payloadLen);
  msg->bytesInBuffer=len;
  return msg;
}



int GWEN_IpcMsg_IsMsgComplete(const GWEN_MSG *msg)
{
  if (msg && msg->bytesInBuffer>4) {
    uint32_t len;

    len=msg->buffer[GWEN_MSGIPC_OFFS_SIZE]+
      ((msg->buffer[GWEN_MSGIPC_OFFS_SIZE+1])<<8)+
      ((msg->buffer[GWEN_MSGIPC_OFFS_SIZE+2])<<16)+
      ((msg->buffer[GWEN_MSGIPC_OFFS_SIZE+2])<<24);
    if (len>msg->maxSize)
      return -1;
    else if (msg->bytesInBuffer>=len)
      return 1;
  }
  return 0;
}



uint32_t GWEN_IpcMsg_GetMsgSize(const GWEN_MSG *msg)
{
  if (msg && msg->bytesInBuffer>4) {
    return
      (msg->buffer[GWEN_MSGIPC_OFFS_SIZE])+
      ((msg->buffer[GWEN_MSGIPC_OFFS_SIZE+1])<<8)+
      ((msg->buffer[GWEN_MSGIPC_OFFS_SIZE+2])<<16)+
      ((msg->buffer[GWEN_MSGIPC_OFFS_SIZE+2])<<24);
  }
  return 0;
}



uint8_t GWEN_IpcMsg_GetProtoId(const GWEN_MSG *msg)
{
  if (msg && msg->bytesInBuffer>=GWEN_MSGIPC_OFFS_PAYLOAD)
    return msg->buffer[GWEN_MSGIPC_OFFS_PROTOID];
  return 0;
}



uint8_t GWEN_IpcMsg_GetProtoVersion(const GWEN_MSG *msg)
{
  if (msg && msg->bytesInBuffer>=GWEN_MSGIPC_OFFS_PAYLOAD)
    return msg->buffer[GWEN_MSGIPC_OFFS_PROTOVER];
  return 0;
}



uint16_t GWEN_IpcMsg_GetCode(const GWEN_MSG *msg)
{
  if (msg && msg->bytesInBuffer>=GWEN_MSGIPC_OFFS_PAYLOAD)
    return msg->buffer[GWEN_MSGIPC_OFFS_CODE]+(msg->buffer[GWEN_MSGIPC_OFFS_CODE+1]<<8);
  return 0;
}





