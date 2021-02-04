/***************************************************************************
 begin       : Fri Feb 07 2003
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#ifndef ICONV_CONST
# define ICONV_CONST
#endif


#include "gui_p.h"
#include "dlg_input_l.h"
#include "dlg_message_l.h"
#include "dlg_progress_l.h"
#include "dlg_showbox_l.h"
#include "i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/syncio_socket.h>
#include <gwenhywfar/syncio_buffered.h>
#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/syncio_http.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#if defined (HAVE_LANGINFO_H)
# include <langinfo.h>
#elif defined (HAVE_LOCALCHARSET_H)
# include <localcharset.h>
#endif
#ifdef HAVE_ICONV_H
# include <iconv.h>
#endif



static GWEN_GUI *gwenhywfar_gui=NULL;


GWEN_INHERIT_FUNCTIONS(GWEN_GUI)



GWEN_GUI *GWEN_Gui_new(void)
{
  GWEN_GUI *gui;

  GWEN_NEW_OBJECT(GWEN_GUI, gui);
  GWEN_INHERIT_INIT(GWEN_GUI, gui);
  gui->refCount=1;

  gui->checkCertFn=GWEN_Gui_CheckCertBuiltIn;
  gui->getSyncIoFn=GWEN_Gui_Internal_GetSyncIo;

  gui->getPasswordFn=GWEN_Gui_Internal_GetPassword;
  gui->setPasswordStatusFn=GWEN_Gui_Internal_SetPasswordStatus;

  gui->progressDataTree=GWEN_ProgressData_Tree_new();
  gui->activeDialogs=GWEN_Dialog_List_new();
  GWEN_Gui_SetCharSet(gui, "");

  gui->dbPasswords=GWEN_DB_Group_new("passwords");
  gui->badPasswords=GWEN_StringList_new();

  gui->minProgressLogLevel=GWEN_LoggerLevel_Info;

  return gui;
}



void GWEN_Gui_free(GWEN_GUI *gui)
{
  if (gui) {
    assert(gui->refCount);
    if ((--gui->refCount)==0) {
      GWEN_INHERIT_FINI(GWEN_GUI, gui);

      GWEN_Dialog_List_free(gui->activeDialogs);
      GWEN_ProgressData_Tree_free(gui->progressDataTree);
      free(gui->name);
      free(gui->charSet);

      GWEN_DB_Group_free(gui->dbPasswords);
      if (gui->passwdStore)
        GWEN_PasswordStore_free(gui->passwdStore);
      GWEN_StringList_free(gui->badPasswords);

      GWEN_FREE_OBJECT(gui);
    }
  }
}



void GWEN_Gui_Attach(GWEN_GUI *gui)
{
  assert(gui);
  assert(gui->refCount);
  gui->refCount++;
}



GWEN_GUI *GWEN_Gui_GetGui(void)
{
  return gwenhywfar_gui;
}



void GWEN_Gui_SetGui(GWEN_GUI *gui)
{
  if (gui)
    GWEN_Gui_Attach(gui);
  if (gwenhywfar_gui)
    GWEN_Gui_free(gwenhywfar_gui);
  gwenhywfar_gui=gui;
}



uint32_t GWEN_Gui_GetFlags(const GWEN_GUI *gui)
{
  assert(gui);
  return gui->flags;
}



void GWEN_Gui_SetFlags(GWEN_GUI *gui, uint32_t fl)
{
  assert(gui);
  gui->flags=fl;
}



void GWEN_Gui_AddFlags(GWEN_GUI *gui, uint32_t fl)
{
  assert(gui);
  gui->flags|=fl;
}



void GWEN_Gui_SubFlags(GWEN_GUI *gui, uint32_t fl)
{
  assert(gui);
  gui->flags&=~fl;
}



void GWEN_Gui_SetName(GWEN_GUI *gui, const char *name)
{
  free(gui->name);
  if (name)
    gui->name=strdup(name);
  else
    gui->name=NULL;
}



const char *GWEN_Gui_GetName(void)
{
  if (gwenhywfar_gui)
    return gwenhywfar_gui->name;
  return NULL;
}



const char *GWEN_Gui_GetCharSet(const GWEN_GUI *gui)
{
  if (gui)
    return gui->charSet;
  return NULL;
}



void GWEN_Gui_SetCharSet(GWEN_GUI *gui, const char *s)
{
  const char *cs;

  assert(gui);

  if (s) {
    if (!*s) {
      /* determine charset according to current locale */
