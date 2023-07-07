/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT2_H
#define GWEN_MSG_ENDPOINT2_H


#include <gwenhywfar/inherit.h>
#include <gwenhywfar/tree2.h>
#include <gwenhywfar/inetsocket.h>



#define GWEN_MSG_ENDPOINT2_FLAGS_DELETE     0x80000000u
#define GWEN_MSG_ENDPOINT2_FLAGS_NOIO       0x40000000u


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
typedef struct GWEN_MSG_ENDPOINT2 GWEN_MSG_ENDPOINT2;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT2, GWENHYWFAR_API)
GWEN_TREE2_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT2, GWEN_MsgEndpoint2, GWENHYWFAR_API)

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
 * @brief This file contains the definition of a GWEN_MSG_ENDPOINT2, an object which
 * can send and receive messages (base class).
 *
 */
/*@{*/


typedef void (*GWEN_MSG_ENDPOINT2_ADDSOCKETS_FN)(GWEN_MSG_ENDPOINT2 *ep,
                                                 GWEN_SOCKETSET *readSet,
                                                 GWEN_SOCKETSET *writeSet,
                                                 GWEN_SOCKETSET *xSet);

typedef void (*GWEN_MSG_ENDPOINT2_CHECKSOCKETS_FN)(GWEN_MSG_ENDPOINT2 *ep,
                                                   GWEN_SOCKETSET *readSet,
                                                   GWEN_SOCKETSET *writeSet,
                                                   GWEN_SOCKETSET *xSet);
typedef void (*GWEN_MSG_ENDPOINT2_RUN_FN)(GWEN_MSG_ENDPOINT2 *ep);




GWENHYWFAR_API GWEN_MSG_ENDPOINT2 *GWEN_MsgEndpoint2_new(const char *name, int groupId);

GWENHYWFAR_API void GWEN_MsgEndpoint2_free(GWEN_MSG_ENDPOINT2 *ep);


