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
                        uint32_t guiid) {
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
                              GWEN_GUI_PASSWORD_METHOD methodId,
                              GWEN_DB_NODE *methodParams,
                              uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->getPassword(flags, token, title, text, buffer, minLen, maxLen, methodId, methodParams, guiid);
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
                            GWEN_SYNCIO *sio,
                            uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->checkCert(cert, sio, guiid);
}



int CppGuiLinker::LogHook(GWEN_GUI *gui,
                          const char *logDomain,
                          GWEN_LOGGER_LEVEL priority, const char *s) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->logHook(logDomain, priority, s);
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
  ,_gui(NULL) {
  _gui=GWEN_Gui_new();

  GWEN_INHERIT_SETDATA(GWEN_GUI, CppGui,
                       _gui, this,
                       CppGuiLinker::freeData);
  GWEN_Gui_UseDialogs(_gui);
  _checkCertFn=GWEN_Gui_SetCheckCertFn(_gui, CppGuiLinker::CheckCert);
  _printFn=GWEN_Gui_SetPrintFn(_gui, CppGuiLinker::Print);
  _getPasswordFn=GWEN_Gui_SetGetPasswordFn(_gui, CppGuiLinker::GetPassword);
  _setPasswordStatusFn=GWEN_Gui_SetSetPasswordStatusFn(_gui, CppGuiLinker::SetPasswordStatus);
  GWEN_Gui_SetLogHookFn(_gui, CppGuiLinker::LogHook);
  _execDialogFn=GWEN_Gui_SetExecDialogFn(_gui, CppGuiLinker::ExecDialog);
  _openDialogFn=GWEN_Gui_SetOpenDialogFn(_gui, CppGuiLinker::OpenDialog);
  _closeDialogFn=GWEN_Gui_SetCloseDialogFn(_gui, CppGuiLinker::CloseDialog);
  _runDialogFn=GWEN_Gui_SetRunDialogFn(_gui, CppGuiLinker::RunDialog);
  _getFileNameFn=GWEN_Gui_SetGetFileNameFn(_gui, CppGuiLinker::GetFileName);
}



CppGui::~CppGui() {
  if (_gui) {
    GWEN_INHERIT_UNLINK(GWEN_GUI, CppGui, _gui)
    GWEN_Gui_free(_gui);
  }
}



int CppGui::checkCert(const GWEN_SSLCERTDESCR *cd,
                      GWEN_SYNCIO *sio,
                      uint32_t guiid) {
  return checkCertBuiltIn(cd, sio, guiid);
}



int CppGui::logHook(const char *logDomain,
                    GWEN_LOGGER_LEVEL priority, const char *s) {
  /* not hooked */
  return 0;
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



int CppGui::print(const char *docTitle,
                  const char *docType,
                  const char *descr,
                  const char *text,
                  uint32_t guiid) {
  if (_printFn)
    return _printFn(_gui, docTitle, docType, descr, text, guiid);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}




int CppGui::checkCertBuiltIn(const GWEN_SSLCERTDESCR *cert,
                             GWEN_SYNCIO *sio,
                             uint32_t guiid) {
  if (_checkCertFn)
    return _checkCertFn(_gui, cert, sio, guiid);
  else {
    DBG_ERROR(0, "No built-in checkcert function?");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int CppGui::getPassword(uint32_t flags,
                        const char *token,
                        const char *title,
                        const char *text,
                        char *buffer,
                        int minLen,
                        int maxLen,
                        GWEN_GUI_PASSWORD_METHOD methodId,
                        GWEN_DB_NODE *methodParams,
                        uint32_t guiid) {
  if (_getPasswordFn)
    return _getPasswordFn(_gui, flags, token, title, text, buffer, minLen, maxLen, methodId, methodParams, guiid);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::setPasswordStatus(const char *token,
                              const char *pin,
                              GWEN_GUI_PASSWORD_STATUS status,
                              uint32_t guiid) {
  if (_setPasswordStatusFn)
    return _setPasswordStatusFn(_gui, token, pin, status, guiid);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}





void CppGui::setPasswordDb(GWEN_DB_NODE *dbPasswords, int persistent) {
  GWEN_Gui_SetPasswordDb(_gui, dbPasswords, persistent);
}



void CppGui::setPasswordStore(GWEN_PASSWD_STORE *sto) {
  GWEN_Gui_SetPasswdStore(_gui, sto);
}



GWEN_GUI *CppGui::getCInterface() {
  return _gui;
}



CppGui *CppGui::getCppGui() {
  GWEN_GUI *gui;
  CppGui *xgui;

  gui=GWEN_Gui_GetGui();
  if (gui==NULL)
    return NULL;
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  return xgui;
}





