/***************************************************************************
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002-2010 by Martin Preuss
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


#ifndef GWENHYWFAR_GUI_GUI_P_H
#define GWENHYWFAR_GUI_GUI_P_H

#include "gui_l.h"
#include "progressdata_l.h"

#include <gwenhywfar/stringlist.h>




struct GWEN_GUI {
  GWEN_INHERIT_ELEMENT(GWEN_GUI)

  GWEN_GUI_MESSAGEBOX_FN messageBoxFn;
  GWEN_GUI_INPUTBOX_FN inputBoxFn;

  GWEN_GUI_SHOWBOX_FN showBoxFn;
  GWEN_GUI_HIDEBOX_FN hideBoxFn;

  GWEN_GUI_PROGRESS_START_FN progressStartFn;
  GWEN_GUI_PROGRESS_ADVANCE_FN progressAdvanceFn;
  GWEN_GUI_PROGRESS_SETTOTAL_FN progressSetTotalFn;
  GWEN_GUI_PROGRESS_LOG_FN progressLogFn;
  GWEN_GUI_PROGRESS_END_FN progressEndFn;

  GWEN_GUI_PRINT_FN printFn;

  GWEN_GUI_GETPASSWORD_FN getPasswordFn;
  GWEN_GUI_SETPASSWORDSTATUS_FN setPasswordStatusFn;

  GWEN_GUI_LOG_HOOK_FN logHookFn;

  GWEN_GUI_WAITFORSOCKETS_FN waitForSocketsFn;

  GWEN_GUI_CHECKCERT_FN checkCertFn;

  GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN keyDataFromTextOpenSslFn;

  GWEN_GUI_EXEC_DIALOG_FN execDialogFn;

  GWEN_GUI_OPEN_DIALOG_FN openDialogFn;
  GWEN_GUI_CLOSE_DIALOG_FN closeDialogFn;
  GWEN_GUI_RUN_DIALOG_FN runDialogFn;

  GWEN_GUI_READ_DIALOG_PREFS_FN readDialogPrefsFn;
  GWEN_GUI_WRITE_DIALOG_PREFS_FN writeDialogPrefsFn;

  GWEN_GUI_GET_FILENAME_FN getFileNameFn;

  GWEN_GUI_GETSYNCIO_FN getSyncIoFn;

  uint32_t flags;

  GWEN_PROGRESS_DATA_TREE *progressDataTree;
  GWEN_DIALOG_LIST *activeDialogs;

  uint32_t nextProgressId;
  uint32_t lastProgressId;

  uint32_t nextDialogId;

  char *name;
  char *charSet;


  int inLogHook;

  GWEN_DB_NODE *dbPasswords;
  GWEN_PASSWD_STORE *passwdStore;
  GWEN_STRINGLIST *badPasswords;
  int persistentPasswords;

  GWEN_LOGGER_LEVEL minProgressLogLevel;

  uint32_t refCount;
};


static int GWENHYWFAR_CB GWEN_Gui_CheckCertBuiltIn(GWEN_GUI *gui,
                                                   const GWEN_SSLCERTDESCR *cd,
                                                   GWEN_SYNCIO *sio, uint32_t guiid);


static void GWEN_Gui_DialogBased_CheckShow(GWEN_GUI *gui, GWEN_PROGRESS_DATA *pd);

static uint32_t GWENHYWFAR_CB GWEN_Gui_DialogBased_ProgressStart(GWEN_GUI *gui,
                                                uint32_t progressFlags,
                                                const char *title,
                                                const char *text,
                                                uint64_t total,
                                                uint32_t guiid);
static int GWENHYWFAR_CB GWEN_Gui_DialogBased_ProgressEnd(GWEN_GUI *gui, uint32_t pid);
static int GWENHYWFAR_CB GWEN_Gui_DialogBased_ProgressAdvance(GWEN_GUI *gui, uint32_t pid, uint64_t progress);
static int GWENHYWFAR_CB GWEN_Gui_DialogBased_ProgressSetTotal(GWEN_GUI *gui, uint32_t pid, uint64_t total);

static int GWENHYWFAR_CB GWEN_Gui_DialogBased_ProgressLog(GWEN_GUI *gui,
                                         uint32_t pid,
                                         GWEN_LOGGER_LEVEL level,
                                         const char *text);

static int GWENHYWFAR_CB GWEN_Gui_DialogBased_InputBox(GWEN_GUI *gui,
                                      uint32_t flags,
                                      const char *title,
                                      const char *text,
                                      char *buffer,
                                      int minLen,
                                      int maxLen,
                                      uint32_t guiid);

static int GWENHYWFAR_CB GWEN_Gui_DialogBased_MessageBox(GWEN_GUI *gui,
                                        uint32_t flags,
                                        const char *title,
                                        const char *text,
                                        const char *b1,
                                        const char *b2,
                                        const char *b3,
                                        uint32_t guiid);

static uint32_t GWENHYWFAR_CB GWEN_Gui_DialogBased_ShowBox(GWEN_GUI *gui,
                                          uint32_t flags,
                                          const char *title,
                                          const char *text,
                                          uint32_t guiid);
static void GWENHYWFAR_CB GWEN_Gui_DialogBased_HideBox(GWEN_GUI *gui, uint32_t id);

static int GWENHYWFAR_CB GWEN_Gui_Internal_GetSyncIo(GWEN_GUI *gui, const char *url,
                                                     const char *defaultProto,
                                                     int defaultPort,
                                                     GWEN_SYNCIO **pSio);

static int GWENHYWFAR_CB GWEN_Gui_Internal_SetPasswordStatus(GWEN_GUI *gui,
                                                             const char *token,
                                                             const char *pin,
                                                             GWEN_GUI_PASSWORD_STATUS status,
                                                             GWEN_UNUSED uint32_t guiid);
static int GWENHYWFAR_CB GWEN_Gui_Internal_GetPassword(GWEN_GUI *gui,
                                                       uint32_t flags,
                                                       const char *token,
                                                       const char *title,
                                                       const char *text,
                                                       char *buffer,
                                                       int minLen,
                                                       int maxLen,
                                                       GWEN_GUI_PASSWORD_METHOD methodId,
                                                       GWEN_DB_NODE *methodParams,
                                                       uint32_t guiid);


static int GWEN_Gui__HashPair(const char *token, const char *pin, GWEN_BUFFER *buf);


#endif
