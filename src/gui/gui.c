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



GWEN_GUI *GWEN_Gui_new(void) {
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



void GWEN_Gui_free(GWEN_GUI *gui) {
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



void GWEN_Gui_UseDialogs(GWEN_GUI *gui) {
  assert(gui);
  DBG_INFO(GWEN_LOGDOMAIN, "Using own callbacks in gui %p", gui);
  gui->progressStartFn=GWEN_Gui_Internal_ProgressStart;
  gui->progressAdvanceFn=GWEN_Gui_Internal_ProgressAdvance;
  gui->progressSetTotalFn=GWEN_Gui_Internal_ProgressSetTotal;
  gui->progressLogFn=GWEN_Gui_Internal_ProgressLog;
  gui->progressEndFn=GWEN_Gui_Internal_ProgressEnd;
  gui->inputBoxFn=GWEN_Gui_Internal_InputBox;
  gui->messageBoxFn=GWEN_Gui_Internal_MessageBox;
  gui->showBoxFn=GWEN_Gui_Internal_ShowBox;
  gui->hideBoxFn=GWEN_Gui_Internal_HideBox;
}



void GWEN_Gui_Attach(GWEN_GUI *gui) {
  assert(gui);
  assert(gui->refCount);
  gui->refCount++;
}



void GWEN_Gui_SetGui(GWEN_GUI *gui) {
  if (gui)
    GWEN_Gui_Attach(gui);
  if (gwenhywfar_gui)
    GWEN_Gui_free(gwenhywfar_gui);
  gwenhywfar_gui=gui;
}



GWEN_GUI *GWEN_Gui_GetGui(void) {
  return gwenhywfar_gui;
}



int GWEN_Gui_ConvertString(const char *text, size_t len, GWEN_BUFFER *tbuf,
                           const char *fromCs, const char *toCs) {
#ifdef HAVE_ICONV
  int rv=0;
  iconv_t ic;

  assert(tbuf);

  ic=iconv_open(toCs, fromCs);
  if (ic==(iconv_t)-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Cannot convert from \"%s\" to \"%s\", %s",
              fromCs, toCs, strerror(errno));
    rv=GWEN_ERROR_GENERIC;
  }
  else {
    /* Some systems have iconv in libc, some have it in libiconv
       (OSF/1 and those with the standalone portable GNU libiconv
       installed). Check which one is available. The define
       ICONV_CONST will be "" or "const" accordingly. */
    ICONV_CONST char *pInbuf;
    char *pOutbuf;
    size_t inLeft;
    size_t outLeft;
    size_t done;
    size_t space;

    /* convert */
    pInbuf=(char*)text;
    inLeft=len;
    outLeft=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    space=outLeft;
retry:
    pOutbuf=GWEN_Buffer_GetPosPointer(tbuf);
    done=iconv(ic, &pInbuf, &inLeft, &pOutbuf, &outLeft);
    GWEN_Buffer_SetPos(tbuf, space-outLeft);
    GWEN_Buffer_AdjustUsedBytes(tbuf);
    if (done==(size_t)-1) {
      if (errno==E2BIG) {
        uint32_t room;

        room=2*inLeft;
        if (room<=outLeft)
          room+=outLeft;
        GWEN_Buffer_AllocRoom(tbuf, room);
        /* How much additional room has actually been allocated? */
        room=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf)-outLeft;
        outLeft+=room;
        space+=room;
        goto retry;
      }

      DBG_ERROR(GWEN_LOGDOMAIN, "Error in conversion: %s (%d)",
                strerror(errno), errno);
      rv=GWEN_ERROR_GENERIC;
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Conversion done.");
    }
    iconv_close(ic);
  }

  return rv;
#else /* !HAVE_ICONV */
  return GWEN_Buffer_AppendBytes(tbuf, text, len);
#endif
}



int GWEN_Gui_ConvertFromUtf8(const GWEN_GUI *gui, const char *text, int len, GWEN_BUFFER *tbuf) {
  int rv;

  rv=GWEN_Gui_ConvertString(text, len, tbuf, "UTF-8", gui->charSet);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_Gui_StdPrintf(const GWEN_GUI *gui, FILE *stream,
                       const char *fmt, ...) {
  va_list args;
  int rv;

  assert(gui);

  va_start(args, fmt);
#ifndef HAVE_ICONV
  rv=vfprintf(stream, fmt, args);
#else
  if (!gui->charSet)
    rv=vfprintf(stderr, fmt, args);
  else {
    GWEN_BUFFER *tbuf;
    GWEN_BUFFER *outbuf;
    size_t space;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    space=vsnprintf(GWEN_Buffer_GetStart(tbuf), 256, fmt, args);
    if (space>=256) {
      GWEN_Buffer_AllocRoom(tbuf, space);
      va_end(args);
      va_start(args, fmt);
      vsprintf(GWEN_Buffer_GetStart(tbuf), fmt, args);
    }
    GWEN_Buffer_IncrementPos(tbuf, space);
    GWEN_Buffer_AdjustUsedBytes(tbuf);

    outbuf=GWEN_Buffer_new(0, 2*space, 0, 1);
    rv=GWEN_Gui_ConvertString(GWEN_Buffer_GetStart(tbuf), space,
                              outbuf, "UTF-8", gui->charSet);
    if (rv) {
      GWEN_Buffer_free(outbuf);
      outbuf=tbuf;
    }
    else
      GWEN_Buffer_free(tbuf);

    /* let's try to return the same value as fprintf() would */
    if (fputs(GWEN_Buffer_GetStart(outbuf), stderr)!=EOF)
      rv=space;
    else
      rv=EOF;

    GWEN_Buffer_free(outbuf);
  }
#endif

  va_end(args);
  return rv;
}



int GWEN_Gui_ReadString(const char *text, GWEN_BUFFER *tbuf) {
#ifdef HAVE_ICONV
  if (gwenhywfar_gui) {
    const char *fromCs;

    if (gwenhywfar_gui->charSet)
      fromCs=gwenhywfar_gui->charSet;
    else
      /* UTF-8 to UTF-8 conversion does not seem to make much sense, but
       * it is a convenient way to check whether the input text actually
       * is properly UTF-8 encoded.
       */
      fromCs="UTF-8";

    return GWEN_Gui_ConvertString(text, strlen(text), tbuf, fromCs, "UTF-8");
  }
  else
#endif /* HAVE_ICONV */
    return GWEN_Buffer_AppendString(tbuf, text);
}



void GWEN_Gui_GetRawText(const GWEN_GUI *gui, const char *text, GWEN_BUFFER *tbuf) {
  size_t len;
  const char *p;

  assert(text);
  p=text;
  while ((p=strchr(p, '<'))) {
    const char *t;

    t=p;
    t++;
    if (toupper(*t)=='H') {
      t++;
      if (toupper(*t)=='T') {
        t++;
        if (toupper(*t)=='M') {
          t++;
          if (toupper(*t)=='L') {
            break;
          }
        }
      }
    }
    p++;
  } /* while */

  if (p)
    len=p-text;
  else
    len=strlen(text);
  GWEN_Buffer_AppendBytes(tbuf, text, len);
}



GWEN_GUI_MESSAGEBOX_FN GWEN_Gui_SetMessageBoxFn(GWEN_GUI *gui,
    GWEN_GUI_MESSAGEBOX_FN f) {
  GWEN_GUI_MESSAGEBOX_FN of;

  assert(gui);
  of=gui->messageBoxFn;
  gui->messageBoxFn=f;
  return of;
}



GWEN_GUI_INPUTBOX_FN GWEN_Gui_SetInputBoxFn(GWEN_GUI *gui,
    GWEN_GUI_INPUTBOX_FN f) {
  GWEN_GUI_INPUTBOX_FN of;

  assert(gui);
  of=gui->inputBoxFn;
  gui->inputBoxFn=f;
  return of;
}



GWEN_GUI_SHOWBOX_FN GWEN_Gui_SetShowBoxFn(GWEN_GUI *gui,
    GWEN_GUI_SHOWBOX_FN f) {
  GWEN_GUI_SHOWBOX_FN of;

  assert(gui);
  of=gui->showBoxFn;
  gui->showBoxFn=f;
  return of;
}



GWEN_GUI_HIDEBOX_FN GWEN_Gui_SetHideBoxFn(GWEN_GUI *gui,
    GWEN_GUI_HIDEBOX_FN f) {
  GWEN_GUI_HIDEBOX_FN of;

  assert(gui);
  of=gui->hideBoxFn;
  gui->hideBoxFn=f;
  return of;
}



GWEN_GUI_PROGRESS_START_FN
GWEN_Gui_SetProgressStartFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_START_FN f) {
  GWEN_GUI_PROGRESS_START_FN of;

  assert(gui);
  of=gui->progressStartFn;
  gui->progressStartFn=f;
  return of;
}



