/***************************************************************************
    begin       : Wed Jan 20 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GWENHYWFAR_GUI_WIDGET_BE_H
#define GWENHYWFAR_GUI_WIDGET_BE_H


#include <gwenhywfar/tree.h>


typedef struct GWEN_WIDGET GWEN_WIDGET;

#ifdef __cplusplus
extern "C" {
#endif


GWEN_TREE_FUNCTION_LIB_DEFS(GWEN_WIDGET, GWEN_Widget, GWENHYWFAR_API)


#define GWEN_WIDGET_TEXTCOUNT       4
#define GWEN_WIDGET_IMPLDATACOUNT   4

#define GWEN_WIDGET_FLAGS_NONE              0x00000000L
#define GWEN_WIDGET_FLAGS_FILLX             0x80000000L
#define GWEN_WIDGET_FLAGS_FILLY             0x40000000L
#define GWEN_WIDGET_FLAGS_READONLY          0x20000000L
#define GWEN_WIDGET_FLAGS_PASSWORD          0x10000000L
#define GWEN_WIDGET_FLAGS_DEFAULT_WIDGET    0x08000000L

#define GWEN_WIDGET_FLAGS_DECOR_SHRINKABLE  0x04000000L
#define GWEN_WIDGET_FLAGS_DECOR_STRETCHABLE 0x02000000L
#define GWEN_WIDGET_FLAGS_DECOR_MINIMIZE    0x01000000L
#define GWEN_WIDGET_FLAGS_DECOR_MAXIMIZE    0x00800000L
#define GWEN_WIDGET_FLAGS_DECOR_CLOSE       0x00400000L
#define GWEN_WIDGET_FLAGS_DECOR_MENU        0x00200000L

#define GWEN_WIDGET_FLAGS_FIXED_WIDTH       0x00100000L
#define GWEN_WIDGET_FLAGS_FIXED_HEIGHT      0x00080000L
#define GWEN_WIDGET_FLAGS_EQUAL_WIDTH       0x00040000L
#define GWEN_WIDGET_FLAGS_EQUAL_HEIGHT      0x00020000L

#define GWEN_WIDGET_FLAGS_JUSTIFY_LEFT      0x00010000L
#define GWEN_WIDGET_FLAGS_JUSTIFY_RIGHT     0x00008000L
#define GWEN_WIDGET_FLAGS_JUSTIFY_TOP       0x00004000L
#define GWEN_WIDGET_FLAGS_JUSTIFY_BOTTOM    0x00002000L
#define GWEN_WIDGET_FLAGS_JUSTIFY_CENTERX   0x00001000L
#define GWEN_WIDGET_FLAGS_JUSTIFY_CENTERY   0x00000800L


typedef enum {
    GWEN_Widget_TypeUnknown=-1,
    GWEN_Widget_TypeNone=0,
    GWEN_Widget_TypeLabel=1,
    GWEN_Widget_TypePushButton,
    GWEN_Widget_TypeLineEdit,
    GWEN_Widget_TypeTextEdit,
    GWEN_Widget_TypeComboBox,
    GWEN_Widget_TypeRadioButton,
    GWEN_Widget_TypeProgressBar,
    GWEN_Widget_TypeRadioGroup,
    GWEN_Widget_TypeGroupBox,
    GWEN_Widget_TypeHSpacer,
    GWEN_Widget_TypeVSpacer,
    GWEN_Widget_TypeHLayout,
    GWEN_Widget_TypeVLayout,
    GWEN_Widget_TypeGridLayout,
    GWEN_Widget_TypeImage,
    GWEN_Widget_TypeListBox,
    GWEN_Widget_TypeDialog,
    GWEN_Widget_TypeTabBook,
    GWEN_Widget_TypeTabPage,
    GWEN_Widget_TypeCheckBox,
    GWEN_Widget_TypeWidgetStack,
    GWEN_Widget_TypeScrollArea,
    GWEN_Widget_TypeHLine,
    GWEN_Widget_TypeVLine
} GWEN_WIDGET_TYPE;




#ifdef __cplusplus
}
#endif



/* other gwen headers */
#include <gwenhywfar/dialog.h>



