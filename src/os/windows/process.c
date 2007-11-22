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

#include "../../io/bufferedio_p.h" /* very ugly */



int GWEN_Process_ModuleInit(){
  return 0;
}



int GWEN_Process_ModuleFini(){
  return 0;
}



GWEN_PROCESS *GWEN_Process_new(){
  GWEN_PROCESS *pr;

  GWEN_NEW_OBJECT(GWEN_PROCESS, pr);
  pr->state=GWEN_ProcessStateNotStarted;
  pr->pflags=GWEN_PROCESS_FLAGS_DEFAULT;
  return pr;
}



void GWEN_Process_free(GWEN_PROCESS *pr){
  if (pr) {
    /* close handles */
    CloseHandle(pr->processInfo.hThread);
    CloseHandle(pr->processInfo.hProcess);
    GWEN_BufferedIO_free(pr->stdIn);
    GWEN_BufferedIO_free(pr->stdOut);
    GWEN_BufferedIO_free(pr->stdErr);
    GWEN_FREE_OBJECT(pr);
  }
}



GWEN_PROCESS_STATE GWEN_Process_Start(GWEN_PROCESS *pr,
                                      const char *prg,
                                      const char *args){
  STARTUPINFO si;
  SECURITY_ATTRIBUTES saAttr;
  char *cmdline;
  HANDLE hChildStdinRd, hChildStdinWr;
  HANDLE hChildStdoutRd, hChildStdoutWr;
  HANDLE hChildStderrRd, hChildStderrWr;
  GWEN_PROCESS_STATE pst;

  memset(&si, 0, sizeof(si));
  si.cb=sizeof(si);
  GetStartupInfo(&si);

  si.dwFlags|=STARTF_USESTDHANDLES;

  /* set the bInheritHandle flag so pipe handles are inherited */
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  /* create a pipe for the child process's STDOUT */
  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDOUT) {
    DBG_INFO(GWEN_LOGDOMAIN, "Redirecting stdout");
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not create stdout pipe");
      return -1;
    }
    si.hStdOutput=hChildStdoutWr;
    DBG_INFO(GWEN_LOGDOMAIN, "Creating WinFile");
    pr->stdOut=GWEN_BufferedIO_WinFile_new(hChildStdoutRd);
    GWEN_BufferedIO_SetReadBuffer(pr->stdOut, 0, 128);
  }
  else
    si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);

  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDIN) {
    /* create a pipe for the child process's STDIN */
    DBG_INFO(GWEN_LOGDOMAIN, "Redirecting stdin");
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not create stdin pipe");
      return -1;
    }
    si.hStdInput=hChildStdinRd;
    pr->stdIn=GWEN_BufferedIO_WinFile_new(hChildStdinWr);
    DBG_INFO(GWEN_LOGDOMAIN, "Creating WinFile");
    GWEN_BufferedIO_SetWriteBuffer(pr->stdIn, 0, 128);
  }
  else
    si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);

  /* create a pipe for the child process's STDERR */
  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDERR) {
    DBG_INFO(GWEN_LOGDOMAIN, "Redirecting stderr");
    if (!CreatePipe(&hChildStderrRd, &hChildStderrWr, &saAttr, 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not create stderr pipe");
      return -1;
    }
    si.hStdError=hChildStderrWr;
    pr->stdErr=GWEN_BufferedIO_WinFile_new(hChildStderrRd);
    DBG_INFO(GWEN_LOGDOMAIN, "Creating WinFile");
    GWEN_BufferedIO_SetReadBuffer(pr->stdErr, 0, 128);
  }
  else
    si.hStdError=GetStdHandle(STD_ERROR_HANDLE);

  /* create the child process */
  DBG_INFO(GWEN_LOGDOMAIN, "Creating Command line");
  cmdline=(char*)malloc(strlen(prg)+strlen(args)+2);
  strcpy(cmdline, prg);
  strcat(cmdline, " ");
  strcat(cmdline, args);

  pr->finished=0;

  DBG_INFO(GWEN_LOGDOMAIN, "Starting process");
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
    DBG_ERROR(GWEN_LOGDOMAIN, "Error executing \"%s\" (%d)",
              cmdline, (int)GetLastError());
    pst=GWEN_ProcessStateNotStarted;
    /* error, close our end of the pipe. The other end will be closed
     * after this block */
    if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDOUT)
      CloseHandle(hChildStdoutRd);
    if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDIN)
      CloseHandle(hChildStdinWr);
    if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDERR)
      CloseHandle(hChildStderrRd);
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Process started");
    pst=GWEN_ProcessStateRunning;
  }

  pr->state=pst;
  free(cmdline);

  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDOUT)
    CloseHandle(hChildStdoutWr);
  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDIN)
    CloseHandle(hChildStdinRd);
  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDERR)
    CloseHandle(hChildStderrWr);

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
    DBG_ERROR(GWEN_LOGDOMAIN, "Error getting exitcode (%d)", (int)GetLastError());
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
    DBG_ERROR(GWEN_LOGDOMAIN, "Error getting exitcode (%d)", (int)GetLastError());
    return -1;
  }
}



