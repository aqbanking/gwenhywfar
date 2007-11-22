/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: error.h 1104 2007-01-03 09:21:32Z martin $
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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

#include "gui_be.h"


struct GWEN_GUI {
  GWEN_INHERIT_ELEMENT(GWEN_GUI)

  GWEN_GUI_MESSAGEBOX_FN messageBoxFn;
  GWEN_GUI_INPUTBOX_FN inputBoxFn;

  GWEN_GUI_SHOWBOX_FN showBoxFn;
  GWEN_GUI_HIDEBOX_FN hideBoxFn;

  GWEN_GUI_PROGRESS_START_FN progressStartFn;
  GWEN_GUI_PROGRESS_ADVANCE_FN progressAdvanceFn;
  GWEN_GUI_PROGRESS_LOG_FN progressLogFn;
  GWEN_GUI_PROGRESS_END_FN progressEndFn;

  GWEN_GUI_PRINT_FN printFn;

  GWEN_GUI_GETPASSWORD_FN getPasswordFn;
  GWEN_GUI_SETPASSWORDSTATUS_FN setPasswordStatusFn;

  GWEN_GUI_LOG_HOOK_FN logHookFn;

  GWEN_GUI_WAITFORSOCKETS_FN waitForSocketsFn;

  GWEN_GUI_CHECKCERT_FN checkCertFn;

  GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN keyDataFromTextOpenSslFn;

  uint32_t refCount;
};


static int GWEN_Gui_CheckCertBuiltIn(GWEN_GUI *gui,
				     const GWEN_SSLCERTDESCR *cd,
				     GWEN_IO_LAYER *io, uint32_t guiid);




#endif
