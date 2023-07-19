/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MUTEX_POSIX_H
#define GWEN_MUTEX_POSIX_H

#include <gwenhywfar/mutex.h>

#include <pthread.h>



pthread_mutex_t *GWEN_Mutex_GetMutex(GWEN_MUTEX *mtx);



#endif
