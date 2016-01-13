/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt5_W_TextBrowser: public Qt5_W_Widget {
public:
  Qt5_W_TextBrowser(GWEN_WIDGET *w):Qt5_W_Widget(w) {
  }



  ~Qt5_W_TextBrowser() {
  }



  virtual int setup() {
    QTextBrowser *qw;
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
      text=QString::fromUtf8(s);

    qw=new QTextBrowser();
    qw->setText(text);

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT5_DIALOG_WIDGET_REAL, (void*) qw);

    if (wParent)
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    return 0;
  }



  int setCharProperty(GWEN_DIALOG_PROPERTY prop,
                      int index,
                      const char *value,
                      int doSignal) {
    QTextBrowser *qw;
    QString text;

    qw=(QTextBrowser*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    if (value)
      text=QT5_Gui::extractHtml(value);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      qw->setText("");
      qw->append(text);
      return 0;

    case GWEN_DialogProperty_AddValue:
      qw->append(text);
      return 0;

    case GWEN_DialogProperty_ClearValues:
      qw->setText("");
      return 0;

    default:
      break;
    }

    DBG_WARN(GWEN_LOGDOMAIN,
             "Function is not appropriate for this type of widget (%s)",
             GWEN_Widget_Type_toString(GWEN_Widget_GetType(_widget)));
    return GWEN_ERROR_INVALID;
  };



};







