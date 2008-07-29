/***************************************************************************
    begin       : Tue Jul 29 2008
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


#ifndef GWEN_SEMAPHORE_P_H
#define GWEN_SEMAPHORE_P_H


#include <gwenhywfar/gwensemaphore.h>

#ifdef APPLE
# include <mach/mach_init.h>
# include <mach/semaphore.h>
#else
# include <semaphore.h>
#endif

#include <pthread.h>


struct GWEN_SEMAPHORE {
#ifdef APPLE
  MPSemaphoreID semId;
#else
  sem_t sem;
#endif
};




#endif