#if defined(HAVE_LANGINFO_H)
      cs=nl_langinfo(CODESET);
#elif defined(HAVE_LOCALCHARSET_H)
      cs=locale_charset();
#else
      /* just pass "" on to libiconv and hope for the best */
      cs=s;
#endif
    }
    else
      cs=s;

    if (strcasecmp(cs, "UTF-8")==0)
      cs=NULL;
  }
  else
    cs=s;

#ifndef HAVE_ICONV
  if (cs) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Missing iconv, cannot convert between UTF-8 and \"%s\"", cs);
  }
#else
  if (cs) {
    iconv_t ic;
    size_t len;
    char *p;

    /* Let iconv apply transliteration where necessary */
    len=strlen(cs);
    p=(char *)malloc(len+11);
    assert(p);
    sprintf(p, "%s//TRANSLIT", cs);
    ic=iconv_open(p, "UTF-8");
    if (ic==(iconv_t)-1) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Charset conversion from \"UTF-8\" to \"%s\" failed: %s (%d)",
                p, strerror(errno), errno);
      free(p);
      return;
    }
    iconv_close(ic);
    cs=p;
  }

  if (gui->charSet)
    free(gui->charSet);
  gui->charSet=(char *)cs;
#endif
}



GWEN_PASSWD_STORE *GWEN_Gui_GetPasswdStore(const GWEN_GUI *gui)
{
  if (gui)
    return gui->passwdStore;
  return NULL;
}



void GWEN_Gui_SetPasswdStore(GWEN_GUI *gui, GWEN_PASSWD_STORE *sto)
{
  if (gui) {
    if (gui->passwdStore && gui->passwdStore!=sto)
      GWEN_PasswordStore_free(gui->passwdStore);
    gui->passwdStore=sto;
    if (sto)
      gui->flags|=GWEN_GUI_FLAGS_PERMPASSWORDS;
    else
      gui->flags&=~GWEN_GUI_FLAGS_PERMPASSWORDS;
  }
}



void GWEN_Gui_SetPasswordDb(GWEN_GUI *gui,
                            GWEN_DB_NODE *dbPasswords,
                            int persistent)
{
  GWEN_DB_Group_free(gui->dbPasswords);
  gui->dbPasswords=dbPasswords;
  gui->persistentPasswords=persistent;
}



GWEN_DB_NODE *GWEN_Gui_GetPasswordDb(const GWEN_GUI *gui)
{
  return gui->dbPasswords;
}












void GWEN_Gui_ShowError(const char *title, const char *fmt, ...)
{
  va_list list;
  char msgbuffer[2048];
  int rv;

  /* prepare list for va_arg */
  va_start(list, fmt);
  rv=vsnprintf(msgbuffer, sizeof(msgbuffer), fmt, list);
  if (rv<0 || rv>=(int)(sizeof(msgbuffer))) {
    DBG_WARN(GWEN_LOGDOMAIN, "Internal buffer too small for message, truncating (%d>%d)",
             rv, (int)(sizeof(msgbuffer)));
  }
  va_end(list);

  GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                      GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                      GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                      title,
                      msgbuffer,
                      I18N("Dismiss"), NULL, NULL, 0);
}



