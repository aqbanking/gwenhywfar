/***************************************************************************
    begin       : Tue Feb 16 2010
    copyright   : (C) 2010 by Martin Preuss
                  (C) 2016 by Christian David
    email       : martin@libchipcard.de
                  christian-david@web.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "qt5dialogbox.hpp"
#include "qt5_gui_dialog.hpp"

#include <QCloseEvent>

#include <gwenhywfar/debug.h>


QT5_DialogBox::QT5_DialogBox(QT5_GuiDialog *dialog,
                             QWidget *parent,
                             bool modal,
                             Qt::WindowFlags f)
  :QDialog(parent, f)
  ,_dialog(dialog) {
  setModal(modal);
}



QT5_DialogBox::~QT5_DialogBox() {
}



void QT5_DialogBox::accept() {
  QDialog::accept();
}



void QT5_DialogBox::reject() {
  QDialog::reject();
}



int QT5_DialogBox::cont() {
  return exec();
}



void QT5_DialogBox::closeEvent(QCloseEvent *e) {
  if (_dialog) {
    int rv;

    rv=GWEN_Dialog_EmitSignal(_dialog->getCInterface(), GWEN_DialogEvent_TypeClose, "");
    if (rv!=GWEN_DialogEvent_ResultReject) {
      e->accept();
    }
  }
  else {
    e->accept();
  }
}



void QT5_DialogBox::unlinkFromDialog() {
  _dialog=NULL;
}



void QT5_DialogBox::slotActivated() {
  const QObject *snd;

  snd=sender();
  if (snd) {
    GWEN_WIDGET *w;
    const char *wname;
    int rv=GWEN_DialogEvent_ResultNotHandled;

    w=GWEN_Dialog_FindWidgetByImplData(_dialog->getCInterface(), QT5_DIALOG_WIDGET_REAL, snd);
    if (w==NULL) {
      DBG_INFO(0, "Widget not found");
      return;
    }
    wname=GWEN_Widget_GetName(w);

    DBG_INFO(GWEN_LOGDOMAIN, "Command for [%s] (type: %s)",
             wname?wname:"(unnamed)",
             GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));

    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeUnknown:
    case GWEN_Widget_TypeNone:
      break;

    case GWEN_Widget_TypePushButton:
    case GWEN_Widget_TypeLineEdit:
    case GWEN_Widget_TypeComboBox:
    case GWEN_Widget_TypeListBox:
    case GWEN_Widget_TypeCheckBox:
    case GWEN_Widget_TypeLabel:
    case GWEN_Widget_TypeTextEdit:
    case GWEN_Widget_TypeSpinBox:
      rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
                                GWEN_DialogEvent_TypeActivated,
                                GWEN_Widget_GetName(w));
      break;
    case GWEN_Widget_TypeRadioButton:
    case GWEN_Widget_TypeProgressBar:
    case GWEN_Widget_TypeGroupBox:
    case GWEN_Widget_TypeHSpacer:
    case GWEN_Widget_TypeVSpacer:
    case GWEN_Widget_TypeHLayout:
    case GWEN_Widget_TypeVLayout:
    case GWEN_Widget_TypeGridLayout:
    case GWEN_Widget_TypeDialog:
    case GWEN_Widget_TypeTabBook:
    case GWEN_Widget_TypeTabPage:
    case GWEN_Widget_TypeScrollArea:
    case GWEN_Widget_TypeWidgetStack:
    case GWEN_Widget_TypeHLine:
    case GWEN_Widget_TypeVLine:
    case GWEN_Widget_TypeTextBrowser:
      /* nothing to do for these types */
      ;
    }

    if (rv==GWEN_DialogEvent_ResultAccept) {
      accept();
    }
    else if (rv==GWEN_DialogEvent_ResultReject) {
      reject();
    }
  }
}



void QT5_DialogBox::slotValueChanged() {
  const QObject *snd;

  snd=sender();
  if (snd) {
    GWEN_WIDGET *w;
    const char *wname;
    int rv=GWEN_DialogEvent_ResultNotHandled;

    w=GWEN_Dialog_FindWidgetByImplData(_dialog->getCInterface(), QT5_DIALOG_WIDGET_REAL, snd);
    if (w==NULL) {
      DBG_INFO(0, "Widget not found");
      return;
    }
    wname=GWEN_Widget_GetName(w);

    DBG_INFO(GWEN_LOGDOMAIN, "ValueChanged for [%s] (type: %s)",
             wname?wname:"(unnamed)",
             GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));

    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeUnknown:
    case GWEN_Widget_TypeNone:
      break;

    case GWEN_Widget_TypePushButton:
    case GWEN_Widget_TypeLineEdit:
    case GWEN_Widget_TypeComboBox:
    case GWEN_Widget_TypeListBox:
    case GWEN_Widget_TypeCheckBox:
    case GWEN_Widget_TypeLabel:
    case GWEN_Widget_TypeTextEdit:
    case GWEN_Widget_TypeSpinBox:
      rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
                                GWEN_DialogEvent_TypeValueChanged,
                                GWEN_Widget_GetName(w));
      break;

    case GWEN_Widget_TypeRadioButton:
    case GWEN_Widget_TypeProgressBar:
    case GWEN_Widget_TypeGroupBox:
    case GWEN_Widget_TypeHSpacer:
    case GWEN_Widget_TypeVSpacer:
    case GWEN_Widget_TypeHLayout:
    case GWEN_Widget_TypeVLayout:
    case GWEN_Widget_TypeGridLayout:
    case GWEN_Widget_TypeDialog:
    case GWEN_Widget_TypeTabBook:
    case GWEN_Widget_TypeTabPage:
    case GWEN_Widget_TypeScrollArea:
    case GWEN_Widget_TypeWidgetStack:
    case GWEN_Widget_TypeHLine:
    case GWEN_Widget_TypeVLine:
    case GWEN_Widget_TypeTextBrowser:
      /* nothing to do for these types */
      ;
    }

    if (rv==GWEN_DialogEvent_ResultAccept) {
      accept();
    }
    else if (rv==GWEN_DialogEvent_ResultReject) {
      reject();
    }
  }
}

#include "qt5dialogbox.moc"
