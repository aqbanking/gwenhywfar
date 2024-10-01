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

/** @defgroup MOD_MSGENDPOINT Message Sender/Receiver
 * @ingroup MOD_MSGIO
 *
 * @brief This file contains the definition of a GWEN_MSG_ENDPOINT, an object which
 * can send and receive messages (base class).
 *
 *
 * <h1>GWEN_MSG_ENDPOINT</h1>
 *
 * There are a few central structures/objects:
 * - a socket (or file descriptor for files and/or unix devices)
 * - list of packets/messages to send (see @ref GWEN_MSG)
 * - list of packets/messages received
 *
 * This class was developed around the system call select() which takes 3 sets of sockets to be checked:
 * - set of sockets to be checked for readability
 * - set of sockets to be checked for writability
 * - set of sockets to be checked for "exceptional conditions" (rarely used).
 * Most derived classes will only populate the first two sets.
 *
 *
 * <h2>Virtual Functions</h2>
 *
 * GWEN_MSG_ENDPOINT has two virtual functions which must be implemented for this class to be usefull:
 * - @ref GWEN_MSG_ENDPOINT_ADDSOCKETS_FN
 * - @ref GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN
 *
 * <h3>GWEN_MSG_ENDPOINT_ADDSOCKETS_FN</h3>
 *
 * This function lets the implementation add sockets to the given socket sets which are then presented to the select()
 * system call.
 * If the given GWEN_MSG_ENDPOINT object is waiting for incoming data it adds its own socket to the read set.
 * if it has data to write it adds its socket to the write set.
 *
 * <h3>GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN</h3>
 *
 * This function is called after return from the select() system call with the socket sets returned by that function.
 * It lets the implementation check whether its socket(s) has/have become readable or writable and act accordingly
 * (e.g. read data if socket is in the given read set or write if the socket is in the given write set).
 *
 *
 * <h2>Sending and Receiving Messages</h2>
 *
 * As seen above the main functionality is implemented in the virtual function @ref GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN.
 * To make it easier to use the MSGIO API there is another class called GWEN_MsgIoEndpoint (see @ref GWEN_MsgIoEndpoint_Extend)
 * which simplifies reading and writing messages. It implements @ref GWEN_MSG_ENDPOINT_ADDSOCKETS_FN
 * and @ref GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN. However, it has to know the format of the received messages in order to
 * correctly read them. Therefore that class has another virtual function called @ref GWEN_ENDPOINT_MSGIO_GETBYTESNEEDED_FN.
 * This function is called when data has been read from the endpoints socket to determine how many of those bytes belong to
 * a given message.
 *
 * <h3>Example Implementation</h3>
 *
 * A first complete example sending an receiving messages is in the GWEN_IpcEndpoint
 * class (see @ref GWEN_IpcEndpoint_CreateIpcTcpClient). This class implements a basic protocol for inter-process-communication.
 * When creating an object of that class the following functions are called:
 * - GWEN_TcpcEndpoint_new() to instantiate an object using TCP sockets as a client
 * - GWEN_MsgIoEndpoint_Extend() to add functionality to send/receive messages, implementing virtual functions
 *   GWEN_MSG_ENDPOINT_ADDSOCKETS_FN and GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN but adding virtual function
 *   @ref GWEN_ENDPOINT_MSGIO_GETBYTESNEEDED_FN
 * - GWEN_IpcEndpoint_Extend() implementing GWEN_ENDPOINT_MSGIO_GETBYTESNEEDED_FN
 *
 * <h3>Example Usage Code</h3>
 *
 * <code>
 * GWEN_MSG_ENDPOINT *epClient;
 * int loop;
 * GWEN_MSG *msg;
 *
 * epClient=GWEN_IpcEndpoint_CreateIpcTcpClient("127.0.0.1", 55555, NULL, 1);
 * msg=GWEN_IpcMsg_new(1, 2, 3, 0, NULL);
 * GWEN_MsgEndpoint_AddSendMessage(epClient, msg);
 *
 * for (loop=0;; loop++) {
 *   GWEN_MsgEndpoint_IoLoop(epClient, 2000);
 *
 *   while( (msg=GWEN_MsgEndpoint_TakeFirstReceivedMessage(epClient)) ) {
 *     DBG_INFO(GWEN_LOGDOMAIN,
 *              "  - received msg: protoId=%d, protoVer=%d, code=%d",
 *              GWEN_IpcMsg_GetProtoId(msg),
 *              GWEN_IpcMsg_GetProtoVersion(msg),
 *              GWEN_IpcMsg_GetCode(msg));
 *     GWEN_Msg_free(msg);
 *   }
 * }
 * return 0;
 * </code>
 *
 * As seen above the application only needs to setup the endpoints according to the type of messages and connections
 * (in this case IPC messages over TCP) and then periodically call a combination of the following functions:
 * - @ref GWEN_MsgEndpoint_IoLoop() which internally calls the addSockets function, the select() systemcall and the checkSockets function
 * - @ref GWEN_MsgEndpoint_TakeFirstReceivedMessage() which takes the first received message from the recv msg queue
 * - @ref GWEN_MsgEndpoint_AddSendMessage() which adds a message to be sent
 *
 * If the application needs to manage multiple connections at the same time it can take advantage of the TREE2 functions for
 * message endpoints to create a tree of endpoints:
 *
 * <code>
 * GWEN_MSG_ENDPOINT *epRoot;
 * GWEN_MSG_ENDPOINT *epClient;
 *
 * epRoot=GWEN_MsgEndpoint_new(NAME, 0);
 * epClient=GWEN_IpcEndpoint_CreateIpcTcpClient("127.0.0.1", 55555, NULL, 1);
 * GWEN_MsgEndpoint_Tree2_AddChild(epRoot, epClient);
 * GWEN_MsgEndpoint_AddSendMessage(epClient, msg);
 *
 * for (loop=0;; loop++) {
 *   GWEN_MsgEndpoint_ChildrenIoLoop(epRoot, 2000);
 *
 *   while( (msg=GWEN_MsgEndpoint_TakeFirstReceivedMessage(epClient)) ) {
 *     DBG_INFO(GWEN_LOGDOMAIN,
 *              "  - received msg: protoId=%d, protoVer=%d, code=%d",
 *              GWEN_IpcMsg_GetProtoId(msg),
 *              GWEN_IpcMsg_GetProtoVersion(msg),
 *              GWEN_IpcMsg_GetCode(msg));
 *     GWEN_Msg_free(msg);
 *   }
 * }
 * return 0;
 * </code>
 *
 * When working with a tree of endpoints (i.e. in this case it rather is a list) the function
 * @ref GWEN_MsgEndpoint_ChildrenIoLoop() needs to be called with the root endpoint as argument instead
 * of @ref GWEN_MsgEndpoint_IoLoop.
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



GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_new(const char *name, int groupId);
GWENHYWFAR_API void GWEN_MsgEndpoint_Attach(GWEN_MSG_ENDPOINT *ep);

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

GWENHYWFAR_API uint32_t GWEN_MsgEndpoint_GetNextMessageId(GWEN_MSG_ENDPOINT *ep);


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

/**
 * Does one IO loop which internally does this:
 * - call @ref GWEN_MsgEndpoint_AddSockets()
 * - call @ref GWEN_Socket_Select()
 * - call @ref GWEN_MsgEndpoint_CheckSockets() which handles reading and writing of messages
 *
 * @param ep endpoint to handle
 * @param timeout timeout in milliseconds for the @ref GWEN_Socket_Select() call.
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_IoLoop(GWEN_MSG_ENDPOINT *ep, int timeout);





GWENHYWFAR_API void GWEN_MsgEndpoint_ChildrenAddSockets(GWEN_MSG_ENDPOINT *ep,
                                                         GWEN_SOCKETSET *readSet,
                                                         GWEN_SOCKETSET *writeSet,
                                                         GWEN_SOCKETSET *xSet);

GWENHYWFAR_API void GWEN_MsgEndpoint_ChildrenCheckSockets(GWEN_MSG_ENDPOINT *ep,
                                                           GWEN_SOCKETSET *readSet,
                                                           GWEN_SOCKETSET *writeSet,
                                                           GWEN_SOCKETSET *xSet);

/**
 * Handles IO for all first-level children of the given endpoint (like @ref GWEN_MsgEndpoint_IoLoop does for
 * a single endpoint).
 *
 * @param ep endpoint whose direct first-level children are handles
 * @param timeout timeout in milliseconds for the select() system call internally used
 */
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


/*@}*/

#ifdef __cplusplus
}
#endif


#endif


