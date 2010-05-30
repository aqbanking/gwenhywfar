

/**
 * @defgroup MOD_DIALOG_IMPL Implementation Notes
 * @ingroup MOD_DIALOG
 *
 * @brief Implementation hints.
 *
 *
 * The dialog framework is part of the GWEN_GUI framework (@ref MOD_GUI).
 * To make your implementation available to the application you will have to
 * set the following callbacks in GWEN_GUI:
 * <ul>
 *   <li>@ref GWEN_Gui_ExecDialog (use @ref GWEN_Gui_SetExecDialogFn)</li>
 *   <li>@ref GWEN_Gui_OpenDialog (use @ref GWEN_Gui_SetOpenDialogFn)</li>
 *   <li>@ref GWEN_Gui_CloseDialog (use @ref GWEN_Gui_SetCloseDialogFn)</li>
 *   <li>@ref GWEN_Gui_RunDialog (use @ref GWEN_Gui_SetRunDialogFn)</li>
 * </ul>
 *
 * The function @ref GWEN_Gui_ExecDialog can be implemented by just calling the other
 * functions (first @ref GWEN_Gui_OpenDialog followed by @ref GWEN_Gui_RunDialog and
 * @ref GWEN_Gui_CloseDialog).
 *
 * Your implementation of @ref GWEN_Gui_OpenDialog must set the follwing  callbacks in
 * the object pointed to by the first argument (@ref GWEN_DIALOG):
 * <ul>
 *   <li>@ref GWEN_Dialog_SetIntProperty (use @ref GWEN_Dialog_SetSetIntPropertyFn)</li>
 *   <li>@ref GWEN_Dialog_GetIntProperty (use @ref GWEN_Dialog_SetGetIntPropertyFn)</li>
 *   <li>@ref GWEN_Dialog_SetCharProperty (use @ref GWEN_Dialog_SetSetCharPropertyFn)</li>
 *   <li>@ref GWEN_Dialog_GetCharProperty (use @ref GWEN_Dialog_SetGetCharPropertyFn)</li>
 * </ul>
 *
 * The GKT2 implementation does it like this:
 * @code
 * GWEN_INHERIT(GWEN_DIALOG, GTK2_GUI_DIALOG)
 *
 * void Gtk2Gui_Dialog_Extend(GWEN_DIALOG *dlg) {
 *   GTK2_GUI_DIALOG *xdlg;
 *
 *   GWEN_NEW_OBJECT(GTK2_GUI_DIALOG, xdlg);
 *   GWEN_INHERIT_SETDATA(GWEN_DIALOG, GTK2_GUI_DIALOG, dlg, xdlg, Gtk2Gui_Dialog_FreeData);
 *
 *   GWEN_Dialog_SetSetIntPropertyFn(dlg, Gtk2Gui_Dialog_SetIntProperty);
 *   GWEN_Dialog_SetGetIntPropertyFn(dlg, Gtk2Gui_Dialog_GetIntProperty);
 *   GWEN_Dialog_SetSetCharPropertyFn(dlg, Gtk2Gui_Dialog_SetCharProperty);
 *   GWEN_Dialog_SetGetCharPropertyFn(dlg, Gtk2Gui_Dialog_GetCharProperty);
 * }
 * @endcode
 *
 * It must also create the actual widgets used by your GUI toolkit (e.g. QLabel for label
 * widgets with QT). To do that you should call @ref GWEN_Dialog_GetWidgets to get the
 * tree of widget descriptions. The most important information about a widget is its type
 * (@ref GWEN_Widget_GetType). The implementation of @ref GWEN_Gui_OpenDialog can use that
 * information to decide what GUI widget should be created to represent that widget description.
 *
 * For example the QT implementation creates a QLabel object for the widget type
 * @ref GWEN_Widget_TypeLabel.
 *
 * You can call the function @ref GWEN_Widget_SetImplData to store a pointer to the GUI widget
 * created (e.g. the QT implementation stores a pointer to the created QLabel object for widgets
 * of type @ref GWEN_Widget_TypeLabel). To retrieve that pointer later call
 * @ref GWEN_Widget_GetImplData. Currently up to 4 pointers can be stored per GWEN_WIDGET object
 * (in the case that there are multiple GUI objects required to represent a given GWEN_WIDGET).
 *
 * Another important hint comes from the widget flags (get them with @ref GWEN_Widget_GetFlags).
 * Those flag tell the implementation about some layout options and other things.
 *
 * For widgets using media like icons or images the implementation can call
 * @ref GWEN_Dialog_GetMediaPaths to get a list of paths where those icons can be found. This
 * information is provided by the dialog itself. You can use functions like
 * @ref GWEN_Directory_FindFileInPaths to actually find the icon or image file using the media
 * paths.
 *
 *
 * This example shows how the GTK2 implementation creates a GUI representation of type
 * @ref GWEN_Widget_TypeLabel:
 * @code
 * #define GTK2_DIALOG_WIDGET_REAL    0
 * #define GTK2_DIALOG_WIDGET_CONTENT 1
 *
 * int Gtk2Gui_WLabel_Setup(GWEN_WIDGET *w) {
 *   GtkWidget *g;
 *   const char *s;
 *   uint32_t flags;
 *   GWEN_WIDGET *wParent;
 *
 *   flags=GWEN_Widget_GetFlags(w);
 *   wParent=GWEN_Widget_Tree_GetParent(w);
 *   s=GWEN_Widget_GetText(w, 0);
 *
 *   g=gtk_label_new(s);
 *   GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_REAL, (void*) g);
 *   GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_CONTENT, (void*) g);
 *
 *   GWEN_Widget_SetSetIntPropertyFn(w, Gtk2Gui_WLabel_SetIntProperty);
 *   GWEN_Widget_SetGetIntPropertyFn(w, Gtk2Gui_WLabel_GetIntProperty);
 *   GWEN_Widget_SetSetCharPropertyFn(w, Gtk2Gui_WLabel_SetCharProperty);
 *   GWEN_Widget_SetGetCharPropertyFn(w, Gtk2Gui_WLabel_GetCharProperty);
 *
 *   if (wParent)
 *     GWEN_Widget_AddChildGuiWidget(wParent, w);
 *
 *   return 0;
 * }
 * @endcode
 *
 *
 */

