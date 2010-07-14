/***************************************************************************
    begin       : Mon Feb 15 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "qt4_gui_dialog.hpp"
#include "qt4dialogbox.hpp"

#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/widget_be.h>
#include <gwenhywfar/debug.h>

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QTextBrowser>
#include <QComboBox>
#include <QGroupBox>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QTreeWidget>
#include <QTabWidget>
#include <QCheckBox>
#include <QStackedWidget>
#include <QScrollArea>
#include <QSpinBox>
#include <QRadioButton>
#include <QHeaderView>

#include <QDebug>

#include <list>
#include <string>




#define QT4_DIALOG_WIDGET_REAL    0
#define QT4_DIALOG_WIDGET_CONTENT 1
#define QT4_DIALOG_WIDGET_LAYOUT  2

#define QT4_DIALOG_STRING_TITLE  0
#define QT4_DIALOG_STRING_VALUE  1



#include "w_widget.cpp"
#include "w_dialog.cpp"
#include "w_vlayout.cpp"
#include "w_hlayout.cpp"
#include "w_gridlayout.cpp"
#include "w_label.cpp"
#include "w_lineedit.cpp"
#include "w_pushbutton.cpp"
#include "w_hline.cpp"
#include "w_vline.cpp"
#include "w_textedit.cpp"
#include "w_combobox.cpp"
#include "w_tabbook.cpp"
#include "w_checkbox.cpp"
#include "w_groupbox.cpp"
#include "w_widgetstack.cpp"
#include "w_textbrowser.cpp"
#include "w_scrollarea.cpp"
#include "w_progressbar.cpp"
#include "w_listbox.cpp"
#include "w_radiobutton.cpp"
#include "w_spinbox.cpp"




QT4_GuiDialog::QT4_GuiDialog(QT4_Gui *gui, GWEN_DIALOG *dlg)
  :CppDialog(dlg)
  ,_gui(gui)
  ,_mainWidget(NULL)
{

}



QT4_GuiDialog::~QT4_GuiDialog() {
  if (_mainWidget)
    _mainWidget->unlinkFromDialog();

}



QT4_GuiDialog *QT4_GuiDialog::getDialog(GWEN_DIALOG *dlg) {
  CppDialog *cppDlg;

  cppDlg=CppDialog::getDialog(dlg);
  if (cppDlg)
    return dynamic_cast<QT4_GuiDialog*>(cppDlg);
  return NULL;
}



int QT4_GuiDialog::execute() {
  QT4_DialogBox *dialogBox;
  int rv;

  dialogBox=dynamic_cast<QT4_DialogBox*>(getMainWindow());
  if (dialogBox==NULL) {
    DBG_INFO(0, "Dialog's main widget is not derived from class FXDialogBox");
    return GWEN_ERROR_GENERIC;
  }

  /* execute dialog */
  rv=dialogBox->exec();
  GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeFini, "");

  if (rv==QT4_DialogBox::Accepted) {
    /* accepted */
    return 1;
  }
  else
    return 0;
}



