/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt4_W_SpinBox: public Qt4_W_Widget {
public:
  Qt4_W_SpinBox(GWEN_WIDGET *w):Qt4_W_Widget(w) {
  }



  ~Qt4_W_SpinBox() {
  }



  virtual int setup() {
    QSpinBox *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;
    QT4_GuiDialog *qtDialog;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);

    qw=new QSpinBox();

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT4_DIALOG_WIDGET_REAL, (void*) qw);

    qtDialog=dynamic_cast<QT4_GuiDialog*>(getDialog());
    assert(qtDialog);

    qw->connect(qw, SIGNAL(valueChanged(int)),
                qtDialog->getMainWindow(),
                SLOT(slotValueChanged()));


    if (wParent)
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    return 0;
  }



  int setIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int value,
                     int doSignal) {
    QSpinBox *qw;

    qw=(QSpinBox*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      qw->setValue(value);
      return 0;

    case GWEN_DialogProperty_MinValue:
      qw->setMinimum(value);
      return 0;

    case GWEN_DialogProperty_MaxValue:
      qw->setMaximum(value);
      return 0;

    default:
      return Qt4_W_Widget::setIntProperty(prop, index, value, doSignal);
    }
  };



  int getIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int defaultValue) {
    QSpinBox *qw;

    qw=(QSpinBox*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      return qw->value();

    case GWEN_DialogProperty_MinValue:
      return qw->minimum();

    case GWEN_DialogProperty_MaxValue:
      return qw->maximum();

    default:
      return Qt4_W_Widget::getIntProperty(prop, index, defaultValue);
    }
  };


};







