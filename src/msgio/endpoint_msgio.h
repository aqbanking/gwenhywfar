/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_MSGIO_H
#define GWEN_MSG_ENDPOINT_MSGIO_H


#include <gwenhywfar/endpoint.h>

#include <time.h>



#ifdef __cplusplus
extern "C" {
#endif


/**
 * This defines a callback function to be used to determine how many bytes a message still needs to receive
 * to complete a message.
 * The function should return exactly 0 if the message is complete (i.e. doesn't need more bytes) and negative values
 * on error. All positive values >0 define how many bytes the given message still needs to be complete.
 *
 * This function is called multiple times per message until it returns 0 (signalling that the message is complete) or
 * a negative value (signalling an error).
 */
typedef int (*GWEN_ENDPOINT_MSGIO_GETBYTESNEEDED_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *msg);


typedef int (*GWEN_ENDPOINT_MSGIO_SENDMSGSTART_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *msg);
typedef void (*GWEN_ENDPOINT_MSGIO_SENDMSGFINISH_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *msg);


/**
 * Create a base class for message based endpoints (which are basically all endpoints).
 * It contains code to automatically read and write messages from/to the internal queues
 * (see @ref GWEN_MsgEndpoint_GetReceivedMessageList and @ref GWEN_MsgEndpoint_GetSendMessageList).
 *
 * For this class to work with every type of message protocol a callback must be set via @ref GWEN_MsgIoEndpoint_SetGetNeededBytesFn.
 */
GWENHYWFAR_API void GWEN_MsgIoEndpoint_Extend(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API void GWEN_MsgIoEndpoint_SetGetNeededBytesFn(GWEN_MSG_ENDPOINT *ep, GWEN_ENDPOINT_MSGIO_GETBYTESNEEDED_FN f);

GWENHYWFAR_API void GWEN_MsgIoEndpoint_SetSendMsgStartFn(GWEN_MSG_ENDPOINT *ep, GWEN_ENDPOINT_MSGIO_SENDMSGSTART_FN f);
GWENHYWFAR_API void GWEN_MsgIoEndpoint_SetSendMsgFinishFn(GWEN_MSG_ENDPOINT *ep, GWEN_ENDPOINT_MSGIO_SENDMSGFINISH_FN f);


#ifdef __cplusplus
}
#endif


#endif


