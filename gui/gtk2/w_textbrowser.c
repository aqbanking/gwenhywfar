/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int Gtk2Gui_WTextBrowser_SetIntProperty(GWEN_WIDGET *w,
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
int Gtk2Gui_WTextBrowser_GetIntProperty(GWEN_WIDGET *w,
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
int Gtk2Gui_WTextBrowser_SetCharProperty(GWEN_WIDGET *w,
					 GWEN_DIALOG_PROPERTY prop,
					 int index,
					 const char *value,
					 int doSignal) {
  GtkWidget *g;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Value: {
    GtkTextBuffer *tb;
    GtkTextIter endIter;

    tb=gtk_text_view_get_buffer(GTK_TEXT_VIEW(g));
    assert(tb);
    if (value && *value)
      gtk_text_buffer_set_text(tb, value, -1);
    else
      gtk_text_buffer_set_text(tb, "", -1);

    gtk_text_buffer_get_end_iter(tb, &endIter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(g), &endIter, 0.5, FALSE, 0.0, 0.0);
    return 0;
  }
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
	   "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* Gtk2Gui_WTextBrowser_GetCharProperty(GWEN_WIDGET *w,
						 GWEN_DIALOG_PROPERTY prop,
						 int index,
						 const char *defaultValue) {
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
  assert(g);

  switch(prop) {
  case GWEN_DialogProperty_Value: {
    GtkTextBuffer *tb;
    GtkTextIter startIter;
    GtkTextIter endIter;
    gchar *s;

    tb=gtk_text_view_get_buffer(GTK_TEXT_VIEW(g));
    assert(tb);

    gtk_text_buffer_get_start_iter(tb, &startIter);
    gtk_text_buffer_get_end_iter(tb, &endIter);

    s=gtk_text_buffer_get_text(tb, &startIter, &endIter, FALSE);
    if (s) {
      GWEN_Widget_SetText(w, GTK2_DIALOG_STRING_VALUE, s);
      g_free(s);
      return GWEN_Widget_GetText(w, GTK2_DIALOG_STRING_VALUE);
    }
    return defaultValue;
  }

    return gtk_button_get_label(g);
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
	   "Function is not appropriate for this type of widget (%s)",
	   GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



int Gtk2Gui_WTextBrowser_Setup(GWEN_WIDGET *w) {
  GtkWidget *g;
  const char *s;
  uint32_t flags;
  GWEN_WIDGET *wParent;

  flags=GWEN_Widget_GetFlags(w);
  wParent=GWEN_Widget_Tree_GetParent(w);
  s=GWEN_Widget_GetText(w, 0);

  /* create widget */
  g=gtk_text_view_new();
  if (s && *s)
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(g)), s, -1);

  GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_REAL, (void*) g);
  GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_CONTENT, (void*) g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk2Gui_WTextBrowser_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk2Gui_WTextBrowser_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk2Gui_WTextBrowser_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk2Gui_WTextBrowser_GetCharProperty);

  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}