GWENHYWFAR_API const char *GWEN_MsgEndpoint2_GetName(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API int GWEN_MsgEndpoint2_GetGroupId(const GWEN_MSG_ENDPOINT2 *ep);

GWENHYWFAR_API GWEN_SOCKET *GWEN_MsgEndpoint2_GetSocket(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint2_SetSocket(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKET *sk);

GWENHYWFAR_API int GWEN_MsgEndpoint2_GetState(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint2_SetState(GWEN_MSG_ENDPOINT2 *ep, int m);
GWENHYWFAR_API time_t GWEN_MsgEndpoint2_GetTimeOfLastStateChange(const GWEN_MSG_ENDPOINT2 *ep);

GWENHYWFAR_API uint32_t GWEN_MsgEndpoint2_GetFlags(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint2_SetFlags(GWEN_MSG_ENDPOINT2 *ep, uint32_t f);
GWENHYWFAR_API void GWEN_MsgEndpoint2_AddFlags(GWEN_MSG_ENDPOINT2 *ep, uint32_t f);
GWENHYWFAR_API void GWEN_MsgEndpoint2_DelFlags(GWEN_MSG_ENDPOINT2 *ep, uint32_t f);

GWENHYWFAR_API int GWEN_MsgEndpoint2_GetDefaultMessageSize(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint2_SetDefaultMessageSize(GWEN_MSG_ENDPOINT2 *ep, int i);


GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint2_GetReceivedMessageList(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint2_GetSendMessageList(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint2_AddReceivedMessage(GWEN_MSG_ENDPOINT2 *ep, GWEN_MSG *m);

GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint2_GetFirstReceivedMessage(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint2_TakeFirstReceivedMessage(GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint2_AddSendMessage(GWEN_MSG_ENDPOINT2 *ep, GWEN_MSG *m);
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint2_GetFirstSendMessage(const GWEN_MSG_ENDPOINT2 *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint2_HaveMessageToSend(const GWEN_MSG_ENDPOINT2 *ep);

GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint2_GetCurrentlyReceivedMsg(const GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint2_SetCurrentlyReceivedMsg(GWEN_MSG_ENDPOINT2 *ep, GWEN_MSG *m);



GWENHYWFAR_API void GWEN_MsgEndpoint2_AddSockets(GWEN_MSG_ENDPOINT2 *ep,
                                                 GWEN_SOCKETSET *readSet,
                                                 GWEN_SOCKETSET *writeSet,
                                                 GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint2_CheckSockets(GWEN_MSG_ENDPOINT2 *ep,
                                                   GWEN_SOCKETSET *readSet,
                                                   GWEN_SOCKETSET *writeSet,
                                                   GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint2_Run(GWEN_MSG_ENDPOINT2 *ep);

GWENHYWFAR_API void GWEN_MsgEndpoint2_IoLoop(GWEN_MSG_ENDPOINT2 *ep, int timeout);





GWENHYWFAR_API void GWEN_MsgEndpoint2_ChildrenAddSockets(GWEN_MSG_ENDPOINT2 *ep,
                                                         GWEN_SOCKETSET *readSet,
                                                         GWEN_SOCKETSET *writeSet,
                                                         GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint2_ChildrenCheckSockets(GWEN_MSG_ENDPOINT2 *ep,
                                                           GWEN_SOCKETSET *readSet,
                                                           GWEN_SOCKETSET *writeSet,
                                                           GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint2_ChildrenRun(GWEN_MSG_ENDPOINT2 *ep);

GWENHYWFAR_API void GWEN_MsgEndpoint2_ChildrenIoLoop(GWEN_MSG_ENDPOINT2 *ep, int timeout);


GWENHYWFAR_API void GWEN_MsgEndpoint2_RemoveUnconnectedAndEmptyChildren(GWEN_MSG_ENDPOINT2 *ep);




GWENHYWFAR_API int GWEN_MsgEndpoint2_ReadFromSocket(GWEN_MSG_ENDPOINT2 *ep, uint8_t *bufferPtr, uint32_t bufferLen);
GWENHYWFAR_API int GWEN_MsgEndpoint2_WriteToSocket(GWEN_MSG_ENDPOINT2 *ep, const uint8_t *bufferPtr, uint32_t bufferLen);
GWENHYWFAR_API void GWEN_MsgEndpoint2_Disconnect(GWEN_MSG_ENDPOINT2 *ep);



/**
 * This function can be used to drain the file descriptor/socket from waiting data.
 * An example is an endpoint implementation from AqHome which uses an USB serial device. When message receiption
 * errors occur (e.g. due to collisions on the data line) the rest of data on the line needs to be discarded
 * because it might belong the the errornous previous message.
 */
GWENHYWFAR_API int GWEN_MsgEndpoint2_DiscardInput(GWEN_MSG_ENDPOINT2 *ep);




GWENHYWFAR_API GWEN_MSG_ENDPOINT2_ADDSOCKETS_FN GWEN_MsgEndpoint2_SetAddSocketsFn(GWEN_MSG_ENDPOINT2 *ep,
                                                                                  GWEN_MSG_ENDPOINT2_ADDSOCKETS_FN fn);
GWENHYWFAR_API GWEN_MSG_ENDPOINT2_CHECKSOCKETS_FN GWEN_MsgEndpoint2_SetCheckSocketsFn(GWEN_MSG_ENDPOINT2 *ep,
                                                                                      GWEN_MSG_ENDPOINT2_CHECKSOCKETS_FN fn);
GWENHYWFAR_API GWEN_MSG_ENDPOINT2_RUN_FN GWEN_MsgEndpoint2_SetRunFn(GWEN_MSG_ENDPOINT2 *ep,
                                                                    GWEN_MSG_ENDPOINT2_RUN_FN fn);


/*@}*/

#ifdef __cplusplus
}
#endif


#endif


