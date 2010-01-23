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


#ifndef GWENHYWFAR_GUI_DIALOG_BE_H
#define GWENHYWFAR_GUI_DIALOG_BE_H


#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef int GWENHYWFAR_CB (*GWEN_DIALOG_SETINTVALUE_FN)(GWEN_DIALOG *dlg,
							GWEN_WIDGET *w,
							int value,
							int doSignal);

typedef int GWENHYWFAR_CB (*GWEN_DIALOG_GETINTVALUE_FN)(GWEN_DIALOG *dlg,
							GWEN_WIDGET *w,
							int defaultValue);

typedef int GWENHYWFAR_CB (*GWEN_DIALOG_SETCHARVALUE_FN)(GWEN_DIALOG *dlg,
							 GWEN_WIDGET *w,
							 const char *value,
							 int doSignal);

typedef const char* GWENHYWFAR_CB (*GWEN_DIALOG_GETCHARVALUE_FN)(GWEN_DIALOG *dlg,
								 GWEN_WIDGET *w,
								 const char *defaultValue);

typedef int GWENHYWFAR_CB (*GWEN_DIALOG_SETRANGE_FN)(GWEN_DIALOG *dlg,
						     GWEN_WIDGET *w,
						     int minValue,
						     int maxValue,
						     int doSignal);

typedef int GWENHYWFAR_CB (*GWEN_DIALOG_SETENABLED_FN)(GWEN_DIALOG *dlg,
						       GWEN_WIDGET *w,
						       int b,
						       int doSignal);

typedef int GWENHYWFAR_CB (*GWEN_DIALOG_GETENABLED_FN)(GWEN_DIALOG *dlg,
						       GWEN_WIDGET *w);




GWENHYWFAR_API
GWEN_DIALOG_SETINTVALUE_FN GWEN_Dialog_SetSetIntValueFn(GWEN_DIALOG *dlg,
							GWEN_DIALOG_SETINTVALUE_FN fn);

GWENHYWFAR_API
GWEN_DIALOG_GETINTVALUE_FN GWEN_Dialog_SetGetIntValueFn(GWEN_DIALOG *dlg,
							GWEN_DIALOG_GETINTVALUE_FN fn);

GWENHYWFAR_API
GWEN_DIALOG_SETCHARVALUE_FN GWEN_Dialog_SetSetCharValueFn(GWEN_DIALOG *dlg,
							  GWEN_DIALOG_SETCHARVALUE_FN fn);

GWENHYWFAR_API
GWEN_DIALOG_GETCHARVALUE_FN GWEN_Dialog_SetGetCharValueFn(GWEN_DIALOG *dlg,
							  GWEN_DIALOG_GETCHARVALUE_FN fn);

GWENHYWFAR_API
GWEN_DIALOG_SETRANGE_FN GWEN_Dialog_SetSetRangeFn(GWEN_DIALOG *dlg,
						  GWEN_DIALOG_SETRANGE_FN fn);

GWENHYWFAR_API
GWEN_DIALOG_SETENABLED_FN GWEN_Dialog_SetSetEnabledFn(GWEN_DIALOG *dlg,
						      GWEN_DIALOG_SETENABLED_FN fn);

GWENHYWFAR_API
GWEN_DIALOG_GETENABLED_FN GWEN_Dialog_SetGetEnabledFn(GWEN_DIALOG *dlg,
						      GWEN_DIALOG_GETENABLED_FN fn);




GWENHYWFAR_API
GWEN_WIDGET_TREE *GWEN_Dialog_GetWidgets(const GWEN_DIALOG *dlg);

GWENHYWFAR_API
GWEN_WIDGET *GWEN_Dialog_FindWidgetByName(GWEN_DIALOG *dlg, const char *name);

GWENHYWFAR_API
GWEN_WIDGET *GWEN_Dialog_FindWidgetByImplData(GWEN_DIALOG *dlg, void *ptr);



GWENHYWFAR_API
int GWEN_Dialog_EmitSignal(GWEN_DIALOG *dlg,
			   GWEN_DIALOG_EVENTTYPE t,
			   const char *sender,
			   int intVal,
			   const char *charVal,
			   void *ptrVal);


#ifdef __cplusplus
}
#endif




#endif
