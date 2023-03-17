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


typedef struct GWEN_MSG GWEN_MSG;
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_MSG, GWEN_Msg, GWENHYWFAR_API)



GWENHYWFAR_API GWEN_MSG *GWEN_Msg_new(uint32_t bufferSize);
GWENHYWFAR_API void GWEN_Msg_Attach(GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_free(GWEN_MSG *msg);
GWENHYWFAR_API GWEN_MSG *GWEN_Msg_dup(const GWEN_MSG *srcMsg);

GWENHYWFAR_API int GWEN_Msg_GetGroupId(const GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_Msg_SetGroupId(GWEN_MSG *msg, int groupId);

GWENHYWFAR_API uint8_t *GWEN_Msg_GetBuffer(GWEN_MSG *msg);
GWENHYWFAR_API const uint8_t *GWEN_Msg_GetConstBuffer(const GWEN_MSG *msg);
GWENHYWFAR_API uint32_t GWEN_Msg_GetBytesInBuffer(const GWEN_MSG *msg);
GWENHYWFAR_API uint32_t GWEN_Msg_GetCurrentPos(const GWEN_MSG *msg);
GWENHYWFAR_API int GWEN_Msg_AddByte(GWEN_MSG *msg, uint8_t b);
GWENHYWFAR_API int GWEN_Msg_ReadNextByte(GWEN_MSG *msg);
GWENHYWFAR_API int GWEN_Msg_IncCurrentPos(GWEN_MSG *msg, uint32_t i);
GWENHYWFAR_API int GWEN_Msg_GetRemainingBytes(const GWEN_MSG *msg);
GWENHYWFAR_API int GWEN_Msg_RewindCurrentPos(GWEN_MSG *msg);



#endif

