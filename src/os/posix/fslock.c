/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Nov 23 2003
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


#include "fslock_p.h"
#include "i18n_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inetsocket.h> /* for select */
#include <gwenhywfar/gui.h>
#include <gwenhywfar/gwentime.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>


#define GWEN_FSLOCK_TIMEOUT_ASK (7000.0)   /* about 7 secs */



GWEN_LIST_FUNCTIONS(GWEN_FSLOCK, GWEN_FSLock)
GWEN_LIST2_FUNCTIONS(GWEN_FSLOCK, GWEN_FSLock)



GWEN_FSLOCK *GWEN_FSLock_new(const char *fname, GWEN_FSLOCK_TYPE t)
{
  GWEN_FSLOCK *fl;
  GWEN_BUFFER *nbuf;
  const char *s;

  assert(fname);
  GWEN_NEW_OBJECT(GWEN_FSLOCK, fl);
  GWEN_LIST_INIT(GWEN_FSLOCK, fl);
  fl->usage=1;
  fl->entryName=strdup(fname);

  switch (t) {
  case GWEN_FSLock_TypeFile:
    s=".lck";
    break;
  case GWEN_FSLock_TypeDir:
    s="/.dir.lck";
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown log type %d", t);
    abort();
  } /* switch */

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, fname);
  GWEN_Buffer_AppendString(nbuf, s);
  fl->baseLockFilename=strdup(GWEN_Buffer_GetStart(nbuf));

  if (GWEN_FSLock__UnifyLockFileName(nbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create unique lockfile name");
    GWEN_Buffer_free(nbuf);
    abort();
  }
  fl->uniqueLockFilename=strdup(GWEN_Buffer_GetStart(nbuf));
  GWEN_Buffer_free(nbuf);

  return fl;
}



void GWEN_FSLock_free(GWEN_FSLOCK *fl)
{
  if (fl) {
    assert(fl->usage);
    if (fl->usage==1) {
      if (fl->lockCount) {
        DBG_WARN(GWEN_LOGDOMAIN,
                 "File \"%s\" still locked", fl->entryName);
      }
      free(fl->entryName);
      free(fl->baseLockFilename);
      free(fl->uniqueLockFilename);
      GWEN_LIST_FINI(GWEN_FSLOCK, fl);
      fl->usage=0;
      GWEN_FREE_OBJECT(fl);
    }
    else {
      fl->usage--;
    }
  }
}



void GWEN_FSLock_Attach(GWEN_FSLOCK *fl)
{
  assert(fl);
  assert(fl->usage);
  fl->usage++;
}



GWEN_FSLOCK_RESULT GWEN_FSLock__Lock(GWEN_FSLOCK *fl)
{
  assert(fl);

  if (fl->lockCount==0) {
    int fd;
    int linkCount;
    struct stat st;

    fd=open(fl->uniqueLockFilename, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not open lock file %s for file %s: %s",
                fl->baseLockFilename,
                fl->entryName,
                strerror(errno));
      return GWEN_FSLock_ResultError;
    }
    close(fd);

    /* get the link count of the new unique file for later comparison */
    if (stat(fl->uniqueLockFilename, &st)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "stat(%s): %s",
                fl->uniqueLockFilename, strerror(errno));
      remove(fl->uniqueLockFilename);
      return GWEN_FSLock_ResultError;
    }
    linkCount=(int)(st.st_nlink);

    /* create a hard link to the new unique file with the name of the
     * real lock file. This is guaranteed to be atomic even on NFS */
    if (link(fl->uniqueLockFilename, fl->baseLockFilename)) {
      int lnerr;

      lnerr=errno;

      DBG_INFO(GWEN_LOGDOMAIN, "link(%s, %s): %s",
               fl->uniqueLockFilename,
               fl->baseLockFilename,
               strerror(errno));
      if (lnerr==EPERM) {
        /* link() is not supported on the destination filesystem, try it the
         * traditional way. This should be ok, since the only FS which does
         * not handle the O_EXCL flag properly is NFS, and NFS would not
         * return EPERM (because it generally supports link()).
         * So for NFS file systems we would not reach this point.
         */
        fd=open(fl->baseLockFilename,
                O_CREAT | O_EXCL | O_TRUNC | O_RDWR,
                S_IRUSR | S_IWUSR);
        if (fd==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "FS-Lock to %s already in use",
                   fl->entryName);
          remove(fl->uniqueLockFilename);
          return GWEN_FSLock_ResultBusy;
        }
        close(fd);
      }
      else {
        /* link() generally is supported on the destination file system,
         * check whether the link count of the unique file has been
               * incremented */
        if (stat(fl->uniqueLockFilename, &st)) {
          DBG_ERROR(GWEN_LOGDOMAIN, "stat(%s): %s",
                    fl->uniqueLockFilename, strerror(errno));
          remove(fl->uniqueLockFilename);
          return GWEN_FSLock_ResultError;
        }
        if ((int)(st.st_nlink)!=linkCount+1) {
          DBG_INFO(GWEN_LOGDOMAIN, "FS-Lock to %s already in use",
                   fl->entryName);
          remove(fl->uniqueLockFilename);
          return GWEN_FSLock_ResultBusy;
        }
      }
    } /* if error on link */

    DBG_VERBOUS(GWEN_LOGDOMAIN, "FS-Lock applied to %s", fl->entryName);
  }
  fl->lockCount++;
  return GWEN_FSLock_ResultOk;
}



