/***************************************************************************
    begin       : Thu Feb 04 2021
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


#ifndef GWEN_THREADLOCALDATA_H
#define GWEN_THREADLOCALDATA_H


#include <gwenhywfar/gwenhywfarapi.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_THREADLOCAL_DATA GWEN_THREADLOCAL_DATA;



GWENHYWFAR_API GWEN_THREADLOCAL_DATA *GWEN_ThreadLocalData_new();

GWENHYWFAR_API void GWEN_ThreadLocalData_free(GWEN_THREADLOCAL_DATA *threadLocalData);

GWENHYWFAR_API int GWEN_ThreadLocalData_SetData(GWEN_THREADLOCAL_DATA *threadLocalData, void *pointer);
GWENHYWFAR_API void *GWEN_ThreadLocalData_GetData(const GWEN_THREADLOCAL_DATA *threadLocalData);


#ifdef __cplusplus
}
#endif



#endif

