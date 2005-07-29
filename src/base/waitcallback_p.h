/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jan 17 2004
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


#ifndef GWEN_WAIT_CALLBACK_P_H
#define GWEN_WAIT_CALLBACK_P_H


#include "waitcallback_l.h"
#include <time.h>


struct GWEN_WAITCALLBACK {
  GWEN_LIST_ELEMENT(GWEN_WAITCALLBACK)
  GWEN_INHERIT_ELEMENT(GWEN_WAITCALLBACK)
  /* No trailing semicolon here because this is a macro call */

  GWEN_WAITCALLBACK *previousCtx;
  GWEN_WAITCALLBACK *originalCtx;
  GWEN_WAITCALLBACK *instantiatedFrom;
  unsigned int level;

  GWEN_WAITCALLBACK_CHECKABORTFN checkAbortFn;
  GWEN_WAITCALLBACK_INSTANTIATEFN instantiateFn;
  GWEN_WAITCALLBACK_LOGFN logFn;

  char *id;

  char *enteredFromFile;
  int enteredFromLine;

  GWEN_TYPE_UINT64 pos;
  GWEN_TYPE_UINT64 total;

  char *text;
  char *units;

  GWEN_WAITCALLBACK_LIST *registeredCallbacks;

  time_t lastCalled;
  time_t lastEntered;
  int proposedDistance;

  int nestingLevel;

  int aborted;

  GWEN_TYPE_UINT32 usage;
};



void *GWEN_WaitCallback__HandlePathElement(const char *entry,
                                           void *data,
                                           unsigned int flags);
GWEN_WAITCALLBACK *GWEN_WaitCallback__FindTemplate(const char *s);

GWEN_WAITCALLBACK *GWEN_WaitCallback_Instantiate(GWEN_WAITCALLBACK *ctx);

GWEN_WAITCALLBACK *GWEN_WaitCallback__GetTemplateOf(GWEN_WAITCALLBACK *ctx);

GWEN_WAITCALLBACK_RESULT GWEN__WaitCallback(GWEN_WAITCALLBACK *ctx);

GWEN_WAITCALLBACK_RESULT GWEN__WaitCallback_r(GWEN_WAITCALLBACK *ctx);

void GWEN_WaitCallback__Dump_r(GWEN_WAITCALLBACK *ctx, int ins);
void GWEN_WaitCallback__Dump(GWEN_WAITCALLBACK *ctx);


#endif





