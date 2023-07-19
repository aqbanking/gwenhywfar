/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_COND_VAR_WINDOWS_P_H
#define GWEN_COND_VAR_WINDOWS_P_H


#include <gwenhywfar/condvar.h>


#include <windows.h>
/*#include <synchapi.h> (needed?)*/


struct GWEN_COND_VAR {
  CONDITION_VARIABLE conditionVar;
  uint32_t flags;
};


#endif

