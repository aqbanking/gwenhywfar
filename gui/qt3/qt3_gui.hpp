/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef QT3_GUI_H
#define QT3_GUI_H

class QT3_Gui;
class QT3_GuiSimpleBox;
class QWidget;


#include <gwen-gui-qt3/cppgui.hpp>

#include <qstring.h>
#include <string>


class QT3GUI_API QT3_Gui: public CppGui {

private:
  QWidget *_parentWidget;
  std::list<QWidget*> _pushedParents;

public:
  QT3_Gui();
  virtual ~QT3_Gui();


  QWidget *getParentWidget() const { return _parentWidget;};

  void pushParentWidget(QWidget *w);
  void popParentWidget();

  std::string qstringToUtf8String(const QString &qs);
  static QString extractHtml(const char *text);

protected:
  /** @name User Interaction
   *
   */
  /*@{*/

  virtual int execDialog(GWEN_DIALOG *dlg, uint32_t guiid);

  virtual int openDialog(GWEN_DIALOG *dlg, uint32_t guiid);
  virtual int closeDialog(GWEN_DIALOG *dlg);
  virtual int runDialog(GWEN_DIALOG *dlg, int untilEnd);

  virtual int getFileName(const char *caption,
			  GWEN_GUI_FILENAME_TYPE fnt,
			  uint32_t flags,
			  const char *patterns,
			  GWEN_BUFFER *pathBuffer,
			  uint32_t guiid);

};



#endif
