/***************************************************************************
    begin       : Sat Dec 27 2003
    copyright   : (C) 2019 by Martin Preuss
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
#include "syncio_file_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>



static GWEN_PROCESS *GWEN_Process_ProcessList=0;


static int _createPipes(GWEN_PROCESS *pr);
static void _closePipesOnError(GWEN_PROCESS *pr);
static void _setupPipeRedirectionsInParent(GWEN_PROCESS *pr);
static void _setupPipeRedirectionsInChild(GWEN_PROCESS *pr);
void _setupFileRedirectionsInChild(GWEN_PROCESS *pr);

static int _setupArgsAndExec(const char *prg, const char *args);

static int _readAndAddToBuffer(int fd, GWEN_BUFFER *buf);
int _waitForActivity(int fdStdOut, int fdStdErr);



int GWEN_Process_ModuleInit(void)
{
  return 0;
}



int GWEN_Process_ModuleFini(void)
{
  GWEN_PROCESS *pr, *prnext;

  pr=GWEN_Process_ProcessList;
  while (pr) {
    prnext=pr->next;

    pr->usage=1;
    GWEN_Process_free(pr);
    pr=prnext;
  } /* while */
  return 0;
}



#if 0
GWEN_PROCESS *GWEN_Process_FindProcess(pid_t pid)
{
  GWEN_PROCESS *pr;

  pr=GWEN_Process_ProcessList;
  while (pr) {
    if (pr->pid==pid)
      return pr;
    pr=pr->next;
  } /* while */
  return 0;
}



