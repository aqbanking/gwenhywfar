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
static struct sigaction original_sigchld_sa;


GWEN_ERRORCODE GWEN_Process_ModuleInit(){
  struct sigaction sa;

  /* setup signal handler for SIGCHLD */
  sa.sa_sigaction = GWEN_Process_SignalHandler;
  sigemptyset(&sa.sa_mask);

  sa.sa_flags = SA_SIGINFO;

  /* FIXME: This needs more work! If the &sa handler is installed,
     then gnucash-HEAD crashes each time on startup because guile
     expected its own signal handler to be used -- the error message
     is something like "in procedure waitpid: no child processes",
     i.e. guile expected its own signal handler to be called for
     SIGCHLD. However, when we execute this below then we get printed
     "original handler was SIG_DFL". But if we call this SIG_DFL
     handler below in SignalHandler, we will get a segfault. Maybe
     some other flags and/or signals have to be checked? I have no
     idea. */
  if (sigaction(SIGCHLD, &sa, &original_sigchld_sa)) {
    DBG_ERROR(0,
	      "Could not setup signal handler for signal SIGCHLD: %s",
	      strerror(errno));
    return GWEN_Error_new(0,
			  GWEN_ERROR_SEVERITY_ERR,
			  0,
			  GWEN_ERROR_UNSPECIFIED);
  }

  if (original_sigchld_sa.sa_handler == SIG_DFL) {
    DBG_NOTICE(0, "original_sigchld handler was SIG_DFL");
  } else if (original_sigchld_sa.sa_handler == SIG_IGN) {
    DBG_NOTICE(0, "original_sigchld handler was SIG_IGN");
  }
  else if (original_sigchld_sa.sa_flags | SA_SIGINFO) {
    DBG_NOTICE(0,
	      "Original signal handler for signal SIGCHLD was using "
	      "SA_SIGINFO. ");
  }

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



void GWEN_Process_SignalHandler(int s, siginfo_t *siginfo, void *info) {
  int status;
  pid_t pid;

  switch(s) {
  case SIGCHLD:
    /* try to get the status */
    pid=waitpid(0, &status, WNOHANG);
    if (pid==-1) {
      DBG_DEBUG(0, "waitdpid(%d): %s", 0, strerror(errno));
    }
    else if (pid==0) {
      /* process still running ?! */
      DBG_DEBUG(0, "Got a SIGCHLD but no child terminated ??");
    }
    else {
      GWEN_PROCESS *pr;

      /* som process terminated */
      pr=GWEN_Process_FindProcess(pid);
      if (!pr) {
	DBG_NOTICE(0, "No infomation about process \"%d\" available", pid);
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
    DBG_ERROR(0, "Got unhandled signal \"%d\"", s);
    break;
  } /* switch */

  /* Now check whether there has been already a different signal
     handler for this signal */
  if ( (original_sigchld_sa.sa_handler != SIG_DFL) && 
       (original_sigchld_sa.sa_handler != SIG_IGN) ) {
    if (original_sigchld_sa.sa_flags | SA_SIGINFO) {
      DBG_NOTICE(0, "About to call original sa_sigaction at signal \"%d\"", s);
      original_sigchld_sa.sa_sigaction(s, siginfo, info);
    } else {
      DBG_NOTICE(0, "About to call original sa_handler at signal \"%d\"", s);
      original_sigchld_sa.sa_handler(s);
    }
  }

}



GWEN_PROCESS *GWEN_Process_new(){
  GWEN_PROCESS *pr;

  GWEN_NEW_OBJECT(GWEN_PROCESS, pr);
  pr->state=GWEN_ProcessStateNotStarted;
  pr->pid=-1;
  pr->usage=1;
  GWEN_LIST_ADD(GWEN_PROCESS, pr, &GWEN_Process_ProcessList);
  return pr;
}



void GWEN_Process_free(GWEN_PROCESS *pr){
  if (pr) {
    assert(pr->usage);
    if (pr->usage==1) {
      /* unlink from list */
      GWEN_LIST_DEL(GWEN_PROCESS, pr, &GWEN_Process_ProcessList);
      free(pr);
    }
    else
      pr->usage--;
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

  pid=fork();
  if (pid==-1) {
    /* error in fork */
    pr->state=GWEN_ProcessStateNotStarted;
    pr->pid=-1;
    return GWEN_ProcessStateNotStarted;
  }
  else if (pid!=0) {
    /* parent */
    pr->state=GWEN_ProcessStateRunning;
    pr->pid=pid;
    return GWEN_ProcessStateRunning;
  }
  /* child, build arguments */
  argc=0;

  argv[0]=strdup(prg);
  argc++;
  p=args;
  while(argc<32 && *p) {
    while(*p && isspace(*p))
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
  DBG_ERROR(0, "Could not start program \"%s\": %s",
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
    DBG_ERROR(0, "waitdpid(%d): %s", pr->pid, strerror(errno));
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
    DBG_INFO(0, "Process %d exited with %d",
	     pr->pid, WEXITSTATUS(status));
    pr->state=GWEN_ProcessStateExited;
    pr->pid=-1;
    /* store result code */
    pr->result=WEXITSTATUS(status);
    return pr->state;
  } /* if exited normally */
  else if (WIFSIGNALED(status)) {
    /* uncaught signal */
    DBG_ERROR(0, "Process %d terminated by signal %d",
	      pr->pid, WTERMSIG(status));
    pr->state=GWEN_ProcessStateAborted;
    pr->pid=-1;
    return pr->state;
  } /* if terminated by signal */
  else if (WIFSTOPPED(status)) {
    /* process stopped by signal */
    DBG_ERROR(0, "Process %d stopped by signal %d",
	      pr->pid, WSTOPSIG(status));
    pr->state=GWEN_ProcessStateStopped;
    pr->pid=-1;
    return pr->state;
  }
  else {
    DBG_ERROR(0, "Unhandled status, assume process %d isn't running (%08x)",
	      pr->pid, (unsigned int)status);
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
    DBG_ERROR(0, "Process is running but we don't have its pid");
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
    DBG_INFO(0, "Process is not running, doing nothing");
    return 0;
  }

  if (pr->pid==-1) {
    /* process is running, but we have no pid ?! */
    DBG_ERROR(0, "Process is running but we don't have its pid");
    return -1;
  }

  /* kill process */
  if (kill(pr->pid, SIGKILL)) {
    DBG_ERROR(0, "Error on kill(%d, SIGKILL): %s",
	      pr->pid, strerror(errno));
    return -1;
  }
  /* wait for process to respond to kill signal (should not take long) */
  return GWEN_Process_Wait(pr);
}


























