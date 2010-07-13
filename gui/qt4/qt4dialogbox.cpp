/***************************************************************************
    begin       : Tue Feb 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "qt4dialogbox.hpp"
#include "qt4_gui_dialog.hpp"

#include <QCloseEvent>

#include <gwenhywfar/debug.h>




QT4_DialogBox::QT4_DialogBox(QT4_GuiDialog *dialog,
			     QWidget *parent,
			     bool modal,
			     Qt::WindowFlags f)
:QDialog(parent, f)
,_dialog(dialog)
{
  setModal(modal);
}



QT4_DialogBox::~QT4_DialogBox() {
}



void QT4_DialogBox::accept() {
  QDialog::accept();
}



void QT4_DialogBox::reject() {
  QDialog::reject();
}



int QT4_DialogBox::cont() {
  return exec();
}



void QT4_DialogBox::closeEvent(QCloseEvent *e){
  if (_dialog) {
    int rv;

    rv=GWEN_Dialog_EmitSignal(_dialog->getCInterface(), GWEN_DialogEvent_TypeClose, "");
    if (rv!=GWEN_DialogEvent_ResultReject) {
      DBG_ERROR(0, "Accepting to close dialog (%d)", rv);
      e->accept();
    }
    else {
      DBG_ERROR(0, "Rejecting to close dialog");
    }
  }
  else {
    DBG_ERROR(0, "Dialog gone, accepting to close dialog");
    e->accept();
  }
}



void QT4_DialogBox::unlinkFromDialog() {
  _dialog=NULL;
}



void QT4_DialogBox::slotActivated() {
  const QObject *snd;

  snd=sender();
  if (snd) {
    GWEN_WIDGET *w;
    const char *wname;
    int rv=GWEN_DialogEvent_ResultNotHandled;

    w=GWEN_Dialog_FindWidgetByImplData(_dialog->getCInterface(), QT4_DIALOG_WIDGET_REAL, snd);
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



void QT4_DialogBox::slotValueChanged() {
  const QObject *snd;

  snd=sender();
  if (snd) {
    GWEN_WIDGET *w;
    const char *wname;
    int rv=GWEN_DialogEvent_ResultNotHandled;

    w=GWEN_Dialog_FindWidgetByImplData(_dialog->getCInterface(), QT4_DIALOG_WIDGET_REAL, snd);
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



#include "qt4dialogbox.moc"