#ifdef __cplusplus
extern "C" {
#endif

GWENHYWFAR_API
void *GWEN_Widget_GetImplData(const GWEN_WIDGET *w, int index);

GWENHYWFAR_API
void GWEN_Widget_SetImplData(GWEN_WIDGET *w, int index, void *ptr);


GWENHYWFAR_API
GWEN_DIALOG *GWEN_Widget_GetDialog(const GWEN_WIDGET *w);

GWENHYWFAR_API
const char *GWEN_Widget_GetName(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetName(GWEN_WIDGET *w, const char *s);

GWENHYWFAR_API
uint32_t GWEN_Widget_GetFlags(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetFlags(GWEN_WIDGET *w, uint32_t fl);

GWENHYWFAR_API
void GWEN_Widget_AddFlags(GWEN_WIDGET *w, uint32_t fl);

GWENHYWFAR_API
void GWEN_Widget_SubFlags(GWEN_WIDGET *w, uint32_t fl);

GWENHYWFAR_API
GWEN_WIDGET_TYPE GWEN_Widget_GetType(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetType(GWEN_WIDGET *w, GWEN_WIDGET_TYPE t);

GWENHYWFAR_API
int GWEN_Widget_GetColumns(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetColumns(GWEN_WIDGET *w, int i);

GWENHYWFAR_API
int GWEN_Widget_GetRows(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetRows(GWEN_WIDGET *w, int i);

GWENHYWFAR_API
const char *GWEN_Widget_GetText(const GWEN_WIDGET *w, int idx);

GWENHYWFAR_API
void GWEN_Widget_SetText(GWEN_WIDGET *w, int idx, const char *s);



GWENHYWFAR_API
const char *GWEN_Widget_GetIconFileName(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetIconFileName(GWEN_WIDGET *w, const char *s);


GWENHYWFAR_API
const char *GWEN_Widget_GetImageFileName(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetImageFileName(GWEN_WIDGET *w, const char *s);



GWENHYWFAR_API
GWEN_WIDGET_TYPE GWEN_Widget_Type_fromString(const char *s);

GWENHYWFAR_API
const char *GWEN_Widget_Type_toString(GWEN_WIDGET_TYPE t);

GWENHYWFAR_API
uint32_t GWEN_Widget_Flags_fromString(const char *s);




/**
 *
 * All Widgets
 *
 * Integer Properties
 *
 * The following properties are handled by all widgets.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Width: The preferred width of
 *     the widget. This is only guaranteed to work for
 *     widgets of type @ref GWEN_Widget_TypeDialogBox.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_Height: The preferred height of
 *     the widget. This is only guaranteed to work for
 *     widgets of type @ref GWEN_Widget_TypeDialogBox.
 *   </li>
 *   <li>
 *     GWEN_DialogProperty_Enabled: The enabled state of
 *     a widget (0=disabled, 1=enabled)
 *   </li>
 * </ul>
 *
 *
 *
 *
 * GWEN_Widget_TypeLabel
 *
 * This is a simple label widget (e.g. QLabel in QT or
 * FXLabel in FOX).
 *
 * Integer Properties
 *
 * This widget does not have special integer properties.
 *
 *
 * String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Title: Text of the label.
 *   </li>
 * </ul>
 *
 *
 * Signals
 *
 * This widget does not emit special signals.
 *
 *
 *
 *
 * GWEN_Widget_TypePushButton
 *
 * This is a simple push button widget (e.g. QPushButton in QT or
 * FXButton in FOX).
 *
 * Integer Properties
 *
 * This widget does not have special integer properties.
 *
 *
 * String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Title: Text of the button.
 *   </li>
 * </ul>
 *
 *
 *
 *
 * GWEN_Widget_TypeLineEdit
 *
 * This is a one line edit field widget (e.g. Q?? in QT or
 * FXTextField in FOX).
 *
 * Integer Properties
 *
 * This widget does not have special integer properties.
 *
 *
 * String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value: Entered text.
 *   </li>
 * </ul>
 *
 *
 * Signals
 *
 * <ul>
 *   <li>
 *     GWEN_DialogEvent_TypeValueChanged: This signal is
 *     emitted as soon as the enter key is pressed or
 *     the widget looses focus.
 *   </li>
 * </ul>
 *
 *
 *
 *
 * GWEN_Widget_TypeTextEdit
 *
 * This is a multi line edit field widget (e.g. Q?? in QT or
 * FXText in FOX).
 *
 * Integer Properties
 *
 * This widget does not have special integer properties.
 *
 *
 * String Properties
 *
 * The following properties are handled by this widget.
 * <ul>
 *   <li>
 *     GWEN_DialogProperty_Value: Entered text.
 *   </li>
 * </ul>
 *
 *
 * Signals
 *
 * No special signals for now.
 *
 *
 *
 *
 * GWEN_Widget_TypeComboBox
 * GWEN_Widget_TypeProgressBar
 * GWEN_Widget_TypeRadioGroup
 * GWEN_Widget_TypeGroupBox
 * GWEN_Widget_TypeHSpacer
 * GWEN_Widget_TypeVSpacer
 * GWEN_Widget_TypeHLayout
 * GWEN_Widget_TypeVLayout
 * GWEN_Widget_TypeGridLayout
 * GWEN_Widget_TypeImage
 * GWEN_Widget_TypeListBox
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



#ifdef __cplusplus
}
#endif


#endif
