/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt5_W_ScrollArea: public Qt5_W_Widget {
public:
  Qt5_W_ScrollArea(GWEN_WIDGET *w):Qt5_W_Widget(w) {
  }



  ~Qt5_W_ScrollArea() {
  }



  virtual int setup() {
    QScrollArea *qw;
    QWidget *qChild;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;
    QLayout *qLayout;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);

    qw=new QScrollArea();

    qChild=new QWidget();
    qChild->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qw->setWidget(qChild);
    qLayout=new QVBoxLayout(qChild);

    qw->setWidgetResizable(true);

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT5_DIALOG_WIDGET_REAL, (void*) qw);
    GWEN_Widget_SetImplData(_widget, QT5_DIALOG_WIDGET_LAYOUT, (void*) qLayout);

    if (wParent)
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    return 0;
  }


  int addChildGuiWidget(GWEN_WIDGET *wChild) {
    QWidget *qw;
    QBoxLayout *qLayout;
    QWidget *qChild;

    qw=(QWidget*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    qLayout=(QBoxLayout*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_LAYOUT);
    assert(qLayout);

    qChild=getQWidget(wChild);
    assert(qChild);

    qChild->setParent(qw);
    qLayout->addWidget(qChild);

    return 0;
  };

};







