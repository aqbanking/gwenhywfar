/***************************************************************************
    begin       : Tue Feb 16 2010
    copyright   : (C) 2010 by Martin Preuss
                  (C) 2016 by Christian David
    email       : martin@libchipcard.de
                  christian-david@web.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef QT5_DIALOGBOX_H
#define QT5_DIALOGBOX_H

#include <QDialog>

class QT5_GuiDialog;

class QT5_DialogBox: public QDialog {
  Q_OBJECT
public:
  QT5_DialogBox(QT5_GuiDialog *dialog, QWidget *parent=0, bool modal=false, Qt::WindowFlags f={});
  virtual ~QT5_DialogBox();

  void accept();
  void reject();

  int cont();

  void closeEvent(QCloseEvent *e);

  void unlinkFromDialog();

public Q_SLOTS:
  void slotActivated();
  void slotValueChanged();

protected:
  QT5_GuiDialog *_dialog;
};

#endif
