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
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>


static GWEN_PROCESS *GWEN_Process_ProcessList=0;


GWEN_ERRORCODE GWEN_Process_ModuleInit(){
  return 0;
}



GWEN_ERRORCODE GWEN_Process_ModuleFini(){
  GWEN_PROCESS *pr, *prnext;

  pr=GWEN_Process_ProcessList;
  while(pr) {
    prnext=pr->next;

    pr->usage=1;
    GWEN_Process_free(pr);
    pr=prnext;
  } /* while */
  return 0;
}



GWEN_PROCESS *GWEN_Process_FindProcess(pid_t pid){
  GWEN_PROCESS *pr;

  pr=GWEN_Process_ProcessList;
  while(pr) {
    if (pr->pid==pid)
      return pr;
    pr=pr->next;
  } /* while */
  return 0;
}



void GWEN_Process_SignalHandler(int s) {
  int status;
  pid_t pid;

  switch(s) {
  case SIGCHLD:
    /* try to get the status */
    pid=waitpid(0, &status, WNOHANG);
    if (pid==-1) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "waitdpid(%d): %s", 0, strerror(errno));
    }
    else if (pid==0) {
      /* process still running ?! */
      DBG_DEBUG(GWEN_LOGDOMAIN, "Got a SIGCHLD but no child terminated ??");
    }
    else {
      GWEN_PROCESS *pr;

      /* som process terminated */
      pr=GWEN_Process_FindProcess(pid);
      if (!pr) {
	DBG_NOTICE(GWEN_LOGDOMAIN, "No infomation about process \"%d\" available", (int)pid);
      }
      else {
	GWEN_Process_MakeState(pr, status);
	/* remove from list. If this process data is not used by the
	 * aplication it will now be freed, otherwise only the usage
	 * counter is decremented */
	GWEN_Process_free(pr);
      }
    }
    break;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Got unhandled signal \"%d\"", s);
    break;
  } /* switch */

}



GWEN_PROCESS *GWEN_Process_new(){
  GWEN_PROCESS *pr;

  GWEN_NEW_OBJECT(GWEN_PROCESS, pr);
  pr->state=GWEN_ProcessStateNotStarted;
  pr->pid=-1;
  pr->pflags=GWEN_PROCESS_FLAGS_DEFAULT;
  pr->usage=1;
  GWEN_LIST_ADD(GWEN_PROCESS, pr, &GWEN_Process_ProcessList);
  return pr;
}



void GWEN_Process_free(GWEN_PROCESS *pr){
  if (pr) {
    assert(pr->usage);
    if (--(pr->usage)==0) {
      /* unlink from list */
      GWEN_LIST_DEL(GWEN_PROCESS, pr, &GWEN_Process_ProcessList);
      GWEN_BufferedIO_free(pr->stdIn);
      GWEN_BufferedIO_free(pr->stdOut);
      GWEN_BufferedIO_free(pr->stdErr);
      free(pr);
    }
  }
}



