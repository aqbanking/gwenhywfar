/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int Gtk2Gui_WHLayout_SetIntProperty(GWEN_WIDGET *w,
				 GWEN_DIALOG_PROPERTY prop,
				 int index,
				 int value,
				 int doSignal) {
  GtkLabel *g;

  g=GTK_LABEL(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(GTK_WIDGET(g), (value==0)?FALSE:TRUE);
    return 0;
  
  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(GTK_WIDGET(g));
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
int Gtk2Gui_WHLayout_GetIntProperty(GWEN_WIDGET *w,
				 GWEN_DIALOG_PROPERTY prop,
				 int index,
				 int defaultValue) {
  GtkLabel *g;

  g=GTK_LABEL(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Enabled:
    return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;

  case GWEN_DialogProperty_Focus:
    return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;
    return 0;

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
	   "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static GWENHYWFAR_CB
int Gtk2Gui_WHLayout_SetCharProperty(GWEN_WIDGET *w,
				     GWEN_DIALOG_PROPERTY prop,
				     int index,
				     const char *value,
				     int doSignal) {
  GtkLabel *g;

  g=GTK_LABEL(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  DBG_WARN(GWEN_LOGDOMAIN,
	   "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* Gtk2Gui_WHLayout_GetCharProperty(GWEN_WIDGET *w,
					     GWEN_DIALOG_PROPERTY prop,
					     int index,
					     const char *defaultValue) {
  GtkLabel *g;

  g=GTK_LABEL(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  DBG_WARN(GWEN_LOGDOMAIN,
	   "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



int Gtk2Gui_WHLayout_Setup(GtkContainer *gcontainer, GtkBox *gbox, GWEN_WIDGET *w) {
  GtkWidget *g;
  const char *s;
  uint32_t flags;

  flags=GWEN_Widget_GetFlags(w);
  s=GWEN_Widget_GetText(w, 0);

  g=gtk_hbox_new((flags & GWEN_WIDGET_FLAGS_EQUAL_WIDTH)?TRUE:FALSE,
		 GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING);
  if (gbox)
    /* add to layout box (if any) */
    gtk_box_pack_start(gbox, g,
		       (flags & (GWEN_WIDGET_FLAGS_FILLX | GWEN_WIDGET_FLAGS_FILLY))?TRUE:FALSE,
		       (flags & (GWEN_WIDGET_FLAGS_FILLX | GWEN_WIDGET_FLAGS_FILLY))?TRUE:FALSE,
		       0);
  else if (gcontainer)
    gtk_container_add(gcontainer, g);

  GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_REAL, (void*) g);
  GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_CONTENT, (void*) g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk2Gui_WHLayout_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk2Gui_WHLayout_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk2Gui_WHLayout_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk2Gui_WHLayout_GetCharProperty);

  return 0;
}