GWEN_FSLOCK_RESULT GWEN_FSLock_Unlock(GWEN_FSLOCK *fl)
{
  assert(fl);

  if (fl->lockCount<1) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Entry \"%s\" not locked", fl->entryName);
    return GWEN_FSLock_ResultNoLock;
  }
  fl->lockCount--;
  if (fl->lockCount<1) {
    remove(fl->baseLockFilename);
    remove(fl->uniqueLockFilename);
    DBG_VERBOUS(GWEN_LOGDOMAIN, "FS-Lock released from %s", fl->entryName);
  }
  return GWEN_FSLock_ResultOk;
}



GWEN_FSLOCK_RESULT GWEN_FSLock_Lock(GWEN_FSLOCK *fl, int timeout, uint32_t gid)
{
  GWEN_TIME *t0;
  int distance;
  int count;
  GWEN_FSLOCK_RESULT rv;
  uint32_t progressId;
  int haveAskedForUnlock=0;
  int isInteractive=1;

  t0=GWEN_CurrentTime();
  assert(t0);

  isInteractive=(GWEN_Gui_GetFlags(GWEN_Gui_GetGui()) & GWEN_GUI_FLAGS_NONINTERACTIVE)==0;
  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                                    GWEN_GUI_PROGRESS_ALLOW_EMBED |
                                    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                                    GWEN_GUI_PROGRESS_SHOW_ABORT,
                                    I18N("Accquiring lock"),
                                    NULL,
                                    (timeout==GWEN_TIMEOUT_FOREVER)
                                    ?0:timeout, gid);

  if (timeout==GWEN_TIMEOUT_NONE)
    distance=GWEN_TIMEOUT_NONE;
  else if (timeout==GWEN_TIMEOUT_FOREVER)
    distance=GWEN_TIMEOUT_FOREVER;
  else {
    distance=GWEN_GUI_CHECK_PERIOD;
    if (distance>timeout)
      distance=timeout;
  }

  for (count=0;; count++) {
    int err;

    err=GWEN_Gui_ProgressAdvance(progressId, GWEN_GUI_PROGRESS_NONE);
    if (err==GWEN_ERROR_USER_ABORTED) {
      DBG_ERROR(GWEN_LOGDOMAIN, "User aborted.");
      GWEN_Time_free(t0);
      GWEN_Gui_ProgressEnd(progressId);
      return GWEN_FSLock_ResultUserAbort;
    }

    rv=GWEN_FSLock__Lock(fl);
    if (rv==GWEN_FSLock_ResultError) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Time_free(t0);
      GWEN_Gui_ProgressEnd(progressId);
      return rv;
    }
    else if (rv==GWEN_FSLock_ResultOk) {
      GWEN_Time_free(t0);
      GWEN_Gui_ProgressEnd(progressId);
      return rv;
    }
    else if (rv==GWEN_FSLock_ResultBusy) {
      int doWait=1;

      /* lock is busy */
      if (timeout==GWEN_TIMEOUT_NONE) {
        /* no timeout, so immediately return the result */
        GWEN_Time_free(t0);
        GWEN_Gui_ProgressEnd(progressId);
        return GWEN_FSLock_ResultBusy;
      }

      /* check timeout */
      if (timeout!=GWEN_TIMEOUT_FOREVER) {
        GWEN_TIME *t1;
        double d;

        if (timeout==GWEN_TIMEOUT_NONE) {
          GWEN_Gui_ProgressEnd(progressId);
          return GWEN_FSLock_ResultTimeout;
        }
        t1=GWEN_CurrentTime();
        assert(t1);
        d=GWEN_Time_Diff(t1, t0);
        GWEN_Time_free(t1);

        /* check timeout */
        if (d>=timeout) {
          DBG_DEBUG(GWEN_LOGDOMAIN,
                    "Could not lock within %d milliseconds, giving up",
                    timeout);
          GWEN_Time_free(t0);
          GWEN_Gui_ProgressEnd(progressId);
          return GWEN_FSLock_ResultTimeout;
        }

        err=GWEN_Gui_ProgressAdvance(progressId, (uint64_t)d);
        if (err) {
          DBG_ERROR(GWEN_LOGDOMAIN, "User aborted.");
          GWEN_Gui_ProgressEnd(progressId);
          return GWEN_FSLock_ResultUserAbort;
        }

        if (isInteractive &&
            haveAskedForUnlock==0 &&
            d>=GWEN_FSLOCK_TIMEOUT_ASK) {
          char buffer[2048];

          haveAskedForUnlock=1;
          snprintf(buffer, sizeof(buffer)-1,
                   I18N("There already is a lock for \"%s\".\n"
                        "Either that lock is valid (e.g. some other process is currently "
                        "holding that lock) or it is a stale lock of a process which "
                        "for whatever reason did not remove the lock before terminating.\n"
                        "\n"
                        "This dialog allows for forced removal of that lock.\n"
                        "\n"
                        "WARNING: Only remove the lock if you are certain that "
                        "no other process is actually holding the lock!\n"
                        "\n"
                        "Do you want to remove the possibly stale lock?\n"
                        "<html>"
                        "<p>There already is a lock for <i>%s</i>.</p>"
                        "<p>Either that lock is valid (e.g. some other process is currently "
                        "holding that lock) or it is a stale lock of a process which "
                        "for whatever reason did not remove the lock before terminating.</p>"
                        "<p>This dialog allows for forced removal of that lock.</p>"
                        "<p><font color=\"red\"><b>Warning</b></font>: "
                        "Only remove the lock if you are certain that "
                        "no other process is actually holding the lock!</p>"
                        "<p>Do you want to remove the possibly stale lock?</p>"
                        "</html>"),
                   fl->entryName,
                   fl->entryName);
          buffer[sizeof(buffer)-1]=0;

          rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO |
                                 GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                                 GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                                 I18N("Possible Stale Lock"),
                                 buffer,
                                 I18N("Wait"),
                                 I18N("Remove Lock"),
                                 I18N("Abort"),
                                 progressId);
          if (rv==3) {
            DBG_ERROR(GWEN_LOGDOMAIN, "User aborted");
            GWEN_Time_free(t0);
            GWEN_Gui_ProgressLog(progressId, GWEN_LoggerLevel_Notice,
                                 I18N("Aborted by user."));
            GWEN_Gui_ProgressEnd(progressId);
            return GWEN_FSLock_ResultUserAbort;
          }
          else if (rv==2) {
            remove(fl->baseLockFilename);
            remove(fl->uniqueLockFilename);
            DBG_WARN(GWEN_LOGDOMAIN, "FS-Lock forcably released from %s", fl->entryName);
            GWEN_Gui_ProgressLog(progressId, GWEN_LoggerLevel_Notice,
                                 I18N("Lock removed by user request."));
            doWait=0;
            /* reset timeout */
            GWEN_Time_free(t0);
            t0=GWEN_CurrentTime();
            assert(t0);
          }
        }
      }

      /* sleep for at most GWEN_GUI_CHECK_PERIOD */
      if (doWait)
        GWEN_Socket_Select(0, 0, 0, distance);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected return code %d", rv);
      GWEN_Time_free(t0);
      GWEN_Gui_ProgressEnd(progressId);
      return rv;
    }
  } /* for */
  GWEN_Gui_ProgressEnd(progressId);

  DBG_ERROR(GWEN_LOGDOMAIN, "We should never reach this point");
  GWEN_Time_free(t0);
  return GWEN_FSLock_ResultError;
}



int GWEN_FSLock__UnifyLockFileName(GWEN_BUFFER *nbuf)
{
  char buffer[256];

  GWEN_Buffer_AppendString(nbuf, ".");

  buffer[0]=0;
  if (gethostname(buffer, sizeof(buffer)-1)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gethostname: %s", strerror(errno));
    return -1;
  }
  buffer[sizeof(buffer)-1]=0;
  GWEN_Buffer_AppendString(nbuf, buffer);
  GWEN_Buffer_AppendString(nbuf, "-");

  buffer[0]=0;
  snprintf(buffer, sizeof(buffer)-1, "%i", (int)getpid());
  buffer[sizeof(buffer)-1]=0;
  GWEN_Buffer_AppendString(nbuf, buffer);

  return 0;
}



const char *GWEN_FSLock_GetName(const GWEN_FSLOCK *fl)
{
  assert(fl);
  assert(fl->usage);
  return fl->entryName;
}








