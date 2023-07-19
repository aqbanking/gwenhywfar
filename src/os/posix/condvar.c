/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "condvar_p.h"
#include "os/posix/mutex.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>





GWEN_COND_VAR *GWEN_ConditionalVar_new(uint32_t flags)
{
  GWEN_COND_VAR *cv;

  GWEN_NEW_OBJECT(GWEN_COND_VAR, cv);
  pthread_cond_init(&(cv->conditionalVar), NULL);
  cv->flags=flags;

  return cv;
}



void GWEN_ConditionalVar_free(GWEN_COND_VAR *cv)
{
  if (cv) {
    pthread_cond_destroy(&(cv->conditionalVar));
    GWEN_FREE_OBJECT(cv);
  }
}



void GWEN_ConditionalVar_Signal(GWEN_COND_VAR *cv)
{
  if (cv->flags & GWEN_COND_VAR_FLAGS_MULTITHREADS)
    pthread_cond_broadcast(&(cv->conditionalVar));
  else
    pthread_cond_signal(&(cv->conditionalVar));
}



void GWEN_ConditionalVar_Wait(GWEN_COND_VAR *cv, GWEN_MUTEX *mtx)
{
  pthread_cond_wait(&(cv->conditionalVar), GWEN_Mutex_GetMutex(mtx));
}




