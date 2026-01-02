/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


class Qt5_W_GridLayout: public Qt5_W_Widget {
public:
  Qt5_W_GridLayout(GWEN_WIDGET *w):Qt5_W_Widget(w) {
  }



  ~Qt5_W_GridLayout() {
  }



  virtual int setup() {
    QWidget *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QLayout *qLayout;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);

    qw=new QWidget();
    qLayout=new QGridLayout(qw);

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
    QGridLayout *qLayout;
    QWidget *qChild;
    GWEN_WIDGET *wt;
    int idx=0;

    qw=(QWidget*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    qLayout=(QGridLayout*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_LAYOUT);
    assert(qLayout);

    qChild=getQWidget(wChild);
    assert(qChild);

    qChild->setParent(qw);

    /* get position of the child widget below myself */
    wt=GWEN_Widget_Tree_GetFirstChild(_widget);
    while(wt && wt!=wChild) {
      idx++;
      wt=GWEN_Widget_Tree_GetNext(wt);
    }

    if (wt) {
      int x=0, y=0;
      int c;

      if ((c=GWEN_Widget_GetColumns(_widget))) {
        x=idx % c;
        y=idx / c;
      }
      else if ((c=GWEN_Widget_GetRows(_widget))) {
        x=idx / c;
        y=idx % c ;
      }

      qLayout->addWidget(qChild, y, x);
    }

    return 0;
  }

};







