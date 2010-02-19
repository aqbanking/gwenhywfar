/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cppgui_p.hpp"
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT(GWEN_GUI, CppGui);




int CppGuiLinker::Print(GWEN_GUI *gui,
			const char *docTitle,
			const char *docType,
			const char *descr,
			const char *text,
			uint32_t guiid){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->print(docTitle, docType, descr, text, guiid);
}



int CppGuiLinker::GetPassword(GWEN_GUI *gui,
			      uint32_t flags,
			      const char *token,
			      const char *title,
			      const char *text,
			      char *buffer,
			      int minLen,
			      int maxLen,
			      uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->getPassword(flags, token, title, text, buffer, minLen, maxLen, guiid);
}



int CppGuiLinker::SetPasswordStatus(GWEN_GUI *gui,
				    const char *token,
				    const char *pin,
				    GWEN_GUI_PASSWORD_STATUS status,
				    uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->setPasswordStatus(token, pin, status, guiid);
}



int CppGuiLinker::CheckCert(GWEN_GUI *gui,
			    const GWEN_SSLCERTDESCR *cert,
			    GWEN_IO_LAYER *io,
			    uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->checkCert(cert, io, guiid);
}



int CppGuiLinker::ExecDialog(GWEN_GUI *gui,
			     GWEN_DIALOG *dlg,
			     uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->execDialog(dlg, guiid);
}



int CppGuiLinker::OpenDialog(GWEN_GUI *gui,
			     GWEN_DIALOG *dlg,
			     uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  DBG_ERROR(0, "CppGuiLinker::OpenDialog");

  return xgui->openDialog(dlg, guiid);
}



int CppGuiLinker::CloseDialog(GWEN_GUI *gui,
			      GWEN_DIALOG *dlg) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->closeDialog(dlg);
}



int CppGuiLinker::RunDialog(GWEN_GUI *gui,
			    GWEN_DIALOG *dlg,
			    int untilEnd) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->runDialog(dlg, untilEnd);
}



int CppGuiLinker::GetFileName(GWEN_GUI *gui,
			      const char *caption,
			      GWEN_GUI_FILENAME_TYPE fnt,
			      uint32_t flags,
			      const char *patterns,
			      GWEN_BUFFER *pathBuffer,
			      uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->getFileName(caption, fnt, flags, patterns, pathBuffer, guiid);
}



GWENHYWFAR_CB
void CppGuiLinker::freeData(void *bp, void *p) {
  CppGui *xgui;

  DBG_NOTICE(0, "CppGuiLinker: Freeing CppGui");
  xgui=(CppGui*)p;
  if (xgui->_gui) {
    xgui->_gui=0;
  }
  delete xgui;
}












CppGui::CppGui()
:_checkCertFn(NULL)
,_dbPasswords(NULL)
,_gui(NULL) {
  _gui=GWEN_Gui_new();
  _dbPasswords=GWEN_DB_Group_new("passwords");

  GWEN_INHERIT_SETDATA(GWEN_GUI, CppGui,
		       _gui, this,
		       CppGuiLinker::freeData);
  GWEN_Gui_UseDialogs(_gui);
  _printFn=GWEN_Gui_SetPrintFn(_gui, CppGuiLinker::Print);
  _getPasswordFn=GWEN_Gui_SetGetPasswordFn(_gui, CppGuiLinker::GetPassword);
  _setPasswordStatusFn=GWEN_Gui_SetSetPasswordStatusFn(_gui, CppGuiLinker::SetPasswordStatus);
  _checkCertFn=GWEN_Gui_SetCheckCertFn(_gui, CppGuiLinker::CheckCert);
  _execDialogFn=GWEN_Gui_SetExecDialogFn(_gui, CppGuiLinker::ExecDialog);
  _openDialogFn=GWEN_Gui_SetOpenDialogFn(_gui, CppGuiLinker::OpenDialog);
  _closeDialogFn=GWEN_Gui_SetCloseDialogFn(_gui, CppGuiLinker::CloseDialog);
  _runDialogFn=GWEN_Gui_SetRunDialogFn(_gui, CppGuiLinker::RunDialog);
  _getFileNameFn=GWEN_Gui_SetGetFileNameFn(_gui, CppGuiLinker::GetFileName);
}



CppGui::~CppGui(){
  if (_gui) {
    GWEN_INHERIT_UNLINK(GWEN_GUI, CppGui, _gui)
    GWEN_Gui_free(_gui);
  }
  GWEN_DB_Group_free(_dbPasswords);
}



