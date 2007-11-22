/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IOREQUEST_H
#define GWEN_IOREQUEST_H

typedef struct GWEN_IO_REQUEST GWEN_IO_REQUEST;


#include <gwenhywfar/list2.h>

#ifdef __cplusplus
extern "C" {
#endif


GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_IO_REQUEST, GWEN_Io_Request, GWENHYWFAR_API)


#include <gwenhywfar/iolayer.h>



#define GWEN_IO_REQUEST_FLAGS_PACKETBEGIN 0x10000000
#define GWEN_IO_REQUEST_FLAGS_PACKETEND   0x20000000
#define GWEN_IO_REQUEST_FLAGS_READALL     0x40000000
#define GWEN_IO_REQUEST_FLAGS_WRITEALL    0x40000000 /* share with READALL */
#define GWEN_IO_REQUEST_FLAGS_FLUSH       0x80000000 

/**
 * This flag is used with disconnect requests. If set then the connection is closed in any case and all enqueued
 * requests for that connection are aborted. If this flag is not set then a disconnect request will fail while
 * there are read/write/accept requests.
 */
#define GWEN_IO_REQUEST_FLAGS_FORCE       0x01000000
#define GWEN_IO_REQUEST_FLAGS_TAKEOVER    0x02000000




typedef enum {
  GWEN_Io_Request_TypeUnknown=-1,
  GWEN_Io_Request_TypeConnect,
  GWEN_Io_Request_TypeDisconnect,
  GWEN_Io_Request_TypeRead,
  GWEN_Io_Request_TypeWrite
} GWEN_IO_REQUEST_TYPE;



typedef enum {
  GWEN_Io_Request_StatusFree=0,
  GWEN_Io_Request_StatusEnqueued,
  GWEN_Io_Request_StatusFinished
} GWEN_IO_REQUEST_STATUS;




typedef void (*GWEN_IO_REQUEST_FINISH_FN)(GWEN_IO_REQUEST *r, void *user_data);



GWENHYWFAR_API const char *GWEN_Io_RequestType_toString(GWEN_IO_REQUEST_TYPE t);
GWENHYWFAR_API const char *GWEN_Io_RequestStatus_toString(GWEN_IO_REQUEST_STATUS st);



GWENHYWFAR_API GWEN_IO_REQUEST *GWEN_Io_Request_new(GWEN_IO_REQUEST_TYPE t,
						    uint8_t *pBuffer,
						    uint32_t lBuffer,
						    GWEN_IO_REQUEST_FINISH_FN finishFn,
						    void *user_data,
						    uint32_t guiid);

GWENHYWFAR_API void GWEN_Io_Request_Attach(GWEN_IO_REQUEST *r);

GWENHYWFAR_API void GWEN_Io_Request_free(GWEN_IO_REQUEST *r);

GWENHYWFAR_API GWEN_IO_REQUEST_TYPE GWEN_Io_Request_GetType(const GWEN_IO_REQUEST *r);

GWENHYWFAR_API uint32_t GWEN_Io_Request_GetFlags(const GWEN_IO_REQUEST *r);
GWENHYWFAR_API void GWEN_Io_Request_SetFlags(GWEN_IO_REQUEST *r, uint32_t f);
GWENHYWFAR_API void GWEN_Io_Request_AddFlags(GWEN_IO_REQUEST *r, uint32_t f);
GWENHYWFAR_API void GWEN_Io_Request_SubFlags(GWEN_IO_REQUEST *r, uint32_t f);

GWENHYWFAR_API uint8_t *GWEN_Io_Request_GetBufferPtr(const GWEN_IO_REQUEST *r);
GWENHYWFAR_API uint32_t GWEN_Io_Request_GetBufferSize(const GWEN_IO_REQUEST *r);
GWENHYWFAR_API uint32_t GWEN_Io_Request_GetBufferPos(const GWEN_IO_REQUEST *r);

GWENHYWFAR_API uint32_t GWEN_Io_Request_GetGuiId(const GWEN_IO_REQUEST *r);
GWENHYWFAR_API void GWEN_Io_Request_SetGuiId(GWEN_IO_REQUEST *r, uint32_t i);

GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_Request_GetIoLayer(const GWEN_IO_REQUEST *r);

GWENHYWFAR_API GWEN_IO_REQUEST_STATUS GWEN_Io_Request_GetStatus(const GWEN_IO_REQUEST *r);

GWENHYWFAR_API int GWEN_Io_Request_GetResultCode(const GWEN_IO_REQUEST *r);

GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_Request_GetIncomingLayer(const GWEN_IO_REQUEST *r);
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_Request_TakeIncomingLayer(GWEN_IO_REQUEST *r);


#ifdef __cplusplus
}
#endif


#endif
