/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt4_W_Widget: public CppWidget {
public:
  Qt4_W_Widget(GWEN_WIDGET *w):CppWidget(w) {
  }



  ~Qt4_W_Widget() {
  }



  virtual int setup() {
    QWidget *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);

    qw=new QWidget();

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



  static QWidget *getQWidget(GWEN_WIDGET *w) {
    QWidget *qw;

    qw=(QWidget*) GWEN_Widget_GetImplData(w, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    return qw;
  }


  int setIntProperty(GWEN_DIALOG_PROPERTY prop,
		     int index,
		     int value,
		     int doSignal) {
    QWidget *qw;

    qw=(QWidget*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
      case GWEN_DialogProperty_Width:
	qw->resize(value, qw->height());
	return 0;
  
      case GWEN_DialogProperty_Height:
	qw->resize(qw->width(), value);
	return 0;
  
      case GWEN_DialogProperty_Enabled:
	qw->setEnabled((value==0)?false:true);
	return 0;
  
      case GWEN_DialogProperty_Focus:
	qw->setFocus();
	return 0;
  
      case GWEN_DialogProperty_Visibility:
	if (value==0)
	  qw->hide();
	else
	  qw->show();
	return 0;

      default:
	break;
    }
  
    DBG_WARN(GWEN_LOGDOMAIN,
	     "Function is not appropriate for this type of widget (%s)",
	     GWEN_Widget_Type_toString(GWEN_Widget_GetType(_widget)));
    return GWEN_ERROR_INVALID;
  };



  int getIntProperty(GWEN_DIALOG_PROPERTY prop,
		     int index,
		     int defaultValue) {
    QWidget *qw;

    qw=(QWidget*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
      case GWEN_DialogProperty_Width:
	return qw->width();

      case GWEN_DialogProperty_Height:
	return qw->height();

      case GWEN_DialogProperty_Enabled:
	return (qw->isEnabled())?1:0;

      case GWEN_DialogProperty_Focus:
	return (qw->hasFocus())?1:0;

      default:
	break;
    }
  
    DBG_WARN(GWEN_LOGDOMAIN,
	     "Function is not appropriate for this type of widget (%s)",
	     GWEN_Widget_Type_toString(GWEN_Widget_GetType(_widget)));
    return defaultValue;
  };



  int setCharProperty(GWEN_DIALOG_PROPERTY prop,
		      int index,
		      const char *value,
		      int doSignal) {
    DBG_WARN(GWEN_LOGDOMAIN,
	     "Function is not appropriate for this type of widget (%s)",
	     GWEN_Widget_Type_toString(GWEN_Widget_GetType(_widget)));
    return GWEN_ERROR_INVALID;
  };



  const char *getCharProperty(GWEN_DIALOG_PROPERTY prop,
			      int index,
			      const char *defaultValue) {
    DBG_WARN(GWEN_LOGDOMAIN,
	     "Function is not appropriate for this type of widget (%s)",
	     GWEN_Widget_Type_toString(GWEN_Widget_GetType(_widget)));
    return defaultValue;
  };

};