GWEN_GUI_PROGRESS_ADVANCE_FN
GWEN_Gui_SetProgressAdvanceFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_ADVANCE_FN f) {
  GWEN_GUI_PROGRESS_ADVANCE_FN of;

  assert(gui);
  of=gui->progressAdvanceFn;
  gui->progressAdvanceFn=f;
  return of;
}



GWEN_GUI_PROGRESS_SETTOTAL_FN
GWEN_Gui_SetProgressSetTotalFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_SETTOTAL_FN f) {
  GWEN_GUI_PROGRESS_SETTOTAL_FN of;

  assert(gui);
  of=gui->progressSetTotalFn;
  gui->progressSetTotalFn=f;
  return of;
}



GWEN_GUI_PROGRESS_LOG_FN
GWEN_Gui_SetProgressLogFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_LOG_FN f) {
  GWEN_GUI_PROGRESS_LOG_FN of;

  assert(gui);
  of=gui->progressLogFn;
  gui->progressLogFn=f;
  return of;
}



GWEN_GUI_PROGRESS_END_FN
GWEN_Gui_SetProgressEndFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_END_FN f) {
  GWEN_GUI_PROGRESS_END_FN of;

  assert(gui);
  of=gui->progressEndFn;
  gui->progressEndFn=f;
  return of;
}



GWEN_GUI_PRINT_FN GWEN_Gui_SetPrintFn(GWEN_GUI *gui,
                                      GWEN_GUI_PRINT_FN f) {
  GWEN_GUI_PRINT_FN of;

  assert(gui);
  of=gui->printFn;
  gui->printFn=f;
  return of;
}



GWEN_GUI_GETPASSWORD_FN GWEN_Gui_SetGetPasswordFn(GWEN_GUI *gui,
    GWEN_GUI_GETPASSWORD_FN f) {
  GWEN_GUI_GETPASSWORD_FN of;

  assert(gui);
  of=gui->getPasswordFn;
  gui->getPasswordFn=f;
  return of;
}



GWEN_GUI_SETPASSWORDSTATUS_FN
GWEN_Gui_SetSetPasswordStatusFn(GWEN_GUI *gui,
                                GWEN_GUI_SETPASSWORDSTATUS_FN f) {
  GWEN_GUI_SETPASSWORDSTATUS_FN of;

  assert(gui);
  of=gui->setPasswordStatusFn;
  gui->setPasswordStatusFn=f;
  return of;
}



GWEN_GUI_LOG_HOOK_FN GWEN_Gui_SetLogHookFn(GWEN_GUI *gui,
    GWEN_GUI_LOG_HOOK_FN f) {
  GWEN_GUI_LOG_HOOK_FN of;

  assert(gui);
  of=gui->logHookFn;
  gui->logHookFn=f;

  return of;
}



GWEN_GUI_WAITFORSOCKETS_FN GWEN_Gui_SetWaitForSocketsFn(GWEN_GUI *gui,
    GWEN_GUI_WAITFORSOCKETS_FN f) {
  GWEN_GUI_WAITFORSOCKETS_FN of;

  assert(gui);
  of=gui->waitForSocketsFn;
  gui->waitForSocketsFn=f;

  return of;
}



GWEN_GUI_CHECKCERT_FN GWEN_Gui_SetCheckCertFn(GWEN_GUI *gui, GWEN_GUI_CHECKCERT_FN f) {
  GWEN_GUI_CHECKCERT_FN of;

  assert(gui);
  of=gui->checkCertFn;
  gui->checkCertFn=f;

  return of;
}



GWEN_GUI_EXEC_DIALOG_FN GWEN_Gui_SetExecDialogFn(GWEN_GUI *gui, GWEN_GUI_EXEC_DIALOG_FN f) {
  GWEN_GUI_EXEC_DIALOG_FN of;

  assert(gui);
  of=gui->execDialogFn;
  gui->execDialogFn=f;

  return of;
}



GWEN_GUI_OPEN_DIALOG_FN GWEN_Gui_SetOpenDialogFn(GWEN_GUI *gui, GWEN_GUI_OPEN_DIALOG_FN f) {
  GWEN_GUI_OPEN_DIALOG_FN of;

  assert(gui);
  of=gui->openDialogFn;
  gui->openDialogFn=f;

  return of;
}



GWEN_GUI_CLOSE_DIALOG_FN GWEN_Gui_SetCloseDialogFn(GWEN_GUI *gui, GWEN_GUI_CLOSE_DIALOG_FN f) {
  GWEN_GUI_CLOSE_DIALOG_FN of;

  assert(gui);
  of=gui->closeDialogFn;
  gui->closeDialogFn=f;

  return of;
}



GWEN_GUI_RUN_DIALOG_FN GWEN_Gui_SetRunDialogFn(GWEN_GUI *gui, GWEN_GUI_RUN_DIALOG_FN f) {
  GWEN_GUI_RUN_DIALOG_FN of;

  assert(gui);
  of=gui->runDialogFn;
  gui->runDialogFn=f;

  return of;
}



GWEN_GUI_READ_DIALOG_PREFS_FN
GWEN_Gui_SetReadDialogPrefsFn(GWEN_GUI *gui, GWEN_GUI_READ_DIALOG_PREFS_FN f) {
  GWEN_GUI_READ_DIALOG_PREFS_FN of;

  assert(gui);
  of=gui->readDialogPrefsFn;
  gui->readDialogPrefsFn=f;

  return of;
}



GWEN_GUI_WRITE_DIALOG_PREFS_FN
GWEN_Gui_SetWriteDialogPrefsFn(GWEN_GUI *gui, GWEN_GUI_WRITE_DIALOG_PREFS_FN f) {
  GWEN_GUI_WRITE_DIALOG_PREFS_FN of;

  assert(gui);
  of=gui->writeDialogPrefsFn;
  gui->writeDialogPrefsFn=f;

  return of;
}



