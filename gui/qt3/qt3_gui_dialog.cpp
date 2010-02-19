/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "qt3_gui_dialog.hpp"
#include "qt3dialogbox.hpp"

#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/widget_be.h>
#include <gwenhywfar/debug.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qprogressbar.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qscrollview.h>
#include <qcheckbox.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qtextbrowser.h>

#include <list>
#include <string>




#define QT3_DIALOG_WIDGET_REAL    0
#define QT3_DIALOG_WIDGET_CONTENT 1
#define QT3_DIALOG_WIDGET_LAYOUT  2

#define QT3_DIALOG_STRING_TITLE  0
#define QT3_DIALOG_STRING_VALUE  1




QT3_GuiDialog::QT3_GuiDialog(QT3_Gui *gui, GWEN_DIALOG *dlg)
  :QObject()
  ,CppDialog(dlg)
  ,_gui(gui)
  ,_mainWidget(NULL)
{

}



QT3_GuiDialog::~QT3_GuiDialog() {
  if (_mainWidget)
    _mainWidget->unlinkFromDialog();

}



QT3_GuiDialog *QT3_GuiDialog::getDialog(GWEN_DIALOG *dlg) {
  CppDialog *cppDlg;

  cppDlg=CppDialog::getDialog(dlg);
  if (cppDlg)
    return dynamic_cast<QT3_GuiDialog*>(cppDlg);
  return NULL;
}



bool QT3_GuiDialog::setup(QWidget *parentWindow) {
  GWEN_WIDGET_TREE *wtree;
  GWEN_WIDGET *w;
  int rv;

  wtree=GWEN_Dialog_GetWidgets(_dialog);
  if (wtree==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No widget tree in dialog");
    return false;
  }
  w=GWEN_Widget_Tree_GetFirst(wtree);
  if (w==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No widgets in dialog");
    return false;
  }

  rv=setupTree(parentWindow, w);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    return false;
  }

  rv=GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeInit, "");
  if (rv<0) {
    DBG_INFO(0, "Error initializing dialog: %d", rv);
    return false;
  }

  return true;
}



int QT3_GuiDialog::execute() {
  QT3_DialogBox *dialogBox;
  int rv;

  dialogBox=dynamic_cast<QT3_DialogBox*>(getMainWindow());
  if (dialogBox==NULL) {
    DBG_INFO(0, "Dialog's main widget is not derived from class FXDialogBox");
    return GWEN_ERROR_GENERIC;
  }

  /* execute dialog */
  rv=dialogBox->exec();
  GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeFini, "");

  if (rv==QT3_DialogBox::Accepted) {
    /* accepted */
    return 1;
  }
  else
    return 0;
}



int QT3_GuiDialog::openDialog() {
  QT3_DialogBox *dialogBox;

  dialogBox=dynamic_cast<QT3_DialogBox*>(getMainWindow());
  if (dialogBox==NULL) {
    DBG_INFO(0, "Dialog's main widget is not derived from class FXDialogBox");
    return GWEN_ERROR_GENERIC;
  }

  /* show dialog */
  dialogBox->show();
  /* gui update */
  qApp->processEvents();

  return 0;
}



int QT3_GuiDialog::closeDialog() {
  QT3_DialogBox *dialogBox;

  dialogBox=dynamic_cast<QT3_DialogBox*>(getMainWindow());
  if (dialogBox==NULL) {
    DBG_INFO(0, "Dialog's main widget is not derived from class FXDialogBox");
    return GWEN_ERROR_GENERIC;
  }

  /* let dialog write its settings */
  GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeFini, "");

  /* hide dialog */
  dialogBox->hide();
  /* gui update */
  qApp->processEvents();

  delete _mainWidget;
  _mainWidget=NULL;

  return 0;
}



int QT3_GuiDialog::runDialog(bool untilEnd) {
  QT3_DialogBox *dialogBox;

  dialogBox=dynamic_cast<QT3_DialogBox*>(getMainWindow());
  if (dialogBox==NULL) {
    DBG_INFO(0, "Dialog's main widget is not derived from class FXDialogBox");
    return GWEN_ERROR_GENERIC;
  }

  if (untilEnd) {
    dialogBox->cont();
  }
  else { {
    /* gui update */
    qApp->processEvents();
  }
  }

  return 0;
}



int QT3_GuiDialog::setIntProperty(GWEN_WIDGET *w,
				  GWEN_DIALOG_PROPERTY prop,
				  int index,
				  int value,
				  int doSignal) {
  const char *name;

  name=GWEN_Widget_GetName(w);

#if 0
  DBG_ERROR(GWEN_LOGDOMAIN,
	    "Function call: type=%s, name=%s, property=%d, index=%d, value=%d",
	    GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)),
	    name?name:"(unnamed)", prop, index, value);
