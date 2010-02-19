/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QT4_DIALOG_HPP
#define QT4_DIALOG_HPP

#include "cppdialog.hpp"
#include "qt4_gui.hpp"

#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/widget_be.h>

#include <QObject>


#include <list>
#include <string>


class QT4_DialogBox;



class QT4_GuiDialog: public QObject, public CppDialog {
  Q_OBJECT
public:

  QT4_GuiDialog(QT4_Gui *gui, GWEN_DIALOG *dlg);
  virtual ~QT4_GuiDialog();

  static QT4_GuiDialog *getDialog(GWEN_DIALOG *dlg);

  bool setup(QWidget *parentWindow);

  int execute();

  int openDialog();
  int closeDialog();
  int runDialog(bool untilEnd);

  QT4_DialogBox *getMainWindow() { return _mainWidget;};

public slots:
  void slotActivated();
  void slotValueChanged();

protected:
  QT4_Gui *_gui;
  int _widgetCount;
  QT4_DialogBox *_mainWidget;

  int setupTree(QWidget *dialogParent, GWEN_WIDGET *w);
  int setupWidget(QWidget *dialogParent, GWEN_WIDGET *w);

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




#endif /* FG_DIALOG_L_HPP */


