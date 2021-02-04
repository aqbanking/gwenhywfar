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


#ifndef GWENHYWFAR_GUI_NOGUI_P_H
#define GWENHYWFAR_GUI_NOGUI_P_H

#include "nogui.h"

#include <gwenhywfar/gui_be.h>




typedef struct GWEN_GUI_NOGUI GWEN_GUI_NOGUI;
struct GWEN_GUI_NOGUI {
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
};


#endif

