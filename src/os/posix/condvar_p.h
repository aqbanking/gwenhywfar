/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_COND_VAR_POSIX_P_H
#define GWEN_COND_VAR_POSIX_P_H


#include <gwenhywfar/condvar.h>

#include <pthread.h>


struct GWEN_COND_VAR {
  pthread_cond_t conditionalVar;
  uint32_t flags;
};


#endif

