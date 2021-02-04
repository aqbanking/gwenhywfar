/***************************************************************************
 begin       : Fri Feb 07 2003
 copyright   : (C) 2021 by Martin Preuss
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

/* included from gui.c */



GWEN_GUI_MESSAGEBOX_FN GWEN_Gui_SetMessageBoxFn(GWEN_GUI *gui,
                                                GWEN_GUI_MESSAGEBOX_FN f)
{
  GWEN_GUI_MESSAGEBOX_FN of;

  assert(gui);
  of=gui->messageBoxFn;
  gui->messageBoxFn=f;
  return of;
}



GWEN_GUI_INPUTBOX_FN GWEN_Gui_SetInputBoxFn(GWEN_GUI *gui,
                                            GWEN_GUI_INPUTBOX_FN f)
{
  GWEN_GUI_INPUTBOX_FN of;

  assert(gui);
  of=gui->inputBoxFn;
  gui->inputBoxFn=f;
  return of;
}



GWEN_GUI_SHOWBOX_FN GWEN_Gui_SetShowBoxFn(GWEN_GUI *gui,
                                          GWEN_GUI_SHOWBOX_FN f)
{
  GWEN_GUI_SHOWBOX_FN of;

  assert(gui);
  of=gui->showBoxFn;
  gui->showBoxFn=f;
  return of;
}



GWEN_GUI_HIDEBOX_FN GWEN_Gui_SetHideBoxFn(GWEN_GUI *gui,
                                          GWEN_GUI_HIDEBOX_FN f)
{
  GWEN_GUI_HIDEBOX_FN of;

  assert(gui);
  of=gui->hideBoxFn;
  gui->hideBoxFn=f;
  return of;
}



GWEN_GUI_PROGRESS_START_FN GWEN_Gui_SetProgressStartFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_START_FN f)
{
  GWEN_GUI_PROGRESS_START_FN of;

  assert(gui);
  of=gui->progressStartFn;
  gui->progressStartFn=f;
  return of;
}



GWEN_GUI_PROGRESS_ADVANCE_FN GWEN_Gui_SetProgressAdvanceFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_ADVANCE_FN f)
{
  GWEN_GUI_PROGRESS_ADVANCE_FN of;

  assert(gui);
  of=gui->progressAdvanceFn;
  gui->progressAdvanceFn=f;
  return of;
}



GWEN_GUI_PROGRESS_SETTOTAL_FN GWEN_Gui_SetProgressSetTotalFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_SETTOTAL_FN f)
{
  GWEN_GUI_PROGRESS_SETTOTAL_FN of;

  assert(gui);
  of=gui->progressSetTotalFn;
  gui->progressSetTotalFn=f;
  return of;
}



GWEN_GUI_PROGRESS_LOG_FN GWEN_Gui_SetProgressLogFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_LOG_FN f)
{
  GWEN_GUI_PROGRESS_LOG_FN of;

  assert(gui);
  of=gui->progressLogFn;
  gui->progressLogFn=f;
  return of;
}



GWEN_GUI_PROGRESS_END_FN GWEN_Gui_SetProgressEndFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_END_FN f)
{
  GWEN_GUI_PROGRESS_END_FN of;

  assert(gui);
  of=gui->progressEndFn;
  gui->progressEndFn=f;
  return of;
}



GWEN_GUI_PRINT_FN GWEN_Gui_SetPrintFn(GWEN_GUI *gui,
                                      GWEN_GUI_PRINT_FN f)
{
  GWEN_GUI_PRINT_FN of;

  assert(gui);
  of=gui->printFn;
  gui->printFn=f;
  return of;
}



GWEN_GUI_GETPASSWORD_FN GWEN_Gui_SetGetPasswordFn(GWEN_GUI *gui,
                                                  GWEN_GUI_GETPASSWORD_FN f)
{
  GWEN_GUI_GETPASSWORD_FN of;

  assert(gui);
  of=gui->getPasswordFn;
  gui->getPasswordFn=f;
  return of;
}



GWEN_GUI_SETPASSWORDSTATUS_FN GWEN_Gui_SetSetPasswordStatusFn(GWEN_GUI *gui,
                                                              GWEN_GUI_SETPASSWORDSTATUS_FN f)
{
  GWEN_GUI_SETPASSWORDSTATUS_FN of;

  assert(gui);
  of=gui->setPasswordStatusFn;
  gui->setPasswordStatusFn=f;
  return of;
}



