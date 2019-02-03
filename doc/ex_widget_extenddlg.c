void Gtk2Gui_Dialog_Extend(GWEN_DIALOG *dlg)
{
  GTK2_GUI_DIALOG *xdlg;

  GWEN_NEW_OBJECT(GTK2_GUI_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, GTK2_GUI_DIALOG, dlg, xdlg, Gtk2Gui_Dialog_FreeData);

  /* set virtual functions */
  xdlg->setIntPropertyFn=GWEN_Dialog_SetSetIntPropertyFn(dlg, Gtk2Gui_Dialog_SetIntProperty);
  xdlg->getIntPropertyFn=GWEN_Dialog_SetGetIntPropertyFn(dlg, Gtk2Gui_Dialog_GetIntProperty);
  xdlg->setCharPropertyFn=GWEN_Dialog_SetSetCharPropertyFn(dlg, Gtk2Gui_Dialog_SetCharProperty);
  xdlg->getCharPropertyFn=GWEN_Dialog_SetGetCharPropertyFn(dlg, Gtk2Gui_Dialog_GetCharProperty);

}
