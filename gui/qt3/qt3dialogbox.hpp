/***************************************************************************
    begin       : Tue Feb 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef QT3_DIALOGBOX_H
#define QT3_DIALOGBOX_H

#include <qdialog.h>


class QT3_GuiDialog;


class QT3_DialogBox: public QDialog {
public:
  QT3_DialogBox(QT3_GuiDialog *dialog, QWidget *parent=0, const char *name=0, bool modal=FALSE, WFlags f=0);
  virtual ~QT3_DialogBox();

  void accept();
  void reject();

  int cont();

  void closeEvent(QCloseEvent *e);
  void hide();

  void unlinkFromDialog();

protected:
  QT3_GuiDialog *_dialog;
  bool _inLoop;
};



#endif
