/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MUTEX_WINDOWS_H
#define GWEN_MUTEX_WINDOWS_H

#include <gwenhywfar/mutex.h>

#include <windows.h>



CRITICAL_SECTION *GWEN_Mutex_GetCriticalSection(GWEN_MUTEX *mtx);


#endif
