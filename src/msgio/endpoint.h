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
#include <gwenhywfar/list.h>



#define GWEN_MSG_ENDPOINT_FLAGS_DELETE        0x80000000u
#define GWEN_MSG_ENDPOINT_FLAGS_NOIO          0x40000000u
#define GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED  0x20000000u


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Object which can send and receive messages (base class).
 *
 */
typedef struct GWEN_MSG_ENDPOINT GWEN_MSG_ENDPOINT;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT, GWEN_MsgEndpoint, GWENHYWFAR_API)

#ifdef __cplusplus
}
#endif


#include <gwenhywfar/endpointmgr.h>
#include <gwenhywfar/msg.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_MSGENDPOINT Message Sender/Receiver
 * @ingroup MOD_MSGIO
 *
 * @brief This file contains the definition of a GWEN_MSG_ENDPOINT, an object which
 * can send and receive messages (base class).
 *
 */
/*@{*/


/** @name Virtual Function Definitions
 *
 */
/*@{*/
typedef int (*GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
typedef int (*GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
typedef void (*GWEN_MSG_ENDPOINT_RUN_FN)(GWEN_MSG_ENDPOINT *ep);
typedef GWEN_MSG_ENDPOINT* (*GWEN_MSG_ENDPOINT_CREATECHILD_FN)(GWEN_MSG_ENDPOINT *ep);
typedef int (*GWEN_MSG_ENDPOINT_GET_READFD_FN)(GWEN_MSG_ENDPOINT *ep);
typedef int (*GWEN_MSG_ENDPOINT_GET_WRITEFD_FN)(GWEN_MSG_ENDPOINT *ep);
typedef void (*GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
typedef int (*GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
/*@}*/



/** @name Constructor, Destructor
 *
 */
/*@{*/

/**
 * Constructor.
 *
 * @return endpoint created (NULL on error)
 * @param name name of this endpoint type (e.g. "tcp client"), for debugging purposes
 * @param groupId can be used by applications or derived endpoint manager classes to differentiate
 *  between multiple message types (not used internally)
 */
GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_new(const char *name, int groupId);

/**
 * Destructor.
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_free(GWEN_MSG_ENDPOINT *ep);
/*@}*/


/** @name Getter, Setter
 *
 */
/*@{*/

GWENHYWFAR_API int GWEN_MsgEndpoint_GetGroupId(const GWEN_MSG_ENDPOINT *ep);

/**
 * The endpoint class stores a file descriptor on behalf of derived classes. This file descriptor is
 * not used by this class directly. When sampling file descriptors for the "select" call internally always
 * the virtual functions @ref GWEN_MsgEndpoint_GetReadFd and @ref GWEN_MsgEndpoint_GetWriteFd are used.
 * @return stored file descriptor (-1 on error or if not set)
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_GetFd(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set file descriptor.
 *
 * @param ep endpoint
 * @param fd file descriptor to store (use -1 for invalid descriptor)
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetFd(GWEN_MSG_ENDPOINT *ep, int fd);

/**
 * Return name of this endpoint class (used for debugging purposes).
 *
 * @return class name as set by the constructor
 * @param ep endpoint
 */
GWENHYWFAR_API const char *GWEN_MsgEndpoint_GetName(const GWEN_MSG_ENDPOINT *ep);


/**
 * Default maximum buffer size when creating message objects (see @ref GWEN_MSG).
 *
 * @return default maximum buffer size
 * @param ep endpoint
 */
GWENHYWFAR_API uint32_t GWEN_MsgEndpoint_GetDefaultBufferSize(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set default maximum buffer size for messages.
 *
 * @param ep endpoint
 * @param v size in bytes
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetDefaultBufferSize(GWEN_MSG_ENDPOINT *ep, uint32_t v);

/**
 * Get flags (see @ref GWEN_MSG_ENDPOINT_FLAGS_DELETE and following).
 *
 * @return current flags
 * @param ep endpoint
 */
GWENHYWFAR_API uint32_t GWEN_MsgEndpoint_GetFlags(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set new flags.
 *
 * @param ep endpoint
 * @param f new flags
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);

/**
 * Add flags.
 *
 * @param ep endpoint
 * @param f flags to add to current flags
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_AddFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);

/**
 * Clear given flags.
 *
 * @param ep endpoint
 * @param f flags to clear from current flags
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_DelFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);
/*@}*/


/** @name Message Management
 *
 * Endpoints have one list for received messages and another list for outbound messages.
 * This base class doesn't have any knowledge of the format of messages exchanged, this is up
 * to derived endpoint classes.
 */
/*@{*/

/**
 * Get list of received messages. The endpoint object keeps ownership of the list returned.
 *
 * @return list of received messages
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetReceivedMessageList(const GWEN_MSG_ENDPOINT *ep);

/**
 * Get list of outbound messages. The endpoint object keeps ownership of the list returned.
 *
 * @return list of outbound messages
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetSendMessageList(const GWEN_MSG_ENDPOINT *ep);

/**
 * Add a received message. Mainly called from inside implementations of the virtual
 * function @ref GWEN_MsgEndpoint_HandleReadable.
 * Messages are directly added to the internal list without further handling.
 *
 * @param ep endpoint
 * @param m message to add (endpoint takes over ownership
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_AddReceivedMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

/**
 * Get the first message from the list of received messages and remove it from the list
 * The caller takes over ownership of the message returned (if any), free it
 * via @ref GWEN_Msg_free.
 *
 * @return first message taken from the incoming list (NULL if none)
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_TakeFirstReceivedMessage(GWEN_MSG_ENDPOINT *ep);

/**
 * Add an outbound message.
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_AddSendMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

/**
 * Return the first outbound message. The object returned (if any) remains in the list
 * of outbound messages and the endpoint object keeps ownership.
 *
 * @return first message in the list of outbound messages (NULL if none)
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetFirstSendMessage(const GWEN_MSG_ENDPOINT *ep);

/**
 * Check whether there are outbound messages in the internal list.
 *
 * @return 1 if there is at least one outbound message, 0 otherwise
 * @param ep endpoint
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_HaveMessageToSend(const GWEN_MSG_ENDPOINT *ep);

/**
 * This virtual function is called with a message to be forwarded to this endpoint.
 * Example:
 * The project AqHome uses multiple message types:
 * - messages received from or sent to a network of microcontrollers
 * - MQTT messages forwarded to an MQTT server
 * - IPC messages for clients connecting to the aqhome daemon.
 * For every message type there is a dedicated endpoint class. However, messages from the network of
 * microcontrollers (e.g. temperature measurements, humidity etc) need to be forwarded from the mcu network to an
 * MQTT server.
 * For this the derived endpoint manager class (see @ref GWEN_MSG_ENDPOINT_MGR) of aqhome takes incoming messages from
 * the MCU endpoint class and calls GWEN_MsgEndpoint_ProcessOutMessage of the MQTT endpoint to transform the message into
 * something the MQTT server understands, because we can't just send an MCU message directly to the MQTT server (different
 * protocol).
 *
 * So the implementation of this class should look at the message presented here and internally make another message from this
 * which can then be send to the recipient of this endpoint (e.g. the MQTT server).
 * The way to look at such a message is to use the group id (see @ref GWEN_Msg_GetGroupId and @ref GWEN_MsgEndpoint_GetGroupId).
 * This is always set by the source endpoint class which received the message to the groupid of the receiving endpoint.
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_ProcessOutMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
GWENHYWFAR_API int GWEN_MsgEndpoint_IsMsgComplete(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetCurrentlyReceivedMsg(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

GWENHYWFAR_API int GWEN_MsgEndpoint_GetAcceptedGroupIds(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetAcceptedGroupIds(GWEN_MSG_ENDPOINT *ep, int i);
GWENHYWFAR_API void GWEN_MsgEndpoint_AddAcceptedGroupIds(GWEN_MSG_ENDPOINT *ep, int i);
GWENHYWFAR_API void GWEN_MsgEndpoint_DelAcceptedGroupIds(GWEN_MSG_ENDPOINT *ep, int i);


/*@}*/


/** @name Input/Output
 *
 * Endpoints are generally based on file descriptors/sockets (see glibc function "open").
 * The implementation of an endpoint class uses these virtual functions here to actually handle
 * IO.
 * The class @ref GWEN_MSG_ENDPOINT_MGR calls @ref GWEN_MsgEndpoint_GetReadFd to get a readable file descriptor
 * from endpoints and @ref GWEN_MsgEndpoint_GetWriteFd to get the file descriptor for writing.
 * When file descriptors are returned those are then used for a call to "select" to wait for activity.
 * If a file descriptor becomes readable @ref GWEN_MsgEndpoint_HandleReadable is called.
 * If a file descriptor becomes writable @ref GWEN_MsgEndpoint_HandleWritable is called.
 * If one of those functions fails (i.e. returns an error code) the endpoint is marked as "DISCONNECTED"
 * (i.e. the flag @ref GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED is added to the endpoints flags).
 */
/*@{*/
GWENHYWFAR_API int GWEN_MsgEndpoint_GetReadFd(GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API int GWEN_MsgEndpoint_GetWriteFd(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint_HandleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
GWENHYWFAR_API int GWEN_MsgEndpoint_HandleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
GWENHYWFAR_API void GWEN_MsgEndpoint_Run(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_CreateChild(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint_DiscardInput(GWEN_MSG_ENDPOINT *ep);

/*@}*/


/** @name Setters for Virtual Functions
 *
 */
/*@{*/
GWENHYWFAR_API GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN GWEN_MsgEndpoint_SetHandleReadableFn(GWEN_MSG_ENDPOINT *ep,
                                                                                        GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN GWEN_MsgEndpoint_SetHandleWritableFn(GWEN_MSG_ENDPOINT *ep,
                                                                                        GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_GET_READFD_FN GWEN_MsgEndpoint_SetGetReadFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                               GWEN_MSG_ENDPOINT_GET_READFD_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_GET_WRITEFD_FN GWEN_MsgEndpoint_SetGetWriteFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                                 GWEN_MSG_ENDPOINT_GET_WRITEFD_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_RUN_FN GWEN_MsgEndpoint_SetRunFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_RUN_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN GWEN_MsgEndpoint_SetProcessOutMsgFn(GWEN_MSG_ENDPOINT *ep,
                                                                                    GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN GWEN_MsgEndpoint_SetIsMsgCompleteFn(GWEN_MSG_ENDPOINT *ep,
                                                                                      GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_CREATECHILD_FN GWEN_MsgEndpoint_SetCreateChildFn(GWEN_MSG_ENDPOINT *ep,
                                                                                  GWEN_MSG_ENDPOINT_CREATECHILD_FN f);

/*@}*/


/*@}*/

#ifdef __cplusplus
}
#endif


#endif