GWEN_GUI_GET_FILENAME_FN GWEN_Gui_SetGetFileNameFn(GWEN_GUI *gui, GWEN_GUI_GET_FILENAME_FN f) {
  GWEN_GUI_GET_FILENAME_FN of;

  assert(gui);
  of=gui->getFileNameFn;
  gui->getFileNameFn=f;

  return of;
}



GWEN_GUI_GETSYNCIO_FN GWEN_Gui_SetGetSyncIoFn(GWEN_GUI *gui, GWEN_GUI_GETSYNCIO_FN f) {
  GWEN_GUI_GETSYNCIO_FN of;

  assert(gui);
  of=gui->getSyncIoFn;
  gui->getSyncIoFn=f;

  return of;
}



GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN
GWEN_Gui_SetKeyDataFromTextOpenSslFn(GWEN_GUI *gui,
                                     GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN f) {
  GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN of;

  assert(gui);
  of=gui->keyDataFromTextOpenSslFn;
  gui->keyDataFromTextOpenSslFn=f;

  return of;

}



uint32_t GWEN_Gui_GetFlags(const GWEN_GUI *gui) {
  assert(gui);
  return gui->flags;
}



void GWEN_Gui_SetFlags(GWEN_GUI *gui, uint32_t fl) {
  assert(gui);
  gui->flags=fl;
}



void GWEN_Gui_AddFlags(GWEN_GUI *gui, uint32_t fl) {
  assert(gui);
  gui->flags|=fl;
}



void GWEN_Gui_SubFlags(GWEN_GUI *gui, uint32_t fl) {
  assert(gui);
  gui->flags&=~fl;
}



void GWEN_Gui_SetName(GWEN_GUI *gui, const char *name) {
  free(gui->name);
  if (name) gui->name=strdup(name);
  else gui->name=NULL;
}



const char *GWEN_Gui_GetName(void) {
  if (gwenhywfar_gui)
    return gwenhywfar_gui->name;
  return NULL;
}



const char *GWEN_Gui_GetCharSet(const GWEN_GUI *gui) {
  if (gui)
    return gui->charSet;
  return NULL;
}



void GWEN_Gui_SetCharSet(GWEN_GUI *gui, const char *s) {
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



GWEN_PASSWD_STORE *GWEN_Gui_GetPasswdStore(const GWEN_GUI *gui) {
  if (gui)
    return gui->passwdStore;
  return NULL;
}



void GWEN_Gui_SetPasswdStore(GWEN_GUI *gui, GWEN_PASSWD_STORE *sto) {
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
                            int persistent) {
  GWEN_DB_Group_free(gui->dbPasswords);
  gui->dbPasswords=dbPasswords;
  gui->persistentPasswords=persistent;
}



GWEN_DB_NODE *GWEN_Gui_GetPasswordDb(const GWEN_GUI *gui) {
  return gui->dbPasswords;
}












int GWEN_Gui_MessageBox(uint32_t flags,
                        const char *title,
                        const char *text,
                        const char *b1,
                        const char *b2,
                        const char *b3,
                        uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->messageBoxFn)
    return gwenhywfar_gui->messageBoxFn(gwenhywfar_gui,
                                        flags,
                                        title,
                                        text,
                                        b1, b2, b3, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



void GWEN_Gui_ShowError(const char *title, const char *fmt, ...) {
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

  GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                      GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                      GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                      title,
                      msgbuffer,
                      I18N("Dismiss"), NULL, NULL, 0);
}



int GWEN_Gui_InputBox(uint32_t flags,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen,
                      uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->inputBoxFn)
    return gwenhywfar_gui->inputBoxFn(gwenhywfar_gui,
                                      flags,
                                      title,
                                      text,
                                      buffer,
                                      minLen, maxLen, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



uint32_t GWEN_Gui_ShowBox(uint32_t flags,
                          const char *title,
                          const char *text,
                          uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->showBoxFn)
    return gwenhywfar_gui->showBoxFn(gwenhywfar_gui,
                                     flags,
                                     title,
                                     text,
                                     guiid);
  return 0;
}



void GWEN_Gui_HideBox(uint32_t id) {
  if (gwenhywfar_gui && gwenhywfar_gui->hideBoxFn)
    return gwenhywfar_gui->hideBoxFn(gwenhywfar_gui, id);
}



uint32_t GWEN_Gui_ProgressStart(uint32_t progressFlags,
                                const char *title,
                                const char *text,
                                uint64_t total,
                                uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressStartFn)
    return gwenhywfar_gui->progressStartFn(gwenhywfar_gui,
                                           progressFlags,
                                           title,
                                           text,
                                           total,
                                           guiid);
  return 0;
}



int GWEN_Gui_ProgressAdvance(uint32_t id, uint32_t progress) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressAdvanceFn)
    return gwenhywfar_gui->progressAdvanceFn(gwenhywfar_gui,
           id,
           progress);
  return 0;
}



int GWEN_Gui_ProgressSetTotal(uint32_t id, uint64_t total) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressSetTotalFn)
    return gwenhywfar_gui->progressSetTotalFn(gwenhywfar_gui,
           id,
           total);
  return 0;
}



int GWEN_Gui_ProgressLog(uint32_t id,
                         GWEN_LOGGER_LEVEL level,
                         const char *text) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressLogFn)
    return gwenhywfar_gui->progressLogFn(gwenhywfar_gui,
                                         id, level, text);
  return 0;
}



int GWEN_Gui_ProgressLog2(uint32_t id,
                          GWEN_LOGGER_LEVEL level,
                          const char *fmt, ...) {
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

  return GWEN_Gui_ProgressLog(id, level, msgbuffer);
}