void GWEN_Process_SignalHandler(int s)
{
  int status;
  pid_t pid;

  switch (s) {
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
#endif



GWEN_PROCESS *GWEN_Process_new(void)
{
  GWEN_PROCESS *pr;

  GWEN_NEW_OBJECT(GWEN_PROCESS, pr);
  pr->state=GWEN_ProcessStateNotStarted;
  pr->pid=-1;
  pr->pflags=GWEN_PROCESS_FLAGS_DEFAULT;
  pr->usage=1;
  GWEN_LIST_ADD(GWEN_PROCESS, pr, &GWEN_Process_ProcessList);

  pr->filesStdin[0]=-1;
  pr->filesStdin[1]=-1;

  pr->filesStdout[0]=-1;
  pr->filesStdout[1]=-1;

  pr->filesStderr[0]=-1;
  pr->filesStderr[1]=-1;

  return pr;
}



void GWEN_Process_free(GWEN_PROCESS *pr)
{
  if (pr) {
    assert(pr->usage);
    if (--(pr->usage)==0) {
      /* unlink from list */
      GWEN_LIST_DEL(GWEN_PROCESS, pr, &GWEN_Process_ProcessList);

      free(pr->filenameStdIn);
      free(pr->filenameStdOut);
      free(pr->filenameStdErr);

      free(pr->folder);
      GWEN_SyncIo_free(pr->stdIn);
      GWEN_SyncIo_free(pr->stdOut);
      GWEN_SyncIo_free(pr->stdErr);
      GWEN_FREE_OBJECT(pr);
    }
  }
}



void GWEN_Process_SetFilenameStdIn(GWEN_PROCESS *pr, const char *s)
{
  assert(pr);
  free(pr->filenameStdIn);
  pr->filenameStdIn=s?strdup(s):NULL;
}



void GWEN_Process_SetFilenameStdOut(GWEN_PROCESS *pr, const char *s)
{
  assert(pr);
  free(pr->filenameStdOut);
  pr->filenameStdOut=s?strdup(s):NULL;
}



void GWEN_Process_SetFilenameStdErr(GWEN_PROCESS *pr, const char *s)
{
  assert(pr);
  free(pr->filenameStdErr);
  pr->filenameStdErr=s?strdup(s):NULL;
}



GWEN_PROCESS_STATE GWEN_Process_Start(GWEN_PROCESS *pr,
                                      const char *prg,
                                      const char *args)
{
  pid_t pid;
  int usePipeRedirs;

  assert(pr);

  usePipeRedirs=(pr->pflags & (GWEN_PROCESS_FLAGS_REDIR_STDIN | GWEN_PROCESS_FLAGS_REDIR_STDOUT | GWEN_PROCESS_FLAGS_REDIR_STDERR))?1:0;

  if (usePipeRedirs) {
    int rv;

    rv=_createPipes(pr);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error creating pipes for redirections (%d)", rv);
      pr->state=GWEN_ProcessStateNotStarted;
      pr->pid=-1;
      return GWEN_ProcessStateNotStarted;
    }
  }

  pid=fork();
  if (pid==-1) {
    /* error in fork */
    pr->state=GWEN_ProcessStateNotStarted;
    pr->pid=-1;
    if (usePipeRedirs)
      _closePipesOnError(pr);
    return GWEN_ProcessStateNotStarted;
  }
  else if (pid!=0) {
    /* parent */
    DBG_INFO(GWEN_LOGDOMAIN, "Process started with id %d", (int)pid);
    pr->state=GWEN_ProcessStateRunning;
    pr->pid=pid;
    if (usePipeRedirs)
      _setupPipeRedirectionsInParent(pr);
    return GWEN_ProcessStateRunning;
  }

  /* child */
  DBG_DEBUG(GWEN_LOGDOMAIN, "I'm the child process");

  if (usePipeRedirs)
    _setupPipeRedirectionsInChild(pr);
  else
    _setupFileRedirectionsInChild(pr);

  if (pr->folder) {
    if (chdir(pr->folder)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Aborting due to error on chdir(%s): %s", pr->folder, strerror(errno));
      exit(2);
    }
  }

  _setupArgsAndExec(prg, args);

  /* keep compiler happy (code doesn't get here because of execvp) */
  return 0;
}



int _setupArgsAndExec(const char *prg, const char *args)
{
  GWEN_STRINGLIST *argumentList;
  GWEN_STRINGLISTENTRY *se;
  int argc=0;
  char **argv;
  int i;

  argumentList=GWEN_StringList_fromString2(args, " ", 0,
                                           GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
                                           GWEN_TEXT_FLAGS_DEL_QUOTES |
                                           GWEN_TEXT_FLAGS_CHECK_BACKSLASH);
  if (argumentList==NULL)
    argumentList=GWEN_StringList_new();
  /* argv[0]=prg name */
  GWEN_StringList_InsertString(argumentList, prg, 0, 0);
  argc=GWEN_StringList_Count(argumentList);

  argv=(char**) malloc((argc+1)*sizeof(char*));
  i=0;
  se=GWEN_StringList_FirstEntry(argumentList);
  while(se && i<argc) {
    argv[i]=(char*) GWEN_StringListEntry_Data(se);
    i++;
    se=GWEN_StringListEntry_Next(se);
  }
  argv[i]=0;

  /* parameters ready, exec */
  execvp(prg, argv);
  /* if we reach this point an error occurred */
  DBG_ERROR(GWEN_LOGDOMAIN, "Could not start program \"%s\": %s",
            prg, strerror(errno));
  exit(EXIT_FAILURE);

}


void _closePipesOnError(GWEN_PROCESS *pr)
{
  if (pr->filesStdin[0]!=-1) {
    close(pr->filesStdin[0]);
    pr->filesStdin[0]=-1;
  }

  if (pr->filesStdin[1]!=-1) {
    close(pr->filesStdin[1]);
    pr->filesStdin[1]=-1;
  }

  if (pr->filesStdout[0]!=-1) {
    close(pr->filesStdout[0]);
    pr->filesStdout[0]=-1;
  }

  if (pr->filesStdout[1]!=-1){
    close(pr->filesStdout[1]);
    pr->filesStdout[1]=-1;
  }

  if (pr->filesStderr[0]!=-1) {
    close(pr->filesStderr[0]);
    pr->filesStderr[0]=-1;
  }

  if (pr->filesStderr[1]!=-1) {
    close(pr->filesStderr[1]);
    pr->filesStderr[1]=-1;
  }
}



void _setupPipeRedirectionsInParent(GWEN_PROCESS *pr)
{
  if (pr->filesStdin[0]!=-1) {
    close(pr->filesStdin[1]);
    pr->filesStdin[1]=-1;
    pr->stdIn=GWEN_SyncIo_File_fromFd(pr->filesStdin[0]);
  }
  if (pr->filesStdout[0]!=-1) {
    close(pr->filesStdout[1]);
    pr->filesStdout[1]=-1;
    pr->stdOut=GWEN_SyncIo_File_fromFd(pr->filesStdout[0]);
  }
  if (pr->filesStderr[0]!=-1) {
    close(pr->filesStderr[1]);
    pr->filesStderr[1]=-1;
    pr->stdErr=GWEN_SyncIo_File_fromFd(pr->filesStdout[0]);
  }
}



void _setupPipeRedirectionsInChild(GWEN_PROCESS *pr)
{
  if (pr->filesStdin[0]!=-1) {
    close(pr->filesStdin[0]);
    pr->filesStdin[0]=-1;
    close(0);
    if (dup(pr->filesStdin[1])==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection");
    }
    close(pr->filesStdin[1]);
    pr->filesStdin[1]=-1;
  }
  if (pr->filesStdout[0]!=-1) {
    close(pr->filesStdout[0]);
    pr->filesStdout[0]=-1;
    close(1);
    if (dup(pr->filesStdout[1])==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection");
    }
    close(pr->filesStdout[1]);
    pr->filesStdout[1]=-1;
  }
  if (pr->filesStderr[0]!=-1) {
    close(pr->filesStderr[0]);
    pr->filesStderr[0]=-1;
    close(2);
    if (dup(pr->filesStderr[1])==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection");
    }
    close(pr->filesStderr[1]);
    pr->filesStderr[1]=-1;
  }
}



void _setupFileRedirectionsInChild(GWEN_PROCESS *pr)
{
  int fd;

  if (pr->filenameStdIn) {
    fd=open(pr->filenameStdIn, O_RDONLY);
    if (fd==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not open \"%s\" (errno=%d, \"%s\")", pr->filenameStdIn, errno, strerror(errno));
    }
    else {
      close(0);
      if (dup(fd)==-1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection STDIN to \"%s\"", pr->filenameStdIn);
      }
      close(fd);
    }
  }

  if (pr->filenameStdOut) {
    fd=open(pr->filenameStdOut, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    if (fd==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not open \"%s\" (errno=%d, \"%s\")", pr->filenameStdOut, errno, strerror(errno));
    }
    else {
      close(1);
      if (dup(fd)==-1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection STDOUT to \"%s\"", pr->filenameStdOut);
      }
      close(fd);
    }
  }

  if (pr->filenameStdErr) {
    if (pr->filenameStdOut && strcmp(pr->filenameStdOut, pr->filenameStdErr)==0) {
      /* stdout and stderr are redirected to the same file, reuse stdout for stderr */
      close(2);
      if (dup(1)==-1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection STDERR to \"%s\"", pr->filenameStdErr);
      }
    }
    else {
      fd=open(pr->filenameStdErr, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
      if (fd==-1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not open \"%s\" (errno=%d, \"%s\")", pr->filenameStdErr, errno, strerror(errno));
      }
      else {
        close(2);
        if (dup(fd)==-1) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup redirection STDERR to \"%s\"", pr->filenameStdErr);
        }
        close(fd);
      }
    }
  }
}



