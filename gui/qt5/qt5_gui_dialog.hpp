/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
                  (C) 2016 by Christian David
    email       : martin@libchipcard.de
                  christian-david@web.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QT5_DIALOG_HPP
#define QT5_DIALOG_HPP

#include <list>
#include <string>

#include <QObject>

#include <gwen-gui-cpp/cppdialog.hpp>
#include "qt5_gui.hpp"

#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/widget_be.h>

class QT5_DialogBox;

#define QT5_DIALOG_WIDGET_REAL 0
#define QT5_DIALOG_WIDGET_CONTENT 1
#define QT5_DIALOG_WIDGET_LAYOUT 2


class QT5_GuiDialog: public CppDialog {
public:
  QT5_GuiDialog(QT5_Gui *gui, GWEN_DIALOG *dlg);
  virtual ~QT5_GuiDialog();

  static QT5_GuiDialog *getDialog(GWEN_DIALOG *dlg);

  bool setup(QWidget *parentWindow);

  int execute();

  int openDialog();
  int closeDialog();
  int runDialog(bool untilEnd);

  QT5_DialogBox *getMainWindow() { return _mainWidget;};

protected:
  QT5_Gui *_gui;
  int _widgetCount;
  QT5_DialogBox *_mainWidget;

  int setupTree(GWEN_WIDGET *w);

  virtual int setIntProperty(GWEN_WIDGET *w,
                             GWEN_DIALOG_PROPERTY prop,
                             int index,
                             int value,
                             int doSignal);

  virtual int getIntProperty(GWEN_WIDGET *w,
                             GWEN_DIALOG_PROPERTY prop,
                             int index,
                             int defaultValue);

  virtual int setCharProperty(GWEN_WIDGET *w,
                              GWEN_DIALOG_PROPERTY prop,
                              int index,
                              const char *value,
                              int doSignal);

  virtual const char *getCharProperty(GWEN_WIDGET *w,
                                      GWEN_DIALOG_PROPERTY prop,
                                      int index,
                                      const char *defaultValue);

};

#endif
