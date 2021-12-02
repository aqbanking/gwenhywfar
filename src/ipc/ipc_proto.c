/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2021 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ipc_proto_p.h"

#include <gwenhywfar/memory.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/pdu_item.h>





GWEN_LIST_FUNCTIONS(GWEN_IPC_PROTOCOL, GWEN_IpcProtocol)




GWEN_IPC_PROTOCOL *GWEN_IpcProtocol_new(uint32_t protocolId, uint32_t protocolVersion)
{
  GWEN_IPC_PROTOCOL *proto;

  GWEN_NEW_OBJECT(GWEN_IPC_PROTOCOL, proto);
  GWEN_LIST_INIT(GWEN_IPC_PROTOCOL, proto);

  proto->protocolId=protocolId;
  proto->protocolVersion=protocolVersion;

  return proto;
}



void GWEN_IpcProtocol_free(GWEN_IPC_PROTOCOL *proto)
{
  if (proto) {
    GWEN_LIST_FINI(GWEN_IPC_PROTOCOL, proto);
    GWEN_FREE_OBJECT(proto);
  }
}



int GWEN_IpcProtocol_Handle(GWEN_IPC_PROTOCOL *proto,
                            const uint8_t *bufferPointer, uint32_t bufferLength,
                            GWEN_BUFFER *responseBuffer)
{
  if (proto &&& proto->handleFn)
    return proto->handleFn(proto, bufferPointer, bufferLength, responseBuffer);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_IPC_PROTOCOL_HANDLE_FN GWEN_IpcProtocol_SetHandleFn(GWEN_IPC_PROTOCOL *proto, GWEN_IPC_PROTOCOL_HANDLE_FN fn)
{
  GWEN_IPC_PROTOCOL_HANDLE_FN oldFn;

  oldFn=proto->handleFn;
  proto->handleFn=fn;
  return oldFn;
}



void GWEN_IpcProtocol_PrepareMessage(const GWEN_IPC_PROTOCOL *proto, GWEN_BUFFER *destBuffer)
{
  GWEN_PduItem_AppendItemHeader32(destBuffer, GWEN_IPC_PROTOCOL_FIELD_IPC, 0);
  GWEN_PduItem_DirectlyWritePdu16AsUint32(GWEN_IPC_PROTOCOL_FIELD_PROTOID, proto->protocolId, destBuffer);
  GWEN_PduItem_DirectlyWritePdu16AsUint32(GWEN_IPC_PROTOCOL_FIELD_PROTOVER, proto->protocolVersion, destBuffer);
}




GWEN_IPC_PROTOCOL *GWEN_IpcProtocol_List_FindByIdAndVersion(const GWEN_IPC_PROTOCOL_LIST *protoList,
                                                            uint32_t protocolId, uint32_t protocolVersion)
{
  if (protoList) {
    GWEN_IPC_PROTOCOL *proto;

    proto=GWEN_IpcProtocol_List_First(protoList);
    while(proto) {
      if ((protocolId==proto->protocolId) &&
          ((protocolVersion==0) || (protocolVersion<=proto->protocolVersion)))
        return proto;
      proto=GWEN_IpcProtocol_List_Next(proto);
    }
  }

  return NULL;
}



