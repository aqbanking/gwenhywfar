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




