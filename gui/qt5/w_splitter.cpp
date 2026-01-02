/***************************************************************************
    begin       : Fri Jan 2 2026
    copyright   : (C) 2026 by Thomas Baumgart
    email       : tbaumgart@kde.org

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

class Qt5_W_Splitter: public Qt5_W_Widget
{
public:
  Qt5_W_Splitter(Qt::Orientation orientation, GWEN_WIDGET* w)
    : Qt5_W_Widget(w)
    , m_orientation(orientation)
  {
  }

  ~Qt5_W_Splitter()
  {
  }


  virtual int setup()
  {
    QSplitter* qw;
    uint32_t flags;
    GWEN_WIDGET* wParent;

    QSizePolicy::Policy hpolicy = QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy = QSizePolicy::Minimum;

    flags = GWEN_Widget_GetFlags(_widget);
    wParent = GWEN_Widget_Tree_GetParent(_widget);

    qw = new QSplitter(m_orientation);

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT5_DIALOG_WIDGET_REAL, (void*) qw);

    if (wParent) {
      GWEN_Widget_AddChildGuiWidget(wParent, _widget);
    }
    return 0;
  }

  int setIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int value,
                     int doSignal) {
    QSplitter* qw;
    QList<int> sizes{0, 0};

    qw = (QSplitter*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      sizes.first() = value;
      sizes.last() = (m_orientation == Qt::Vertical ? qw->height() : qw->width()) - value;
      if (qw->count() < 2) {
        DBG_WARN(GWEN_LOGDOMAIN, "Less than two children assigned to splitter. Additional data ignored.");
      }
      qw->setSizes(sizes);
      return 0;

    default:
      return Qt5_W_Widget::setIntProperty(prop, index, value, doSignal);
    }
  };

  int getIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int defaultValue) {
    QSplitter* qw;

    qw = (QSplitter*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value:
      return qw->sizes().first();

    default:
      return Qt5_W_Widget::getIntProperty(prop, index, defaultValue);
    }
  };


  int addChildGuiWidget(GWEN_WIDGET* wChild)
  {
    QSplitter* qw;
    QWidget* qChild;

    qw =(QSplitter*) GWEN_Widget_GetImplData(_widget, QT5_DIALOG_WIDGET_REAL);
    assert(qw);

    // even though Qt supports more than 2 children, we limit it to two
    if (qw->count() < 2) {
        qChild = getQWidget(wChild);
        assert(qChild);

        qw->addWidget(qChild);
        return 0;
    }

    DBG_WARN(GWEN_LOGDOMAIN, "Qt Dialogs do not support more than two children for splitter");
    return GWEN_ERROR_INVALID;
  }

private:
  Qt::Orientation m_orientation;
};

class Qt5_W_HSplitter : public Qt5_W_Splitter
{
public:
  Qt5_W_HSplitter(GWEN_WIDGET* w)
    : Qt5_W_Splitter(Qt::Horizontal, w)
  {
  }

  ~Qt5_W_HSplitter()
  {
  }
};

class Qt5_W_VSplitter : public Qt5_W_Splitter
{
public:
  Qt5_W_VSplitter(GWEN_WIDGET* w)
    : Qt5_W_Splitter(Qt::Vertical, w)
  {
  }

  ~Qt5_W_VSplitter()
  {
  }
};
