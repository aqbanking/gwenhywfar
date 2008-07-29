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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "semaphore_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




GWEN_SEMAPHORE *GWEN_Semaphore_new() {
  GWEN_SEMAPHORE *sm;

  GWEN_NEW_OBJECT(GWEN_SEMAPHORE, sm);
  sem_init(&(sm->sem), 0, 0);

  return sm;
}



void GWEN_Semaphore_free(GWEN_SEMAPHORE *sm) {
  if (sm) {
    sem_destroy(&(sm->sem));
    GWEN_FREE_OBJECT(sm);
  }
}



void GWEN_Semaphore_Wait(GWEN_SEMAPHORE *sm) {
  assert(sm);
  sem_wait(&(sm->sem));
}



void GWEN_Semaphore_Post(GWEN_SEMAPHORE *sm) {
  assert(sm);
  sem_post(&(sm->sem));
}





