/***************************************************************************
  $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Dec 28 2003
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
#include <windows.h>



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
  return pr;
}



void GWEN_Process_free(GWEN_PROCESS *pr){
  if (pr) {
    /* close handles */
    CloseHandle(pr->processInfo.hThread);
    CloseHandle(pr->processInfo.hProcess);
    free(pr);
  }
}



GWEN_PROCESS_STATE GWEN_Process_Start(GWEN_PROCESS *pr,
                                      const char *prg,
                                      const char *args){
  STARTUPINFO si;
  char *cmdline;

  memset(&si, 0, sizeof(si));
  si.cb=sizeof(si);
  GetStartupInfo(&si);

  cmdline=(char*)malloc(strlen(prg)+strlen(args)+2);
  strcpy(cmdline, prg);
  strcat(cmdline, " ");
  strcat(cmdline, args);

  pr->finished=0;

  if (CreateProcess(NULL,              /* lpszApplicationName */
                    cmdline,           /* lpszCommandLine */
                    NULL,              /* lpsaProcess */
                    NULL,              /* lpsaThread */
                    TRUE,              /* fInheritHandles */
                    0,                 /* fdwCreate */
                    NULL,              /* lpvEnvironment */
                    NULL,              /* lpszCurDir */
                    &si,               /* lpsiStartInfo */
                    &(pr->processInfo) /* lppiProcInfo */
                   )!=TRUE) {
    DBG_ERROR(0, "Error executing \"%s\" (%d)",
              cmdline, (int)GetLastError());
    free(cmdline);
    return GWEN_ProcessStateNotStarted;
  }
  pr->state=GWEN_ProcessStateRunning;
  free(cmdline);
  return pr->state;
}



GWEN_PROCESS_STATE GWEN_Process_CheckState(GWEN_PROCESS *pr){
  DWORD dwExitCode;

  if (pr->finished)
    return pr->state;

  if (GetExitCodeProcess(pr->processInfo.hProcess,
                         &dwExitCode)==TRUE) {
    /* ok */
    return GWEN_Process_MakeState(pr, dwExitCode);
  }
  else {
    DBG_ERROR(0, "Error getting exitcode (%d)", (int)GetLastError());
    return GWEN_ProcessStateUnknown;
  }
}



int GWEN_Process_GetResult(GWEN_PROCESS *pr){
  GWEN_Process_CheckState(pr);
  if (pr->finished)
    return pr->result;
  return -1;
}



GWEN_PROCESS_STATE GWEN_Process_MakeState(GWEN_PROCESS *pr, DWORD status){
  if (status==STILL_ACTIVE) {
    pr->state=GWEN_ProcessStateRunning;
  }
  else {
    pr->state=GWEN_ProcessStateExited;
    pr->result=status;
    pr->finished=1;
  }

  return pr->state;
}



int GWEN_Process_Wait(GWEN_PROCESS *pr){
  DWORD dwExitCode;

  if (pr->finished)
    return 0;

  WaitForSingleObject(pr->processInfo.hProcess, INFINITE);
  if (GetExitCodeProcess(pr->processInfo.hProcess,
                         &dwExitCode)==TRUE) {
    /* ok */
    GWEN_Process_MakeState(pr, dwExitCode);
    return 0;
  }
  else {
    DBG_ERROR(0, "Error getting exitcode (%d)", (int)GetLastError());
    return -1;
  }
}



int GWEN_Process_Terminate(GWEN_PROCESS *pr){

  if (TerminateProcess(pr->processInfo.hProcess,
                       GWEN_PROCESS_EXITCODE_ABORT)!=TRUE) {
    DBG_ERROR(0, "Error terminating process (%d)", (int)GetLastError());
    return -1;
  }
  if (GWEN_Process_Wait(pr)) {
    return -1;
  }

  return 0;
}