int GWEN_Gui_ProgressEnd(uint32_t id) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressEndFn)
    return gwenhywfar_gui->progressEndFn(gwenhywfar_gui, id);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_Print(const char *docTitle,
                   const char *docType,
                   const char *descr,
                   const char *text,
                   uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->printFn)
    return gwenhywfar_gui->printFn(gwenhywfar_gui,
                                   docTitle,
                                   docType,
                                   descr,
                                   text,
                                   guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_GetPassword(uint32_t flags,
                         const char *token,
                         const char *title,
                         const char *text,
                         char *buffer,
                         int minLen,
                         int maxLen,
                         uint32_t guiid) {
  if (gwenhywfar_gui) {
    if (gwenhywfar_gui->getPasswordFn)
      return gwenhywfar_gui->getPasswordFn(gwenhywfar_gui,
                                           flags,
                                           token,
                                           title,
                                           text,
                                           buffer,
                                           minLen,
                                           maxLen,
                                           guiid);
    else if (gwenhywfar_gui->inputBoxFn)
      return gwenhywfar_gui->inputBoxFn(gwenhywfar_gui,
                                        flags,
                                        title,
                                        text,
                                        buffer,
                                        minLen,
                                        maxLen,
                                        guiid);
  }
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_SetPasswordStatus(const char *token,
                               const char *pin,
                               GWEN_GUI_PASSWORD_STATUS status,
                               uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->setPasswordStatusFn)
    return gwenhywfar_gui->setPasswordStatusFn(gwenhywfar_gui,
           token, pin, status, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_LogHook(const char *logDomain,
                     GWEN_LOGGER_LEVEL priority, const char *s) {
  if (gwenhywfar_gui && gwenhywfar_gui->logHookFn) {
    if (priority>=GWEN_LoggerLevel_Debug &&
        logDomain &&
        strcasecmp(logDomain, "gwenhywfar")==0)
      /* don't send possibly sensitive data to the log function because
       * some application tend to store the messages indiscriminately.
       * In some cases sensitive information can be send to this function
       * which we don't want the application to store */
      return 0;
    else {
      int rv;

      if (gwenhywfar_gui->inLogHook==0) {
        /* otherwise the log message seems to be uncritical, convey it */
        gwenhywfar_gui->inLogHook++;
        rv=gwenhywfar_gui->logHookFn(gwenhywfar_gui, logDomain, priority, s);
        gwenhywfar_gui->inLogHook--;
        return rv;
      }
      else
        /* loghook recursion, don't convey */
        return 0;
    }
  }
  else
    /* handle as usual */
    return 0;
}



int GWEN_Gui_WaitForSockets(GWEN_SOCKET_LIST2 *readSockets,
                            GWEN_SOCKET_LIST2 *writeSockets,
                            uint32_t guiid,
                            int msecs) {
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
    while(1) {
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

          while(s) {
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

          while(s) {
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
          v=(int) difftime(t1, t0);
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



int GWEN_Gui_CheckCert(const GWEN_SSLCERTDESCR *cd, GWEN_SYNCIO *sio, uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->checkCertFn)
    return gwenhywfar_gui->checkCertFn(gwenhywfar_gui, cd, sio, guiid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWENHYWFAR_CB GWEN_Gui_CheckCertBuiltIn(GWEN_UNUSED GWEN_GUI *gui,
    const GWEN_SSLCERTDESCR *cd,
    GWEN_UNUSED GWEN_SYNCIO *sio, uint32_t guiid) {
  int rv;
  int isError;
  const char *hashMd5;
  const char *hashSha1;
  const char *hashSha512;
  const char *status;
  //const char *ipAddr;
  const char *statusOn;
  const char *statusOff;
  char varName[128];
  char dbuffer1[32];
  char dbuffer2[32];
  char buffer[8192];
  const GWEN_TIME *ti;
  const char *unknown;
  const char *commonName;
  const char *organizationName;
  const char *organizationalUnitName;
  const char *countryName;
  const char *localityName;
  const char *stateOrProvinceName;

  char *msg=I18S(
              "The following certificate has been received:\n"
              "Name         : %s\n"
              "Organisation : %s\n"
              "Department   : %s\n"
              "Country      : %s\n"
              "City         : %s\n"
              "State        : %s\n"
              "Valid after  : %s\n"
              "Valid until  : %s\n"
              "Hash (MD5)   : %s\n"
              "Hash (SHA1)  : %s\n"
              "Hash (SHA512): %s\n"
              "Status       : %s\n"
              "Do you wish to accept this certificate?"

              "<html>"
              " <p>"
              "  The following certificate has been received:"
              " </p>"
              " <table>"
              "  <tr><td>Name</td><td>%s</td></tr>"
              "  <tr><td>Organisation</td><td>%s</td></tr>"
              "  <tr><td>Department</td><td>%s</td></tr>"
              "  <tr><td>Country</td><td>%s</td></tr>"
              "  <tr><td>City</td><td>%s</td></tr>"
              "  <tr><td>State</td><td>%s</td></tr>"
              "  <tr><td>Valid after</td><td>%s</td></tr>"
              "  <tr><td>Valid until</td><td>%s</td></tr>"
              "  <tr><td>Hash (MD5)</td><td>%s</td></tr>"
              "  <tr><td>Hash (SHA1)</td><td>%s</td></tr>"
              "  <tr><td>Hash (SHA512)</td><td>%s</td></tr>"
              "  <tr><td>Status</td><td>%s%s%s</td></tr>"
              " </table>"
              " <p>"
              "  Do you wish to accept this certificate?"
              " </p>"
              "</html>"
            );

  memset(dbuffer1, 0, sizeof(dbuffer1));
  memset(dbuffer2, 0, sizeof(dbuffer2));
  memset(varName, 0, sizeof(varName));

  isError=GWEN_SslCertDescr_GetIsError(cd);

  hashMd5=GWEN_SslCertDescr_GetFingerPrint(cd);
  hashSha1=GWEN_SslCertDescr_GetFingerPrintSha1(cd);
  hashSha512=GWEN_SslCertDescr_GetFingerPrintSha512(cd);
  status=GWEN_SslCertDescr_GetStatusText(cd);
  //ipAddr=GWEN_SslCertDescr_GetIpAddress(cd);

  ti=GWEN_SslCertDescr_GetNotBefore(cd);
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    /* TRANSLATORS: This string is used as a template string to
       convert a given time into your local translated timeformat. The
       following characters are accepted in the template string: Y -
       digit of the year, M - digit of the month, D - digit of the day
       of month, h - digit of the hour, m - digit of the minute, s-
       digit of the second. All other characters are left unchanged. */
    if (GWEN_Time_toString(ti, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert beforeDate to string");
      abort();
    }
    strncpy(dbuffer1, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer1)-1);
    GWEN_Buffer_free(tbuf);
  }

  ti=GWEN_SslCertDescr_GetNotAfter(cd);
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(ti, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert untilDate to string");
      abort();
    }
    strncpy(dbuffer2, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer2)-1);
    GWEN_Buffer_free(tbuf);
  }

  if (isError) {
    statusOn="<font color=red>";
    statusOff="</font>";
  }
  else {
    statusOn="<font color=green>";
    statusOff="</font>";
  }

  unknown=I18N("unknown");
  commonName=GWEN_SslCertDescr_GetCommonName(cd);
  if (!commonName)
    commonName=unknown;
  organizationName=GWEN_SslCertDescr_GetOrganizationName(cd);
  if (!organizationName)
    organizationName=unknown;
  organizationalUnitName=GWEN_SslCertDescr_GetOrganizationalUnitName(cd);
  if (!organizationalUnitName)
    organizationalUnitName=unknown;
  countryName=GWEN_SslCertDescr_GetCountryName(cd);
  if (!countryName)
    countryName=unknown;
  localityName=GWEN_SslCertDescr_GetLocalityName(cd);
  if (!localityName)
    localityName=unknown;
  stateOrProvinceName=GWEN_SslCertDescr_GetStateOrProvinceName(cd);
  if (!stateOrProvinceName)
    stateOrProvinceName=unknown;
  if (!status)
    status=unknown;

  snprintf(buffer, sizeof(buffer)-1,
           I18N(msg),
           commonName,
           organizationName,
           organizationalUnitName,
           countryName,
           localityName,
           stateOrProvinceName,
           dbuffer1, dbuffer2,
           hashMd5,
           hashSha1,
           hashSha512,
           status,
           /* the same again for HTML */
           commonName,
           organizationName,
           organizationalUnitName,
           countryName,
           localityName,
           stateOrProvinceName,
           dbuffer1, dbuffer2,
           hashMd5,
           hashSha1,
           hashSha512,
           statusOn,
           status,
           statusOff
          );

  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
                         GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
                         GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                         I18N("Certificate Received"),
                         buffer,
                         I18N("Yes"), I18N("No"), 0, guiid);
  if (rv==1) {
    return 0;
  }
  else {
    DBG_NOTICE(GWEN_LOGDOMAIN, "User rejected certificate");

    return GWEN_ERROR_SSL_SECURITY;
  }
}



int GWEN_Gui_KeyDataFromText_OpenSSL(const char *text,
                                     unsigned char *buffer,
                                     unsigned int bufLength) {
  if (gwenhywfar_gui && gwenhywfar_gui->keyDataFromTextOpenSslFn)
    return gwenhywfar_gui->keyDataFromTextOpenSslFn(gwenhywfar_gui,
           text,
           buffer,
           bufLength);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_ExecDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->execDialogFn)
    return gwenhywfar_gui->execDialogFn(gwenhywfar_gui, dlg, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_OpenDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->openDialogFn)
    return gwenhywfar_gui->openDialogFn(gwenhywfar_gui, dlg, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_CloseDialog(GWEN_DIALOG *dlg) {
  if (gwenhywfar_gui && gwenhywfar_gui->closeDialogFn)
    return gwenhywfar_gui->closeDialogFn(gwenhywfar_gui, dlg);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_RunDialog(GWEN_DIALOG *dlg, int untilEnd) {
  if (gwenhywfar_gui && gwenhywfar_gui->runDialogFn)
    return gwenhywfar_gui->runDialogFn(gwenhywfar_gui, dlg, untilEnd);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}




int GWEN_Gui_GetFileName(const char *caption,
                         GWEN_GUI_FILENAME_TYPE fnt,
                         uint32_t flags,
                         const char *patterns,
                         GWEN_BUFFER *pathBuffer,
                         uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->getFileNameFn)
    return gwenhywfar_gui->getFileNameFn(gwenhywfar_gui,
                                         caption,
                                         fnt,
                                         flags,
                                         patterns,
                                         pathBuffer,
                                         guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_ReadDialogPrefs(const char *groupName,
                             const char *altName,
                             GWEN_DB_NODE **pDb) {
  if (gwenhywfar_gui && gwenhywfar_gui->readDialogPrefsFn)
    return gwenhywfar_gui->readDialogPrefsFn(gwenhywfar_gui, groupName, altName, pDb);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_WriteDialogPrefs(const char *groupName,
                              GWEN_DB_NODE *db) {
  if (gwenhywfar_gui && gwenhywfar_gui->writeDialogPrefsFn)
    return gwenhywfar_gui->writeDialogPrefsFn(gwenhywfar_gui, groupName, db);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_GetSyncIo(const char *url,
                       const char *defaultProto,
                       int defaultPort,
                       GWEN_SYNCIO **pSio) {
  if (gwenhywfar_gui && gwenhywfar_gui->getSyncIoFn)
    return gwenhywfar_gui->getSyncIoFn(gwenhywfar_gui, url, defaultProto, defaultPort, pSio);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}














int GWEN_Gui_ShowProgress(GWEN_PROGRESS_DATA *pd) {
  GWEN_PROGRESS_DATA *highest=NULL;
  GWEN_PROGRESS_DATA *t;
  GWEN_DIALOG *dlg=NULL;

  assert(gwenhywfar_gui);

  t=pd;
  while(t) {
    highest=t;
    t=GWEN_ProgressData_Tree_GetParent(t);
  }

  /* highest must always be visible */
  if (GWEN_ProgressData_GetShown(highest)==0)
    GWEN_ProgressData_SetShown(highest, 1);

  dlg=GWEN_ProgressData_GetDialog(highest);
  if (dlg==NULL) {
    int rv;

    /* need to create dialog for it */
    dlg=GWEN_DlgProgress_new();
    if (dlg==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unable to create progress dialog, maybe data not installed?");
      return GWEN_ERROR_INTERNAL;
    }
    if (GWEN_ProgressData_GetFlags(pd) & GWEN_GUI_PROGRESS_KEEP_OPEN)
      GWEN_DlgProgress_SetStayOpen(dlg, 1);

    if (GWEN_ProgressData_GetFlags(pd) & GWEN_GUI_PROGRESS_SHOW_LOG)
      GWEN_DlgProgress_SetShowLog(dlg, 1);

    rv=GWEN_Gui_OpenDialog(dlg, 0);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unable to openDialog: %d", rv);
      GWEN_Dialog_free(dlg);
      return rv;
    }

    DBG_INFO(GWEN_LOGDOMAIN, "Setting new firstprogress: %08x",
             GWEN_ProgressData_GetId(pd));
    GWEN_DlgProgress_SetFirstProgress(dlg, highest);
    GWEN_ProgressData_SetDialog(highest, dlg);
  }

  if (pd!=highest) {
    DBG_INFO(GWEN_LOGDOMAIN, "Setting new second progress: %08x",
             GWEN_ProgressData_GetId(pd));
    GWEN_DlgProgress_SetSecondProgress(dlg, pd);
    GWEN_ProgressData_SetDialog(pd, dlg);
    GWEN_ProgressData_SetShown(pd, 1);
  }

  GWEN_Gui_RunDialog(dlg, 0);

  return 0;
}



void GWEN_Gui_Internal_CheckShow(GWEN_GUI *gui, GWEN_PROGRESS_DATA *pd) {
  if (GWEN_ProgressData_GetShown(pd)==0) {
    if (GWEN_ProgressData_GetFlags(pd) & GWEN_GUI_PROGRESS_DELAY) {
      double dt;
      time_t t1;

      t1=time(0);
      dt=difftime(t1, GWEN_ProgressData_GetStartTime(pd));
      if ((int)dt>=GWEN_GUI_DELAY_SECS) {
        DBG_INFO(GWEN_LOGDOMAIN, "Progress %08x open for %d secs, showing",
                 GWEN_ProgressData_GetId(pd), (int) dt);
        GWEN_ProgressData_SetShown(pd, 1);
      }
    }
    else
      GWEN_ProgressData_SetShown(pd, 1);
  }

  if (GWEN_ProgressData_GetShown(pd)==1) {
    if (GWEN_ProgressData_GetDialog(pd)==NULL) {
      GWEN_Gui_ShowProgress(pd);
    }
  }
}



uint32_t GWEN_Gui_Internal_ProgressStart(GWEN_GUI *gui,
                                         uint32_t progressFlags,
                                         const char *title,
                                         const char *text,
                                         uint64_t total,
                                         uint32_t guiid) {
  GWEN_PROGRESS_DATA *pdParent=NULL;
  GWEN_PROGRESS_DATA *pd;
  uint32_t id;

  id=++(gui->nextProgressId);

  DBG_DEBUG(GWEN_LOGDOMAIN, "ProgressStart: flags=%08x, title=[%s], total=%08x, guiid=%08x",
            progressFlags, title?title:"(none)", (uint32_t) total, guiid);

  if (guiid==0) {
    guiid=gui->lastProgressId;
  }

  if (guiid) {
    pdParent=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, guiid);
    if (pdParent==NULL) {
      DBG_WARN(GWEN_LOGDOMAIN, "Parent progress by id %08x not found", guiid);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Title: [%s], Text: [%s]",
                title?title:"no title",
                text?text:"no text");
    }
  }

  pd=GWEN_ProgressData_new(gui, id, progressFlags, title, text, total);
  assert(pd);
  GWEN_ProgressData_SetPreviousId(pd, gui->lastProgressId);
  if (pdParent)
    GWEN_ProgressData_Tree_AddChild(pdParent, pd);
  else
    GWEN_ProgressData_Tree_Add(gui->progressDataTree, pd);

  GWEN_Gui_Internal_CheckShow(gui, pd);

  gui->lastProgressId=id;

  return id;
}



int GWEN_Gui_Internal_ProgressEnd(GWEN_GUI *gui, uint32_t pid) {
  GWEN_PROGRESS_DATA *pd;
  uint32_t parentPid=0;

  DBG_DEBUG(GWEN_LOGDOMAIN, "ProgressEnd: guiid=%08x", pid);

  if (pid==0) {
    pid=gui->lastProgressId;
    if (pid==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
      return GWEN_ERROR_INVALID;
    }
  }

  pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
  if (pd==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DIALOG *dlg;
    GWEN_PROGRESS_DATA *previousPd;

    /* set previous progress id */
    gui->lastProgressId=GWEN_ProgressData_GetPreviousId(pd);

    /* find next highest active progress */
    previousPd=GWEN_ProgressData_Tree_GetParent(pd);
    if (previousPd)
      parentPid=GWEN_ProgressData_GetId(previousPd);
    while(previousPd) {
      if (GWEN_ProgressData_GetShown(previousPd))
        break;
      previousPd=GWEN_ProgressData_Tree_GetParent(previousPd);
    }

    dlg=GWEN_ProgressData_GetDialog(pd);
    if (dlg) {
      GWEN_PROGRESS_DATA *primary;
      GWEN_PROGRESS_DATA *secondary;

      primary=GWEN_DlgProgress_GetFirstProgress(dlg);
      secondary=GWEN_DlgProgress_GetSecondProgress(dlg);

      /* force update of progress bar */
      GWEN_DlgProgress_Advanced(dlg, pd);
      GWEN_Gui_RunDialog(dlg, 0);

      if (primary==pd) {
        int rv;

        DBG_DEBUG(GWEN_LOGDOMAIN, "Progress %08x is primary, closing dialog",
                  GWEN_ProgressData_GetId(pd));

        if (secondary) {
          DBG_WARN(GWEN_LOGDOMAIN, "There is still a secondary progress!");
          GWEN_DlgProgress_SetSecondProgress(dlg, NULL);
          GWEN_ProgressData_SetDialog(secondary, NULL);
        }

        /* this is the primary progress, with this closed we can also
         * close the dialog */
        DBG_INFO(GWEN_LOGDOMAIN, "Closing progress dialog");
        GWEN_DlgProgress_AddLogText(dlg, GWEN_LoggerLevel_Info, I18N("Operation finished, you can now close this window."));

        // run dialog until end, close then
        GWEN_DlgProgress_SetAllowClose(dlg, 1);
        if (GWEN_DlgProgress_GetStayOpen(dlg)) {
          rv=GWEN_Gui_RunDialog(dlg, 1);
          if (rv<0) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Unable to runDialog: %d", rv);
            /*GWEN_Dialog_free(dlg);
            return rv;*/
          }
        }

        rv=GWEN_Gui_CloseDialog(dlg);
        if (rv<0) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Unable to closeDialog: %d", rv);
          GWEN_Dialog_free(dlg);
          return rv;
        }
        GWEN_Dialog_free(dlg);
      }
      else if (secondary==pd) {
        /* t is maybe the next higher progress, it will become the second progress */
        if (previousPd && previousPd!=GWEN_DlgProgress_GetFirstProgress(dlg)) {
          DBG_DEBUG(GWEN_LOGDOMAIN, "Progress %08x becomes new second progress",
                    GWEN_ProgressData_GetId(previousPd));
          GWEN_DlgProgress_SetSecondProgress(dlg, pd);
          GWEN_ProgressData_SetDialog(pd, dlg);
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "No next secondary progress");
          GWEN_DlgProgress_SetSecondProgress(dlg, NULL);
        }
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Progress %08x is neither primary nor secondary, SNH",
                  GWEN_ProgressData_GetId(pd));
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Progress %08x has no dialog", GWEN_ProgressData_GetId(pd));
    }

    GWEN_ProgressData_SetDialog(pd, NULL);
    GWEN_ProgressData_Tree_Del(pd);
    GWEN_ProgressData_free(pd);
  }

  return 0;
}



int GWEN_Gui_Internal_ProgressAdvance(GWEN_GUI *gui, uint32_t pid, uint64_t progress) {
  GWEN_PROGRESS_DATA *pd;
  int aborted=0;

  if (pid==0) {
    pid=gui->lastProgressId;
    if (pid==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
      return GWEN_ERROR_INVALID;
    }
  }

  pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
  if (pd==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DIALOG *dlg;

    if (progress==GWEN_GUI_PROGRESS_ONE)
      progress=GWEN_ProgressData_GetCurrent(pd)+1;
    else if (progress==GWEN_GUI_PROGRESS_NONE)
      progress=GWEN_ProgressData_GetCurrent(pd);
    GWEN_ProgressData_SetCurrent(pd, progress);
    GWEN_Gui_Internal_CheckShow(gui, pd);

    dlg=GWEN_ProgressData_GetDialog(pd);
    if (dlg) {
      time_t t0;
      time_t t1;

      t0=GWEN_ProgressData_GetCheckTime(pd);
      t1=time(0);
      if (t0!=t1) {
        GWEN_DlgProgress_Advanced(dlg, pd);
        GWEN_Gui_RunDialog(dlg, 0);
        GWEN_ProgressData_SetCheckTime(pd, t1);
      }
    }
    aborted=GWEN_ProgressData_GetAborted(pd);
  }

  if (aborted)
    return GWEN_ERROR_USER_ABORTED;
  return 0;
}



int GWEN_Gui_Internal_ProgressSetTotal(GWEN_GUI *gui, uint32_t pid, uint64_t total) {
  GWEN_PROGRESS_DATA *pd;
  int aborted=0;

  if (pid==0) {
    pid=gui->lastProgressId;
    if (pid==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
      return GWEN_ERROR_INVALID;
    }
  }

  pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
  if (pd==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DIALOG *dlg;

    GWEN_ProgressData_SetTotal(pd, total);
    GWEN_Gui_Internal_CheckShow(gui, pd);

    dlg=GWEN_ProgressData_GetDialog(pd);
    if (dlg) {
      time_t t0;
      time_t t1;

      t0=GWEN_ProgressData_GetCheckTime(pd);
      t1=time(0);
      if (t0!=t1) {
        GWEN_DlgProgress_TotalChanged(dlg, pd);
        GWEN_Gui_RunDialog(dlg, 0);
        GWEN_ProgressData_SetCheckTime(pd, t1);
      }
    }
    aborted=GWEN_ProgressData_GetAborted(pd);
  }

  if (aborted)
    return GWEN_ERROR_USER_ABORTED;
  return 0;
}



int GWEN_Gui_Internal_ProgressLog(GWEN_GUI *gui,
                                  uint32_t pid,
                                  GWEN_LOGGER_LEVEL level,
                                  const char *text) {
  assert(gui);

  /* only show messages with log level lower or equal threshold */
  if (level<=gui->minProgressLogLevel) {
    GWEN_PROGRESS_DATA *pd;
    int aborted=0;

    if (pid==0) {
      pid=gui->lastProgressId;
      if (pid==0) {
        DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
        return GWEN_ERROR_INVALID;
      }
    }

    pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
    if (pd==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
      return GWEN_ERROR_INVALID;
    }
    else {
      GWEN_DIALOG *dlg;

      if (level<=GWEN_LoggerLevel_Notice)
        GWEN_ProgressData_SetShown(pd, 1);
      if (level<=GWEN_LoggerLevel_Warning)
        GWEN_ProgressData_AddFlags(pd, GWEN_GUI_PROGRESS_KEEP_OPEN);
      GWEN_Gui_Internal_CheckShow(gui, pd);

      dlg=GWEN_ProgressData_GetDialog(pd);
      if (dlg) {
        if (level<=GWEN_LoggerLevel_Warning) {
          GWEN_DlgProgress_SetStayOpen(dlg, 1);
          GWEN_DlgProgress_SetShowLog(dlg, 1);
        }

        GWEN_DlgProgress_AddLogText(dlg, level, text);
        GWEN_Gui_RunDialog(dlg, 0);
      }
      else
        GWEN_ProgressData_AddLogText(pd, level, text);

      aborted=GWEN_ProgressData_GetAborted(pd);
    }

    if (aborted)
      return GWEN_ERROR_USER_ABORTED;
  }
  return 0;
}



int GWEN_Gui_Internal_InputBox(GWEN_GUI *gui,
                               uint32_t flags,
                               const char *title,
                               const char *text,
                               char *buffer,
                               int minLen,
                               int maxLen,
                               uint32_t guiid) {
  GWEN_DIALOG *dlg;
  int rv;

  dlg=GWEN_DlgInput_new(flags, title, text, minLen, maxLen);
  if (dlg==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create dialog");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_Gui_ExecDialog(dlg, 0);
  if (rv==1) {
    rv=GWEN_DlgInput_CopyInput(dlg, buffer, maxLen);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Dialog_free(dlg);
      return rv;
    }
    if (GWEN_DlgInput_GetFlagAllowStore(dlg))
      rv=1;
    else
      rv=0;
    GWEN_Dialog_free(dlg);
    return rv;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "User aborted");
    GWEN_Dialog_free(dlg);
    return GWEN_ERROR_USER_ABORTED;
  }
}



int GWEN_Gui_Internal_MessageBox(GWEN_GUI *gui,
                                 uint32_t flags,
                                 const char *title,
                                 const char *text,
                                 const char *b1,
                                 const char *b2,
                                 const char *b3,
                                 uint32_t guiid) {
  GWEN_DIALOG *dlg;
  int rv;

  dlg=GWEN_DlgMessage_new(flags, title, text, b1, b2, b3);
  if (dlg==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create dialog");
    return GWEN_ERROR_INTERNAL;
  }

  GWEN_Gui_ExecDialog(dlg, 0);
  rv=GWEN_DlgMessage_GetResponse(dlg);
  GWEN_Dialog_free(dlg);
  return rv;
}



uint32_t GWEN_Gui_Internal_ShowBox(GWEN_GUI *gui,
                                   uint32_t flags,
                                   const char *title,
                                   const char *text,
                                   uint32_t guiid) {
  GWEN_DIALOG *dlg;
  int rv;
  uint32_t id;

  id=++(gui->nextDialogId);

  dlg=GWEN_DlgShowBox_new(flags, title, text);
  if (dlg==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create dialog");
    return 0;
  }

  GWEN_Dialog_SetGuiId(dlg, id);

  rv=GWEN_Gui_OpenDialog(dlg, guiid);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Dialog_free(dlg);
    return 0;
  }

  GWEN_Dialog_List_Add(dlg, gui->activeDialogs);

  return id;
}



void GWEN_Gui_Internal_HideBox(GWEN_GUI *gui, uint32_t id) {
  GWEN_DIALOG *dlg;

  if (id) {
    dlg=GWEN_Dialog_List_First(gui->activeDialogs);
    while(dlg) {
      if (GWEN_Dialog_GetGuiId(dlg)==id)
        break;
      dlg=GWEN_Dialog_List_Next(dlg);
    }
  }
  else
    dlg=GWEN_Dialog_List_Last(gui->activeDialogs);

  if (dlg) {
    int rv;

    rv=GWEN_Gui_CloseDialog(dlg);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    }
    GWEN_Dialog_List_Del(dlg);
    GWEN_Dialog_free(dlg);
  }
}



