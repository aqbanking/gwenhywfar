/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/


#ifndef GWEN_MSG_MSG_H
#define GWEN_MSG_MSG_H


#include <gwenhywfar/list.h>
#include <gwenhywfar/buffer.h>



#define GWEN_MSG_FLAGS_PAYLOADINFO_SET 0x80000000u



typedef struct GWEN_MSG GWEN_MSG;
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_MSG, GWEN_Msg, GWENHYWFAR_API)



GWENHYWFAR_API GWEN_MSG *GWEN_Msg_new(uint32_t bufferSize);
GWENHYWFAR_API GWEN_MSG *GWEN_Msg_fromBytes(const uint8_t *ptr, uint32_t len);

GWENHYWFAR_API void GWEN_Msg_Attach(GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_free(GWEN_MSG *msg);
GWENHYWFAR_API GWEN_MSG *GWEN_Msg_dup(const GWEN_MSG *srcMsg);

GWENHYWFAR_API int GWEN_Msg_GetGroupId(const GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_SetGroupId(GWEN_MSG *msg, int groupId);

GWENHYWFAR_API uint32_t GWEN_Msg_GetMaxSize(const GWEN_MSG *msg);
GWENHYWFAR_API uint8_t *GWEN_Msg_GetBuffer(GWEN_MSG *msg);
GWENHYWFAR_API const uint8_t *GWEN_Msg_GetConstBuffer(const GWEN_MSG *msg);
GWENHYWFAR_API uint32_t GWEN_Msg_GetBytesInBuffer(const GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_SetBytesInBuffer(GWEN_MSG *msg, uint32_t i);
GWENHYWFAR_API uint32_t GWEN_Msg_GetCurrentPos(const GWEN_MSG *msg);
GWENHYWFAR_API int GWEN_Msg_AddByte(GWEN_MSG *msg, uint8_t b);
GWENHYWFAR_API int GWEN_Msg_AddBytes(GWEN_MSG *msg, const uint8_t *bufferPtr, uint32_t bufferLen);
GWENHYWFAR_API int GWEN_Msg_ReadNextByte(GWEN_MSG *msg);
GWENHYWFAR_API int GWEN_Msg_IncCurrentPos(GWEN_MSG *msg, uint32_t i);
GWENHYWFAR_API int GWEN_Msg_GetRemainingBytes(const GWEN_MSG *msg);
GWENHYWFAR_API int GWEN_Msg_RewindCurrentPos(GWEN_MSG *msg);


/** @name Parsed Information
 *
 * This group concerns data which is parsed by e.g. implementations of @ref GWEN_MSG_ENDPOINT
 */
/*@{*/
GWENHYWFAR_API uint32_t GWEN_Msg_GetParsedPayloadSize(const GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_SetParsedPayloadSize(GWEN_MSG *msg, uint32_t v);

GWENHYWFAR_API uint32_t GWEN_Msg_GetParsedPayloadOffset(const GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_SetParsedPayloadOffset(GWEN_MSG *msg, uint32_t v);
/*@}*/


GWENHYWFAR_API uint32_t GWEN_Msg_GetFlags(const GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_SetFlags(GWEN_MSG *msg, uint32_t f);
GWENHYWFAR_API void GWEN_Msg_AddFlags(GWEN_MSG *msg, uint32_t f);
GWENHYWFAR_API void GWEN_Msg_DelFlags(GWEN_MSG *msg, uint32_t f);


GWENHYWFAR_API uint32_t GWEN_Msg_GetUint32At(const GWEN_MSG *msg, int offs, int defaultValue);
GWENHYWFAR_API uint16_t GWEN_Msg_GetUint16At(const GWEN_MSG *msg, int offs, int defaultValue);
GWENHYWFAR_API uint8_t GWEN_Msg_GetUint8At(const GWEN_MSG *msg, int offs, int defaultValue);


GWENHYWFAR_API void GWEN_Msg_Dump(const GWEN_MSG *msg, GWEN_BUFFER *buf);


#endif


