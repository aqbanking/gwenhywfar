/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt5_W_LineEdit: public Qt5_W_Widget {
public:
  Qt5_W_LineEdit(GWEN_WIDGET *w):Qt5_W_Widget(w) {
  }



  ~Qt5_W_LineEdit() {
  }



  virtual int setup() {
    QLineEdit *qw;
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

    qw=new QLineEdit(text);

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);
    qw->setReadOnly(flags & GWEN_WIDGET_FLAGS_READONLY);

    if (flags & GWEN_WIDGET_FLAGS_PASSWORD)
      qw->setEchoMode(QLineEdit::Password);
    else
      qw->setEchoMode(QLineEdit::Normal);

    GWEN_Widget_SetImplData(_widget, QT5_DIALOG_WIDGET_REAL, (void*) qw);

    qtDialog=dynamic_cast<QT5_GuiDialog*>(getDialog());
    assert(qtDialog);

    qw->connect(qw, SIGNAL(returnPressed()),
                qtDialog->getMainWindow(),
                SLOT(slotActivated()));

    qw->connect(qw, SIGNAL(textChanged(const QString&)),
                qtDialog->getMainWindow(),
                SLOT(slotValueChanged()));


    if (wParent)
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    return 0;
  }



  int setCharProperty(GWEN_DIALOG_PROPERTY prop,
                      GWEN_UNUSED int index,
                      const char *value,
                      GWEN_UNUSED int doSignal) {
    QLineEdit *qw;
    QString text;

    qw=(QLineEdit*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    if (value)
      text=QString::fromUtf8(value);

    switch(prop) {
    case GWEN_DialogProperty_Value:
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
                              GWEN_UNUSED int index,
                              const char *defaultValue) {
    QLineEdit *qw;
    QString str;

    qw=(QLineEdit*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
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







