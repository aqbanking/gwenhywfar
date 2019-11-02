/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwen-gui-cpp/cppwidget.hpp>


class Qt4_W_ListBox: public Qt4_W_Widget {
public:
  Qt4_W_ListBox(GWEN_WIDGET *w):Qt4_W_Widget(w) {
  }



  ~Qt4_W_ListBox() {
  }



  virtual int setup() {
    QTreeWidget *qw;
    uint32_t flags;
    GWEN_WIDGET *wParent;
    QSizePolicy::Policy hpolicy=QSizePolicy::Minimum;
    QSizePolicy::Policy vpolicy=QSizePolicy::Minimum;
    QT4_GuiDialog *qtDialog;

    flags=GWEN_Widget_GetFlags(_widget);
    wParent=GWEN_Widget_Tree_GetParent(_widget);

    qw=new QTreeWidget();
    qw->setAllColumnsShowFocus(true);
    qw->setSortingEnabled(true);
    qw->setRootIsDecorated(false);
    qw->setItemsExpandable(false);
    qw->setSelectionBehavior(QAbstractItemView::SelectRows);

    /* handle flags */
    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    qw->setSizePolicy(hpolicy, vpolicy);

    GWEN_Widget_SetImplData(_widget, QT4_DIALOG_WIDGET_REAL, (void*) qw);

    qtDialog=dynamic_cast<QT4_GuiDialog*>(getDialog());
    assert(qtDialog);

    qw->connect(qw, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
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
    QTreeWidget *qw;

    qw=(QTreeWidget*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_ClearValues:
      qw->clear();
      return 0;

    case GWEN_DialogProperty_Value: {
      QTreeWidgetItem *item;

      item=qw->topLevelItem(index);

      if (item==NULL) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Value %d out of range", value);
        return GWEN_ERROR_INVALID;
      }

      qw->setCurrentItem(item);
      return 0;
    }

    case GWEN_DialogProperty_ColumnWidth:
      qw->setColumnWidth(index, value);
      return 0;

    case GWEN_DialogProperty_SelectionMode:
      switch(value) {
      case GWEN_Dialog_SelectionMode_None:
        qw->setSelectionMode(QAbstractItemView::NoSelection);
        return 0;
      case GWEN_Dialog_SelectionMode_Single:
        qw->setSelectionMode(QAbstractItemView::SingleSelection);
        return 0;
      case GWEN_Dialog_SelectionMode_Multi:
        qw->setSelectionMode(QAbstractItemView::ExtendedSelection);
        return 0;
        ;
      }
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown SelectionMode %d", value);
      return GWEN_ERROR_INVALID;

    case GWEN_DialogProperty_SortDirection:
      switch(value) {
      case GWEN_DialogSortDirection_None:
        qw->sortByColumn(-1, Qt::AscendingOrder);
        break;
      case GWEN_DialogSortDirection_Up:
        qw->sortByColumn(index, Qt::AscendingOrder);
        break;
      case GWEN_DialogSortDirection_Down:
        qw->sortByColumn(index, Qt::DescendingOrder);
        break;
      }
      return 0;

    case GWEN_DialogProperty_Sort: {
      int c;

      c=qw->sortColumn();
      if (c!=-1) {
        QHeaderView *h;

        h=qw->header();
        qw->sortItems(c, h->sortIndicatorOrder());
      }
      return 0;
    }

    default:
      return Qt4_W_Widget::setIntProperty(prop, index, value, doSignal);
    }
  };



