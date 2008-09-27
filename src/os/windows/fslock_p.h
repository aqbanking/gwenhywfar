/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Mar 24 2004
    copyright   : (C) 2004 by Martin Preuss
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


#ifndef GWEN_FSLOCK_P_H
#define GWEN_FSLOCK_P_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>

#include "../fslock.h"
/* specify the include path here directly because when we add that
   directory to the include path, our file process.h shadows the
   mingw's version of that header file (process.h necessary for getpid
   on mingw). */


struct GWEN_FSLOCK {
  GWEN_LIST_ELEMENT(GWEN_FSLOCK)
  GWEN_FSLOCK_TYPE type;
  char *entryName;
  char *baseLockFilename;
  char *uniqueLockFilename;
  int lockCount;
};


GWEN_FSLOCK_RESULT GWEN_FSLock__Lock(GWEN_FSLOCK *fl);
int GWEN_FSLock__UnifyLockFileName(GWEN_BUFFER *nbuf);


#endif


