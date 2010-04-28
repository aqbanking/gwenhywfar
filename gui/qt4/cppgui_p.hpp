/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef CPPGUI_P_HPP
#define CPPGUI_P_HPP


#include "cppgui.hpp"


class CppGuiLinker {
  friend class CppGui;

  static int Print(GWEN_GUI *gui,
		   const char *docTitle,
		   const char *docType,
		   const char *descr,
		   const char *text,
		   uint32_t guiid);

  static int GetPassword(GWEN_GUI *gui,
			 uint32_t flags,
			 const char *token,
			 const char *title,
			 const char *text,
			 char *buffer,
			 int minLen,
			 int maxLen,
			 uint32_t guiid);

  static int SetPasswordStatus(GWEN_GUI *gui,
			       const char *token,
			       const char *pin,
			       GWEN_GUI_PASSWORD_STATUS status,
			       uint32_t guiid);

  static int CheckCert(GWEN_GUI *gui,
		       const GWEN_SSLCERTDESCR *cert,
		       GWEN_SYNCIO *sio,
		       uint32_t guiid);

  static int ExecDialog(GWEN_GUI *gui,
			GWEN_DIALOG *dlg,
			uint32_t guiid);

  static int OpenDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, uint32_t guiid);
  static int CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg);
  static int RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int untilEnd);


  static int GetFileName(GWEN_GUI *gui,
			 const char *caption,
			 GWEN_GUI_FILENAME_TYPE fnt,
			 uint32_t flags,
			 const char *patterns,
			 GWEN_BUFFER *pathBuffer,
			 uint32_t guiid);

  static GWENHYWFAR_CB void freeData(void *bp, void *p);
};




#endif /* CPPGUI_P_HPP */


