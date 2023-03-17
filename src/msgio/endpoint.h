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



#define GWEN_MSG_ENDPOINT_FLAGS_NOIO       0x0001


#ifdef __cplusplus
extern "C" {
#endif


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


typedef int (*GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
typedef int (*GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
typedef void (*GWEN_MSG_ENDPOINT_RUN_FN)(GWEN_MSG_ENDPOINT *ep);
typedef int (*GWEN_MSG_ENDPOINT_GET_READFD_FN)(GWEN_MSG_ENDPOINT *ep);
typedef int (*GWEN_MSG_ENDPOINT_GET_WRITEFD_FN)(GWEN_MSG_ENDPOINT *ep);




GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_new(const char *name, int groupId);
GWENHYWFAR_API void GWEN_MsgEndpoint_free(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint_GetGroupId(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint_GetFd(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetFd(GWEN_MSG_ENDPOINT *ep, int fd);

GWENHYWFAR_API const char *GWEN_MsgEndpoint_GetName(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API uint32_t GWEN_MsgEndpoint_GetFlags(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);
GWENHYWFAR_API void GWEN_MsgEndpoint_AddFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);
GWENHYWFAR_API void GWEN_MsgEndpoint_DelFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);

GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetReceivedMessageList(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetSendMessageList(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API void GWEN_MsgEndpoint_AddReceivedMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_TakeFirstReceivedMessage(GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_AddSendMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetFirstSendMessage(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API int GWEN_MsgEndpoint_HaveMessageToSend(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetCurrentlyReceivedMsg(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);


GWENHYWFAR_API int GWEN_MsgEndpoint_GetReadFd(GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API int GWEN_MsgEndpoint_GetWriteFd(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint_HandleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
GWENHYWFAR_API int GWEN_MsgEndpoint_HandleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
GWENHYWFAR_API void GWEN_MsgEndpoint_Run(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpoint_DiscardInput(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN GWEN_MsgEndpoint_SetHandleReadableFn(GWEN_MSG_ENDPOINT *ep,
                                                                                        GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN GWEN_MsgEndpoint_SetHandleWritableFn(GWEN_MSG_ENDPOINT *ep,
                                                                                        GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_GET_READFD_FN GWEN_MsgEndpoint_SetGetReadFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                               GWEN_MSG_ENDPOINT_GET_READFD_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_GET_WRITEFD_FN GWEN_MsgEndpoint_SetGetWriteFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                                 GWEN_MSG_ENDPOINT_GET_WRITEFD_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_RUN_FN GWEN_MsgEndpoint_SetRunFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_RUN_FN f);


#ifdef __cplusplus
}
#endif


#endif


