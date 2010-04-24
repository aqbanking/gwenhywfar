


/**
 * @defgroup MOD_DIALOG Toolkit Independent Dialog Framework
 *
 * @brief Platform and toolkit independent XML-based dialog framework.
 *
 * @section SEC_Dialog GWEN_Dialog Framework
 *
 * A dialog consists of a tree of widgets.
 *
 * Widgets can have integer and string properties. Some widgets also emit signals
 * (like a push button which emits the "Activated" signal when clicked).
 * See @ref GWEN_Dialog_SetIntProperty, @ref GWEN_Dialog_GetIntProperty,
 * @ref GWEN_Dialog_SetCharProperty and @ref GWEN_Dialog_GetCharProperty about
 * how to get or manipulate widget properties.
 *
 * There is a special signal @ref GWEN_DialogEvent_TypeInit which is emitted
 * immediately before a dialog is shown to the user (e.g. upon @ref GWEN_Gui_ExecDialog).
 * It is only after the reception of this signal that properties become available for the
 * property retrieval and manipulation functions mentioned above.
 *
 *
 *
 * @section SEC_STRING String Properties
 *
 * Some widgets may allow for HTML text, e.g. the FOX and QT3/4 implementations allow for
 * HTML text as title property. However, not all implementations might have HTML support,
 * so it must still be possible to set standard text which is then used by HTML-agnostic
 * implementations. The chosen way to do this in the GWEN_Dialog framework is to specify both
 * formats in a single string.
 * String properties with the "html" mark in the following list may contain text in the following
 * format:
 * @code
 *   "<html>this is the <b>html</b> version</html> and this is the standard text"
 * @endcode
 *
 * As you can see HTML text is enclosed by the HTML element "html". Everything outside that is
 * supposed to be standard text.
 *
 * Implementations which are aware of HTML text will only use that enclosed part and ignore the
 * rest. HTML-agnostic implementations will use the text outside those "html" markings.
 *
 *
 *
 * @section SEC_ALL_WIDGETS All Widgets
 *
 * @subsection SUBSEC_INT Integer Properties
 *
 * The following properties are handled by all widgets.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Width (rw): The preferred width of
 *     the widget. This is only guaranteed to work for
 *     widgets of type @ref GWEN_Widget_TypeDialog.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_Height (rw): The preferred height of
 *     the widget. This is only guaranteed to work for
 *     widgets of type @ref GWEN_Widget_TypeDialog.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_Enabled (rw): The enabled state of
 *     a widget (0=disabled, 1=enabled)
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_Focus (rw): Whether a widget
 *     has the focus (0=no focus, 1=has focus)
 *   </li>
 * </ul>
 *
 *
 *
 *
 * @section SEC_LABEL GWEN_Widget_TypeLabel
 *
 * This is a simple label widget (e.g. QLabel in QT or
 * FXLabel in FOX).
 *
 *
 * @subsection SUBSEC_STRING String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Title (rw, html): Text of the label.
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_SIGNALS Signals
 *
 * This widget does not emit special signals.
 *
 *
 *
 *
 * @section SEC_PUSHBUTTON GWEN_Widget_TypePushButton
 *
 * This is a simple push button widget (e.g. QPushButton in QT or
 * FXButton in FOX).
 *
 *
 * @subsection SUBSEC_INT Integer Properties
 *
 * This widget does not have special integer properties.
 *
 *
 * @subsection SUBSEC_STRING String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Title (rw): Text of the button.
 *   </li>
 * </ul>
 *
 *
 *
 *
 * @section SEC_LINEEDIT GWEN_Widget_TypeLineEdit
 *
 * This is a one line edit field widget (e.g. QLineEdit in QT or
 * FXTextField in FOX).
 *
 *
 * @subsection SUBSEC_INT Integer Properties
 *
 * This widget does not have special integer properties.
 *
 *
 * @subsection SUBSEC_STRING String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value (rw): Entered text.
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_SIGNALS Signals
 *
 * <ul>
 *   <li>
 *     GWEN_DialogEvent_TypeValueChanged: This signal is
 *     emitted whenever the content of the line edit changes.
 *   </li>
 *   <li>
 *     GWEN_DialogEvent_TypeValueCommand: This signal is
 *     emitted as soon as the enter key is pressed or
 *     the widget looses focus.
 *   </li>
 * </ul>
 *
 *
 *
 *
 * @section SEC_TEXTEDIT GWEN_Widget_TypeTextEdit
 *
 * This is a multi line edit field widget (e.g. Q?? in QT or
 * FXText in FOX).
 *
 *
 * @subsection SUBSEC_INT Integer Properties
 *
 * This widget does not have special integer properties.
 *
 *
 * @subsection SUBSEC_STRING String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value (rw): Entered text.
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_SIGNALS Signals
 *
 * No special signals for now.
 *
 *
 *
 *
 * @section SEC_COMBOBOX GWEN_Widget_TypeComboBox
 *
 * This is a combobox widget (e.g. QComboBox in QT or FXComboBox in FOX).
 *
 *
 * @subsection SUBSEC_INT Integer Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value (rw): Represents the index of the
 *     currently selected entry.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_ClearChoices (wo): Any value set here will clear
 *     the list of entries of this widget.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_ChoiceCount (ro): Number of entries in the combobox.
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_STRING String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value (rw): Text of the currently active entry.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_AddChoice (wo): Appends an entry at the end
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_SIGNALS Signals
 *
 * <ul>
 *   <li>
 *     GWEN_DialogEvent_TypeValueChanged: This signal is
 *     emitted when an entry has been chosen from the combo box
 *     and also when the widget looses focus.
 *   </li>
 * </ul>
 *
 *
 *
 *
 * @section SEC_PROGRESSBAR GWEN_Widget_TypeProgressBar
 *
 * This widget is a simple progress bar. It has a total value which
 * represents 100 percent and a progress value (from which the percentage
 * is calculated and shown).
 *
 *
 * @subsection SUBSEC_INT Integer Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value (rw): Progress value
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_MaxValue (rw): The value which represents 100 percent.
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_SIGNALS Signals
 *
 * No special signals.
 *
 *
 *
 *
 * @section SEC_GROUPBOX GWEN_Widget_TypeGroupBox
 *
 * This widget is a box surrounding other widgets with a thin line. It may also
 * have a title.
 *
 *
 * @subsection SUBSEC_STRING String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Title (rw): Text written at the border of the widget
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_SIGNALS Signals
 *
 * This widget does not emit special signals.
 *
 *
 *
 *
 * @section SEC_LISTBOX GWEN_Widget_TypeListBox
 *
 * A list box is a simple list with a horizontal title header at the top.
 * Entries of a list may have multiple columns.
 *
 *
 * @subsection SUBSEC_INT Integer Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value (rw): Represents the index of the
 *     currently selected entry.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_ClearChoices (wo): Any value set here will clear
 *     the list of entries of this widget.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_ChoiceCount (ro): Number of entries in the combobox.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_ColumnWidth (rw): Width of the column given by @c index
 *     (see @ref GWEN_Dialog_SetIntProperty). When setting this property
 *     @c index is the column whose width is to be changed and @c value is the new
 *     width.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_SelectionMode (rw): Determines how to handle selecting of
 *     entries in this widget.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_SelectionState (rw): Selection state of the item given by
 *     @c index (see @ref GWEN_Dialog_SetIntProperty). 
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_SortDirection (rw): Sorting direction of the column given
 *     by @c index (see @ref GWEN_Dialog_SetIntProperty). Only one column can determine
 *     the sort direction at any given time, so setting the direction of one column will
 *     remove the direction tag from every other column. If a column has a sorting
 *     direction other than @ref GWEN_DialogSortDirection_None then that column will
 *     determine how the list will be sorted.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_Sort (wo): Any value written here will make the widget
 *     sort its entries according to the current sort direction. Applications should
 *     be aware that the implementation might or might not automatically sort items
 *     when adding them to the list. So if the application wants to make sure the list
 *     is always sorted it should set this property after adding items.
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_STRING String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Title (rw): String of the header line.
 *     Since list boxes might have multiple columns the title might
 *     contain multiple strings divided by tab ("\t").
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_AddChoice (wo): Appends an entry to the list.
 *     Please note that the implementation might or might not sort-in new
 *     entries. So if the application needs to be sure that the list is properly
 *     sorted it should use the property @ref GWEN_DialogProperty_Sort after
 *     appending items.
 *     Since list boxes might have multiple columns an entry might
 *     contain multiple strings divided by tab ( e.g. "col1\tcol2").
 *   </li>
 * </ul>
 *
 *
 * @subsection SUBSEC_SIGNALS Signals
 *
 * <ul>
 *   <li>
 *     GWEN_DialogEvent_TypeValueChanged: This signal is
 *     emitted when the current entry changes (e.g. another entry
 *     becomes the current entry).
 *   </li>
 * </ul>
 *
 *
 *
 *
 *
 *
 * GWEN_Widget_TypeHSpacer
 * GWEN_Widget_TypeVSpacer
 * GWEN_Widget_TypeHLayout
 * GWEN_Widget_TypeVLayout
 * GWEN_Widget_TypeGridLayout
 * GWEN_Widget_TypeImage
 * GWEN_Widget_TypeDialog
 * GWEN_Widget_TypeTabBook
 * GWEN_Widget_TypeTabPage
 * GWEN_Widget_TypeCheckBox
 * GWEN_Widget_TypeWidgetStack
 * GWEN_Widget_TypeScrollArea
 * GWEN_Widget_TypeHLine
 * GWEN_Widget_TypeVLine
 * GWEN_Widget_TypeRadioButton
 *
 *
 */


