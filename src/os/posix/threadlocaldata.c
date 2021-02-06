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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "threadlocaldata_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <string.h>
#include <errno.h>



GWEN_THREADLOCAL_DATA *GWEN_ThreadLocalData_new()
{
  GWEN_THREADLOCAL_DATA *threadLocalData;
  int rv;

  GWEN_NEW_OBJECT(GWEN_THREADLOCAL_DATA, threadLocalData);
  rv=pthread_key_create(&(threadLocalData->key), NULL);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on pthread_key_create: %d (%s)", rv, strerror(rv));
    GWEN_ThreadLocalData_free(threadLocalData);
    return NULL;
  }

  return threadLocalData;
}


void GWEN_ThreadLocalData_free(GWEN_THREADLOCAL_DATA *threadLocalData)
{
  if (threadLocalData) {
    pthread_key_delete(threadLocalData->key);
    GWEN_FREE_OBJECT(threadLocalData);
  }
}



int GWEN_ThreadLocalData_SetData(GWEN_THREADLOCAL_DATA *threadLocalData, void *pointer)
{
  int rv;

  rv=pthread_setspecific(threadLocalData->key, pointer);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on pthread_setspecific: %d (%s)", rv, strerror(rv));
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



void *GWEN_ThreadLocalData_GetData(const GWEN_THREADLOCAL_DATA *threadLocalData)
{
  return pthread_getspecific(threadLocalData->key);
}





