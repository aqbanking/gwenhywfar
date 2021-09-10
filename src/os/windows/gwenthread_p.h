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


#ifndef GWEN_THREAD_P_H
#define GWEN_THREAD_P_H


#include "gwenthread.h"

#include <windows.h>




struct GWEN_THREAD {
  GWEN_INHERIT_ELEMENT(GWEN_THREAD)
  GWEN_LIST_ELEMENT(GWEN_THREAD)

  HANDLE threadHandle;
  DWORD threadId;
  GWEN_THREAD_RUN_FN runFn;
  uint32_t flags;
};




#endif

