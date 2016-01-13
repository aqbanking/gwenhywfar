/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt5_W_CheckBox: public Qt5_W_Widget {
public:
  Qt5_W_CheckBox(GWEN_WIDGET *w):Qt5_W_Widget(w) {
  }



  ~Qt5_W_CheckBox() {
  }



  virtual int setup() {
    QWidget *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;
    const char *s;
    QString text;
    QT5_GuiDialog *qtDialog;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);
    s=GWEN_Widget_GetText(_widget, 0);
    if (s)
      text=QString::fromUtf8(s);

    qw=new QCheckBox(text);

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT5_DIALOG_WIDGET_REAL, (void*) qw);

    qtDialog=dynamic_cast<QT5_GuiDialog*>(getDialog());
    assert(qtDialog);

    qw->connect(qw, SIGNAL(toggled(bool)),
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
    QCheckBox *qw;

    qw=(QCheckBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      qw->setChecked((value==0)?false:true);
      return 0;

    default:
      return Qt5_W_Widget::setIntProperty(prop, index, value, doSignal);
    }
  };



  int getIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int defaultValue) {
    QCheckBox *qw;

    qw=(QCheckBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      return (qw->isChecked())?1:0;

    default:
      return Qt5_W_Widget::getIntProperty(prop, index, defaultValue);
    }
  };



  int setCharProperty(GWEN_DIALOG_PROPERTY prop,
                      int index,
                      const char *value,
                      int doSignal) {
    QCheckBox *qw;
    QString text;

    qw=(QCheckBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    if (value)
      text=QString::fromUtf8(value);

    switch(prop) {
    case GWEN_DialogProperty_Title:
      qw->setText(text);
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
    QCheckBox *qw;
    QString str;

    qw=(QCheckBox*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Title:
      str=qw->text();
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







