/***************************************************************************
  $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Dec 27 2003
    copyright   : (C) 2003 by Martin Preuss
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



#include "process_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>



GWEN_ERRORCODE GWEN_Process_ModuleInit(){
  return 0;
}



GWEN_ERRORCODE GWEN_Process_ModuleFini(){
  return 0;
}



GWEN_PROCESS *GWEN_Process_new(){
  GWEN_PROCESS *pr;

  GWEN_NEW_OBJECT(GWEN_PROCESS, pr);
  pr->state=GWEN_ProcessStateNotStarted;
  pr->pid=-1;
  return pr;
}



void GWEN_Process_free(GWEN_PROCESS *pr){
  if (pr) {
    /* unlink from list */
    free(pr);
  }
}



GWEN_PROCESS_STATE GWEN_Process_Start(GWEN_PROCESS *pr,
				      const char *prg,
				      const char *args){
  return GWEN_ProcessStateNotStarted;
}



GWEN_PROCESS_STATE GWEN_Process_CheckState(GWEN_PROCESS *pr){
  return GWEN_ProcessStateUnknown;
}



int GWEN_Process_GetResult(GWEN_PROCESS *pr){
  assert(pr);
  return -1;
}



int GWEN_Process_Wait(GWEN_PROCESS *pr){
  return -1;
}



int GWEN_Process_Terminate(GWEN_PROCESS *pr){
  return -1;
}


























