/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int Gtk2Gui_WLineEdit_SetIntProperty(GWEN_WIDGET *w,
				       GWEN_DIALOG_PROPERTY prop,
				       int index,
				       int value,
				       int doSignal) {
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(GTK_WIDGET(g), (value==0)?FALSE:TRUE);
    return 0;
  
  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(GTK_WIDGET(g));
    return 0;

  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
    /* just ignore these for now */
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
int Gtk2Gui_WLineEdit_GetIntProperty(GWEN_WIDGET *w,
				       GWEN_DIALOG_PROPERTY prop,
				       int index,
				       int defaultValue) {
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Enabled:
    return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;

  case GWEN_DialogProperty_Focus:
    return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;
    return 0;

  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
    /* just ignore these for now */
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
int Gtk2Gui_WLineEdit_SetCharProperty(GWEN_WIDGET *w,
					GWEN_DIALOG_PROPERTY prop,
					int index,
					const char *value,
					int doSignal) {
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Title:
    gtk_button_set_label(g, value);
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
const char* Gtk2Gui_WLineEdit_GetCharProperty(GWEN_WIDGET *w,
						GWEN_DIALOG_PROPERTY prop,
						int index,
						const char *defaultValue) {
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Title:
    return gtk_button_get_label(g);
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
	   "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static void deleted_text_handler(GtkEntryBuffer *entrybuffer,
				 guint arg1,
				 guint arg2,
				 gpointer data) {
  GWEN_WIDGET *w;
  int rv;

  w=data;
  assert(w);
  rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
			    GWEN_DialogEvent_TypeValueChanged,
			    GWEN_Widget_GetName(w));
  if (rv==GWEN_DialogEvent_ResultAccept)
    Gtk2Gui_Dialog_Leave(GWEN_Widget_GetDialog(w), 1);
  else if (rv==GWEN_DialogEvent_ResultReject)
    Gtk2Gui_Dialog_Leave(GWEN_Widget_GetDialog(w), 0);
}



static void inserted_text_handler(GtkEntryBuffer *entrybuffer,
				  guint arg1,
				  gchar *arg2,
				  guint arg3,
				  gpointer data) {
  GWEN_WIDGET *w;
  int rv;

  w=data;
  assert(w);
  rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
			    GWEN_DialogEvent_TypeValueChanged,
			    GWEN_Widget_GetName(w));
  if (rv==GWEN_DialogEvent_ResultAccept)
    Gtk2Gui_Dialog_Leave(GWEN_Widget_GetDialog(w), 1);
  else if (rv==GWEN_DialogEvent_ResultReject)
    Gtk2Gui_Dialog_Leave(GWEN_Widget_GetDialog(w), 0);
}



int Gtk2Gui_WLineEdit_Setup(GWEN_WIDGET *w) {
  GtkWidget *g;
  const char *s;
  uint32_t flags;
  GWEN_WIDGET *wParent;
  gulong deleted_text_handler_id;
  gulong inserted_text_handler_id;

  flags=GWEN_Widget_GetFlags(w);
  wParent=GWEN_Widget_Tree_GetParent(w);
  s=GWEN_Widget_GetText(w, 0);

  /* create widget */
  g=gtk_entry_new();
  if (s && *s)
    gtk_entry_set_text(GTK_ENTRY(g), s);

  GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_REAL, (void*) g);
  GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_CONTENT, (void*) g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk2Gui_WLineEdit_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk2Gui_WLineEdit_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk2Gui_WLineEdit_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk2Gui_WLineEdit_GetCharProperty);

  deleted_text_handler_id=g_signal_connect(gtk_entry_get_buffer(GTK_ENTRY(g)),
					   "deleted-text",
					   G_CALLBACK (deleted_text_handler),
					   w);

  inserted_text_handler_id=g_signal_connect(gtk_entry_get_buffer(GTK_ENTRY(g)),
					    "inserted-text",
					    G_CALLBACK (inserted_text_handler),
					    w);

  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}