GWEN_PROCESS_STATE GWEN_Process_GetState(GWEN_PROCESS *pr, int w)
{
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



GWEN_PROCESS_STATE GWEN_Process_MakeState(GWEN_PROCESS *pr, int status)
{
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
    DBG_INFO(GWEN_LOGDOMAIN, "Process %d terminated by signal %d",
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



GWEN_PROCESS_STATE GWEN_Process_CheckState(GWEN_PROCESS *pr)
{
  assert(pr);

  if (pr->pid==-1)
    /* we already know the state, return it */
    return pr->state;

  /* otherwise try to get the status */
  return GWEN_Process_GetState(pr, 0);
}



int GWEN_Process_GetResult(GWEN_PROCESS *pr)
{
  assert(pr);
  if (GWEN_Process_CheckState(pr)==GWEN_ProcessStateExited)
    return pr->result;
  else
    return -1;
}



int GWEN_Process_Wait(GWEN_PROCESS *pr)
{
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



int GWEN_Process_Terminate(GWEN_PROCESS *pr)
{
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



const char *GWEN_Process_GetFolder(const GWEN_PROCESS *pr)
{
  return pr->folder;
}



void GWEN_Process_SetFolder(GWEN_PROCESS *pr, const char *s)
{
  free(pr->folder);
  pr->folder=s?strdup(s):NULL;
}



uint32_t GWEN_Process_GetFlags(const GWEN_PROCESS *pr)
{
  assert(pr);
  return pr->pflags;
}



void GWEN_Process_SetFlags(GWEN_PROCESS *pr, uint32_t f)
{
  assert(pr);
  pr->pflags=f;
}



void GWEN_Process_AddFlags(GWEN_PROCESS *pr, uint32_t f)
{
  assert(pr);
  pr->pflags|=f;
}



void GWEN_Process_SubFlags(GWEN_PROCESS *pr, uint32_t f)
{
  assert(pr);
  pr->pflags&=~f;
}



GWEN_SYNCIO *GWEN_Process_GetStdin(const GWEN_PROCESS *pr)
{
  assert(pr);
  return pr->stdIn;
}



GWEN_SYNCIO *GWEN_Process_GetStdout(const GWEN_PROCESS *pr)
{
  assert(pr);
  return pr->stdOut;
}



GWEN_SYNCIO *GWEN_Process_GetStderr(const GWEN_PROCESS *pr)
{
  assert(pr);
  return pr->stdErr;
}



int _createPipes(GWEN_PROCESS *pr)
{
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



int GWEN_Process_WaitAndRead(GWEN_PROCESS *pr, GWEN_BUFFER *stdOutBuffer, GWEN_BUFFER *stdErrBuffer)
{
  GWEN_SYNCIO *sioStdOut;
  GWEN_SYNCIO *sioStdErr;
  int fdStdOut=-1;
  int fdStdErr=-1;
  int rv;

  sioStdOut=GWEN_Process_GetStdout(pr);
  if (sioStdOut)
    fdStdOut=GWEN_SyncIo_File_GetFd(sioStdOut);
  sioStdErr=GWEN_Process_GetStderr(pr);
  if (sioStdErr)
    fdStdErr=GWEN_SyncIo_File_GetFd(sioStdErr);

  if (fdStdOut!=-1) {
    int flags;

    flags=fcntl(fdStdOut, F_GETFL, 0);
    fcntl(fdStdOut, F_SETFL, flags | O_NONBLOCK);
  }
  if (fdStdErr!=-1) {
    int flags;

    flags=fcntl(fdStdErr, F_GETFL, 0);
    fcntl(fdStdErr, F_SETFL, flags | O_NONBLOCK);
  }

  for (;;) {
    GWEN_PROCESS_STATE state;

    if (fdStdOut!=-1) {
      rv=_readAndAddToBuffer(fdStdOut, stdOutBuffer);
      if (rv<0) {
        if (rv==GWEN_ERROR_EOF) {
          DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
          fdStdOut=-1;
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Process_CheckState(pr); /* avoid zombies */
          return rv;
        }
      }
    }

    if (fdStdErr!=-1) {
      rv=_readAndAddToBuffer(fdStdErr, stdErrBuffer);
      if (rv<0) {
        if (rv==GWEN_ERROR_EOF) {
          DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
          fdStdErr=-1;
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Process_CheckState(pr); /* avoid zombies */
          return rv;
        }
      }
    }

    if (fdStdOut!=-1 || fdStdErr!=-1) {
    }


    if (fdStdOut==-1 && fdStdErr==-1) {
      state=GWEN_Process_Wait(pr);
      if (state!=GWEN_ProcessStateRunning)
	break;
    }
    else {
      state=GWEN_Process_CheckState(pr);
      if (state!=GWEN_ProcessStateRunning)
	break;
      _waitForActivity(fdStdOut, fdStdErr);
    }
  }

  return 0;
}



int _readAndAddToBuffer(int fd, GWEN_BUFFER *buf)
{
  for (;;) {
    uint8_t localBuffer[1024];
    size_t rv;

    rv=read(fd, localBuffer, sizeof(localBuffer));
    if (rv==-1) {
      if (errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR)
        return 0;

      if (errno!=EINTR) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on read(): %s", strerror(errno));
        return GWEN_ERROR_GENERIC;
      }
      /* otherwise just try again */
    }
    else if (rv==0) {
      return GWEN_ERROR_EOF;
    }
    else {
      GWEN_Buffer_AppendBytes(buf, (const char*) localBuffer, rv);
    }
  }
  return 0;
}



int _waitForActivity(int fdStdOut, int fdStdErr)
{
  fd_set rfds;
  struct timeval tv;
  int retval;

  //do {
    FD_ZERO(&rfds);

    if (fdStdOut!=-1)
      FD_SET(fdStdOut, &rfds);
    if (fdStdErr!=-1)
      FD_SET(fdStdErr, &rfds);

    tv.tv_sec=5;
    tv.tv_usec=0;

    retval=select(((fdStdOut>fdStdErr)?fdStdOut:fdStdErr)+1,
                  &rfds, NULL, NULL, &tv);
  //} while(retval==-1 && errno==EINTR);

  if (retval==-1) {
    if (retval!=EINTR) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on select(): %s", strerror(errno));
      return GWEN_ERROR_GENERIC;
    }

  }
  else if (retval)
    return 1;
  return 0;
}