GWEN_PROCESS_STATE GWEN_Process_Start(GWEN_PROCESS *pr,
				      const char *prg,
				      const char *args){
  pid_t pid;
  char buffer[128];
  char *argv[32];
  int argc;
  const char *p, *p2;

  assert(pr);

  if (GWEN_Process_Redirect(pr)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirections");
    pr->state=GWEN_ProcessStateNotStarted;
    pr->pid=-1;
    return GWEN_ProcessStateNotStarted;
  }

  pid=fork();
  if (pid==-1) {
    /* error in fork */
    pr->state=GWEN_ProcessStateNotStarted;
    pr->pid=-1;

    /* close all pipes */
    if (pr->filesStdin[0]!=-1) {
      close(pr->filesStdin[0]);
      close(pr->filesStdin[1]);
    }
    if (pr->filesStdout[0]!=-1) {
      close(pr->filesStdout[0]);
      close(pr->filesStdout[1]);
    }
    if (pr->filesStderr[0]!=-1) {
      close(pr->filesStderr[0]);
      close(pr->filesStderr[1]);
    }

    return GWEN_ProcessStateNotStarted;
  }
  else if (pid!=0) {
    /* parent */
    DBG_INFO(GWEN_LOGDOMAIN, "Process started with id %d", (int)pid);
    pr->state=GWEN_ProcessStateRunning;
    pr->pid=pid;

    /* setup redirections */
    if (pr->filesStdin[0]!=-1) {
      close(pr->filesStdin[1]);
      pr->stdIn=GWEN_BufferedIO_File_new(pr->filesStdin[0]);
      GWEN_BufferedIO_SetWriteBuffer(pr->stdIn, 0, 128);
    }
    if (pr->filesStdout[0]!=-1) {
      close(pr->filesStdout[1]);
      pr->stdOut=GWEN_BufferedIO_File_new(pr->filesStdout[0]);
      GWEN_BufferedIO_SetReadBuffer(pr->stdOut, 0, 128);
    }
    if (pr->filesStderr[0]!=-1) {
      close(pr->filesStderr[1]);
      pr->stdErr=GWEN_BufferedIO_File_new(pr->filesStdout[0]);
      GWEN_BufferedIO_SetReadBuffer(pr->stdErr, 0, 128);
    }

    return GWEN_ProcessStateRunning;
  }
  /* child, build arguments */
  argc=0;

  DBG_DEBUG(GWEN_LOGDOMAIN, "I'm the child process");

  /* setup redirections */
  if (pr->filesStdin[0]!=-1) {
    close(pr->filesStdin[0]);
    close(0);
    if (dup(pr->filesStdin[1])==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection");
    }
  }
  if (pr->filesStdout[0]!=-1) {
    close(pr->filesStdout[0]);
    close(1);
    if (dup(pr->filesStdout[1])==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection");
    }
  }
  if (pr->filesStderr[0]!=-1) {
    close(pr->filesStderr[0]);
    close(2);
    if (dup(pr->filesStderr[1])==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection");
    }
  }

  argv[0]=strdup(prg);
  argc++;
  p=args;
  while(argc<32 && *p) {
    while(*p && isspace((int)*p))
      p++;
    if (!(*p))
      break;
    p2=GWEN_Text_GetWord(p, " ",
			 buffer, sizeof(buffer),
			 GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
			 GWEN_TEXT_FLAGS_DEL_QUOTES |
			 GWEN_TEXT_FLAGS_CHECK_BACKSLASH,
			 &p);
    if (!p2)
      break;

    argv[argc]=strdup(p2);
    argc++;
  } /* while */
  argv[argc]=0;
  /* parameters ready, exec */
  execvp(prg, argv);
  /* if we reach this point an error occurred */
  DBG_ERROR(GWEN_LOGDOMAIN, "Could not start program \"%s\": %s",
	    prg, strerror(errno));
  exit(EXIT_FAILURE);
}



GWEN_PROCESS_STATE GWEN_Process_GetState(GWEN_PROCESS *pr, int w){
  int rv;
  int status;

  assert(pr);
  /* try to get the status */
  rv=waitpid(pr->pid, &status, w?0:WNOHANG);
  if (rv==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "waitdpid(%d): %s", (int)pr->pid, strerror(errno));
    return GWEN_ProcessStateUnknown;
  }
  else if (rv==0) {
    /* process still running */
    return GWEN_ProcessStateRunning;
  }
  else {
    return GWEN_Process_MakeState(pr, status);
  }
}



GWEN_PROCESS_STATE GWEN_Process_MakeState(GWEN_PROCESS *pr, int status){
  /* process has terminated for any reason */
  if (WIFEXITED(status)) {
    /* normal termination */
    DBG_INFO(GWEN_LOGDOMAIN, "Process %d exited with %d",
	     (int)pr->pid, WEXITSTATUS(status));
    pr->state=GWEN_ProcessStateExited;
    pr->pid=-1;
    /* store result code */
    pr->result=WEXITSTATUS(status);
    return pr->state;
  } /* if exited normally */
  else if (WIFSIGNALED(status)) {
    /* uncaught signal */
    DBG_ERROR(GWEN_LOGDOMAIN, "Process %d terminated by signal %d",
	      (int)pr->pid, WTERMSIG(status));
    pr->state=GWEN_ProcessStateAborted;
    pr->pid=-1;
    return pr->state;
  } /* if terminated by signal */
  else if (WIFSTOPPED(status)) {
    /* process stopped by signal */
    DBG_ERROR(GWEN_LOGDOMAIN, "Process %d stopped by signal %d",
	      (int)pr->pid, WSTOPSIG(status));
    pr->state=GWEN_ProcessStateStopped;
    pr->pid=-1;
    return pr->state;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled status, assume process %d isn't running (%08x)",
	      (int)pr->pid, (unsigned int)status);
    return GWEN_ProcessStateUnknown;
  }
}



