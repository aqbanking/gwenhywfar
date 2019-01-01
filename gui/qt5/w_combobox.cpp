/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt5_W_ComboBox: public Qt5_W_Widget {
public:
  Qt5_W_ComboBox(GWEN_WIDGET *w):Qt5_W_Widget(w) {
  }



  ~Qt5_W_ComboBox() {
  }



  virtual int setup() {
    QComboBox *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;
    QT5_GuiDialog *qtDialog;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);

    qw=new QComboBox();

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);
    qw->setEditable((flags & GWEN_WIDGET_FLAGS_READONLY)?false:true);

    GWEN_Widget_SetImplData(_widget, QT5_DIALOG_WIDGET_REAL, (void*) qw);

    qtDialog=dynamic_cast<QT5_GuiDialog*>(getDialog());
    assert(qtDialog);

    qw->connect(qw, SIGNAL(activated(int)),
                qtDialog->getMainWindow(),
                SLOT(slotActivated()));


    if (wParent)
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    return 0;
  }



  int setIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int value,
                     int doSignal) {
    QComboBox *qw;

    qw=(QComboBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      qw->setCurrentIndex(value);
      return 0;

    case GWEN_DialogProperty_ClearValues:
      qw->clear();
      return 0;

    default:
      return Qt5_W_Widget::setIntProperty(prop, index, value, doSignal);
    }
  };



  int getIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int defaultValue) {
    QComboBox *qw;

    qw=(QComboBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      return qw->currentIndex();

    case GWEN_DialogProperty_ValueCount:
      return qw->count();

    default:
      return Qt5_W_Widget::getIntProperty(prop, index, defaultValue);
    }
  };



  int setCharProperty(GWEN_DIALOG_PROPERTY prop,
                      GWEN_UNUSED int index,
                      const char *value,
                      GWEN_UNUSED int doSignal) {
    QComboBox *qw;
    QString text;

    qw=(QComboBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    if (value)
      text=QString::fromUtf8(value);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      // undefined
      break;

    case GWEN_DialogProperty_AddValue:
      qw->addItem(text);
      return 0;

    case GWEN_DialogProperty_ClearValues:
      qw->clear();
      return 0;
    default:
      break;
    }

    DBG_WARN(GWEN_LOGDOMAIN,
             "Function is not appropriate for this type of widget (%s)",
             GWEN_Widget_Type_toString(GWEN_Widget_GetType(_widget)));
    return GWEN_ERROR_INVALID;
  };



  const char *getCharProperty(GWEN_DIALOG_PROPERTY prop,
                              int index,
                              const char *defaultValue) {
    QComboBox *qw;
    QString str;

    qw=(QComboBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      str=qw->itemText(index);
      if (str.isEmpty())
        return defaultValue;
      else {
        GWEN_Widget_SetText(_widget, QT5_DIALOG_STRING_TITLE, str.toUtf8());
        return GWEN_Widget_GetText(_widget, QT5_DIALOG_STRING_TITLE);
      }
      break;

    default:
      break;
    }

    DBG_WARN(GWEN_LOGDOMAIN,
             "Function is not appropriate for this type of widget (%s)",
             GWEN_Widget_Type_toString(GWEN_Widget_GetType(_widget)));
    return defaultValue;
  };

};