#endif

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
  case GWEN_Widget_TypeNone:
    break;

  case GWEN_Widget_TypeComboBox:
    {
      QComboBox *f;

      f=(QComboBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCurrentItem(value);
	return 0;

      case GWEN_DialogProperty_ClearValues:
	f->clear();
	return 0;

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypeRadioButton:
    {
      QRadioButton *f;

      f=(QRadioButton*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setChecked((value==0)?false:true);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      QCheckBox *f;

      f=(QCheckBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setChecked((value==0)?false:true);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeProgressBar:
    {
      QProgressBar *f;

      f=(QProgressBar*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setProgress(value);
	return 0;
 
      case GWEN_DialogProperty_MinValue:
	if (value!=0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "MinValue should be 0!");
	  return GWEN_ERROR_INVALID;
	}
	return 0;

      case GWEN_DialogProperty_MaxValue:
	f->setTotalSteps(value);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      QListView *f;

      f=(QListView*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_ClearValues:
	f->clear();
	return 0;

      case GWEN_DialogProperty_Value:
	{
	  QListViewItem *item;
	  int i=0;

	  item=f->firstChild();
	  while(item && i<value) {
	    item=item->itemBelow();
	    i++;
	  }

	  if (item==NULL) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Value %d out of range", value);
            return GWEN_ERROR_INVALID;
	  }

          f->setCurrentItem(item);
          return 0;
	}

      case GWEN_DialogProperty_ColumnWidth:
        f->setColumnWidth(index, value);
	return 0;

      case GWEN_DialogProperty_SelectionMode:
	switch(value) {
	case GWEN_Dialog_SelectionMode_None:
	  f->setSelectionMode(QListView::NoSelection);
          return 0;
	case GWEN_Dialog_SelectionMode_Single:
	  f->setSelectionMode(QListView::Single);
          return 0;
	case GWEN_Dialog_SelectionMode_Multi:
	  f->setSelectionMode(QListView::Extended);
          return 0;
	  ;
	}
	DBG_ERROR(GWEN_LOGDOMAIN, "Unknown SelectionMode %d", value);
	return GWEN_ERROR_INVALID;

      case GWEN_DialogProperty_SortDirection:
	switch(value) {
	case GWEN_DialogSortDirection_None:
	  f->setSortColumn(-1);
	  break;
	case GWEN_DialogSortDirection_Up:
	  f->setSortColumn(index);
	  f->setSortOrder(Ascending);
	  break;
	case GWEN_DialogSortDirection_Down:
	  f->setSortColumn(index);
	  f->setSortOrder(Descending);
	  break;
	}
	return 0;

      case GWEN_DialogProperty_Sort:
	f->sort();
	return 0;

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypeSpinBox:
    {
      QSpinBox *f;

      f=(QSpinBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setValue(value);
	return 0;

      case GWEN_DialogProperty_MinValue:
        f->setMinValue(value);
	return 0;

      case GWEN_DialogProperty_MaxValue:
	f->setMaxValue(value);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeWidgetStack:
    {
      QWidgetStack *f;

      f=(QWidgetStack*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->raiseWidget(value);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTabBook:
    {
      QTabWidget *f;

      f=(QTabWidget*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCurrentPage(value);
	return 0;

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeDialog:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeImage:
  case GWEN_Widget_TypeRadioGroup:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeTextBrowser:
    break;
  }

  /* generic properties every widget has */
  {

    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeUnknown:
    case GWEN_Widget_TypeNone:
    case GWEN_Widget_TypeHLayout:
    case GWEN_Widget_TypeVLayout:
    case GWEN_Widget_TypeGridLayout:
    case GWEN_Widget_TypeHSpacer:
    case GWEN_Widget_TypeVSpacer:
    case GWEN_Widget_TypeRadioGroup:
      break;

    case GWEN_Widget_TypeComboBox:
    case GWEN_Widget_TypeRadioButton:
    case GWEN_Widget_TypeCheckBox:
    case GWEN_Widget_TypeProgressBar:
    case GWEN_Widget_TypeListBox:
    case GWEN_Widget_TypeSpinBox:
    case GWEN_Widget_TypeWidgetStack:
    case GWEN_Widget_TypeTabBook:
    case GWEN_Widget_TypePushButton:
    case GWEN_Widget_TypeLabel:
    case GWEN_Widget_TypeLineEdit:
    case GWEN_Widget_TypeTextEdit:
    case GWEN_Widget_TypeGroupBox:
    case GWEN_Widget_TypeHLine:
    case GWEN_Widget_TypeVLine:
    case GWEN_Widget_TypeDialog:
    case GWEN_Widget_TypeScrollArea:
    case GWEN_Widget_TypeImage:
    case GWEN_Widget_TypeTabPage:
    case GWEN_Widget_TypeTextBrowser:
      {
	QWidget *f;

	f=(QWidget*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
	assert(f);

	switch(prop) {
	case GWEN_DialogProperty_Width:
          f->resize(value, f->height());
	  return 0;

	case GWEN_DialogProperty_Height:
	  f->resize(f->width(), value);
	  return 0;

	case GWEN_DialogProperty_Enabled:
	  f->setEnabled((value==0)?false:true);
	  return 0;

	case GWEN_DialogProperty_Focus:
	  f->setFocus();
	  return 0;

	case GWEN_DialogProperty_Title:
	case GWEN_DialogProperty_Value:
	case GWEN_DialogProperty_MinValue:
	case GWEN_DialogProperty_MaxValue:
	case GWEN_DialogProperty_AddValue:
	case GWEN_DialogProperty_ClearValues:
	case GWEN_DialogProperty_ValueCount:
	case GWEN_DialogProperty_ColumnWidth:
	case GWEN_DialogProperty_SelectionMode:
	case GWEN_DialogProperty_SelectionState:
	case GWEN_DialogProperty_SortDirection:
	case GWEN_DialogProperty_Sort:
	case GWEN_DialogProperty_None:
	case GWEN_DialogProperty_Unknown:
	  break;
	}
      }
    }
  }


  DBG_WARN(0,
	   "Inappropriate function call: type=%s, name=%s, property=%d, index=%d",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)),
	   name?name:"(unnamed)", prop, index);
  return GWEN_ERROR_INVALID;
}



int QT3_GuiDialog::getIntProperty(GWEN_WIDGET *w,
				  GWEN_DIALOG_PROPERTY prop,
				  int index,
				  int defaultValue) {
  const char *name;

  name=GWEN_Widget_GetName(w);

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
  case GWEN_Widget_TypeNone:
    break;

  case GWEN_Widget_TypeComboBox:
    {
      QComboBox *f;

      f=(QComboBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->currentItem();

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypeRadioButton:
    {
      QRadioButton *f;

      f=(QRadioButton*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return (f->isChecked())?1:0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      QCheckBox *f;

      f=(QCheckBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return (f->isChecked())?1:0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeProgressBar:
    {
      QProgressBar *f;

      f=(QProgressBar*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->progress();
 
      case GWEN_DialogProperty_MinValue:
	return 0;

      case GWEN_DialogProperty_MaxValue:
	return f->totalSteps();

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      QListView *f;

      f=(QListView*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_ClearValues:
	f->clear();
	return 0;

      case GWEN_DialogProperty_Value:
	{
	  QListViewItem *ci;
	  QListViewItem *item=NULL;
	  int i=0;

	  ci=f->currentItem();
	  if (ci) {
	    item=f->firstChild();
	    while(item && item!=ci) {
	      item=item->itemBelow();
	      i++;
	    }
	    assert(item==ci);
	    return i;
	  }
	  else {
	    DBG_ERROR(GWEN_LOGDOMAIN, "No current item");
	    return defaultValue;
	  }
	}

      case GWEN_DialogProperty_ColumnWidth:
	return f->columnWidth(index);

      case GWEN_DialogProperty_SelectionMode:
	switch(f->selectionMode()) {
	case QListView::NoSelection:
	  return GWEN_Dialog_SelectionMode_None;
	case QListView::Single:
	  return GWEN_Dialog_SelectionMode_Single;
	case QListView::Extended:
	  return GWEN_Dialog_SelectionMode_Multi;
	default:
	  DBG_ERROR(GWEN_LOGDOMAIN, "Unknown SelectionMode %d", f->selectionMode());
	  return defaultValue;
	}

      case GWEN_DialogProperty_SortDirection:
	if (f->sortColumn()!=index)
	  return GWEN_DialogSortDirection_None;
	else {
	  switch(f->sortOrder()) {
	  case Qt::Ascending:
            return GWEN_DialogSortDirection_Up;
	  case Qt::Descending:
	    return GWEN_DialogSortDirection_Down;
	  default:
	    return GWEN_DialogSortDirection_None;
	  }
	}
	return 0;

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypeSpinBox:
    {
      QSpinBox *f;

      f=(QSpinBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->value();

      case GWEN_DialogProperty_MinValue:
	return f->minValue();

      case GWEN_DialogProperty_MaxValue:
	return f->maxValue();

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeWidgetStack:
    {
      QWidgetStack *f;

      f=(QWidgetStack*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	{
	  QWidget *widget;

	  widget=f->visibleWidget();
	  if (widget==NULL) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "No visible widget");
            return defaultValue;
	  }
	  return f->id(widget);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTabBook:
    {
      QTabWidget *f;

      f=(QTabWidget*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->currentPageIndex();

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeDialog:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeImage:
  case GWEN_Widget_TypeRadioGroup:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeTextBrowser:
    break;
  }

  /* generic properties every widget has */
  {

    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeUnknown:
    case GWEN_Widget_TypeNone:
    case GWEN_Widget_TypeHLayout:
    case GWEN_Widget_TypeVLayout:
    case GWEN_Widget_TypeGridLayout:
    case GWEN_Widget_TypeHSpacer:
    case GWEN_Widget_TypeVSpacer:
    case GWEN_Widget_TypeRadioGroup:
      break;

    case GWEN_Widget_TypeComboBox:
    case GWEN_Widget_TypeRadioButton:
    case GWEN_Widget_TypeCheckBox:
    case GWEN_Widget_TypeProgressBar:
    case GWEN_Widget_TypeListBox:
    case GWEN_Widget_TypeSpinBox:
    case GWEN_Widget_TypeWidgetStack:
    case GWEN_Widget_TypeTabBook:
    case GWEN_Widget_TypePushButton:
    case GWEN_Widget_TypeLabel:
    case GWEN_Widget_TypeLineEdit:
    case GWEN_Widget_TypeTextEdit:
    case GWEN_Widget_TypeGroupBox:
    case GWEN_Widget_TypeHLine:
    case GWEN_Widget_TypeVLine:
    case GWEN_Widget_TypeDialog:
    case GWEN_Widget_TypeScrollArea:
    case GWEN_Widget_TypeImage:
    case GWEN_Widget_TypeTabPage:
    case GWEN_Widget_TypeTextBrowser:
      {
	QWidget *f;

	f=(QWidget*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
	assert(f);

	switch(prop) {
	case GWEN_DialogProperty_Width:
	  return f->width();

	case GWEN_DialogProperty_Height:
	  return f->height();

	case GWEN_DialogProperty_Enabled:
	  return (f->isEnabled())?1:0;

	case GWEN_DialogProperty_Focus:
	  return (f->hasFocus())?1:0;

	case GWEN_DialogProperty_Title:
	case GWEN_DialogProperty_Value:
	case GWEN_DialogProperty_MinValue:
	case GWEN_DialogProperty_MaxValue:
	case GWEN_DialogProperty_AddValue:
	case GWEN_DialogProperty_ClearValues:
	case GWEN_DialogProperty_ValueCount:
	case GWEN_DialogProperty_ColumnWidth:
	case GWEN_DialogProperty_SelectionMode:
	case GWEN_DialogProperty_SelectionState:
	case GWEN_DialogProperty_SortDirection:
	case GWEN_DialogProperty_Sort:
	case GWEN_DialogProperty_None:
	case GWEN_DialogProperty_Unknown:
	  break;
	}
      }
    }
  }

  DBG_WARN(0,
	   "Inappropriate function call: type=%s, name=%s, property=%d, index=%d",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)),
	   name?name:"(unnamed)", prop, index);
  return defaultValue;
}



int QT3_GuiDialog::setCharProperty(GWEN_WIDGET *w,
				   GWEN_DIALOG_PROPERTY prop,
				   int index,
				   const char *value,
				   int doSignal) {
  QString strValue;
  const char *name;

  if (value && *value)
    strValue=QT3_Gui::extractHtml(value);

  name=GWEN_Widget_GetName(w);

#if 0
  DBG_ERROR(0,
	    "Function call: type=%s, name=%s, property=%d, index=%d, value=%s",
	    GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)),
	    name?name:"(unnamed)", prop, index, value?value:"(empty)");
#endif

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
  case GWEN_Widget_TypeNone:
    break;

  case GWEN_Widget_TypeLabel:
    {
      QLabel *f;

      f=(QLabel*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setText(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTextBrowser:
    {
      QTextBrowser *f;

      f=(QTextBrowser*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setText(strValue);
	f->scrollToBottom();
        return 0;

      case GWEN_DialogProperty_AddValue:
	DBG_ERROR(0, "Adding text: [%s]", value?value:"(empty)");
	f->setText(f->text()+strValue);
	f->scrollToBottom();
        return 0;

      case GWEN_DialogProperty_ClearValues:
	f->setText("");
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypePushButton:
    {
      QPushButton *f;

      f=(QPushButton*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setText(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeComboBox:
    {
      QComboBox *f;

      f=(QComboBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCurrentText(strValue);
	return 0;

      case GWEN_DialogProperty_AddValue:
	f->insertItem(strValue);
	return 0;

      case GWEN_DialogProperty_ClearValues:
	f->clear();
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      QListView *f;

      f=(QListView*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title: {
        QString str;
	QString t;
        int n=0;

	/* remove all columns */
	while(f->columns())
	  f->removeColumn(0);

	str=strValue;
	while(!(t=str.section('\t', n, n)).isEmpty()){
	  f->addColumn(t, -1);
	  n++;
	}
	return 0;
      }

      case GWEN_DialogProperty_ClearValues:
	f->clear();
	return 0;

      case GWEN_DialogProperty_AddValue: {
	QString str;
	QString t;
	int n=0;
	QListViewItem *item;

	item=new QListViewItem(f);
	str=strValue;
	while(!(t=str.section('\t', n, n)).isEmpty()){
	  item->setText(n, t);
	  n++;
	}
	return 0;
      }

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypeLineEdit:
    {
      QLineEdit *f;

      f=(QLineEdit*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setText(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTextEdit:
    {
      QTextEdit *f;

      f=(QTextEdit*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setText(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeGroupBox:
    {
      QGroupBox *f;

      f=(QGroupBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setTitle(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeDialog:
    {
      QT3_DialogBox *f;

      f=(QT3_DialogBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setCaption(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTabPage:
    {
      QWidget *f;
      GWEN_WIDGET *parentWidget;
      QTabWidget *tabWidget;

      f=(QWidget*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      parentWidget=GWEN_Widget_Tree_GetParent(w);
      if (parentWidget==NULL) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] has no parent (%s), SNH",
		  name?name:"(unnamed)",
		  GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
      }

      if (GWEN_Widget_GetType(parentWidget)!=GWEN_Widget_TypeTabBook) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Parent of widget [%s] is not a tabBook (%s)",
		  name?name:"(unnamed)",
		  GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
      }

      tabWidget=(QTabWidget*)GWEN_Widget_GetImplData(parentWidget, QT3_DIALOG_WIDGET_REAL);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	tabWidget->changeTab(f, strValue);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      QCheckBox *f;

      f=(QCheckBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setText(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeRadioButton:
    {
      QRadioButton *f;

      f=(QRadioButton*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setText(strValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeSpinBox:
  case GWEN_Widget_TypeImage:
  case GWEN_Widget_TypeRadioGroup:
    break;
  }

  /* this block is just to make sure we get warnings when new
   * properties are available but not handled here
   */
  switch(prop) {
  case GWEN_DialogProperty_Title:
  case GWEN_DialogProperty_Value:
  case GWEN_DialogProperty_MinValue:
  case GWEN_DialogProperty_MaxValue:
  case GWEN_DialogProperty_Enabled:
  case GWEN_DialogProperty_AddValue:
  case GWEN_DialogProperty_ClearValues:
  case GWEN_DialogProperty_ValueCount:
  case GWEN_DialogProperty_ColumnWidth:
  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
  case GWEN_DialogProperty_SelectionMode:
  case GWEN_DialogProperty_SelectionState:
  case GWEN_DialogProperty_Focus:
  case GWEN_DialogProperty_SortDirection:
  case GWEN_DialogProperty_Sort:
  case GWEN_DialogProperty_None:
  case GWEN_DialogProperty_Unknown:
    break;
  }

  DBG_WARN(0,
	   "Inappropriate function call: type=%s, name=%s, property=%d, index=%d",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)),
	   name?name:"(unnamed)", prop, index);
  return GWEN_ERROR_INVALID;
}



const char *QT3_GuiDialog::getCharProperty(GWEN_WIDGET *w,
					   GWEN_DIALOG_PROPERTY prop,
					   int index,
					   const char *defaultValue) {
  QString str;
  const char *name;

  name=GWEN_Widget_GetName(w);

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
  case GWEN_Widget_TypeNone:
    break;

  case GWEN_Widget_TypeLabel:
    {
      QLabel *f;

      f=(QLabel*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->text();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTextBrowser:
    {
      QTextBrowser *f;

      f=(QTextBrowser*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->text();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_VALUE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypePushButton:
    {
      QPushButton *f;

      f=(QPushButton*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->text();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeComboBox:
    {
      QComboBox *f;

      f=(QComboBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->currentText();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_VALUE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      QListView *f;

      f=(QListView*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title: {
        int i;

	for (i=0; i<f->columns(); i++) {
	  if (i)
	    str+='\t';
	  str+=f->columnText(i);
	}
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}
      }

      case GWEN_DialogProperty_Value:
	{
	  QListViewItem *item;
	  int i;

	  item=f->currentItem();
	  if (item==NULL)
	    return defaultValue;
  
	  for (i=0; i<f->columns(); i++) {
	    if (i)
	      str+='\t';
	    str+=item->text(i);
	  }
	  if (str.isEmpty())
	    return defaultValue;
	  else {
	    GWEN_Widget_SetText(w, QT3_DIALOG_STRING_VALUE, str.utf8());
	    return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_VALUE);
	  }
	}

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypeLineEdit:
    {
      QLineEdit *f;

      f=(QLineEdit*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->text();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_VALUE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTextEdit:
    {
      QTextEdit *f;

      f=(QTextEdit*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->text();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_VALUE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeGroupBox:
    {
      QGroupBox *f;

      f=(QGroupBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->title();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeDialog:
    {
      QT3_DialogBox *f;

      f=(QT3_DialogBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->caption();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTabPage:
    {
      QWidget *f;
      GWEN_WIDGET *parentWidget;
      QTabWidget *tabWidget;

      f=(QWidget*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      parentWidget=GWEN_Widget_Tree_GetParent(w);
      if (parentWidget==NULL) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] has no parent (%s), SNH",
		  name?name:"(unnamed)",
		  GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
      }

      if (GWEN_Widget_GetType(parentWidget)!=GWEN_Widget_TypeTabBook) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Parent of widget [%s] is not a tabBook (%s)",
		  name?name:"(unnamed)",
		  GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
      }

      tabWidget=(QTabWidget*)GWEN_Widget_GetImplData(parentWidget, QT3_DIALOG_WIDGET_REAL);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=tabWidget->tabLabel(f);
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      QCheckBox *f;

      f=(QCheckBox*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->text();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeRadioButton:
    {
      QRadioButton *f;

      f=(QRadioButton*)GWEN_Widget_GetImplData(w, QT3_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->text();
	if (str.isEmpty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, QT3_DIALOG_STRING_TITLE, str.utf8());
	  return GWEN_Widget_GetText(w, QT3_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeSpinBox:
  case GWEN_Widget_TypeImage:
  case GWEN_Widget_TypeRadioGroup:
    break;
  }

  /* this block is just to make sure we get warnings when new
   * properties are available but not handled here
   */
  switch(prop) {
  case GWEN_DialogProperty_Title:
  case GWEN_DialogProperty_Value:
  case GWEN_DialogProperty_MinValue:
  case GWEN_DialogProperty_MaxValue:
  case GWEN_DialogProperty_Enabled:
  case GWEN_DialogProperty_AddValue:
  case GWEN_DialogProperty_ClearValues:
  case GWEN_DialogProperty_ValueCount:
  case GWEN_DialogProperty_ColumnWidth:
  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
  case GWEN_DialogProperty_SelectionMode:
  case GWEN_DialogProperty_SelectionState:
  case GWEN_DialogProperty_Focus:
  case GWEN_DialogProperty_SortDirection:
  case GWEN_DialogProperty_Sort:
  case GWEN_DialogProperty_None:
  case GWEN_DialogProperty_Unknown:
    break;
  }


  DBG_WARN(0,
	   "Inappropriate function call: type=%s, name=%s, property=%d, index=%d",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)),
	   name?name:"(unnamed)", prop, index);
  return defaultValue;
}



void QT3_GuiDialog::slotActivated() {
  const QObject *snd;

  snd=sender();
  if (snd) {
    GWEN_WIDGET *w;
    const char *wname;
    int rv=GWEN_DialogEvent_ResultNotHandled;
    QT3_DialogBox *dialogBox;

    w=GWEN_Dialog_FindWidgetByImplData(_dialog, QT3_DIALOG_WIDGET_REAL, snd);
    if (w==NULL) {
      DBG_INFO(0, "Widget not found");
      return;
    }
    wname=GWEN_Widget_GetName(w);
    dialogBox=_mainWidget;

    DBG_INFO(GWEN_LOGDOMAIN, "Command for [%s] (type: %s)",
	     wname?wname:"(unnamed)",
	     GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));

    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeUnknown:
    case GWEN_Widget_TypeNone:
      break;

    case GWEN_Widget_TypePushButton:
    case GWEN_Widget_TypeLineEdit:
    case GWEN_Widget_TypeComboBox:
    case GWEN_Widget_TypeListBox:
    case GWEN_Widget_TypeCheckBox:
    case GWEN_Widget_TypeSpinBox:
      rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
				GWEN_DialogEvent_TypeActivated,
				GWEN_Widget_GetName(w));
      break;
  
    case GWEN_Widget_TypeLabel:
    case GWEN_Widget_TypeTextEdit:
    case GWEN_Widget_TypeRadioButton:
    case GWEN_Widget_TypeProgressBar:
    case GWEN_Widget_TypeRadioGroup:
    case GWEN_Widget_TypeGroupBox:
    case GWEN_Widget_TypeHSpacer:
    case GWEN_Widget_TypeVSpacer:
    case GWEN_Widget_TypeHLayout:
    case GWEN_Widget_TypeVLayout:
    case GWEN_Widget_TypeGridLayout:
    case GWEN_Widget_TypeImage:
    case GWEN_Widget_TypeDialog:
    case GWEN_Widget_TypeTabBook:
    case GWEN_Widget_TypeTabPage:
    case GWEN_Widget_TypeScrollArea:
    case GWEN_Widget_TypeWidgetStack:
    case GWEN_Widget_TypeHLine:
    case GWEN_Widget_TypeVLine:
    case GWEN_Widget_TypeTextBrowser:
      /* nothing to do for these types */
      ;
    }
  
    if (rv==GWEN_DialogEvent_ResultAccept) {
      dialogBox->accept();
    }
    else if (rv==GWEN_DialogEvent_ResultReject) {
      dialogBox->reject();
    }
  }
}



void QT3_GuiDialog::slotValueChanged() {
  const QObject *snd;

  snd=sender();
  if (snd) {
    GWEN_WIDGET *w;
    const char *wname;
    int rv=GWEN_DialogEvent_ResultNotHandled;
    QT3_DialogBox *dialogBox;

    w=GWEN_Dialog_FindWidgetByImplData(_dialog, QT3_DIALOG_WIDGET_REAL, snd);
    if (w==NULL) {
      DBG_INFO(0, "Widget not found");
      return;
    }
    wname=GWEN_Widget_GetName(w);

    dialogBox=_mainWidget;

    DBG_INFO(GWEN_LOGDOMAIN, "ValueChanged for [%s] (type: %s)",
	     wname?wname:"(unnamed)",
	      GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));

    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeUnknown:
    case GWEN_Widget_TypeNone:
      break;

    case GWEN_Widget_TypePushButton:
    case GWEN_Widget_TypeLineEdit:
    case GWEN_Widget_TypeComboBox:
    case GWEN_Widget_TypeListBox:
    case GWEN_Widget_TypeCheckBox:
    case GWEN_Widget_TypeSpinBox:
      rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
				GWEN_DialogEvent_TypeValueChanged,
				GWEN_Widget_GetName(w));
      break;
  
    case GWEN_Widget_TypeLabel:
    case GWEN_Widget_TypeTextEdit:
    case GWEN_Widget_TypeRadioButton:
    case GWEN_Widget_TypeProgressBar:
    case GWEN_Widget_TypeRadioGroup:
    case GWEN_Widget_TypeGroupBox:
    case GWEN_Widget_TypeHSpacer:
    case GWEN_Widget_TypeVSpacer:
    case GWEN_Widget_TypeHLayout:
    case GWEN_Widget_TypeVLayout:
    case GWEN_Widget_TypeGridLayout:
    case GWEN_Widget_TypeImage:
    case GWEN_Widget_TypeDialog:
    case GWEN_Widget_TypeTabBook:
    case GWEN_Widget_TypeTabPage:
    case GWEN_Widget_TypeScrollArea:
    case GWEN_Widget_TypeWidgetStack:
    case GWEN_Widget_TypeHLine:
    case GWEN_Widget_TypeVLine:
    case GWEN_Widget_TypeTextBrowser:
      /* nothing to do for these types */
      ;
    }
  
    if (rv==GWEN_DialogEvent_ResultAccept) {
      dialogBox->accept();
    }
    else if (rv==GWEN_DialogEvent_ResultReject) {
      dialogBox->reject();
    }
  }
}



int QT3_GuiDialog::setupTree(QWidget *dialogParent, GWEN_WIDGET *w) {
  int rv;
  GWEN_WIDGET *wt;

  /* setup this widget */
  rv=setupWidget(dialogParent, w);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* setup children */
  wt=GWEN_Widget_Tree_GetFirstChild(w);
  while(wt) {
    rv=setupTree(dialogParent, wt);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    wt=GWEN_Widget_Tree_GetNext(wt);
  }

  return 0;
}



int QT3_GuiDialog::setupWidget(QWidget *dialogParent, GWEN_WIDGET *w) {
  GWEN_WIDGET *gwParent;
  QWidget *wParent=NULL;
  QLayout *lParent=NULL;

  const char *s;
  const char *name;
  int cols;
  int rows;
  uint32_t flags;
  QString text;
  QWidget *wChild=NULL;
  QWidget *wContent=NULL;
  QLayout *wLayout=NULL;

  /* sample data */
  flags=GWEN_Widget_GetFlags(w);
  s=GWEN_Widget_GetText(w, 0);
  if (s)
    text=QString(s);
  name=GWEN_Widget_GetName(w);
  cols=GWEN_Widget_GetColumns(w);
  rows=GWEN_Widget_GetRows(w);

  DBG_INFO(GWEN_LOGDOMAIN, "Setting up widget [%s] (type %d)",
	   name?name:"(unnamed)",
	   GWEN_Widget_GetType(w));

  gwParent=GWEN_Widget_Tree_GetParent(w);
  if (gwParent) {
    GWEN_WIDGET *wt;

    lParent=(QLayout*) GWEN_Widget_GetImplData(gwParent, QT3_DIALOG_WIDGET_LAYOUT);
    wt=gwParent;
    while (wt) {
      wParent=(QWidget*)GWEN_Widget_GetImplData(wt, QT3_DIALOG_WIDGET_CONTENT);
      if (wParent==NULL)
	wParent=(QWidget*)GWEN_Widget_GetImplData(wt, QT3_DIALOG_WIDGET_REAL);
      if (wParent)
	break;
      wt=GWEN_Widget_Tree_GetParent(wt);
    }
  }

  if (gwParent==NULL || wParent==NULL) {
    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeDialog:
      /* these types don't need parents */
      break;
    default:
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] (type %d) has no parent when it should",
		name?name:"(unnamed)", GWEN_Widget_GetType(w));
      return GWEN_ERROR_BAD_DATA;;
    }
  }

  /* create THIS widget */
  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
    DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] is of type \'unknown\'", name?name:"(unnamed)");
    return GWEN_ERROR_BAD_DATA;;

  case GWEN_Widget_TypeNone:
    DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] is of type \'none\'", name?name:"(unnamed)");
    return GWEN_ERROR_BAD_DATA;;

  case GWEN_Widget_TypeLabel:
    wChild=new QLabel(text, wParent, name);
    break;

  case GWEN_Widget_TypePushButton:
    {
      QPushButton *f;

      f=new QPushButton(text, wParent, name);
      connect(f, SIGNAL(clicked()), this, SLOT(slotActivated()));
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeLineEdit:
    {
      QLineEdit *f;

      f=new QLineEdit("", wParent, name);
      if (flags & GWEN_WIDGET_FLAGS_PASSWORD)
	f->setEchoMode(QLineEdit::Password);
      connect(f, SIGNAL(returnPressed()), this, SLOT(slotActivated()));
      //connect(f, SIGNAL(lostFocus()), this, SLOT(slotValueChanged()));
      connect(f, SIGNAL(textChanged(const QString&)), this, SLOT(slotValueChanged()));
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeTextEdit:
    {
      QTextEdit *f;

      f=new QTextEdit(wParent, name);
      // no signals for now
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeTextBrowser:
    {
      QTextBrowser *f;

      f=new QTextBrowser(wParent, name);
      // no signals for now
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeComboBox:
    {
      QComboBox *f;

      f=new QComboBox((flags & GWEN_WIDGET_FLAGS_READONLY)?false:true,
		      wParent, name);
      connect(f, SIGNAL(activated(int)), this, SLOT(slotActivated()));
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeProgressBar:
    wChild=new QProgressBar(wParent, name);
    break;

  case GWEN_Widget_TypeGroupBox:
    {
      QGroupBox *f;

      f=new QGroupBox(text, wParent, name);
      f->setColumnLayout(1, Qt::Vertical);
      wContent=new QWidget(f);
      wContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, false);
      wChild=f;
      wLayout=new QVBoxLayout(wContent);
      break;
    }

  case GWEN_Widget_TypeHLayout:
    if (wParent && gwParent && GWEN_Widget_GetType(gwParent)==GWEN_Widget_TypeWidgetStack) {
      wChild=new QWidget(wParent);
      wLayout=new QHBoxLayout(wChild);
    }
    else {
      if (lParent)
	wLayout=new QHBoxLayout();
      else
	wLayout=new QHBoxLayout(wParent);
    }
    break;

  case GWEN_Widget_TypeVLayout:
    if (wParent && gwParent && GWEN_Widget_GetType(gwParent)==GWEN_Widget_TypeWidgetStack) {
      wChild=new QWidget(wParent);
      wLayout=new QVBoxLayout(wChild);
    }
    else {
      if (lParent)
	wLayout=new QVBoxLayout();
      else
	wLayout=new QVBoxLayout(wParent);
    }
    break;

  case GWEN_Widget_TypeGridLayout:
    {
      int wrows=rows;
      int wcols=cols;

      if (wcols)
	wrows=GWEN_Widget_Tree_GetChildrenCount(w)/wcols;
      else
	wcols=GWEN_Widget_Tree_GetChildrenCount(w)/wrows;

      if (wParent && gwParent && GWEN_Widget_GetType(gwParent)==GWEN_Widget_TypeWidgetStack) {
	wChild=new QWidget(wParent);
	wLayout=new QGridLayout(wChild, wrows, wcols);
      }
      else {
	if (lParent)
	  wLayout=new QGridLayout(wrows, wcols);
	else
	  wLayout=new QGridLayout(wParent, wrows, wcols);
      }
    }
    break;

  case GWEN_Widget_TypeHSpacer:
    if (lParent==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Parent of widget [%s] (type %d) is no layout",
		name?name:"(unnamed)", GWEN_Widget_GetType(w));
      return GWEN_ERROR_BAD_DATA;;
    }
    lParent->addItem(new QSpacerItem(0, 0,
				     QSizePolicy::Expanding,
				     QSizePolicy::Minimum));
    break;

  case GWEN_Widget_TypeVSpacer:
    if (lParent==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Parent of widget [%s] (type %d) is no layout",
		name?name:"(unnamed)", GWEN_Widget_GetType(w));
      return GWEN_ERROR_BAD_DATA;;
    }
    lParent->addItem(new QSpacerItem(0, 0,
				     QSizePolicy::Minimum,
				     QSizePolicy::Expanding));
    break;

  case GWEN_Widget_TypeHLine:
    {
      QFrame *f;

      f=new QFrame(wParent, name);
      f->setFrameShape(QFrame::HLine);
      f->setFrameShadow(QFrame::Sunken);
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeVLine:
    {
      QFrame *f;

      f=new QFrame(wParent, name);
      f->setFrameShape(QFrame::VLine);
      f->setFrameShadow(QFrame::Sunken);
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      QListView *f;

      f=new QListView(wParent, name);
      f->setAllColumnsShowFocus(true);
      f->setShowSortIndicator(true);
      connect(f, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(slotActivated()));
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeDialog:
    _mainWidget=new QT3_DialogBox(this, dialogParent, name);
    wChild=_mainWidget;
    break;

  case GWEN_Widget_TypeTabBook:
    {
      QTabWidget *f;

      f=new QTabWidget(wParent, name);
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeTabPage:
    {
      QTabWidget *f;

      f=dynamic_cast<QTabWidget*>(wParent);
      if (f==NULL) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Parent of widget [%s] (type %d) is no TabBook",
		  name?name:"(unnamed)", GWEN_Widget_GetType(w));
	return GWEN_ERROR_BAD_DATA;;
      }

      wChild=new QWidget(wParent, name);
      f->addTab(wChild, text);
      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      QCheckBox *f;

      f=new QCheckBox(text, wParent, name);
      connect(f, SIGNAL(toggled(bool)), this, SLOT(slotActivated()));
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeWidgetStack:
    wChild=new QWidgetStack(wParent, name);
    break;

  case GWEN_Widget_TypeScrollArea:
    {
      QScrollView *f;
      QWidget *vbox;

      f=new QScrollView(wParent, name);
      f->setResizePolicy(QScrollView::AutoOneFit);
      vbox=new QWidget(f->viewport());
      vbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, false);
      f->addChild(vbox);
      wLayout=new QVBoxLayout(vbox);
      wChild=f;
      wContent=vbox;
      break;
    }

  case GWEN_Widget_TypeSpinBox:
    {
      QSpinBox *f;

      f=new QSpinBox(wParent, name);
      connect(f, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged()));
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeRadioButton:
    {
      QRadioButton *f;

      f=new QRadioButton(text, wParent, name);
      connect(f, SIGNAL(clicked()), this, SLOT(slotActivated()));
      wChild=f;
      break;
    }


  case GWEN_Widget_TypeImage:
  case GWEN_Widget_TypeRadioGroup:
    break;
  }

  _widgetCount++;
  if (wContent==NULL)
    wContent=wChild;

  GWEN_Widget_SetImplData(w, QT3_DIALOG_WIDGET_REAL, (void*) wChild);
  GWEN_Widget_SetImplData(w, QT3_DIALOG_WIDGET_CONTENT, (void*) wContent);
  GWEN_Widget_SetImplData(w, QT3_DIALOG_WIDGET_LAYOUT, (void*) wLayout);

  if (wChild && wParent && gwParent && GWEN_Widget_GetType(gwParent)==GWEN_Widget_TypeWidgetStack) {
    QWidgetStack *wst;
    int rv;
    int nextId=0;
    GWEN_WIDGET *wt;

    /* special handling for widget stacks: we need to call addWidget*/
    wst=(QWidgetStack*) GWEN_Widget_GetImplData(gwParent, QT3_DIALOG_WIDGET_REAL);
    assert(wst);

    wt=GWEN_Widget_Tree_GetFirstChild(gwParent);
    while(wt && wt!=w) {
      wt=GWEN_Widget_Tree_GetNext(wt);
      nextId++;
    }

    rv=wst->addWidget(wChild, nextId);
    DBG_INFO(GWEN_LOGDOMAIN, "Added widget %d to QWidgetStack", rv);
  }

  if (wChild) {
    QSizePolicy::SizeType hpolicy=QSizePolicy::Minimum;
    QSizePolicy::SizeType vpolicy=QSizePolicy::Minimum;

    if (flags & GWEN_WIDGET_FLAGS_FILLX)
      hpolicy=QSizePolicy::Expanding;
    if (flags & GWEN_WIDGET_FLAGS_FILLY)
      vpolicy=QSizePolicy::Expanding;
    wChild->setSizePolicy(hpolicy, vpolicy, false);
  }

  /* parent GWEN_WIDGET is a layout, so add the new widget/layout to it */
  if (lParent) {
    QBoxLayout *boxLayout;
    QGridLayout *gridLayout;

    boxLayout=dynamic_cast<QBoxLayout*>(lParent);
    gridLayout=dynamic_cast<QGridLayout*>(lParent);

    if (boxLayout) {
      if (wChild)
	/* if wChild and wLayout, then we still add the widget rather than the layout */
	boxLayout->addWidget(wChild);
      else if (wLayout)
        /* no wChild, but wLayout; add that */
	boxLayout->addLayout(wLayout);
    }
    else if (gridLayout) {
      GWEN_WIDGET *wt;
      int idx=0;

      wt=GWEN_Widget_Tree_GetFirstChild(gwParent);
      while(wt && wt!=w) {
        idx++;
	wt=GWEN_Widget_Tree_GetNext(wt);
      }

      if (wt) {
	int x=0, y=0;
        int c;

	if ((c=GWEN_Widget_GetColumns(gwParent))) {
	  x=idx % c;
	  y=idx / c;
	}
	else if ((c=GWEN_Widget_GetRows(gwParent))) {
	  x=idx / c;
	  y=idx % c ;
	}

	if (wChild)
	  /* if wChild and wLayout, then we still add the widget rather than the layout */
	  gridLayout->addWidget(wChild, y, x);
	else if (wLayout)
	  /* no wChild, but wLayout; add that */
	  gridLayout->addLayout(wLayout, y, x);
      }
    }
  }

  return 0;
}



#include "qt3_gui_dialog.moc"


