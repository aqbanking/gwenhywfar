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

#include "cppwidget_p.hpp"
#include "cppdialog.hpp"

#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/widget_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)



GWEN_INHERIT(GWEN_WIDGET, CppWidget);




int CppWidgetLinker::SetIntProperty(GWEN_WIDGET *w,
                                    GWEN_DIALOG_PROPERTY prop,
                                    int index,
                                    int value,
                                    int doSignal) {
  CppWidget *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, CppWidget, w);
  assert(xw);

  return xw->setIntProperty(prop, index, value, doSignal);
}




int CppWidgetLinker::GetIntProperty(GWEN_WIDGET *w,
                                    GWEN_DIALOG_PROPERTY prop,
                                    int index,
                                    int defaultValue) {
  CppWidget *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, CppWidget, w);
  assert(xw);

  return xw->getIntProperty(prop, index, defaultValue);
}



int CppWidgetLinker::SetCharProperty(GWEN_WIDGET *w,
                                     GWEN_DIALOG_PROPERTY prop,
                                     int index,
                                     const char *value,
                                     int doSignal) {
  CppWidget *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, CppWidget, w);
  assert(xw);

  return xw->setCharProperty(prop, index, value, doSignal);
}



const char *CppWidgetLinker::GetCharProperty(GWEN_WIDGET *w,
    GWEN_DIALOG_PROPERTY prop,
    int index,
    const char *defaultValue) {
  CppWidget *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, CppWidget, w);
  assert(xw);

  return xw->getCharProperty(prop, index, defaultValue);
}



int CppWidgetLinker::AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
  CppWidget *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, CppWidget, w);
  assert(xw);

  return xw->addChildGuiWidget(wChild);
}



void CppWidgetLinker::freeData(void *bp, void *p) {
  CppWidget *xw;

  xw=(CppWidget*) p;
  if (xw->_widget)
    xw->_widget=NULL;
  delete xw;
}





CppWidget::CppWidget()
  :_widget(NULL) {
}



CppWidget::CppWidget(GWEN_WIDGET *w)
  :_widget(w) {
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, CppWidget,
                       _widget, this,
                       CppWidgetLinker::freeData);

  _setIntPropertyFn=GWEN_Widget_SetSetIntPropertyFn(_widget, CppWidgetLinker::SetIntProperty);
  _getIntPropertyFn=GWEN_Widget_SetGetIntPropertyFn(_widget, CppWidgetLinker::GetIntProperty);
  _setCharPropertyFn=GWEN_Widget_SetSetCharPropertyFn(_widget, CppWidgetLinker::SetCharProperty);
  _getCharPropertyFn=GWEN_Widget_SetGetCharPropertyFn(_widget, CppWidgetLinker::GetCharProperty);
  _addChildGuiWidgetFn=GWEN_Widget_SetAddChildGuiWidgetFn(_widget, CppWidgetLinker::AddChildGuiWidget);
}



CppWidget::~CppWidget() {
  if (_widget) {
    GWEN_INHERIT_UNLINK(GWEN_WIDGET, CppWidget, _widget)
  }
}



GWEN_WIDGET *CppWidget::getCInterface() {
  return _widget;
}



CppWidget *CppWidget::getWidget(GWEN_WIDGET *w) {
  CppWidget *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, CppWidget, w);
  assert(xw);

  return xw;
}



CppDialog *CppWidget::getDialog() {
  GWEN_DIALOG *dlg;

  dlg=GWEN_Widget_GetDialog(_widget);
  if (dlg)
    return CppDialog::getDialog(dlg);
  else
    return NULL;
}



int CppWidget::setIntProperty(GWEN_DIALOG_PROPERTY prop,
                              int index,
                              int value,
                              int doSignal) {
  if (_setIntPropertyFn)
    return _setIntPropertyFn(_widget, prop, index, value, doSignal);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



int CppWidget::getIntProperty(GWEN_DIALOG_PROPERTY prop,
                              int index,
                              int defaultValue) {
  if (_getIntPropertyFn)
    return _getIntPropertyFn(_widget, prop, index, defaultValue);
  else
    return defaultValue;
}



int CppWidget::setCharProperty(GWEN_DIALOG_PROPERTY prop,
                               int index,
                               const char *value,
                               int doSignal) {
  if (_setCharPropertyFn)
    return _setCharPropertyFn(_widget, prop, index, value, doSignal);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



const char *CppWidget::getCharProperty(GWEN_DIALOG_PROPERTY prop,
                                       int index,
                                       const char *defaultValue) {
  if (_getCharPropertyFn)
    return _getCharPropertyFn(_widget, prop, index, defaultValue);
  else
    return defaultValue;
}



int CppWidget::addChildGuiWidget(GWEN_WIDGET *wChild) {
  if (_addChildGuiWidgetFn)
    return _addChildGuiWidgetFn(_widget, wChild);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



const char *CppWidget::getName() {
  return GWEN_Widget_GetName(_widget);
}



GWEN_WIDGET_TYPE CppWidget::getType() {
  return GWEN_Widget_GetType(_widget);
}



int CppWidget::getColumns() {
  return GWEN_Widget_GetColumns(_widget);
}



int CppWidget::getRows() {
  return GWEN_Widget_GetRows(_widget);
}



uint32_t CppWidget::getFlags() {
  return GWEN_Widget_GetFlags(_widget);
}



int CppWidget::getGroupId() {
  return GWEN_Widget_GetGroupId(_widget);
}



int CppWidget::getWidth() {
  return GWEN_Widget_GetWidth(_widget);
}



int CppWidget::getHeight() {
  return GWEN_Widget_GetHeight(_widget);
}



const char *CppWidget::getText(int idx) {
  return GWEN_Widget_GetText(_widget, idx);
}



const char *CppWidget::getIconFileName() {
  return GWEN_Widget_GetIconFileName(_widget);
}



const char *CppWidget::getImageFileName() {
  return GWEN_Widget_GetImageFileName(_widget);
}








