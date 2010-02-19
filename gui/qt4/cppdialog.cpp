/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cppdialog_p.hpp"
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>


#define I18N(msg) GWEN_I18N_Translate("aqfinance", msg)



GWEN_INHERIT(GWEN_DIALOG, CppDialog);




int CppDialogLinker::SetIntProperty(GWEN_DIALOG *dlg,
				    GWEN_WIDGET *w,
				    GWEN_DIALOG_PROPERTY prop,
				    int index,
				    int value,
				    int doSignal) {
  CppDialog *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, CppDialog, dlg);
  assert(xdlg);

  return xdlg->setIntProperty(w, prop, index, value, doSignal);
}




int CppDialogLinker::GetIntProperty(GWEN_DIALOG *dlg,
				    GWEN_WIDGET *w,
				    GWEN_DIALOG_PROPERTY prop,
				    int index,
				    int defaultValue) {
  CppDialog *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, CppDialog, dlg);
  assert(xdlg);

  return xdlg->getIntProperty(w, prop, index, defaultValue);
}



int CppDialogLinker::SetCharProperty(GWEN_DIALOG *dlg,
				     GWEN_WIDGET *w,
				     GWEN_DIALOG_PROPERTY prop,
				     int index,
				     const char *value,
				     int doSignal) {
  CppDialog *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, CppDialog, dlg);
  assert(xdlg);

  return xdlg->setCharProperty(w, prop, index, value, doSignal);
}



const char *CppDialogLinker::GetCharProperty(GWEN_DIALOG *dlg,
					     GWEN_WIDGET *w,
					     GWEN_DIALOG_PROPERTY prop,
					     int index,
					     const char *defaultValue) {
  CppDialog *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, CppDialog, dlg);
  assert(xdlg);

  return xdlg->getCharProperty(w, prop, index, defaultValue);
}



void CppDialogLinker::freeData(void *bp, void *p) {
  CppDialog *xdlg;

  xdlg=(CppDialog*) p;
  if (xdlg->_dialog)
    xdlg->_dialog=NULL;
  delete xdlg;
}





CppDialog::CppDialog()
:_dialog(NULL)
{
}



CppDialog::CppDialog(GWEN_DIALOG *dlg)
:_dialog(NULL)
{
  _dialog=dlg;

  GWEN_INHERIT_SETDATA(GWEN_DIALOG, CppDialog,
		       _dialog, this,
		       CppDialogLinker::freeData);

  _setIntPropertyFn=GWEN_Dialog_SetSetIntPropertyFn(_dialog, CppDialogLinker::SetIntProperty);
  _getIntPropertyFn=GWEN_Dialog_SetGetIntPropertyFn(_dialog, CppDialogLinker::GetIntProperty);
  _setCharPropertyFn=GWEN_Dialog_SetSetCharPropertyFn(_dialog, CppDialogLinker::SetCharProperty);
  _getCharPropertyFn=GWEN_Dialog_SetGetCharPropertyFn(_dialog, CppDialogLinker::GetCharProperty);
}



CppDialog::~CppDialog() {
  if (_dialog) {
    GWEN_INHERIT_UNLINK(GWEN_DIALOG, CppDialog, _dialog)
  }
}



CppDialog *CppDialog::getDialog(GWEN_DIALOG *dlg) {
  CppDialog *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, CppDialog, dlg);
  assert(xdlg);

  return xdlg;
}



GWEN_DIALOG *CppDialog::getCInterface() {
  return _dialog;
}



int CppDialog::setIntProperty(GWEN_WIDGET *w,
			      GWEN_DIALOG_PROPERTY prop,
			      int index,
			      int value,
			      int doSignal) {
  if (_setIntPropertyFn)
    return _setIntPropertyFn(_dialog, w, prop, index, value, doSignal);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



int CppDialog::getIntProperty(GWEN_WIDGET *w,
			      GWEN_DIALOG_PROPERTY prop,
			      int index,
			      int defaultValue) {
  if (_getIntPropertyFn)
    return _getIntPropertyFn(_dialog, w, prop, index, defaultValue);
  else
    return defaultValue;
}



int CppDialog::setCharProperty(GWEN_WIDGET *w,
			       GWEN_DIALOG_PROPERTY prop,
			       int index,
			       const char *value,
			       int doSignal) {
  if (_setCharPropertyFn)
    return _setCharPropertyFn(_dialog, w, prop, index, value, doSignal);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



const char *CppDialog::getCharProperty(GWEN_WIDGET *w,
				       GWEN_DIALOG_PROPERTY prop,
				       int index,
				       const char *defaultValue) {
  if (_getCharPropertyFn)
    return _getCharPropertyFn(_dialog, w, prop, index, defaultValue);
  else
    return defaultValue;
}



int CppDialog::emitSignal(GWEN_DIALOG_EVENTTYPE t,
			  const char *sender,
			  int intVal,
			  const char *charVal) {
  assert(_dialog);
  return GWEN_Dialog_EmitSignal(_dialog, t, sender);
}



GWEN_WIDGET_TREE *CppDialog::getWidgets() const {
  assert(_dialog);
  return GWEN_Dialog_GetWidgets(_dialog);
}



GWEN_WIDGET *CppDialog::findWidgetByName(const char *name) {
  assert(_dialog);
  return GWEN_Dialog_FindWidgetByName(_dialog, name);
}



GWEN_WIDGET *CppDialog::findWidgetByImplData(int index, void *ptr) {
  assert(_dialog);
  return GWEN_Dialog_FindWidgetByImplData(_dialog, index, ptr);
}