int GWENHYWFAR_CB GWEN_Gui_Internal_GetSyncIo(GWEN_GUI *gui,
                                              const char *url,
                                              const char *defaultProto,
                                              int defaultPort,
                                              GWEN_SYNCIO **pSio) {
  GWEN_URL *u;
  const char *s;
  int port;
  const char *addr;

  if (!(url && *url)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty URL");
    return GWEN_ERROR_INVALID;
  }

  u=GWEN_Url_fromString(url);
  if (u==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid URL [%s]", url);
    return GWEN_ERROR_INVALID;
  }

  /* determine protocol and port */
  s=GWEN_Url_GetProtocol(u);
  if (!(s && *s))
    s=defaultProto;
  if (!(s && *s))
    s="http";
  port=GWEN_Url_GetPort(u);
  if (port<1)
    port=defaultPort;
  if (port<1)
    port=80;
  addr=GWEN_Url_GetServer(u);
  if (!(addr && *addr)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing server in URL [%s]", url);
    GWEN_Url_free(u);
    return GWEN_ERROR_INVALID;
  }

  if (strcasecmp(s, "http")==0 ||
      strcasecmp(s, "https")==0) {
    GWEN_SYNCIO *sio;
    GWEN_SYNCIO *baseLayer;
    GWEN_DB_NODE *db;
    GWEN_BUFFER *tbuf;
    int rv;

    /* create base io */
    sio=GWEN_SyncIo_Socket_new(GWEN_SocketTypeTCP, GWEN_AddressFamilyIP);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Url_free(u);
      return GWEN_ERROR_GENERIC;
    }

    GWEN_SyncIo_Socket_SetAddress(sio, addr);
    GWEN_SyncIo_Socket_SetPort(sio, port);
    baseLayer=sio;

    if (strcasecmp(s, "https")==0) {
      /* create TLS layer */
      sio=GWEN_SyncIo_Tls_new(baseLayer);
      if (sio==NULL) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        GWEN_SyncIo_free(baseLayer);
        GWEN_Url_free(u);
        return GWEN_ERROR_GENERIC;
      }
      GWEN_SyncIo_Tls_SetRemoteHostName(sio, addr);
      baseLayer=sio;
    }

    /* create buffered layer as needed for HTTP */
    sio=GWEN_SyncIo_Buffered_new(baseLayer);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_SyncIo_free(baseLayer);
      GWEN_Url_free(u);
      return GWEN_ERROR_GENERIC;
    }
    baseLayer=sio;

    /* create HTTP layer */
    sio=GWEN_SyncIo_Http_new(baseLayer);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_SyncIo_free(baseLayer);
      GWEN_Url_free(u);
      return GWEN_ERROR_GENERIC;
    }

    /* setup default command and header */
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    db=GWEN_SyncIo_Http_GetDbCommandOut(sio);

    /* get command string (e.g. server-relative path plus variables) */
    rv=GWEN_Url_toCommandString(u, tbuf);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid path in URL, ignoring (%d)", rv);
    }
    else
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "url", GWEN_Buffer_GetStart(tbuf));
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "command", "GET");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", "HTTP/1.0");

    /* preset some headers */
    db=GWEN_SyncIo_Http_GetDbHeaderOut(sio);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Host", addr);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");

    /* done */
    GWEN_Url_free(u);
    *pSio=sio;
    return 0;
  }
  else {
    GWEN_SYNCIO *sio;

    /* create base io */
    sio=GWEN_SyncIo_Socket_new(GWEN_SocketTypeTCP, GWEN_AddressFamilyIP);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Url_free(u);
      return GWEN_ERROR_GENERIC;
    }
    GWEN_SyncIo_Socket_SetAddress(sio, addr);
    GWEN_SyncIo_Socket_SetPort(sio, port);

    /* done */
    GWEN_Url_free(u);
    *pSio=sio;
    return 0;
  }

}



