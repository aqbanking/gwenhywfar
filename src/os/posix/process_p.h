/***************************************************************************
    begin       : Sat Dec 27 2003
    copyright   : (C) 2003-2010 by Martin Preuss
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


#ifndef GWENHYWFAR_PROCESS_P_H
#define GWENHYWFAR_PROCESS_P_H "$Id"


#include <gwenhywfar/process.h>
#include <sys/types.h>
#include <signal.h>



struct GWEN_PROCESS {
  GWEN_PROCESS *next;
  unsigned int usage;
  pid_t pid;
  int result;
  unsigned int flags;
  int filesStdin[2];
  int filesStdout[2];
  int filesStderr[2];
  GWEN_PROCESS_STATE state;
  uint32_t pflags;
  GWEN_SYNCIO *stdIn;
  GWEN_SYNCIO *stdOut;
  GWEN_SYNCIO *stdErr;

  char *filenameStdIn;
  char *filenameStdOut;
  char *filenameStdErr;

  char *folder;
};



static
GWEN_PROCESS_STATE GWEN_Process_GetState(GWEN_PROCESS *pr, int w);

static
GWEN_PROCESS_STATE GWEN_Process_MakeState(GWEN_PROCESS *pr, int status);

#endif