int CppGui::print(const char *docTitle,
		  const char *docType,
		  const char *descr,
		  const char *text,
		  uint32_t guiid){
  if (_printFn)
    return _printFn(_gui, docTitle, docType, descr, text, guiid);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



std::string CppGui::_getPasswordHash(const char *token, const char *pin) {
  GWEN_MDIGEST *md;
  std::string s;
  GWEN_BUFFER *buf;
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
    return "";
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Text_ToHexBuffer((const char*)GWEN_MDigest_GetDigestPtr(md),
			GWEN_MDigest_GetDigestSize(md),
			buf,
			0, 0, 0);
  s=std::string(GWEN_Buffer_GetStart(buf),
		GWEN_Buffer_GetUsedBytes(buf));
  GWEN_Buffer_free(buf);

  GWEN_MDigest_free(md);
  return s;
}



int CppGui::getPassword(uint32_t flags,
			const char *token,
			const char *title,
			const char *text,
			char *buffer,
			int minLen,
			int maxLen,
			uint32_t guiid) {
  if (flags & GWEN_GUI_INPUT_FLAGS_TAN) {
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
      s=GWEN_DB_GetCharValue(_dbPasswords,
			     GWEN_Buffer_GetStart(buf),
			     0, NULL);
      if (s) {
	int i;

	i=strlen(s);
	if (i>=minLen && i<=maxLen) {
	  memmove(buffer, s, i+1);
	  GWEN_Buffer_free(buf);
	  return 0;
	}
      }
    }
  
    for (;;) {
      rv=GWEN_Gui_InputBox(flags,
			   title,
			   text,
			   buffer,
			   minLen,
			   maxLen,
			   guiid);
      if (rv) {
	GWEN_Buffer_free(buf);
	return rv;
      }
      else {
	std::string s;
	std::list<std::string>::iterator it;
	bool isBad=false;
  
	s=_getPasswordHash(token, buffer);
	for (it=_badPasswords.begin();
	     it!=_badPasswords.end();
	     it++) {
	  if (*it==s) {
	    /* password is bad */
	    isBad=true;
	    break;
	  }
	}
    
	if (!isBad)
	  break;
	rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
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
			       I18N("Use my input"),
			       I18N("Re-enter"),
			       0,
			       guiid);
	if (rv==1) {
	  /* accept this input */
	  _badPasswords.remove(s);
	  break;
	}
      }
    }

    GWEN_Buffer_free(buf);
    return 0;
  }
}



int CppGui::checkCert(const GWEN_SSLCERTDESCR *cd,
		      GWEN_IO_LAYER *io,
		      uint32_t guiid) {
  return checkCertBuiltIn(cd, io, guiid);
}



int CppGui::execDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::openDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  DBG_ERROR(0, "CppGui::OpenDialog");

  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::closeDialog(GWEN_DIALOG *dlg) {
  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::runDialog(GWEN_DIALOG *dlg, int untilEnd) {
  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::getFileName(const char *caption,
			GWEN_GUI_FILENAME_TYPE fnt,
			uint32_t flags,
			const char *patterns,
			GWEN_BUFFER *pathBuffer,
			uint32_t guiid) {
  DBG_ERROR(0, "Not supported");
  return GWEN_ERROR_NOT_SUPPORTED;
}




int CppGui::checkCertBuiltIn(const GWEN_SSLCERTDESCR *cert,
			     GWEN_IO_LAYER *io,
			     uint32_t guiid) {
  if (_checkCertFn)
    return _checkCertFn(_gui, cert, io, guiid);
  else {
    DBG_ERROR(0, "No built-in checkcert function?");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int CppGui::setPasswordStatus(const char *token,
			      const char *pin,
			      GWEN_GUI_PASSWORD_STATUS status,
			      uint32_t guiid) {
  if (token==NULL && pin==NULL && status==GWEN_Gui_PasswordStatus_Remove) {
    GWEN_DB_ClearGroup(_dbPasswords, NULL);
  }
  else {
    GWEN_BUFFER *buf;
    std::string s;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_EscapeToBufferTolerant(token, buf);

    s=_getPasswordHash(token, pin);
    if (status==GWEN_Gui_PasswordStatus_Bad) {
      std::list<std::string>::iterator it;

      s=_getPasswordHash(token, pin);
      for (it=_badPasswords.begin();
	   it!=_badPasswords.end();
	   it++) {
	if (*it==s) {
	  /* bad password already in list */
	  GWEN_Buffer_free(buf);
	  return 0;
	}
      }
      _badPasswords.push_back(s);
    }
    else if (status==GWEN_Gui_PasswordStatus_Ok) {
      /* only store passwords of which we know that they are ok */
      GWEN_DB_SetCharValue(_dbPasswords, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   GWEN_Buffer_GetStart(buf), pin);
    }
    GWEN_Buffer_free(buf);
  }

  return 0;
}



GWEN_GUI *CppGui::getCInterface(){
  return _gui;
}



CppGui *CppGui::getCppGui(){
  GWEN_GUI *gui;
  CppGui *xgui;

  gui=GWEN_Gui_GetGui();
  if (gui==NULL)
    return NULL;
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  return xgui;
}





