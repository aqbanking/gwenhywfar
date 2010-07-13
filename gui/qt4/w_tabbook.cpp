/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt4_W_TabBook: public Qt4_W_Widget {
public:
  Qt4_W_TabBook(GWEN_WIDGET *w):Qt4_W_Widget(w) {
  }



  ~Qt4_W_TabBook() {
  }



  virtual int setup() {
    QWidget *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);

    qw=new QTabWidget();

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT4_DIALOG_WIDGET_REAL, (void*) qw);
    GWEN_Widget_SetImplData(_widget, QT4_DIALOG_WIDGET_CONTENT, (void*) qw);

    if (wParent)
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    return 0;
  }



  int addChildGuiWidget(GWEN_WIDGET *wChild) {
    QTabWidget *qw;
    QWidget *qChild;
    const char *s;
    QString text;

    qw=(QTabWidget*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    if (GWEN_Widget_GetType(wChild)!=GWEN_Widget_TypeTabPage) {
      DBG_ERROR(GWEN_LOGDOMAIN, "You can only add TabPages to a TabBook");
      return GWEN_ERROR_INVALID;
    }

    qChild=getQWidget(wChild);
    assert(qChild);

    s=GWEN_Widget_GetText(wChild, 0);
    if (s)
      text=QString::fromUtf8(s);

    qw->addTab(qChild, text);

    return 0;
  }

};