GWEN_PROCESS_STATE GWEN_Process_CheckState(GWEN_PROCESS *pr){
  assert(pr);

  if (pr->pid==-1)
    /* we already know the state, return it */
    return pr->state;

  /* otherwise try to get the status */
  return GWEN_Process_GetState(pr, 0);
}



int GWEN_Process_GetResult(GWEN_PROCESS *pr){
  assert(pr);
  if (GWEN_Process_CheckState(pr)==GWEN_ProcessStateExited)
    return pr->result;
  else
    return -1;
}



int GWEN_Process_Wait(GWEN_PROCESS *pr){
  GWEN_PROCESS_STATE pst;

  assert(pr);

  if (pr->state!=GWEN_ProcessStateRunning)
    /* process is not running, so return */
    return 0;

  if (pr->pid==-1) {
    /* process is running, but we have no pid ?! */
    DBG_ERROR(GWEN_LOGDOMAIN, "Process is running but we don't have its pid");
    return -1;
  }
  pst=GWEN_Process_GetState(pr, 1);
  if (pst==GWEN_ProcessStateUnknown)
    return -1;
  return 0;
}



int GWEN_Process_Terminate(GWEN_PROCESS *pr){
  assert(pr);

  if (pr->state!=GWEN_ProcessStateRunning) {
    /* process is not running, so return */
    DBG_INFO(GWEN_LOGDOMAIN, "Process is not running, doing nothing");
    return 0;
  }

  if (pr->pid==-1) {
    /* process is running, but we have no pid ?! */
    DBG_ERROR(GWEN_LOGDOMAIN, "Process is running but we don't have its pid");
    return -1;
  }

  /* kill process */
  if (kill(pr->pid, SIGKILL)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on kill(%d, SIGKILL): %s",
	      (int)pr->pid, strerror(errno));
    return -1;
  }
  /* wait for process to respond to kill signal (should not take long) */
  return GWEN_Process_Wait(pr);
}



GWEN_TYPE_UINT32 GWEN_Process_GetFlags(const GWEN_PROCESS *pr){
  assert(pr);
  return pr->pflags;
}



void GWEN_Process_SetFlags(GWEN_PROCESS *pr, GWEN_TYPE_UINT32 f){
  assert(pr);
  pr->pflags=f;
}



void GWEN_Process_AddFlags(GWEN_PROCESS *pr, GWEN_TYPE_UINT32 f){
  assert(pr);
  pr->pflags|=f;
}



void GWEN_Process_SubFlags(GWEN_PROCESS *pr, GWEN_TYPE_UINT32 f){
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



int GWEN_Process_Redirect(GWEN_PROCESS *pr) {
  assert(pr);

  pr->filesStdin[0]=-1;
  pr->filesStdout[0]=-1;
  pr->filesStderr[0]=-1;

  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDIN) {
    int filedes[2];

    DBG_DEBUG(GWEN_LOGDOMAIN, "Redirecting stdin");
    if (pipe(filedes)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "pipe(): %s", strerror(errno));
      return -1;
    }
    pr->filesStdin[0]=filedes[1];
    pr->filesStdin[1]=filedes[0];
  }

  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDOUT) {
    int filedes[2];

    DBG_DEBUG(GWEN_LOGDOMAIN, "Redirecting stdout");
    if (pipe(filedes)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "pipe(): %s", strerror(errno));
      return -1;
    }
    pr->filesStdout[0]=filedes[0];
    pr->filesStdout[1]=filedes[1];
  }

  if (pr->pflags & GWEN_PROCESS_FLAGS_REDIR_STDERR) {
    int filedes[2];

    DBG_DEBUG(GWEN_LOGDOMAIN, "Redirecting stderr");
    if (pipe(filedes)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "pipe(): %s", strerror(errno));
      return -1;
    }
    pr->filesStderr[0]=filedes[0];
    pr->filesStderr[1]=filedes[1];
  }

  return 0;
}






























