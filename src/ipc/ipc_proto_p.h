/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2021 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_IPC_PROTO_P_H
#define GWEN_IPC_PROTO_P_H


#include <gwenhywfar/ipc_proto.h>



struct GWEN_IPC_PROTOCOL {
  GWEN_LIST_ELEMENT(GWEN_IPC_PROTOCO)

  uint32_t protocolId;
  uint32_t protocolVersion;

  GWEN_IPC_PROTOCOL_HANDLE_FN handleFn;

};





#endif