  int getIntProperty(GWEN_DIALOG_PROPERTY prop,
                     int index,
                     int defaultValue) {
    QTreeWidget *qw;

    qw=(QTreeWidget*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Value: {
      QTreeWidgetItem *item;
      int i=-1;

      item=qw->currentItem();
      while(item) {
        item=qw->itemAbove(item);
        i++;
      }

      return i;
    }

    case GWEN_DialogProperty_ColumnWidth:
      return qw->columnWidth(index);

    case GWEN_DialogProperty_SelectionMode:
      switch(qw->selectionMode()) {
      case QAbstractItemView::NoSelection:
        return GWEN_Dialog_SelectionMode_None;
      case QAbstractItemView::SingleSelection:
        return GWEN_Dialog_SelectionMode_Single;
      case QAbstractItemView::ExtendedSelection:
        return GWEN_Dialog_SelectionMode_Multi;
      default:
        break;
      }
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown SelectionMode %d",
                qw->selectionMode());
      return GWEN_ERROR_INVALID;

    case GWEN_DialogProperty_SortDirection:
      if (qw->sortColumn()!=index)
        return GWEN_DialogSortDirection_None;
      else {
        switch(qw->header()->sortIndicatorOrder()) {
        case Qt::AscendingOrder:
          return GWEN_DialogSortDirection_Up;
        case Qt::DescendingOrder:
          return GWEN_DialogSortDirection_Down;
        default:
          return GWEN_DialogSortDirection_None;
        }
      }
      break;

    default:
      return Qt4_W_Widget::getIntProperty(prop, index, defaultValue);
    }
  };



  int setCharProperty(GWEN_DIALOG_PROPERTY prop,
                      int index,
                      const char *value,
                      int doSignal) {
    QTreeWidget *qw;
    QString text;

    qw=(QTreeWidget*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    if (value)
      text=QT4_Gui::extractHtml(value);

    switch(prop) {
    case GWEN_DialogProperty_Title: {
      QString str;
      QString t;
      QStringList sl;
      int n=0;

      /* remove all columns */
      qw->header()->reset();

      str=text;
      while(!(t=str.section('\t', n, n)).isEmpty()) {
        sl+=t;
        n++;
      }
      qw->setHeaderLabels(sl);
      return 0;
    }

    case GWEN_DialogProperty_ClearValues:
      qw->clear();
      return 0;

    case GWEN_DialogProperty_AddValue: {
      QString t;
      int n=0;
      QStringList sl;

      QString str=text;
      while(!(t=str.section('\t', n, n)).isEmpty()) {
        sl+=t;
        n++;
      }
      (void)new QTreeWidgetItem(qw, sl);
      return 0;
    }

    default:
      return Qt4_W_Widget::setCharProperty(prop, index, value, doSignal);
    }
  };



  const char *getCharProperty(GWEN_DIALOG_PROPERTY prop,
                              int index,
                              const char *defaultValue) {
    QTreeWidget *qw;
    QString str;

    qw=(QTreeWidget*) GWEN_Widget_GetImplData(_widget, QT4_DIALOG_WIDGET_REAL);
    assert(qw);

    switch(prop) {
    case GWEN_DialogProperty_Title: {
      QTreeWidgetItem *item;

      item=qw->headerItem();
      if (item) {
        int i;

        for (i=0; i<qw->columnCount(); i++) {
          if (i)
            str+='\t';
          str+=item->text(i);
        }
        if (str.isEmpty())
          return defaultValue;
        else {
          GWEN_Widget_SetText(_widget, QT4_DIALOG_STRING_TITLE, str.toUtf8());
          return GWEN_Widget_GetText(_widget, QT4_DIALOG_STRING_TITLE);
        }
      }
      return defaultValue;
    }

    case GWEN_DialogProperty_Value: {
      QTreeWidgetItem *item;
      int i;

      item=qw->topLevelItem(index);

      if (item==NULL) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Value %d out of range", index);
        return defaultValue;
      }

      for (i=0; i<qw->columnCount(); i++) {
        if (i)
          str+='\t';
        str+=item->text(i);
      }
      if (str.isEmpty())
        return defaultValue;
      else {
        GWEN_Widget_SetText(_widget, QT4_DIALOG_STRING_VALUE, str.toUtf8());
        return GWEN_Widget_GetText(_widget, QT4_DIALOG_STRING_VALUE);
      }
    }

    default:
      return Qt4_W_Widget::getCharProperty(prop, index, defaultValue);
    }
  };

};







