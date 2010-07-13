/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "fox16_gui_dialog_l.hpp"
#include "fox16_gui_sortinglist_l.hpp"
#include "fox16_htmllabel.hpp"
#include "fox16_htmltext.hpp"

#include "theme.h"

#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/debug.h>

#include <list>
#include <string>
#include <assert.h>

#include <fxkeys.h>


#define FOX16_DIALOG_WIDGET_REAL    0
#define FOX16_DIALOG_WIDGET_CONTENT 1

#define FOX16_DIALOG_STRING_TITLE  0
#define FOX16_DIALOG_STRING_VALUE  1





FXDEFMAP(FOX16_GuiDialog) FOX16_GuiDialogMap[]={
  FXMAPFUNCS(SEL_COMMAND, FOX16_GuiDialog::ID_WIDGET_FIRST, FOX16_GuiDialog::ID_WIDGET_LAST, FOX16_GuiDialog::onSelCommand),
  FXMAPFUNCS(SEL_CHANGED, FOX16_GuiDialog::ID_WIDGET_FIRST, FOX16_GuiDialog::ID_WIDGET_LAST, FOX16_GuiDialog::onSelChanged),
  FXMAPFUNCS(SEL_KEYPRESS, FOX16_GuiDialog::ID_WIDGET_FIRST, FOX16_GuiDialog::ID_WIDGET_LAST, FOX16_GuiDialog::onSelKeyPress),
  FXMAPFUNCS(SEL_KEYRELEASE, FOX16_GuiDialog::ID_WIDGET_FIRST, FOX16_GuiDialog::ID_WIDGET_LAST, FOX16_GuiDialog::onSelKeyRelease)
};


FXIMPLEMENT(FOX16_GuiDialog, FXObject, FOX16_GuiDialogMap, ARRAYNUMBER(FOX16_GuiDialogMap))



FOX16_GuiDialog::FOX16_GuiDialog()
:FXObject()
,CppDialog()
,_gui(NULL)
,_widgetCount(0)
,_mainWidget(NULL)
,m_iconSource(NULL)
{
}



FOX16_GuiDialog::FOX16_GuiDialog(FOX16_Gui *gui, GWEN_DIALOG *dlg)
:FXObject()
,CppDialog(dlg)
,_gui(gui)
,_widgetCount(0)
,_mainWidget(NULL)
,m_iconSource(NULL)
{
}



FOX16_GuiDialog::~FOX16_GuiDialog() {
  if (_mainWidget)
    delete _mainWidget;
  if (!m_iconList.empty()) {
    std::list<FXIcon*>::iterator it;

    for (it=m_iconList.begin(); it!=m_iconList.end(); it++)
      delete *it;
    m_iconList.clear();
  }
  if (m_iconSource)
    delete m_iconSource;

  if (!m_radioGroups.empty()) {
    std::list<RadioButtonGroup*>::iterator it;

    for (it=m_radioGroups.begin(); it!=m_radioGroups.end(); it++)
      delete *it;
  }
}



FOX16_GuiDialog *FOX16_GuiDialog::getDialog(GWEN_DIALOG *dlg) {
  CppDialog *cppDlg;

  cppDlg=CppDialog::getDialog(dlg);
  if (cppDlg)
    return dynamic_cast<FOX16_GuiDialog*>(cppDlg);
  return NULL;
}



FXIcon *FOX16_GuiDialog::getIcon(const char *fileName) {
  GWEN_STRINGLIST *sl;

  sl=GWEN_Dialog_GetMediaPaths(_dialog);
  if (sl) {
    GWEN_BUFFER *tbuf;
    int rv;
    FXIcon *ic;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Directory_FindFileInPaths(sl, fileName, tbuf);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return NULL;
    }

    if (m_iconSource==NULL)
      m_iconSource=new FXIconSource(FXApp::instance());

    DBG_ERROR(0, "Loading [%s]", GWEN_Buffer_GetStart(tbuf));
    ic=m_iconSource->loadIconFile(GWEN_Buffer_GetStart(tbuf));
    if (ic==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not load icon [%s]", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      return NULL;
    }
    m_iconList.push_back(ic);
    return ic;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No media paths in dialog");
    return NULL;
  }
}



int FOX16_GuiDialog::execute() {
  FXDialogBox *dialogBox;
  int rv;

  dialogBox=_mainWidget;

  /* execute dialog */
  dialogBox->show(PLACEMENT_OWNER);
  rv=dialogBox->execute();
  GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeFini, "");

  if (rv==0) {
    /* aborted */
    return 0;
  }
  else
    /* accepted */
    return 1;
}


