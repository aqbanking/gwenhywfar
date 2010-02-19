/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QT3_DIALOG_HPP
#define QT3_DIALOG_HPP

#include "cppdialog.hpp"
#include "qt3_gui.hpp"

#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/widget_be.h>

#include <qwidget.h>


#include <list>
#include <string>


class QT3_DialogBox;



class QT3_GuiDialog: public QObject, public CppDialog {
  Q_OBJECT
public:

  QT3_GuiDialog(QT3_Gui *gui, GWEN_DIALOG *dlg);
  virtual ~QT3_GuiDialog();

  static QT3_GuiDialog *getDialog(GWEN_DIALOG *dlg);

  bool setup(QWidget *parentWindow);

  int execute();

  int openDialog();
  int closeDialog();
  int runDialog(bool untilEnd);

  QT3_DialogBox *getMainWindow() { return _mainWidget;};

public slots:
  void slotActivated();
  void slotValueChanged();

protected:
  QT3_Gui *_gui;
  int _widgetCount;
  QT3_DialogBox *_mainWidget;

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


