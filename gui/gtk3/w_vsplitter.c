/***************************************************************************
    begin       : Thu Dec 04 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int Gtk3Gui_WVSplitter_SetIntProperty(GWEN_WIDGET *w,
                                    GWEN_DIALOG_PROPERTY prop,
                                    GWEN_UNUSED int index,
                                    int value,
                                    GWEN_UNUSED int doSignal)
{
  GtkWidget *g;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK3_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(GTK_WIDGET(g), (value==0)?FALSE:TRUE);
    return 0;

  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(GTK_WIDGET(g));
    return 0;

  case GWEN_DialogProperty_Value:
    gtk_paned_set_position(GTK_PANED(g), value);
    return 0;

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}




static GWENHYWFAR_CB
int Gtk3Gui_WVSplitter_GetIntProperty(GWEN_WIDGET *w,
                                    GWEN_DIALOG_PROPERTY prop,
                                    GWEN_UNUSED int index,
                                    int defaultValue)
{
  GtkWidget *g;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK3_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;

  case GWEN_DialogProperty_Focus:
    return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;

  case GWEN_DialogProperty_Value:
    return gtk_paned_get_position(GTK_PANED(g));

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static GWENHYWFAR_CB
int Gtk3Gui_WVSplitter_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild)
{
  GtkWidget *g;
  GtkWidget *gChild;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK3_DIALOG_WIDGET_REAL));
  assert(g);

  gChild=GTK_WIDGET(GWEN_Widget_GetImplData(wChild, GTK3_DIALOG_WIDGET_REAL));
  assert(gChild);

  if (GWEN_Widget_Tree_GetPrevious(wChild))
    gtk_paned_add2(GTK_PANED(g), gChild);
  else
    gtk_paned_add1(GTK_PANED(g), gChild);

  return 0;
}



int Gtk3Gui_WVSplitter_Setup(GWEN_WIDGET *w)
{
  GtkWidget *g;
  GWEN_WIDGET *wParent;

  wParent=GWEN_Widget_Tree_GetParent(w);

  g=gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  GWEN_Widget_SetImplData(w, GTK3_DIALOG_WIDGET_REAL, (void *) g);
  GWEN_Widget_SetImplData(w, GTK3_DIALOG_WIDGET_CONTENT, (void *) g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk3Gui_WVSplitter_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk3Gui_WVSplitter_GetIntProperty);
  GWEN_Widget_SetAddChildGuiWidgetFn(w, Gtk3Gui_WVSplitter_AddChildGuiWidget);

  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}


