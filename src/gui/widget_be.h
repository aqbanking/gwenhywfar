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
#include <gwenhywfar/inherit.h>


typedef struct GWEN_WIDGET GWEN_WIDGET;

#ifdef __cplusplus
extern "C" {
#endif


GWEN_TREE_FUNCTION_LIB_DEFS(GWEN_WIDGET, GWEN_Widget, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_WIDGET, GWENHYWFAR_API)


#define GWEN_WIDGET_TEXTCOUNT       4
#define GWEN_WIDGET_IMPLDATACOUNT   4


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
    GWEN_Widget_TypeVLine,
    GWEN_Widget_TypeSpinBox,
    GWEN_Widget_TypeTextBrowser
} GWEN_WIDGET_TYPE;



#ifdef __cplusplus
}
#endif



/* other gwen headers */
#include <gwenhywfar/dialog.h>



#ifdef __cplusplus
extern "C" {
#endif


typedef int GWENHYWFAR_CB (*GWEN_WIDGET_SETINTPROPERTY_FN)(GWEN_WIDGET *w,
							   GWEN_DIALOG_PROPERTY prop,
							   int index,
							   int value,
							   int doSignal);

typedef int GWENHYWFAR_CB (*GWEN_WIDGET_GETINTPROPERTY_FN)(GWEN_WIDGET *w,
							   GWEN_DIALOG_PROPERTY prop,
							   int index,
							   int defaultValue);

typedef int GWENHYWFAR_CB (*GWEN_WIDGET_SETCHARPROPERTY_FN)(GWEN_WIDGET *w,
							    GWEN_DIALOG_PROPERTY prop,
							    int index,
							    const char *value,
							    int doSignal);

typedef const char* GWENHYWFAR_CB (*GWEN_WIDGET_GETCHARPROPERTY_FN)(GWEN_WIDGET *w,
								    GWEN_DIALOG_PROPERTY prop,
								    int index,
								    const char *defaultValue);

typedef int GWENHYWFAR_CB (*GWEN_WIDGET_ADDCHILDGUIWIDGET_FN)(GWEN_WIDGET *w, GWEN_WIDGET *wChild);



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

/**
 * Returns the flags of the given widget
 * (see @ref GWEN_WIDGET_FLAGS_FILLX and following).
 */
GWENHYWFAR_API
uint32_t GWEN_Widget_GetFlags(const GWEN_WIDGET *w);

GWENHYWFAR_API
void GWEN_Widget_SetFlags(GWEN_WIDGET *w, uint32_t fl);

GWENHYWFAR_API
void GWEN_Widget_AddFlags(GWEN_WIDGET *w, uint32_t fl);

GWENHYWFAR_API
void GWEN_Widget_SubFlags(GWEN_WIDGET *w, uint32_t fl);

/**
 * Returns the type of the widget (see @ref GWEN_Widget_TypeLabel and following).
 */
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



GWENHYWFAR_API
GWEN_WIDGET_SETINTPROPERTY_FN GWEN_Widget_SetSetIntPropertyFn(GWEN_WIDGET *w,
							      GWEN_WIDGET_SETINTPROPERTY_FN fn);

GWENHYWFAR_API
GWEN_WIDGET_GETINTPROPERTY_FN GWEN_Widget_SetGetIntPropertyFn(GWEN_WIDGET *w,
							      GWEN_WIDGET_GETINTPROPERTY_FN fn);

GWENHYWFAR_API
GWEN_WIDGET_SETCHARPROPERTY_FN GWEN_Widget_SetSetCharPropertyFn(GWEN_WIDGET *w,
								GWEN_WIDGET_SETCHARPROPERTY_FN fn);

GWENHYWFAR_API
GWEN_WIDGET_GETCHARPROPERTY_FN GWEN_Widget_SetGetCharPropertyFn(GWEN_WIDGET *w,
								GWEN_WIDGET_GETCHARPROPERTY_FN fn);

GWENHYWFAR_API
GWEN_WIDGET_ADDCHILDGUIWIDGET_FN GWEN_Widget_SetAddChildGuiWidgetFn(GWEN_WIDGET *w,
								    GWEN_WIDGET_ADDCHILDGUIWIDGET_FN fn);




GWENHYWFAR_API
int GWEN_Widget_SetIntProperty(GWEN_WIDGET *w,
			       GWEN_DIALOG_PROPERTY prop,
			       int index,
			       int value,
			       int doSignal);

GWENHYWFAR_API
int GWEN_Widget_GetIntProperty(GWEN_WIDGET *w,
			       GWEN_DIALOG_PROPERTY prop,
			       int index,
			       int defaultValue);

GWENHYWFAR_API
int GWEN_Widget_SetCharProperty(GWEN_WIDGET *w,
				GWEN_DIALOG_PROPERTY prop,
				int index,
				const char *value,
				int doSignal);

GWENHYWFAR_API
const char* GWEN_Widget_GetCharProperty(GWEN_WIDGET *w,
					GWEN_DIALOG_PROPERTY prop,
					int index,
					const char *defaultValue);

GWENHYWFAR_API
int GWEN_Widget_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild);


#ifdef __cplusplus
}
#endif


#endif
