/***************************************************************************
    begin       : Tue Feb 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef QT4_DIALOGBOX_H
#define QT4_DIALOGBOX_H

#include <qdialog.h>


class QT4_GuiDialog;


class QT4_DialogBox: public QDialog {
public:
  QT4_DialogBox(QT4_GuiDialog *dialog, QWidget *parent=0, bool modal=FALSE, Qt::WindowFlags f=0);
  virtual ~QT4_DialogBox();

  void accept();
  void reject();

  int cont();

  void closeEvent(QCloseEvent *e);

  void unlinkFromDialog();

protected:
  QT4_GuiDialog *_dialog;
};



#endif