static int GWEN_Gui__HashPair(const char *token, const char *pin, GWEN_BUFFER *buf) {
  GWEN_MDIGEST *md;
  int rv;

  /* hash token and pin */
  md=GWEN_MDigest_Md5_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)token, strlen(token));
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)pin, strlen(pin));
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  GWEN_Text_ToHexBuffer((const char*)GWEN_MDigest_GetDigestPtr(md),
                        GWEN_MDigest_GetDigestSize(md),
                        buf,
                        0, 0, 0);
  GWEN_MDigest_free(md);
  return 0;
}




static int GWENHYWFAR_CB GWEN_Gui_Internal_GetPassword(GWEN_GUI *gui,
    uint32_t flags,
    const char *token,
    const char *title,
    const char *text,
    char *buffer,
    int minLen,
    int maxLen,
    uint32_t guiid) {
  if ((flags & GWEN_GUI_INPUT_FLAGS_TAN) ||
      (flags & GWEN_GUI_INPUT_FLAGS_DIRECT) ||
      (gui->dbPasswords==NULL)
     ) {
    return GWEN_Gui_InputBox(flags,
                             title,
                             text,
                             buffer,
                             minLen,
                             maxLen,
                             guiid);
  }
  else {
    GWEN_BUFFER *buf;
    int rv;
    const char *s;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_EscapeToBufferTolerant(token, buf);

    if (!(flags & GWEN_GUI_INPUT_FLAGS_CONFIRM)) {
      s=GWEN_DB_GetCharValue(gui->dbPasswords,
                             GWEN_Buffer_GetStart(buf),
                             0, NULL);
      if (s) {
        int i;

        i=strlen(s);
        if (i>=minLen && i < maxLen) {
          memmove(buffer, s, i+1);
          GWEN_Buffer_free(buf);
          return 0;
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "Stored password [%s] is not within size limits (%d), rejecting.",
                    GWEN_Buffer_GetStart(buf), i);
        }
      }
    }

    /* passwd not in password cache, look for it in password storage */
    if (gui->passwdStore) {
      rv=GWEN_PasswordStore_GetPassword(gui->passwdStore, token, buffer, minLen, maxLen);
      if (rv<0) {
        if (rv==GWEN_ERROR_NOT_FOUND || rv==GWEN_ERROR_NO_DATA) {
          DBG_INFO(GWEN_LOGDOMAIN, "Password not found in PasswordStore");
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Buffer_free(buf);
          return rv;
        }
      }
      else {
        /* got password */
        return 0;
      }
    }

    if (gui->flags & GWEN_GUI_FLAGS_NONINTERACTIVE) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Password for [%s] missing in noninteractive mode, "
                "aborting", GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
      return GWEN_ERROR_USER_ABORTED;
    }

    for (;;) {
      int rv2;

      rv=GWEN_Gui_InputBox(flags,
                           title,
                           text,
                           buffer,
                           minLen,
                           maxLen,
                           guiid);
      if (rv<0) {
        GWEN_Buffer_free(buf);
        return rv;
      }
      else {
        GWEN_BUFFER *hbuf;
        int isBad=0;

        hbuf=GWEN_Buffer_new(0, 64, 0, 1);
        GWEN_Gui__HashPair(token, buffer, hbuf);
        isBad=GWEN_StringList_HasString(gui->badPasswords,
                                        GWEN_Buffer_GetStart(hbuf));
        if (!isBad) {
          GWEN_Buffer_free(hbuf);
          break;
        }
        rv2=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                                GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
                                GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                                I18N("Enforce PIN"),
                                I18N(
                                  "You entered the same PIN twice.\n"
                                  "The PIN is marked as bad, do you want\n"
                                  "to use it anyway?"
                                  "<html>"
                                  "<p>"
                                  "You entered the same PIN twice."
                                  "</p>"
                                  "<p>"
                                  "The PIN is marked as <b>bad</b>, "
                                  "do you want to use it anyway?"
                                  "</p>"
                                  "</html>"),
                                I18N("Yes, use anyway"),
                                I18N("Re-enter"),
                                0,
                                guiid);
        if (rv2==1) {
          /* accept this input */
          GWEN_StringList_RemoveString(gui->badPasswords,
                                       GWEN_Buffer_GetStart(hbuf));
          GWEN_Buffer_free(hbuf);
          break;
        }
        GWEN_Buffer_free(hbuf);
      }
    } /* for */

    /* store in temporary cache */
    GWEN_DB_SetCharValue(gui->dbPasswords, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         GWEN_Buffer_GetStart(buf), buffer);

    /* only store passwd in storage if allowed by the user */
    if (rv==1 && gui->passwdStore) {
      rv=GWEN_PasswordStore_SetPassword(gui->passwdStore, token, buffer);
      if (rv<0) {
        DBG_WARN(GWEN_LOGDOMAIN, "Could not store password (%d)", rv);
      }
    }

    GWEN_Buffer_free(buf);
    return 0;
  }
}



