/***************************************************************************
    begin       : Wed Feb 03 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GWEN_THREAD_H
#define GWEN_THREAD_H


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list1.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_THREAD GWEN_THREAD;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_THREAD, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_THREAD, GWEN_Thread, GWENHYWFAR_API)


typedef GWENHYWFAR_CB void (*GWEN_THREAD_RUN_FN)(GWEN_THREAD *thr);



GWENHYWFAR_API GWEN_THREAD *GWEN_Thread_new();
GWENHYWFAR_API void GWEN_Thread_free(GWEN_THREAD *thr);

GWENHYWFAR_API int GWEN_Thread_Start(GWEN_THREAD *thr);

GWENHYWFAR_API int GWEN_Thread_Join(GWEN_THREAD *thr);


GWENHYWFAR_API GWEN_THREAD_RUN_FN GWEN_Thread_SetRunFn(GWEN_THREAD *thr, GWEN_THREAD_RUN_FN fn);



#ifdef __cplusplus
}
#endif



#endif

