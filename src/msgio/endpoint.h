/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_H
#define GWEN_MSG_ENDPOINT_H


#include <gwenhywfar/inherit.h>
#include <gwenhywfar/tree2.h>
#include <gwenhywfar/inetsocket.h>



#define GWEN_MSG_ENDPOINT_FLAGS_DELETE     0x80000000u
#define GWEN_MSG_ENDPOINT_FLAGS_NOIO       0x40000000u


#define GWEN_MSG_ENDPOINT_STATE_UNCONNECTED 0
#define GWEN_MSG_ENDPOINT_STATE_CONNECTING  1
#define GWEN_MSG_ENDPOINT_STATE_CONNECTED   2


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Object which can send and receive messages (base class).
 *
 */
typedef struct GWEN_MSG_ENDPOINT GWEN_MSG_ENDPOINT;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT, GWENHYWFAR_API)
GWEN_TREE2_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT, GWEN_MsgEndpoint, GWENHYWFAR_API)

#ifdef __cplusplus
}
#endif


#include <gwenhywfar/msg.h>

#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_MSGENDPOINT2 Message Sender/Receiver
 * @ingroup MOD_MSGIO
 *
 * @brief This file contains the definition of a GWEN_MSG_ENDPOINT, an object which
 * can send and receive messages (base class).
 *
 */
/*@{*/


typedef void (*GWEN_MSG_ENDPOINT_ADDSOCKETS_FN)(GWEN_MSG_ENDPOINT *ep,
                                                 GWEN_SOCKETSET *readSet,
                                                 GWEN_SOCKETSET *writeSet,
                                                 GWEN_SOCKETSET *xSet);

typedef void (*GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN)(GWEN_MSG_ENDPOINT *ep,
                                                   GWEN_SOCKETSET *readSet,
                                                   GWEN_SOCKETSET *writeSet,
                                                   GWEN_SOCKETSET *xSet);
typedef void (*GWEN_MSG_ENDPOINT_RUN_FN)(GWEN_MSG_ENDPOINT *ep);




GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_new(const char *name, int groupId);

GWENHYWFAR_API void GWEN_MsgEndpoint_free(GWEN_MSG_ENDPOINT *ep);


GWENHYWFAR_API const char *GWEN_MsgEndpoint_GetName(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API int GWEN_MsgEndpoint_GetGroupId(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API GWEN_SOCKET *GWEN_MsgEndpoint_GetSocket(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetSocket(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKET *sk);

GWENHYWFAR_API int GWEN_MsgEndpoint_GetState(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetState(GWEN_MSG_ENDPOINT *ep, int m);
GWENHYWFAR_API time_t GWEN_MsgEndpoint_GetTimeOfLastStateChange(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API uint32_t GWEN_MsgEndpoint_GetFlags(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);
GWENHYWFAR_API void GWEN_MsgEndpoint_AddFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);
GWENHYWFAR_API void GWEN_MsgEndpoint_DelFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);

GWENHYWFAR_API int GWEN_MsgEndpoint_GetDefaultMessageSize(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetDefaultMessageSize(GWEN_MSG_ENDPOINT *ep, int i);


GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetReceivedMessageList(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetSendMessageList(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_AddReceivedMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetFirstReceivedMessage(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_TakeFirstReceivedMessage(GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_AddSendMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetFirstSendMessage(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint_HaveMessageToSend(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetCurrentlyReceivedMsg(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);



GWENHYWFAR_API void GWEN_MsgEndpoint_AddSockets(GWEN_MSG_ENDPOINT *ep,
                                                 GWEN_SOCKETSET *readSet,
                                                 GWEN_SOCKETSET *writeSet,
                                                 GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint_CheckSockets(GWEN_MSG_ENDPOINT *ep,
                                                   GWEN_SOCKETSET *readSet,
                                                   GWEN_SOCKETSET *writeSet,
                                                   GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint_Run(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API void GWEN_MsgEndpoint_IoLoop(GWEN_MSG_ENDPOINT *ep, int timeout);





GWENHYWFAR_API void GWEN_MsgEndpoint_ChildrenAddSockets(GWEN_MSG_ENDPOINT *ep,
                                                         GWEN_SOCKETSET *readSet,
                                                         GWEN_SOCKETSET *writeSet,
                                                         GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint_ChildrenCheckSockets(GWEN_MSG_ENDPOINT *ep,
                                                           GWEN_SOCKETSET *readSet,
                                                           GWEN_SOCKETSET *writeSet,
                                                           GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint_ChildrenRun(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API void GWEN_MsgEndpoint_ChildrenIoLoop(GWEN_MSG_ENDPOINT *ep, int timeout);


GWENHYWFAR_API void GWEN_MsgEndpoint_RemoveUnconnectedAndEmptyChildren(GWEN_MSG_ENDPOINT *ep);




GWENHYWFAR_API int GWEN_MsgEndpoint_ReadFromSocket(GWEN_MSG_ENDPOINT *ep, uint8_t *bufferPtr, uint32_t bufferLen);
GWENHYWFAR_API int GWEN_MsgEndpoint_WriteToSocket(GWEN_MSG_ENDPOINT *ep, const uint8_t *bufferPtr, uint32_t bufferLen);
GWENHYWFAR_API void GWEN_MsgEndpoint_Disconnect(GWEN_MSG_ENDPOINT *ep);



/**
 * This function can be used to drain the file descriptor/socket from waiting data.
 * An example is an endpoint implementation from AqHome which uses an USB serial device. When message receiption
 * errors occur (e.g. due to collisions on the data line) the rest of data on the line needs to be discarded
 * because it might belong the the errornous previous message.
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_DiscardInput(GWEN_MSG_ENDPOINT *ep);




GWENHYWFAR_API GWEN_MSG_ENDPOINT_ADDSOCKETS_FN GWEN_MsgEndpoint_SetAddSocketsFn(GWEN_MSG_ENDPOINT *ep,
                                                                                  GWEN_MSG_ENDPOINT_ADDSOCKETS_FN fn);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN GWEN_MsgEndpoint_SetCheckSocketsFn(GWEN_MSG_ENDPOINT *ep,
                                                                                      GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN fn);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_RUN_FN GWEN_MsgEndpoint_SetRunFn(GWEN_MSG_ENDPOINT *ep,
                                                                  GWEN_MSG_ENDPOINT_RUN_FN fn);


/*@}*/

#ifdef __cplusplus
}
#endif


#endif