int FOX16_GuiDialog::setIntProperty(GWEN_WIDGET *w,
				    GWEN_DIALOG_PROPERTY prop,
				    int index,
				    int value,
				    int doSignal) {
  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
    return GWEN_ERROR_GENERIC;

  case GWEN_Widget_TypeNone:
    return GWEN_ERROR_GENERIC;

  case GWEN_Widget_TypeComboBox:
    {
      THEMECOMBOBOX *f;

      f=(THEMECOMBOBOX*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCurrentItem(value, doSignal?TRUE:FALSE);
	return 0;

      case GWEN_DialogProperty_ClearValues:
	f->clearItems();
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeRadioButton:
    {
      FXRadioButton *f;

      f=(FXRadioButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCheck((value==0)?FALSE:TRUE, doSignal?TRUE:FALSE);
	return 0;

      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypeProgressBar:
    {
      FXProgressBar *f;

      f=(FXProgressBar*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setProgress(value);
        return 0;

      case GWEN_DialogProperty_MinValue:
	if (value!=0) {
	  DBG_ERROR(0, "MinValue should be 0!");
	  return GWEN_ERROR_INVALID;
	}
        return 0;

      case GWEN_DialogProperty_MaxValue:
	f->setTotal(value);
        return 0;

      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      FOX16_GuiSortingList *f;
      FXFoldingItem *fi;
      int i=0;

      f=(FOX16_GuiSortingList*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	fi=f->getFirstItem();
	while(fi && i<value) {
	  fi=fi->getNext();
	  i++;
	}
	if (fi && i==value)
	  f->setCurrentItem(fi, doSignal?TRUE:FALSE);
	else {
	  DBG_ERROR(0, "Value %d out of range", value);
	  return GWEN_ERROR_INVALID;
	}
        return 0;

      case GWEN_DialogProperty_ColumnWidth:
	f->setHeaderSize(index, value);
	return 0;

      case GWEN_DialogProperty_SelectionMode:
	switch(value) {
	case GWEN_Dialog_SelectionMode_None:
          /* simply fall-through */
	case GWEN_Dialog_SelectionMode_Single:
	  f->setListStyle(FOLDINGLIST_BROWSESELECT);
          return 0;
	case GWEN_Dialog_SelectionMode_Multi:
	  f->setListStyle(FOLDINGLIST_EXTENDEDSELECT);
	  return 0;
	  ;
	}
	DBG_ERROR(0, "Unknown SelectionMode %d", value);
	return GWEN_ERROR_INVALID;

      case GWEN_DialogProperty_SelectionState:
	{
	  FXFoldingItem *ti;

	  ti=f->getItem(index);
	  if (ti==NULL) {
	    DBG_ERROR(0, "Index %d out of range", index);
            return GWEN_ERROR_INVALID;
	  }
	  ti->setSelected((value==0)?FALSE:TRUE);
	  return 0;
	}

      case GWEN_DialogProperty_ClearValues:
	f->clearItems();
	return 0;

      case GWEN_DialogProperty_SortDirection:
	{
	  int i;

	  for (i=0; i<f->getNumHeaders(); i++) {
	    if (i==index) {
	      switch(value) {
	      case GWEN_DialogSortDirection_None:
		f->setHeaderArrowDir(i, MAYBE);
		break;
	      case GWEN_DialogSortDirection_Up:
		f->setHeaderArrowDir(i, TRUE);
		break;
	      case GWEN_DialogSortDirection_Down:
		f->setHeaderArrowDir(i, FALSE);
		break;
	      }
	    }
	    else
	      f->setHeaderArrowDir(i, MAYBE);
	  }

	  switch(value) {
	  case GWEN_DialogSortDirection_None:
	    break;
	  case GWEN_DialogSortDirection_Up:
	    f->sortByColumn(i, true);
	    break;
	  case GWEN_DialogSortDirection_Down:
	    f->sortByColumn(i, false);
	    break;
	  }
	}
	return 0;

      case GWEN_DialogProperty_Sort:
	{
	  int i;

	  for (i=0; i<f->getNumHeaders(); i++) {
	    FXbool b;

	    b=f->getHeaderArrowDir(i);
	    if (b!=MAYBE) {
	      if (b==TRUE)
		f->sortByColumn(i, true);
              else
		f->sortByColumn(i, false);
              break;
	    }
	  }
	}
	return 0;


      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      FXCheckButton *f;

      f=(FXCheckButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCheck((value==0)?FALSE:TRUE, doSignal?TRUE:FALSE);
	return 0;

      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypeTabBook:
    {
      FXTabBook *f;

      f=(FXTabBook*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCurrent(value, doSignal?TRUE:FALSE);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeWidgetStack:
    {
      FXSwitcher *f;

      f=(FXSwitcher*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setCurrent(value, doSignal?TRUE:FALSE);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeDialog:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeTextBrowser:
    ;
  }

  {
    FXWindow *f;

    f=(FXWindow*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
    assert(f);

    switch(prop) {
    case GWEN_DialogProperty_Width:
      f->recalc();
      f->resize(value, f->getHeight());
      return 0;
    case GWEN_DialogProperty_Height:
      f->recalc();
      f->resize(f->getWidth(), value);
      return 0;
    case GWEN_DialogProperty_Enabled:
      if (value==0)
	f->disable();
      else
	f->enable();
      return 0;

    case GWEN_DialogProperty_Focus:
      f->setFocus();
      return 0;

    case GWEN_DialogProperty_Visibility:
      if (value==0) {
	f->hide();
        f->recalc();
      }
      else {
	f->show();
	f->recalc();
      }
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
      ;
    }
  }

  DBG_WARN(0, "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}



int FOX16_GuiDialog::getIntProperty(GWEN_WIDGET *w,
			      GWEN_DIALOG_PROPERTY prop,
			      int index,
			      int defaultValue) {
  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
  case GWEN_Widget_TypeNone:
    return defaultValue;

  case GWEN_Widget_TypeComboBox:
    {
      THEMECOMBOBOX *f;

      f=(THEMECOMBOBOX*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->getCurrentItem();

      case GWEN_DialogProperty_ValueCount:
	return f->getNumItems();

      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypeRadioButton:
    {
      FXRadioButton *f;

      f=(FXRadioButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return (f->getCheck()==TRUE)?1:0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeProgressBar:
    {
      FXProgressBar *f;

      f=(FXProgressBar*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->getProgress();

      case GWEN_DialogProperty_MinValue:
        return 0;

      case GWEN_DialogProperty_MaxValue:
	return f->getTotal();

      default:
        break;
      }

      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      FOX16_GuiSortingList *f;
      FXFoldingItem *fi;
      int i=0;

      f=(FOX16_GuiSortingList*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	fi=f->getCurrentItem();
	if (fi==NULL)
	  return defaultValue;
	else {
	  FXFoldingItem *ti;

          ti=fi;
	  while( (ti=ti->getPrev()) )
	    i++;

          return i;
	}

      case GWEN_DialogProperty_ValueCount:
        return f->getNumItems();

      case GWEN_DialogProperty_ColumnWidth:
	return f->getHeaderSize(index);

      case GWEN_DialogProperty_SelectionMode:
	{
	  switch(f->getListStyle()) {
	  case FOLDINGLIST_BROWSESELECT:
            return GWEN_Dialog_SelectionMode_Single;
	  case FOLDINGLIST_EXTENDEDSELECT:
	    return GWEN_Dialog_SelectionMode_Multi;
	  default:
	    return GWEN_Dialog_SelectionMode_None;
	  }
          break;
	}

      case GWEN_DialogProperty_SelectionState:
	{
	  FXFoldingItem *ti;
          int i=index;

	  ti=f->getFirstItem();
	  while(ti && i) {
	    ti=ti->getNext();
            i--;
	  }

	  if (ti)
	    return (ti->isSelected()==TRUE)?1:0;
          return defaultValue;
	}

      case GWEN_DialogProperty_SortDirection:
	{
	  int i;

	  for (i=0; i<f->getNumHeaders(); i++) {
	    if (i==index) {
	      FXbool b;

	      b=f->getHeaderArrowDir(i);
	      if (b==MAYBE)
		return GWEN_DialogSortDirection_None;
	      else if (b==TRUE)
                return GWEN_DialogSortDirection_Up;
              else
		return GWEN_DialogSortDirection_Down;
	    }
	  }
	  DBG_ERROR(0, "Column %d out of range", index);
	}
	return defaultValue;

      default:
	break;
      }

      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      FXCheckButton *f;

      f=(FXCheckButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return (f->getCheck()==TRUE)?1:0;

      default:
        break;
      }

      break;
    }

  case GWEN_Widget_TypeTabBook:
    {
      FXTabBook *f;

      f=(FXTabBook*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->getCurrent();

      default:
        break;
      }

      break;
    }

  case GWEN_Widget_TypeWidgetStack:
    {
      FXSwitcher *f;

      f=(FXSwitcher*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	return f->getCurrent();

      default:
	break;
      }

      break;
    }

  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeDialog:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeTextBrowser:
    break;
  }

  /* generic properties every widget has */
  {
    FXWindow *f;

    f=(FXWindow*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
    assert(f);

    switch(prop) {
    case GWEN_DialogProperty_Width:
      return f->getWidth();

    case GWEN_DialogProperty_Height:
      return f->getHeight();

    case GWEN_DialogProperty_Enabled:
      return (f->isEnabled()==TRUE)?1:0;

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
    case GWEN_DialogProperty_Visibility:
    case GWEN_DialogProperty_None:
    case GWEN_DialogProperty_Unknown:
      ;
    }
  }


  DBG_WARN(0, "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



int FOX16_GuiDialog::setCharProperty(GWEN_WIDGET *w,
				     GWEN_DIALOG_PROPERTY prop,
				     int index,
				     const char *value,
				     int doSignal) {

  FXString strValue;
  FXString htmlValue;

  if (value && *value) {
    strValue=FOX16_Gui::getRawText(value);
    htmlValue=FOX16_Gui::getHtmlText(value);
  }

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
    return GWEN_ERROR_GENERIC;
  case GWEN_Widget_TypeNone:
    return GWEN_ERROR_GENERIC;

  case GWEN_Widget_TypeLabel:
    {
      FOX16_HtmlLabel *f;

      f=(FOX16_HtmlLabel*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setText(htmlValue);
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTextBrowser:
    {
      FOX16_HtmlText *f;

      f=(FOX16_HtmlText*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setText(htmlValue);
        f->makePositionVisible(strValue.length());
	return 0;

      case GWEN_DialogProperty_AddValue:
	f->setText(f->getText()+htmlValue);
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
      THEMEBUTTON *f;

      f=(THEMEBUTTON*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f->setText(value);
	return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeLineEdit:
    {
      FXTextField *f;

      f=(FXTextField*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setText(value, doSignal?TRUE:FALSE);
        return 0;
      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypeTextEdit:
    {
      FXText *f;

      f=(FXText*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
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

  case GWEN_Widget_TypeComboBox:
    {
      THEMECOMBOBOX *f;

      f=(THEMECOMBOBOX*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	f->setText(strValue);
        return 0;

      case GWEN_DialogProperty_AddValue:
	{
	  int i;

	  f->appendItem(strValue);
	  i=f->getNumItems();
	  if (i>10)
	    i=10;
	  f->setNumVisible(i);
	  return 0;
	}

      case GWEN_DialogProperty_ClearValues:
	f->clearItems();
        return 0;

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeGroupBox:
    {
      FXGroupBox *f;

      f=(FXGroupBox*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
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
      FXRadioButton *f;

      f=(FXRadioButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
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

  case GWEN_Widget_TypeCheckBox:
    {
      FXCheckButton *f;

      f=(FXCheckButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
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

  case GWEN_Widget_TypeTabPage:
    {
      FXWindow *f1;
      THEMETABITEM *f2;

      f1=(FXWindow*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f1);
      f2=(THEMETABITEM*) (f1->getPrev());
      assert(f2);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	f2->setText(strValue);
	return 0;
      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeDialog:
    {
      FXDialogBox *f;

      f=(FXDialogBox*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
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

  case GWEN_Widget_TypeListBox:
    {
      FXFoldingList *f;
      FXString str;
      FXString t;
      FXint n=0;

      f=(FXFoldingList*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
        f->getHeader()->clearItems();
	str=strValue;
	while(!(t=str.section('\t',n)).empty()){
	  f->appendHeader(t, NULL, 20);
	  n++;
	}
	return 0;

      case GWEN_DialogProperty_AddValue:
	f->appendItem(NULL, strValue);
	return 0;

      case GWEN_DialogProperty_ClearValues:
	f->clearItems();
        return 0;

      default:
        break;
      }
      break;
    }


  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
    ;
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
  case GWEN_DialogProperty_Visibility:
  case GWEN_DialogProperty_None:
  case GWEN_DialogProperty_Unknown:
    break;
  }

  DBG_WARN(0, "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}



const char *FOX16_GuiDialog::getCharProperty(GWEN_WIDGET *w,
					     GWEN_DIALOG_PROPERTY prop,
					     int index,
					     const char *defaultValue) {
  FXString str;

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
    return defaultValue;
  case GWEN_Widget_TypeNone:
    return defaultValue;

  case GWEN_Widget_TypeLabel:
    {
      FOX16_HtmlLabel *f;

      f=(FOX16_HtmlLabel*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypePushButton:
    {
      THEMEBUTTON *f;

      f=(THEMEBUTTON*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeLineEdit:
    {
      FXTextField *f;

      f=(FXTextField*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_VALUE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTextEdit:
    {
      FXText *f;

      f=(FXText*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_VALUE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeTextBrowser:
    {
      FOX16_HtmlText *f;

      f=(FOX16_HtmlText*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_VALUE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeComboBox:
    {
      THEMECOMBOBOX *f;

      f=(THEMECOMBOBOX*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Value:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_VALUE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_VALUE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeGroupBox:
    {
      FXGroupBox *f;

      f=(FXGroupBox*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeRadioButton:
    {
      FXRadioButton *f;

      f=(FXRadioButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeCheckBox:
    {
      FXCheckButton *f;

      f=(FXCheckButton*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      default:
	break;
      }
      break;
    }

  case GWEN_Widget_TypeTabPage:
    {
      FXWindow *f1;
      THEMETABITEM *f2;

      f1=(FXWindow*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f1);
      f2=(THEMETABITEM*) (f1->getPrev());
      assert(f2);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f2->getText();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }

      break;
    }

  case GWEN_Widget_TypeDialog:
    {
      FXDialogBox *f;

      f=(FXDialogBox*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	str=f->getTitle();
	if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      default:
        break;
      }
      break;
    }

  case GWEN_Widget_TypeListBox:
    {
      FXFoldingList *f;
      FXHeader *fh;
      FXFoldingItem *fi;

      f=(FXFoldingList*)GWEN_Widget_GetImplData(w, FOX16_DIALOG_WIDGET_REAL);
      assert(f);

      switch(prop) {
      case GWEN_DialogProperty_Title:
	fh=f->getHeader();
	if (fh) {
	  int i;

	  for (i=0; i<fh->getNumItems(); i++) {
	    if (!str.empty())
	      str+="\t";
	    str+=fh->getItemText(i);
	  }
	}

        if (str.empty())
	  return defaultValue;
	else {
	  GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_TITLE, str.text());
	  return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_TITLE);
	}

      case GWEN_DialogProperty_Value:
        DBG_ERROR(0, "GetCharProperty %d", index);
	fi=f->getFirstItem();
	if (fi) {
	  int i=index;

	  while(fi && i>0) {
	    fi=fi->getNext();
            i--;
	  }
	  if (fi) {
	    str=fi->getText();
	    if (str.empty())
              return defaultValue;
	    GWEN_Widget_SetText(w, FOX16_DIALOG_STRING_VALUE, str.text());
	    return GWEN_Widget_GetText(w, FOX16_DIALOG_STRING_VALUE);
	  }
	  else {
	    DBG_ERROR(0, "Index %d out of range", index);
	    return defaultValue;
	  }
	}
	else {
	  DBG_ERROR(0, "Empty list");
          return defaultValue;
	}

      default:
	break;
      }
      break;
    }


  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
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
  case GWEN_DialogProperty_Visibility:
  case GWEN_DialogProperty_None:
  case GWEN_DialogProperty_Unknown:
    break;
  }


  DBG_WARN(0, "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}





long FOX16_GuiDialog::onSelCommand(FXObject *sender, FXSelector sel, void *ptr) {
  GWEN_WIDGET *w;
  const char *wname;
  int rv=GWEN_DialogEvent_ResultNotHandled;
  FXDialogBox *dialogBox;

  w=GWEN_Dialog_FindWidgetByImplData(_dialog, FOX16_DIALOG_WIDGET_REAL, sender);
  if (w==NULL) {
    if (!m_radioGroups.empty()) {
      std::list<RadioButtonGroup*>::iterator it;
      RadioButtonGroup *grp=NULL;

      for (it=m_radioGroups.begin(); it!=m_radioGroups.end(); it++) {
	if ((*it)->getDataTarget()==sender) {
	  grp=*it;
          break;
	}
      }

      if (grp==NULL) {
	DBG_WARN(GWEN_LOGDOMAIN, "Widget or RadioButtonGroup not found");
	return 0;
      }
      else {
	DBG_INFO(0, "Found button group %d: %d", grp->getGroupId(), grp->getRadioValue());
	// no signal for now
        return 1;
      }
    }
  }
  wname=GWEN_Widget_GetName(w);

  dialogBox=_mainWidget;

  DBG_ERROR(0, "Command for [%s] (type: %s)",
	    wname?wname:"(unnamed)",
	    GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
    return GWEN_ERROR_GENERIC;

  case GWEN_Widget_TypeNone:
    return GWEN_ERROR_GENERIC;

  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeComboBox:
  case GWEN_Widget_TypeListBox:
  case GWEN_Widget_TypeCheckBox:
  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypeTextEdit:
    rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
			      GWEN_DialogEvent_TypeActivated,
			      GWEN_Widget_GetName(w));
    break;

  case GWEN_Widget_TypeRadioButton: /* use SEL_UPDATED for FXRadioButton */
  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
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
    dialogBox->getApp()->stopModal(dialogBox, 1);
  }
  else if (rv==GWEN_DialogEvent_ResultReject) {
    dialogBox->getApp()->stopModal(dialogBox, 0);
  }

  return 1;
}



long FOX16_GuiDialog::onSelChanged(FXObject *sender, FXSelector sel, void *ptr) {
  GWEN_WIDGET *w;
  int rv=GWEN_DialogEvent_ResultNotHandled;
  FXDialogBox *dialogBox;

  w=GWEN_Dialog_FindWidgetByImplData(_dialog, FOX16_DIALOG_WIDGET_REAL, sender);
  if (w==NULL) {
    DBG_INFO(0, "Widget not found");
    return 0;
  }

  dialogBox=_mainWidget;

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
    return GWEN_ERROR_GENERIC;
  case GWEN_Widget_TypeNone:
    return GWEN_ERROR_GENERIC;
  case GWEN_Widget_TypeLineEdit:
    rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
			      GWEN_DialogEvent_TypeValueChanged,
			      GWEN_Widget_GetName(w));
    break;

  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeComboBox:
  case GWEN_Widget_TypeRadioButton:
  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeListBox:
  case GWEN_Widget_TypeDialog:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeCheckBox:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeTextBrowser:
    ;
  }

  if (rv==GWEN_DialogEvent_ResultAccept) {
    GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeFini, "");
    dialogBox->getApp()->stopModal(dialogBox, 1);
  }
  else if (rv==GWEN_DialogEvent_ResultReject) {
    GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeFini, "");
    dialogBox->getApp()->stopModal(dialogBox, 0);
  }

  return 1;
}



long FOX16_GuiDialog::onSelKeyPress(FXObject *sender, FXSelector sel, void *ptr) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByImplData(_dialog, FOX16_DIALOG_WIDGET_REAL, sender);
  if (w==NULL) {
    DBG_INFO(0, "Widget not found");
    return 0;
  }

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeUnknown:
  case GWEN_Widget_TypeNone:
    return 0;

  case GWEN_Widget_TypeDialog:
    /* catch ENTER key */
    if (((FXEvent*)ptr)->code==KEY_Return || ((FXEvent*)ptr)->code==KEY_KP_Enter) {
      return 1;
    }
    return 0;

  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeComboBox:
  case GWEN_Widget_TypeRadioButton:
  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeListBox:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeCheckBox:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeTextBrowser:
    ;
  }

  return 0;
}



long FOX16_GuiDialog::onSelKeyRelease(FXObject *sender, FXSelector sel, void *ptr) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByImplData(_dialog, FOX16_DIALOG_WIDGET_REAL, sender);
  if (w==NULL) {
    DBG_INFO(0, "Widget not found");
    return 0;
  }

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeDialog:
    /* catch ENTER key */
    if (((FXEvent*)ptr)->code==KEY_Return || ((FXEvent*)ptr)->code==KEY_KP_Enter) {
      return 1;
    }
    return 0;

  case GWEN_Widget_TypeUnknown:
  case GWEN_Widget_TypeNone:
  case GWEN_Widget_TypeLabel:
  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeComboBox:
  case GWEN_Widget_TypeRadioButton:
  case GWEN_Widget_TypeProgressBar:
  case GWEN_Widget_TypeGroupBox:
  case GWEN_Widget_TypeHSpacer:
  case GWEN_Widget_TypeVSpacer:
  case GWEN_Widget_TypeHLayout:
  case GWEN_Widget_TypeVLayout:
  case GWEN_Widget_TypeGridLayout:
  case GWEN_Widget_TypeListBox:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeCheckBox:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeTextBrowser:
    ;
  }

  return 0;
}



bool FOX16_GuiDialog::setup(FXWindow *parentWindow) {
  FXWindow *xw;
  GWEN_WIDGET_TREE *wtree;
  GWEN_WIDGET *w;
  int rv;

  wtree=GWEN_Dialog_GetWidgets(_dialog);
  if (wtree==NULL) {
    DBG_ERROR(0, "No widget tree in dialog");
    return false;
  }
  w=GWEN_Widget_Tree_GetFirst(wtree);
  if (w==NULL) {
    DBG_ERROR(0, "No widgets in dialog");
    return false;
  }

  xw=setupTree(parentWindow, w);
  if (xw==NULL) {
    DBG_INFO(0, "here");
    return false;
  }

  _mainWidget=dynamic_cast<FXDialogBox*>(xw);
  assert(_mainWidget);

  rv=GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeInit, "");
  if (rv<0) {
    DBG_INFO(0, "Error initializing dialog: %d", rv);
    return false;
  }

  /* create X11 server side resources */
  xw->create();

  xw->layout();

  return true;
}




FXWindow *FOX16_GuiDialog::setupTree(FXWindow *parentWindow, GWEN_WIDGET *w) {
  const char *s;
  const char *name;
  FXuint opts=0;
  uint32_t flags;
  FXString text;
  FXString htmlText;
  FXComposite *parentComposite=NULL;
  FXWindow *wChild=NULL;
  FXWindow *wContent=NULL;
  GWEN_WIDGET *parentWidget;
  int cols;
  int rows;

  /* sample data */
  flags=GWEN_Widget_GetFlags(w);
  s=GWEN_Widget_GetText(w, 0);
  if (s) {
    text=FXString(s);
    htmlText=FOX16_Gui::getHtmlText(s);
  }
  name=GWEN_Widget_GetName(w);
  cols=GWEN_Widget_GetColumns(w);
  rows=GWEN_Widget_GetRows(w);

  parentWidget=GWEN_Widget_Tree_GetParent(w);

  /* check for parent type */
  if (parentWindow)
    parentComposite=dynamic_cast<FXComposite*>(parentWindow);
  if (parentComposite==NULL) {
    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeDialog:
      /* these types don't need the parent to be a FXComposite */
      break;
    default:
      DBG_ERROR(0, "Parent of widget [%s] (type %d) is not a composite",
		name?name:"(unnamed)", GWEN_Widget_GetType(w));
      return NULL;
    }
  }

  /* setup info for new widget */
  if (flags & GWEN_WIDGET_FLAGS_FILLX)
    opts|=LAYOUT_FILL_X | LAYOUT_FILL_COLUMN;
  if (flags & GWEN_WIDGET_FLAGS_FILLY)
    opts|=LAYOUT_FILL_Y;
  if (flags & GWEN_WIDGET_FLAGS_DECOR_SHRINKABLE)
    opts|=DECOR_SHRINKABLE;
  if (flags & GWEN_WIDGET_FLAGS_DECOR_STRETCHABLE)
    opts|=DECOR_STRETCHABLE;
  if (flags & GWEN_WIDGET_FLAGS_DECOR_MINIMIZE)
    opts|=DECOR_MINIMIZE;
  if (flags & GWEN_WIDGET_FLAGS_DECOR_MAXIMIZE)
    opts|=DECOR_MAXIMIZE;
  if (flags & GWEN_WIDGET_FLAGS_DECOR_CLOSE)
    opts|=DECOR_CLOSE;
  if (flags & GWEN_WIDGET_FLAGS_DECOR_MENU)
    opts|=DECOR_MENU;
  if (flags & GWEN_WIDGET_FLAGS_EQUAL_WIDTH)
    opts|=PACK_UNIFORM_WIDTH;
  if (flags & GWEN_WIDGET_FLAGS_EQUAL_HEIGHT)
    opts|=PACK_UNIFORM_HEIGHT;
  if (flags & GWEN_WIDGET_FLAGS_JUSTIFY_LEFT)
    opts|=JUSTIFY_LEFT;
  if (flags & GWEN_WIDGET_FLAGS_JUSTIFY_RIGHT)
    opts|=JUSTIFY_RIGHT;
  if (flags & GWEN_WIDGET_FLAGS_JUSTIFY_TOP)
    opts|=JUSTIFY_TOP;
  if (flags & GWEN_WIDGET_FLAGS_JUSTIFY_BOTTOM)
    opts|=JUSTIFY_BOTTOM;
  if (flags & GWEN_WIDGET_FLAGS_JUSTIFY_CENTERX)
    opts|=JUSTIFY_CENTER_X;
  if (flags & GWEN_WIDGET_FLAGS_JUSTIFY_CENTERY)
    opts|=JUSTIFY_CENTER_Y;

  /* create THIS widget */
  switch(GWEN_Widget_GetType(w)) {

  case GWEN_Widget_TypeLabel: {
    FOX16_HtmlLabel *label;
    int wi;
    const char *s;

    if (flags & GWEN_WIDGET_FLAGS_NO_WORDWRAP)
      opts|=FOX16_HtmlLabel::FLAGS_NO_WORDWRAP;
    label=new FOX16_HtmlLabel(parentComposite,
			      htmlText,
			      opts);
    s=GWEN_Widget_GetIconFileName(w);
    if (s && *s) {
      FXIcon *ic;

      ic=getIcon(s);
      if (ic)
	label->setIcon(ic);
    }

    wi=GWEN_Widget_GetWidth(w);
    if (wi>0)
      label->setMaxDefaultWidth(wi);
    wChild=label;
    break;
  }

  case GWEN_Widget_TypePushButton: {
    const char *s;
    FXIcon *ic=NULL;

    if (flags & GWEN_WIDGET_FLAGS_DEFAULT_WIDGET)
      opts|=BUTTON_DEFAULT | BUTTON_INITIAL | BUTTON_NORMAL;
    else
      opts|=BUTTON_NORMAL;
    s=GWEN_Widget_GetIconFileName(w);
    if (s && *s)
      ic=getIcon(s);

    wChild=new THEMEBUTTON(parentComposite,
			   text,
			   ic,  /* icon */
			   this,
			   ID_WIDGET_FIRST+_widgetCount,
			   opts);
    break;
  }

  case GWEN_Widget_TypeLineEdit:
    if (flags & GWEN_WIDGET_FLAGS_PASSWORD)
      opts|=TEXTFIELD_PASSWD;
    if (flags & GWEN_WIDGET_FLAGS_READONLY)
      opts|=TEXTFIELD_READONLY;
    DBG_ERROR(0, "TextField with %d columns", cols);
    wChild=new FXTextField(parentComposite,
			   cols?cols:16,
			   this,
			   ID_WIDGET_FIRST+_widgetCount,
			   opts | TEXTFIELD_NORMAL | TEXTFIELD_ENTER_ONLY);
    break;

  case GWEN_Widget_TypeTextEdit:
    {
      FXText *f;

      if (flags & GWEN_WIDGET_FLAGS_READONLY)
	opts|=TEXT_READONLY;
      f=new FXText(parentComposite,
		   this,
		   ID_WIDGET_FIRST+_widgetCount,
		   opts | HSCROLLING_OFF);
      if (cols)
        f->setVisibleColumns(cols);
      if (rows)
        f->setVisibleRows(rows);
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeTextBrowser:
    {
      FOX16_HtmlText *f;

      f=new FOX16_HtmlText(parentComposite, "",
			   opts | HSCROLLING_OFF);
      wChild=f;
      break;
    }

  case GWEN_Widget_TypeComboBox:
    if (flags & GWEN_WIDGET_FLAGS_READONLY)
      opts|=COMBOBOX_STATIC;
    wChild=new THEMECOMBOBOX(parentComposite,
			     cols?cols:16,
			     this,
			     ID_WIDGET_FIRST+_widgetCount,
			     opts);
    break;

  case GWEN_Widget_TypeRadioButton: {
    FXRadioButton *rb;
    int groupId;
    RadioButtonGroup *grp=NULL;
    std::list<RadioButtonGroup*>::iterator it;

    groupId=GWEN_Widget_GetGroupId(w);


    for (it=m_radioGroups.begin(); it!=m_radioGroups.end(); it++) {
      if ((*it)->getGroupId()==groupId) {
        grp=*it;
      }
    }
    if (grp==NULL) {
      grp=new RadioButtonGroup(groupId, this, ID_WIDGET_FIRST+_widgetCount);
      m_radioGroups.push_back(grp);
    }

    rb=new FXRadioButton(parentComposite,
			 text,
			 grp->getDataTarget(),
			 FXDataTarget::ID_OPTION+grp->getButtonCount(),
			 opts | RADIOBUTTON_NORMAL);
    grp->addButton(rb);
    wChild=rb;
    break;
  }

  case GWEN_Widget_TypeProgressBar:
    wChild=new FXProgressBar(parentComposite,
			     this,
			     ID_WIDGET_FIRST+_widgetCount,
			     opts | PROGRESSBAR_NORMAL | PROGRESSBAR_PERCENTAGE);
    break;

  case GWEN_Widget_TypeGroupBox:
    wChild=new FXGroupBox(parentComposite,
			  text,
			  opts | GROUPBOX_NORMAL | FRAME_LINE);
    break;

  case GWEN_Widget_TypeHSpacer:
    wChild=new FXSpring(parentComposite, opts | LAYOUT_FILL_X);
    break;

  case GWEN_Widget_TypeVSpacer:
    wChild=new FXSpring(parentComposite, opts | LAYOUT_FILL_Y);
    break;

  case GWEN_Widget_TypeHLayout:
    wChild=new FXHorizontalFrame(parentComposite, opts,
				 0, 0, 0, 0, 0, 0, 0, 0);
    break;

  case GWEN_Widget_TypeVLayout:
    wChild=new FXVerticalFrame(parentComposite, opts,
			       0, 0, 0, 0, 0, 0, 0, 0);
    break;

  case GWEN_Widget_TypeGridLayout:
    DBG_ERROR(0, "GridLayout: %d cols, %d rows", cols, rows);
    if (cols & rows) {
      DBG_ERROR(0, "State columns *or* rows, not both in widget [%s]",
		name?name:"(unnamed)");
      return NULL;
    }
    if (cols)
      wChild=new FXMatrix(parentComposite, cols,
			  opts | MATRIX_BY_COLUMNS,
			  0, 0, 0, 0, 0, 0, 0, 0);
    else
      wChild=new FXMatrix(parentComposite, rows,
			  opts | MATRIX_BY_ROWS,
			  0, 0, 0, 0, 0, 0, 0, 0);
    break;

  case GWEN_Widget_TypeListBox:
    wChild=new FOX16_GuiSortingList(parentComposite,
				    this,
				    ID_WIDGET_FIRST+_widgetCount,
				    opts | FRAME_SUNKEN|FRAME_THICK | LISTBOX_NORMAL);
    break;

  case GWEN_Widget_TypeDialog:
    if (parentWindow)
      wChild=new FXDialogBox(parentWindow,
			     name?FXString(name):FXString(""),
			     opts | DECOR_TITLE | DECOR_BORDER);
    else
      wChild=new FXDialogBox(FXApp::instance(),
			     name?FXString(name):FXString(""),
			     opts | DECOR_TITLE | DECOR_BORDER);
    break;

  case GWEN_Widget_TypeTabBook:
    wChild=new FXTabBook(parentComposite,
			 this,
			 ID_WIDGET_FIRST+_widgetCount,
			 opts | TABBOOK_NORMAL);
    break;

  case GWEN_Widget_TypeTabPage:
    if (parentWidget==NULL) {
      DBG_ERROR(0, "Widget [%s] has no parent", name?name:"(unnamed)");
      return NULL;
    }
    else {
      FXTabBook *tbook=dynamic_cast<FXTabBook*>(parentWindow);
      if (tbook==NULL) {
	DBG_ERROR(0, "Parent of widget [%s] needs to be of type TabBook", name?name:"(unnamed)");
	return NULL;
      }

      new THEMETABITEM(tbook, text, NULL, opts | TAB_TOP_NORMAL);
      wChild=new FXVerticalFrame(tbook, opts);
    }
    break;

  case GWEN_Widget_TypeCheckBox:
    wChild=new FXCheckButton(parentComposite,
			     text,
			     this,
			     ID_WIDGET_FIRST+_widgetCount,
			     opts | CHECKBUTTON_NORMAL);
    break;

  case GWEN_Widget_TypeScrollArea:
    {
      FXScrollWindow *f;

      f=new FXScrollWindow(parentComposite, opts);
      wChild=f;
      wContent=f->contentWindow();
      break;
    }

  case GWEN_Widget_TypeWidgetStack:
    wChild=new FXSwitcher(parentComposite, opts);
    break;

  case GWEN_Widget_TypeHLine:
    wChild=new FXHorizontalSeparator(parentComposite, opts | SEPARATOR_GROOVE);
    break;

  case GWEN_Widget_TypeVLine:
    wChild=new FXVerticalSeparator(parentComposite, opts | SEPARATOR_GROOVE);
    break;

  case GWEN_Widget_TypeUnknown:
    DBG_ERROR(0, "Widget [%s] is of type \'unknown\'", name?name:"(unnamed)");
    return NULL;
  case GWEN_Widget_TypeNone:
    DBG_ERROR(0, "Widget [%s] is of type \'none\'", name?name:"(unnamed)");
    return NULL;
  }

  assert(wChild);
  _widgetCount++;

  if (wContent==NULL)
    wContent=wChild;

  GWEN_Widget_SetImplData(w, FOX16_DIALOG_WIDGET_REAL, (void*) wChild);
  GWEN_Widget_SetImplData(w, FOX16_DIALOG_WIDGET_CONTENT, (void*) wContent);

  /* handle children */
  w=GWEN_Widget_Tree_GetFirstChild(w);
  while(w) {
    if (NULL==setupTree(wContent, w))
      return NULL;
    w=GWEN_Widget_Tree_GetNext(w);
  }

  return wChild;
}



int FOX16_GuiDialog::cont() {
  FXDialogBox *dialogBox;

  dialogBox=_mainWidget;
  return dialogBox->getApp()->runModalFor(dialogBox);
}



int FOX16_GuiDialog::openDialog() {
  FXDialogBox *dialogBox;

  dialogBox=_mainWidget;

  /* show dialog */
  DBG_ERROR(0, "Showing...");
  dialogBox->layout();
  dialogBox->show(PLACEMENT_OWNER);

  return 0;
}



int FOX16_GuiDialog::closeDialog() {
  FXDialogBox *dialogBox;

  dialogBox=_mainWidget;

  /* let dialog write its settings */
  GWEN_Dialog_EmitSignalToAll(_dialog, GWEN_DialogEvent_TypeFini, "");

  /* hide dialog */
  dialogBox->hide();
  delete _mainWidget;
  _mainWidget=NULL;

  return 0;
}