GWEN_GUI_LOG_HOOK_FN GWEN_Gui_SetLogHookFn(GWEN_GUI *gui,
                                           GWEN_GUI_LOG_HOOK_FN f)
{
  GWEN_GUI_LOG_HOOK_FN of;

  assert(gui);
  of=gui->logHookFn;
  gui->logHookFn=f;

  return of;
}



GWEN_GUI_WAITFORSOCKETS_FN GWEN_Gui_SetWaitForSocketsFn(GWEN_GUI *gui,
                                                        GWEN_GUI_WAITFORSOCKETS_FN f)
{
  GWEN_GUI_WAITFORSOCKETS_FN of;

  assert(gui);
  of=gui->waitForSocketsFn;
  gui->waitForSocketsFn=f;

  return of;
}



GWEN_GUI_CHECKCERT_FN GWEN_Gui_SetCheckCertFn(GWEN_GUI *gui, GWEN_GUI_CHECKCERT_FN f)
{
  GWEN_GUI_CHECKCERT_FN of;

  assert(gui);
  of=gui->checkCertFn;
  gui->checkCertFn=f;

  return of;
}



GWEN_GUI_EXEC_DIALOG_FN GWEN_Gui_SetExecDialogFn(GWEN_GUI *gui, GWEN_GUI_EXEC_DIALOG_FN f)
{
  GWEN_GUI_EXEC_DIALOG_FN of;

  assert(gui);
  of=gui->execDialogFn;
  gui->execDialogFn=f;

  return of;
}



GWEN_GUI_OPEN_DIALOG_FN GWEN_Gui_SetOpenDialogFn(GWEN_GUI *gui, GWEN_GUI_OPEN_DIALOG_FN f)
{
  GWEN_GUI_OPEN_DIALOG_FN of;

  assert(gui);
  of=gui->openDialogFn;
  gui->openDialogFn=f;

  return of;
}



GWEN_GUI_CLOSE_DIALOG_FN GWEN_Gui_SetCloseDialogFn(GWEN_GUI *gui, GWEN_GUI_CLOSE_DIALOG_FN f)
{
  GWEN_GUI_CLOSE_DIALOG_FN of;

  assert(gui);
  of=gui->closeDialogFn;
  gui->closeDialogFn=f;

  return of;
}



GWEN_GUI_RUN_DIALOG_FN GWEN_Gui_SetRunDialogFn(GWEN_GUI *gui, GWEN_GUI_RUN_DIALOG_FN f)
{
  GWEN_GUI_RUN_DIALOG_FN of;

  assert(gui);
  of=gui->runDialogFn;
  gui->runDialogFn=f;

  return of;
}



GWEN_GUI_READ_DIALOG_PREFS_FN GWEN_Gui_SetReadDialogPrefsFn(GWEN_GUI *gui, GWEN_GUI_READ_DIALOG_PREFS_FN f)
{
  GWEN_GUI_READ_DIALOG_PREFS_FN of;

  assert(gui);
  of=gui->readDialogPrefsFn;
  gui->readDialogPrefsFn=f;

  return of;
}



GWEN_GUI_WRITE_DIALOG_PREFS_FN GWEN_Gui_SetWriteDialogPrefsFn(GWEN_GUI *gui, GWEN_GUI_WRITE_DIALOG_PREFS_FN f)
{
  GWEN_GUI_WRITE_DIALOG_PREFS_FN of;

  assert(gui);
  of=gui->writeDialogPrefsFn;
  gui->writeDialogPrefsFn=f;

  return of;
}



GWEN_GUI_GET_FILENAME_FN GWEN_Gui_SetGetFileNameFn(GWEN_GUI *gui, GWEN_GUI_GET_FILENAME_FN f)
{
  GWEN_GUI_GET_FILENAME_FN of;

  assert(gui);
  of=gui->getFileNameFn;
  gui->getFileNameFn=f;

  return of;
}



GWEN_GUI_GETSYNCIO_FN GWEN_Gui_SetGetSyncIoFn(GWEN_GUI *gui, GWEN_GUI_GETSYNCIO_FN f)
{
  GWEN_GUI_GETSYNCIO_FN of;

  assert(gui);
  of=gui->getSyncIoFn;
  gui->getSyncIoFn=f;

  return of;
}



GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN GWEN_Gui_SetKeyDataFromTextOpenSslFn(GWEN_GUI *gui,
                                                                         GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN f)
{
  GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN of;

  assert(gui);
  of=gui->keyDataFromTextOpenSslFn;
  gui->keyDataFromTextOpenSslFn=f;

  return of;

}





