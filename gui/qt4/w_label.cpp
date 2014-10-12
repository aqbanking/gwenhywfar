/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt4_W_Label: public Qt4_W_Widget {
public:
  Qt4_W_Label(GWEN_WIDGET *w):Qt4_W_Widget(w) {
  }



  ~Qt4_W_Label() {
  }



  virtual int setup() {
    QLabel *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;
    const char *s;
    QString text;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);
    s=GWEN_Widget_GetText(_widget, 0);
    if (s)
      text=QT4_Gui::extractHtml(s);

    qw=new QLabel(text);
    qw->setWordWrap(true);

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT4_DIALOG_WIDGET_REAL, (void*) qw);

    if (wParent)
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    return 0;
  }



  int setCharProperty(GWEN_DIALOG_PROPERTY prop,
                      int index,
                      const char *value,
                      int doSignal) {
    QLabel *qw;
    QString text;

    qw=(QLabel*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    if (value)
      text=QT4_Gui::extractHtml(value);

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
    QLabel *qw;
    QString str;

    qw=(QLabel*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Title:
      str=qw->text();
      if (str.isEmpty())
        return defaultValue;
      else {
        GWEN_Widget_SetText(_widget, QT4_DIALOG_STRING_TITLE, str.toUtf8());
        return GWEN_Widget_GetText(_widget, QT4_DIALOG_STRING_TITLE);
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







