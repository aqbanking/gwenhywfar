/***************************************************************************
    begin       : Mon Jul 14 2008
    copyright   : (C) 2008 by Martin Preuss
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

#include "mutex_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




GWEN_MUTEX *GWEN_Mutex_new(void)
{
  GWEN_MUTEX *mtx;

  GWEN_NEW_OBJECT(GWEN_MUTEX, mtx);
  InitializeCriticalSection(&(mtx->criticalSection));

  return mtx;
}



void GWEN_Mutex_free(GWEN_MUTEX *mtx)
{
  if (mtx) {
    DeleteCriticalSection(&(mtx->criticalSection));
    GWEN_FREE_OBJECT(mtx);
  }
}



int GWEN_Mutex_Lock(GWEN_MUTEX *mtx)
{
  EnterCriticalSection(&(mtx->criticalSection));
  return 0;
}



int GWEN_Mutex_Unlock(GWEN_MUTEX *mtx)
{
  LeaveCriticalSection(&(mtx->criticalSection));
  return 0;
}



CRITICAL_SECTION *GWEN_Mutex_GetCriticalSection(GWEN_MUTEX *mtx)
{
  return &(mtx->criticalSection);
}