int GWEN_Gui_MessageBox(uint32_t flags,
                        const char *title,
                        const char *text,
                        const char *b1,
                        const char *b2,
                        const char *b3,
                        uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->messageBoxFn)
    return gui->messageBoxFn(gui, flags, title, text, b1, b2, b3, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_InputBox(uint32_t flags,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen,
                      uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->inputBoxFn)
    return gui->inputBoxFn(gui, flags, title, text, buffer, minLen, maxLen, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



uint32_t GWEN_Gui_ShowBox(uint32_t flags,
                          const char *title,
                          const char *text,
                          uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->showBoxFn)
    return gui->showBoxFn(gui, flags, title, text, guiid);
  return 0;
}



void GWEN_Gui_HideBox(uint32_t id)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->hideBoxFn)
    return gui->hideBoxFn(gui, id);
}



uint32_t GWEN_Gui_ProgressStart(uint32_t progressFlags,
                                const char *title,
                                const char *text,
                                uint64_t total,
                                uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->progressStartFn)
    return gui->progressStartFn(gui, progressFlags, title, text, total, guiid);
  return 0;
}



int GWEN_Gui_ProgressAdvance(uint32_t id, uint32_t progress)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->progressAdvanceFn)
    return gui->progressAdvanceFn(gui, id, progress);
  return 0;
}



int GWEN_Gui_ProgressSetTotal(uint32_t id, uint64_t total)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->progressSetTotalFn)
    return gui->progressSetTotalFn(gui, id, total);
  return 0;
}



int GWEN_Gui_ProgressLog(uint32_t id,
                         GWEN_LOGGER_LEVEL level,
                         const char *text)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->progressLogFn)
    return gui->progressLogFn(gui, id, level, text);
  return 0;
}



int GWEN_Gui_ProgressLog2(uint32_t id,
                          GWEN_LOGGER_LEVEL level,
                          const char *fmt, ...)
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

  return GWEN_Gui_ProgressLog(id, level, msgbuffer);
}



int GWEN_Gui_ProgressEnd(uint32_t id)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->progressEndFn)
    return gui->progressEndFn(gui, id);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_Print(const char *docTitle,
                   const char *docType,
                   const char *descr,
                   const char *text,
                   uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->printFn)
    return gui->printFn(gui, docTitle, docType, descr, text, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_LogHook(const char *logDomain,
                     GWEN_LOGGER_LEVEL priority, const char *s)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->logHookFn) {
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

      if (gui->inLogHook==0) {
        /* otherwise the log message seems to be uncritical, convey it */
        gui->inLogHook++;
        rv=gui->logHookFn(gui, logDomain, priority, s);
        gui->inLogHook--;
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



int GWEN_Gui_KeyDataFromText_OpenSSL(const char *text,
                                     unsigned char *buffer,
                                     unsigned int bufLength)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->keyDataFromTextOpenSslFn)
    return gui->keyDataFromTextOpenSslFn(gui, text, buffer, bufLength);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_ExecDialog(GWEN_DIALOG *dlg, uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->execDialogFn)
    return gui->execDialogFn(gui, dlg, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_OpenDialog(GWEN_DIALOG *dlg, uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->openDialogFn)
    return gui->openDialogFn(gui, dlg, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_CloseDialog(GWEN_DIALOG *dlg)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->closeDialogFn)
    return gui->closeDialogFn(gui, dlg);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_RunDialog(GWEN_DIALOG *dlg, int untilEnd)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->runDialogFn)
    return gui->runDialogFn(gui, dlg, untilEnd);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}




int GWEN_Gui_GetFileName(const char *caption,
                         GWEN_GUI_FILENAME_TYPE fnt,
                         uint32_t flags,
                         const char *patterns,
                         GWEN_BUFFER *pathBuffer,
                         uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->getFileNameFn)
    return gui->getFileNameFn(gui, caption, fnt, flags, patterns, pathBuffer, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_ReadDialogPrefs(const char *groupName,
                             const char *altName,
                             GWEN_DB_NODE **pDb)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->readDialogPrefsFn)
    return gui->readDialogPrefsFn(gui, groupName, altName, pDb);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_WriteDialogPrefs(const char *groupName,
                              GWEN_DB_NODE *db)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->writeDialogPrefsFn)
    return gui->writeDialogPrefsFn(gui, groupName, db);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_GetSyncIo(const char *url,
                       const char *defaultProto,
                       int defaultPort,
                       GWEN_SYNCIO **pSio)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->getSyncIoFn)
    return gui->getSyncIoFn(gui, url, defaultProto, defaultPort, pSio);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}





