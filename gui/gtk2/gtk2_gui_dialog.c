/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gtk2_gui_dialog_p.h"
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)


#define GTK2_DIALOG_WIDGET_REAL    0
#define GTK2_DIALOG_WIDGET_CONTENT 1

#define GTK2_DIALOG_STRING_TITLE  0
#define GTK2_DIALOG_STRING_VALUE  1


#include "w_label.c"



GWEN_INHERIT(GWEN_DIALOG, GTK2_GUI_DIALOG)




void Gtk2Gui_Dialog_Extend(GWEN_DIALOG *dlg) {
  GTK2_GUI_DIALOG *xdlg;

  GWEN_NEW_OBJECT(GTK2_GUI_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, GTK2_GUI_DIALOG, dlg, xdlg, Gtk2Gui_Dialog_FreeData);

  /* set virtual functions */
  xdlg->setIntPropertyFn=GWEN_Dialog_SetSetIntPropertyFn(dlg, Gtk2Gui_Dialog_SetIntProperty);
  xdlg->getIntPropertyFn=GWEN_Dialog_SetGetIntPropertyFn(dlg, Gtk2Gui_Dialog_GetIntProperty);
  xdlg->setCharPropertyFn=GWEN_Dialog_SetSetCharPropertyFn(dlg, Gtk2Gui_Dialog_SetCharProperty);
  xdlg->getCharPropertyFn=GWEN_Dialog_SetGetCharPropertyFn(dlg, Gtk2Gui_Dialog_GetCharProperty);

}



void Gtk2Gui_Dialog_Unextend(GWEN_DIALOG *dlg){
  GTK2_GUI_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK2_GUI_DIALOG, dlg);
  assert(xdlg);

  /* reset virtual functions */
  GWEN_Dialog_SetSetIntPropertyFn(dlg, xdlg->setIntPropertyFn);
  GWEN_Dialog_SetGetIntPropertyFn(dlg, xdlg->getIntPropertyFn);
  GWEN_Dialog_SetSetCharPropertyFn(dlg, xdlg->setCharPropertyFn);
  GWEN_Dialog_SetGetCharPropertyFn(dlg, xdlg->getCharPropertyFn);

  GWEN_INHERIT_UNLINK(GWEN_DIALOG, GTK2_GUI_DIALOG, dlg);
}



void GWENHYWFAR_CB Gtk2Gui_Dialog_FreeData(void *bp, void *p) {
  GTK2_GUI_DIALOG *xdlg;

  xdlg=(GTK2_GUI_DIALOG*) p;

  if (xdlg->mainWidget)
    gtk_widget_destroy(xdlg->mainWidget);

  GWEN_FREE_OBJECT(xdlg);
}



int Gtk2Gui_Dialog_SetIntProperty(GWEN_DIALOG *dlg,
                                  GWEN_WIDGET *w,
				  GWEN_DIALOG_PROPERTY prop,
				  int index,
				  int value,
				  int doSignal) {
  return GWEN_Widget_SetIntProperty(w, prop, index, value, doSignal);
}



int Gtk2Gui_Dialog_GetIntProperty(GWEN_DIALOG *dlg,
                                  GWEN_WIDGET *w,
				  GWEN_DIALOG_PROPERTY prop,
				  int index,
				  int defaultValue) {
  return GWEN_Widget_GetIntProperty(w, prop, index, defaultValue);
}



int Gtk2Gui_Dialog_SetCharProperty(GWEN_DIALOG *dlg,
				   GWEN_WIDGET *w,
				   GWEN_DIALOG_PROPERTY prop,
				   int index,
				   const char *value,
				   int doSignal) {
  return GWEN_Widget_SetCharProperty(w, prop, index, value, doSignal);
}



const char *Gtk2Gui_Dialog_GetCharProperty(GWEN_DIALOG *dlg,
					   GWEN_WIDGET *w,
					   GWEN_DIALOG_PROPERTY prop,
					   int index,
					   const char *defaultValue) {
  return GWEN_Widget_GetCharProperty(w, prop, index, defaultValue);
}



int Gtk2Gui_Dialog_SetupTree(GWEN_WIDGET *w) {
  GWEN_WIDGET *wParent;
  int rv;
  GtkBox *gbox=NULL;
  GtkContainer *gcontainer=NULL;

  wParent=GWEN_Widget_Tree_GetParent(w);
  if (wParent) {
    switch(GWEN_Widget_GetType(w)) {
    case GWEN_Widget_TypeHLayout:
    case GWEN_Widget_TypeVLayout:
    case GWEN_Widget_TypeGridLayout:
      gbox=GTK_BOX(GWEN_Widget_GetImplData(wParent, GTK2_DIALOG_WIDGET_CONTENT));
      break;
    default:
      gcontainer=GTK_CONTAINER(GWEN_Widget_GetImplData(wParent, GTK2_DIALOG_WIDGET_CONTENT));
      break;
    }
  }

  switch(GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeLabel:
    rv=Gtk2Gui_Label_Setup(gcontainer, gbox, w);
    break;
  case GWEN_Widget_TypePushButton:
  case GWEN_Widget_TypeLineEdit:
  case GWEN_Widget_TypeTextEdit:
  case GWEN_Widget_TypeComboBox:
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
  case GWEN_Widget_TypeListBox:
  case GWEN_Widget_TypeDialog:
  case GWEN_Widget_TypeTabBook:
  case GWEN_Widget_TypeTabPage:
  case GWEN_Widget_TypeCheckBox:
  case GWEN_Widget_TypeWidgetStack:
  case GWEN_Widget_TypeScrollArea:
  case GWEN_Widget_TypeHLine:
  case GWEN_Widget_TypeVLine:
  case GWEN_Widget_TypeSpinBox:
  case GWEN_Widget_TypeTextBrowser:
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled widget type %d", GWEN_Widget_GetType(w));
    rv=GWEN_ERROR_INVALID;
    break;
  }

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    GWEN_WIDGET *wChild;

    /* handle children */
    wChild=GWEN_Widget_Tree_GetFirstChild(w);
    while(wChild) {
      /* recursion */
      rv=Gtk2Gui_Dialog_SetupTree(wChild);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      wChild=GWEN_Widget_Tree_GetNext(wChild);
    }
  }

  return 0;
}




int Gtk2Gui_Dialog_Setup(GtkWidget *parentWindow) {
  // TODO: create GtkDialog as main widget and then create the children
}




