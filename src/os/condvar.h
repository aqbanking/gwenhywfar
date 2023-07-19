/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_COND_VAR_H
#define GWEN_COND_VAR_H

#include <gwenhywfar/mutex.h>


#define GWEN_COND_VAR_FLAGS_MULTITHREADS 0x0001


typedef struct GWEN_COND_VAR GWEN_COND_VAR;


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_COND_VAR *GWEN_ConditionalVar_new(uint32_t flags);
GWENHYWFAR_API void GWEN_ConditionalVar_free(GWEN_COND_VAR *cv);

GWENHYWFAR_API void GWEN_ConditionalVar_Signal(GWEN_COND_VAR *cv);
GWENHYWFAR_API void GWEN_ConditionalVar_Wait(GWEN_COND_VAR *cv, GWEN_MUTEX *mtx);



#ifdef __cplusplus
}
#endif



#endif

