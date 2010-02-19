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



#include "qt3dialogbox.hpp"
#include "qt3_gui_dialog.hpp"

#include <qapplication.h>
#include <qeventloop.h>

#include <gwenhywfar/debug.h>




QT3_DialogBox::QT3_DialogBox(QT3_GuiDialog *dialog,
			     QWidget *parent,
			     const char *name,
			     bool modal,
			     WFlags f)
:QDialog(parent, name, modal, f)
,_dialog(dialog)
, _inLoop(false)
{
}



QT3_DialogBox::~QT3_DialogBox() {
}



void QT3_DialogBox::accept() {
  QDialog::accept();
}



void QT3_DialogBox::reject() {
  QDialog::reject();
}



void QT3_DialogBox::hide() {
  if (isHidden())
    return;

  QDialog::hide();
  if (_inLoop) {
    _inLoop=false;
    DBG_ERROR(0, "Leaving loop");
    qApp->exit_loop();
  }
}



int QT3_DialogBox::cont() {
  if (_inLoop) {
    DBG_ERROR(0, "Recursion detected!!");
    return GWEN_ERROR_INTERNAL;
  }

  bool destructiveClose=testWFlags(WDestructiveClose);
  clearWFlags(WDestructiveClose);

  bool wasShowModal=testWFlags(WShowModal);
  setWFlags(WShowModal);
  setResult(0);

  _inLoop=true;
  QApplication::eventLoop()->enterLoop();

  if (!wasShowModal)
    clearWFlags(WShowModal);

  int res=result();

  if (destructiveClose)
    delete this;

  return res;
}



void QT3_DialogBox::closeEvent(QCloseEvent *e){
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



void QT3_DialogBox::unlinkFromDialog() {
  _dialog=NULL;
}