int GWEN_Process_Terminate(GWEN_PROCESS *pr){
  if (TerminateProcess(pr->processInfo.hProcess,
                       GWEN_PROCESS_EXITCODE_ABORT)!=TRUE) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error terminating process (%d)", (int)GetLastError());
    return -1;
  }
  if (GWEN_Process_Wait(pr)) {
    return -1;
  }

  return 0;
}



void GWEN_Process_SetFlags(GWEN_PROCESS *pr, uint32_t f){
  assert(pr);
  pr->pflags=f;
}



void GWEN_Process_AddFlags(GWEN_PROCESS *pr, uint32_t f){
  assert(pr);
  pr->pflags|=f;
}



void GWEN_Process_SubFlags(GWEN_PROCESS *pr, uint32_t f){
  assert(pr);
  pr->pflags&=~f;
}



GWEN_BUFFEREDIO *GWEN_Process_GetStdin(const GWEN_PROCESS *pr){
  assert(pr);
  return pr->stdIn;
}



GWEN_BUFFEREDIO *GWEN_Process_GetStdout(const GWEN_PROCESS *pr){
  assert(pr);
  return pr->stdOut;
}



GWEN_BUFFEREDIO *GWEN_Process_GetStderr(const GWEN_PROCESS *pr){
  assert(pr);
  return pr->stdErr;
}








GWEN_INHERIT(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_WINFILE);

GWEN_BUFFEREDIO_WINFILE *GWEN_BufferedIO_WinFile_Table__new() {
  GWEN_BUFFEREDIO_WINFILE *bft;

  GWEN_NEW_OBJECT(GWEN_BUFFEREDIO_WINFILE, bft);
  bft->fd=NULL;
  return bft;
}



void GWEN_BufferedIO_WinFile_Table__free(GWEN_BUFFEREDIO_WINFILE *bft) {
  GWEN_FREE_OBJECT(bft);
}





int GWEN_BufferedIO_WinFile__Read(GWEN_BUFFEREDIO *dm,
                                             char *buffer,
                                             int *size,
                                             int timeout){
  GWEN_BUFFEREDIO_WINFILE *bft;
  DWORD bytesRead;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_WINFILE, dm);
  assert(bft);
  if (*size<1) {
    DBG_WARN(GWEN_LOGDOMAIN, "Nothing to read");
    *size=0;
    return 0;
  }
  if (!ReadFile(bft->fd, buffer, *size, &bytesRead, 0)) {
    DWORD werr;

    werr=GetLastError();
    if (werr==ERROR_BROKEN_PIPE) {
      DBG_INFO(GWEN_LOGDOMAIN, "EOF met (broken pipe)");
      *size=0;
      return 0;
    }
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not read (%ld)", werr);
    return GWEN_ERROR_READ;
  }
  if (bytesRead==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "EOF met");
    *size=0;
    return 0;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "%ld bytes read", bytesRead);
  *size=bytesRead;
  return 0;
}



int GWEN_BufferedIO_WinFile__Write(GWEN_BUFFEREDIO *dm,
                                              const char *buffer,
                                              int *size,
                                              int timeout){
  GWEN_BUFFEREDIO_WINFILE *bft;
  DWORD written;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_WINFILE, dm);
  assert(bft);
  if (*size<1) {
    DBG_WARN(GWEN_LOGDOMAIN, "Nothing to write");
    *size=0;
    return 0;
  }
  if (!WriteFile(bft->fd, buffer, *size, &written, 0)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not write (%ld)", GetLastError());
    return GWEN_ERROR_WRITE;
  }
  DBG_INFO(GWEN_LOGDOMAIN, "%ld bytes written", written);
  *size=written;
  return 0;
}



int GWEN_BufferedIO_WinFile__Close(GWEN_BUFFEREDIO *dm){
  GWEN_BUFFEREDIO_WINFILE *bft;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_WINFILE, dm);
  assert(bft);
  if (!CloseHandle(bft->fd)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not close (%ld)", GetLastError());
    return GWEN_ERROR_CLOSE;
  }
  return 0;
}



void GWENHYWFAR_CB GWEN_BufferedIO_WinFile_FreeData(void *bp, void *p){
  GWEN_BUFFEREDIO_WINFILE *bft;

  bft=(GWEN_BUFFEREDIO_WINFILE *)p;
  GWEN_BufferedIO_WinFile_Table__free(bft);
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_WinFile_new(HANDLE fd){
  GWEN_BUFFEREDIO *bt;
  GWEN_BUFFEREDIO_WINFILE *bft;

  bt=GWEN_BufferedIO_new();
  bft=GWEN_BufferedIO_WinFile_Table__new();
  bft->fd=fd;

  GWEN_INHERIT_SETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_WINFILE,
                       bt, bft,
                       GWEN_BufferedIO_WinFile_FreeData);
  GWEN_BufferedIO_SetReadFn(bt, GWEN_BufferedIO_WinFile__Read);
  GWEN_BufferedIO_SetWriteFn(bt, GWEN_BufferedIO_WinFile__Write);
  GWEN_BufferedIO_SetCloseFn(bt, GWEN_BufferedIO_WinFile__Close);
  GWEN_BufferedIO_SetTimeout(bt, GWEN_BUFFEREDIO_WINFILE_TIMEOUT);

  return bt;
}


