static int GWENHYWFAR_CB GWEN_Gui_Internal_SetPasswordStatus(GWEN_GUI *gui,
                                                             const char *token,
                                                             const char *pin,
                                                             GWEN_GUI_PASSWORD_STATUS status,
                                                             GWEN_UNUSED uint32_t guiid) {
  if (token==NULL && pin==NULL && status==GWEN_Gui_PasswordStatus_Remove) {
    /* complete cleaning is requested */
    if (gui->passwdStore)
      GWEN_PasswordStore_ClearStoragePasswd(gui->passwdStore);
    if (gui->persistentPasswords==0)
      GWEN_DB_ClearGroup(gui->dbPasswords, NULL);
  }
  else {
    GWEN_BUFFER *hbuf;

    /* setting ststus of a specific password/pin */
    hbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Gui__HashPair(token, pin, hbuf);
    if (status==GWEN_Gui_PasswordStatus_Bad) {
      GWEN_StringList_AppendString(gui->badPasswords,
                                   GWEN_Buffer_GetStart(hbuf),
                                   0, 1);
      /* remove from permanent passwd storage */
      if (gui->passwdStore) {
        int rv;

        rv=GWEN_PasswordStore_SetPassword(gui->passwdStore, token, NULL);
        if (rv<0) {
          DBG_WARN(GWEN_LOGDOMAIN, "Could not remove password from storage (%d)", rv);
        }
      }
    }
    else if (status==GWEN_Gui_PasswordStatus_Ok ||
             status==GWEN_Gui_PasswordStatus_Remove) {
      if (gui->persistentPasswords==0)
        GWEN_StringList_RemoveString(gui->badPasswords, GWEN_Buffer_GetStart(hbuf));
    }
    GWEN_Buffer_free(hbuf);
  }

  return 0;
}




GWEN_LOGGER_LEVEL GWEN_Gui_GetMinProgressLogLevel(const GWEN_GUI *gui) {
  assert(gui);
  return gui->minProgressLogLevel;
}



void GWEN_Gui_SetMinProgressLogLevel(GWEN_GUI *gui, GWEN_LOGGER_LEVEL ll) {
  assert(gui);
  gui->minProgressLogLevel=ll;
}




