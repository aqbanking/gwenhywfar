/***************************************************************************
 begin       : Thu Feb 04 2021
 copyright   : (C) 2021 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWEN_TEST_THREAD_P_H
#define GWEN_TEST_THREAD_P_H


#include "testthread.h"


typedef struct TEST_THREAD TEST_THREAD;
struct TEST_THREAD {
  int threadId;
  int loops;
};


#endif
