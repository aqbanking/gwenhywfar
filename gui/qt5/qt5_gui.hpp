/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
                  (C) 2016 by Christian David
    email       : martin@libchipcard.de
                  christian-david@web.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef QT5_GUI_H
#define QT5_GUI_H


#if (defined __GNUC__ && (! defined (__sun)) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))) || defined _MSC_VER
# ifdef BUILDING_QT5_GUI
#   define QT5GUI_API __attribute__ ((visibility("default")))
# else
#   define QT5GUI_API
# endif
#else
# define QT5GUI_API
#endif


class QT5_Gui;
class QWidget;


#include <gwen-gui-cpp/cppgui.hpp>

#include <QString>

/**
 * This is an implementation of GWEN_GUI for QT5.
 *
 * It implements the GWEN_DIALOG framework.
 *
 * @todo: @ref GWEN_Gui_Print() needs to be implemented
 */
class QT5GUI_API QT5_Gui: public CppGui {

private:
  QWidget *_parentWidget;
  std::list<QWidget*> _pushedParents;

public:
  QT5_Gui();
  virtual ~QT5_Gui();

  QWidget *getParentWidget() const { return _parentWidget;};

  void pushParentWidget(QWidget *w);
  void popParentWidget();

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