int GWEN_Gui_WaitForSockets(GWEN_SOCKET_LIST2 *readSockets,
                            GWEN_SOCKET_LIST2 *writeSockets,
                            uint32_t guiid,
                            int msecs)
{
  if (gwenhywfar_gui && gwenhywfar_gui->waitForSocketsFn)
    return gwenhywfar_gui->waitForSocketsFn(gwenhywfar_gui, readSockets, writeSockets, guiid, msecs);
  else {
    uint32_t pid;
    time_t t0;
    int wt;
    int dist;

    t0=time(0);
    if (msecs==GWEN_TIMEOUT_NONE) {
      wt=0;
      dist=0;
    }
    else if (msecs==GWEN_TIMEOUT_FOREVER) {
      wt=0;
      dist=500;
    }
    else {
      wt=msecs/1000;
      dist=500;
    }

    pid=GWEN_Gui_ProgressStart(((wt!=0)?GWEN_GUI_PROGRESS_SHOW_PROGRESS:0) |
                               GWEN_GUI_PROGRESS_SHOW_ABORT |
                               GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_ALLOW_EMBED,
                               I18N("Waiting for Data"),
                               "Waiting for data to become available",
                               wt,
                               0);
    while (1) {
      GWEN_SOCKETSET *rset;
      GWEN_SOCKETSET *wset;
      GWEN_SOCKET_LIST2_ITERATOR *sit;

      rset=GWEN_SocketSet_new();
      wset=GWEN_SocketSet_new();

      /* fill read socket set */
      if (readSockets) {
        sit=GWEN_Socket_List2_First(readSockets);
        if (sit) {
          GWEN_SOCKET *s;

          s=GWEN_Socket_List2Iterator_Data(sit);
          assert(s);

          while (s) {
            GWEN_SocketSet_AddSocket(rset, s);
            s=GWEN_Socket_List2Iterator_Next(sit);
          }
          GWEN_Socket_List2Iterator_free(sit);
        }
      }

      /* fill write socket set */
      if (writeSockets) {
        sit=GWEN_Socket_List2_First(writeSockets);
        if (sit) {
          GWEN_SOCKET *s;

          s=GWEN_Socket_List2Iterator_Data(sit);
          assert(s);

          while (s) {
            GWEN_SocketSet_AddSocket(wset, s);
            s=GWEN_Socket_List2Iterator_Next(sit);
          }
          GWEN_Socket_List2Iterator_free(sit);
        }
      }

      if (GWEN_SocketSet_GetSocketCount(rset)==0 &&
          GWEN_SocketSet_GetSocketCount(wset)==0) {
        /* no sockets to wait for, sleep for a few ms to keep cpu load down */
        GWEN_SocketSet_free(wset);
        GWEN_SocketSet_free(rset);

        if (msecs) {
          /* only sleep if a timeout was given */
          DBG_DEBUG(GWEN_LOGDOMAIN, "Sleeping (no socket)");
          GWEN_Socket_Select(NULL, NULL, NULL, GWEN_GUI_CPU_TIMEOUT);
        }
        GWEN_Gui_ProgressEnd(pid);
        return GWEN_ERROR_TIMEOUT;
      }
      else {
        int rv;
        int v=0;

        rv=GWEN_Socket_Select(rset, wset, NULL, dist);
        GWEN_SocketSet_free(wset);
        GWEN_SocketSet_free(rset);

        if (rv!=GWEN_ERROR_TIMEOUT) {
          GWEN_Gui_ProgressEnd(pid);
          return rv;
        }

        if (wt) {
          time_t t1;

          t1=time(0);
          v=difftime(t1, t0);
          if (v>wt) {
            GWEN_Gui_ProgressEnd(pid);
            return GWEN_ERROR_TIMEOUT;
          }
        }
        rv=GWEN_Gui_ProgressAdvance(pid, v);
        if (rv==GWEN_ERROR_USER_ABORTED) {
          GWEN_Gui_ProgressEnd(pid);
          return rv;
        }
      }
    } /* loop */
  }
}



GWEN_LOGGER_LEVEL GWEN_Gui_GetMinProgressLogLevel(const GWEN_GUI *gui)
{
  assert(gui);
  return gui->minProgressLogLevel;
}



void GWEN_Gui_SetMinProgressLogLevel(GWEN_GUI *gui, GWEN_LOGGER_LEVEL ll)
{
  assert(gui);
  gui->minProgressLogLevel=ll;
}




#include "gui_cert.c"
#include "gui_dialogs.c"
#include "gui_passwd.c"
#include "gui_syncio.c"
#include "gui_utils.c"
#include "gui_virtual.c"