int QT4_GuiDialog::openDialog() {
  QT4_DialogBox *dialogBox;

  dialogBox=dynamic_cast<QT4_DialogBox*>(getMainWindow());
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



int QT4_GuiDialog::closeDialog() {
  QT4_DialogBox *dialogBox;

  dialogBox=dynamic_cast<QT4_DialogBox*>(getMainWindow());
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



int QT4_GuiDialog::runDialog(bool untilEnd) {
  QT4_DialogBox *dialogBox;

  dialogBox=dynamic_cast<QT4_DialogBox*>(getMainWindow());
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



int QT4_GuiDialog::setIntProperty(GWEN_WIDGET *w,
				  GWEN_DIALOG_PROPERTY prop,
				  int index,
				  int value,
				  int doSignal) {
  return GWEN_Widget_SetIntProperty(w, prop, index, value, doSignal);
}



int QT4_GuiDialog::getIntProperty(GWEN_WIDGET *w,
				  GWEN_DIALOG_PROPERTY prop,
				  int index,
				  int defaultValue) {
  return GWEN_Widget_GetIntProperty(w, prop, index, defaultValue);
}



int QT4_GuiDialog::setCharProperty(GWEN_WIDGET *w,
				   GWEN_DIALOG_PROPERTY prop,
				   int index,
				   const char *value,
				   int doSignal) {
  return GWEN_Widget_SetCharProperty(w, prop, index, value, doSignal);
}



const char *QT4_GuiDialog::getCharProperty(GWEN_WIDGET *w,
					   GWEN_DIALOG_PROPERTY prop,
					   int index,
					   const char *defaultValue) {
  return GWEN_Widget_GetCharProperty(w, prop, index, defaultValue);
}



int QT4_GuiDialog::setupTree(GWEN_WIDGET *w) {
  int rv;
  Qt4_W_Widget *xw=NULL;

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeDialog:
    xw=new Qt4_W_Dialog(w);
    break;
  case GWEN_Widget_TypeVLayout:
    xw=new Qt4_W_VLayout(w);
    break;
  case GWEN_Widget_TypeHLayout:
    xw=new Qt4_W_HLayout(w);
    break;
  case GWEN_Widget_TypeGridLayout:
    xw=new Qt4_W_GridLayout(w);
    break;
  case GWEN_Widget_TypeLabel:
    xw=new Qt4_W_Label(w);
    break;
  case GWEN_Widget_TypeLineEdit:
    xw=new Qt4_W_LineEdit(w);
    break;
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHSpacer:
    /* abuse widget */
    xw=new Qt4_W_Widget(w);
    break;
  case GWEN_Widget_TypePushButton:
    xw=new Qt4_W_PushButton(w);
    break;
  case GWEN_Widget_TypeHLine:
    xw=new Qt4_W_HLine(w);
    break;
  case GWEN_Widget_TypeVLine:
    xw=new Qt4_W_VLine(w);
    break;
  case GWEN_Widget_TypeTextEdit:
    xw=new Qt4_W_TextEdit(w);
    break;
  case GWEN_Widget_TypeComboBox:
    xw=new Qt4_W_ComboBox(w);
    break;
  case GWEN_Widget_TypeTabBook:
    xw=new Qt4_W_TabBook(w);
    break;
  case GWEN_Widget_TypeTabPage:
    xw=new Qt4_W_VLayout(w);
    break;
  case GWEN_Widget_TypeCheckBox:
    xw=new Qt4_W_CheckBox(w);
    break;
  case GWEN_Widget_TypeGroupBox:
    xw=new Qt4_W_GroupBox(w);
    break;
  case GWEN_Widget_TypeWidgetStack:
    xw=new Qt4_W_WidgetStack(w);
    break;
  case GWEN_Widget_TypeTextBrowser:
    xw=new Qt4_W_TextBrowser(w);
    break;
  case GWEN_Widget_TypeScrollArea:
    xw=new Qt4_W_ScrollArea(w);
    break;
  case GWEN_Widget_TypeProgressBar:
    xw=new Qt4_W_ProgressBar(w);
    break;
  case GWEN_Widget_TypeListBox:
    xw=new Qt4_W_ListBox(w);
    break;
  case GWEN_Widget_TypeRadioButton:
    xw=new Qt4_W_RadioButton(w);
    break;
  case GWEN_Widget_TypeSpinBox:
    xw=new Qt4_W_SpinBox(w);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled widget type %d (%s)",
	      GWEN_Widget_GetType(w), GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
    break;
  }

  if (xw==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No widget created.");
    return GWEN_ERROR_INTERNAL;
  }

  rv=xw->setup();
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    GWEN_WIDGET *wChild;

    if (GWEN_Widget_GetType(w)==GWEN_Widget_TypeDialog)
      _mainWidget=(QT4_DialogBox*) GWEN_Widget_GetImplData(xw->getCInterface(), QT4_DIALOG_WIDGET_REAL);

    /* handle children */
    wChild=GWEN_Widget_Tree_GetFirstChild(w);
    while(wChild) {
      /* recursion */
      rv=setupTree(wChild);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      wChild=GWEN_Widget_Tree_GetNext(wChild);
    }
  }

  return 0;
}


bool QT4_GuiDialog::setup(QWidget *parentWindow) {
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

  rv=setupTree(w);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    return false;
  }

  _mainWidget=(QT4_DialogBox*) GWEN_Widget_GetImplData(w, QT4_DIALOG_WIDGET_REAL);

  rv=GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeInit, "");
  if (rv<0) {
    DBG_INFO(0, "Error initializing dialog: %d", rv);
    return false;
  }


  return true;
}





