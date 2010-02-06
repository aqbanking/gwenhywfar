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


#ifndef GWENHYWFAR_GUI_DIALOG_H
#define GWENHYWFAR_GUI_DIALOG_H


#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list1.h>
#include <gwenhywfar/xml.h>


/** @defgroup MOD_GUI_DIALOG GUI Dialogs
 * @ingroup MOD_GUI
 *
 * @brief This module contains the definition of GWEN_GUI dialogs.
 *
 * A dialog contains widgets which can be accessed by name. A dialog can contain multiple
 * subdialogs. Widgets of a dialog and its subdialogs should have unique names.
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_DIALOG GWEN_DIALOG;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_DIALOG, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_DIALOG, GWEN_Dialog, GWENHYWFAR_API)



typedef enum {
  GWEN_DialogEvent_TypeInit=0,
  GWEN_DialogEvent_TypeFini,
  GWEN_DialogEvent_TypeValueChanged,
  GWEN_DialogEvent_TypeActivated,
  GWEN_DialogEvent_TypeEnabled,
  GWEN_DialogEvent_TypeDisabled,

  /**
   * This signal asks for the full path of an image.
   * Please note that no GWEN_DIALOG function must be called from within the handler of this
   * signal, because it is called by the GUI implementation upon initialization, so not all
   * widgets have been initialized, yet.
   * <ul>
   *  <li>charVal points to the name of the image (e.g. "aqbanking/dialogs/wizard/image01.png")</li>
   *  <li>ptrVal points to a buffer to receive the full path (including trailing null byte)</li>
   *  <li>intVal contains the size of that buffer</li>
   * </ul>
   * If the full path can't be determined by the signal handler the code
   * @ref GWEN_DialogEvent_ResultNotHandled should be returned.
   */
  GWEN_DialogEvent_TypeGetImagePath,

  /**
   * This signal asks for the full path of an icon.
   * Please note that no GWEN_DIALOG function must be called from within the handler of this
   * signal, because it is called by the GUI implementation upon initialization, so not all
   * widgets have been initialized, yet.
   * <ul>
   *  <li>charVal points to the name of the image (e.g. "aqbanking/dialogs/wizard/icon01.png")</li>
   *  <li>ptrVal points to a buffer to receive the full path (including trailing null byte)</li>
   *  <li>intVal contains the size of that buffer</li>
   * </ul>
   * If the full path can't be determined by the signal handler the code
   * @ref GWEN_DialogEvent_ResultNotHandled should be returned.
   */
  GWEN_DialogEvent_TypeGetIconPath,

  GWEN_DialogEvent_TypeLast
} GWEN_DIALOG_EVENTTYPE;



/**
 * These are the predefined result codes to be returned by a signal handler.
 * (Note: this is not a typedef because the signal handler has to be able to
 * return GWEN_ERROR codes as well).
 */
enum {
  GWEN_DialogEvent_ResultHandled=0,
  GWEN_DialogEvent_ResultNotHandled,
  GWEN_DialogEvent_ResultAccept,
  GWEN_DialogEvent_ResultReject
};


/**
 * The signal handler should return one of the event result code
 * (see @ref GWEN_DialogEvent_ResultHandled and following) or a GWEN_ERROR
 * code.
 */
typedef int GWENHYWFAR_CB (*GWEN_DIALOG_SIGNALHANDLER)(GWEN_DIALOG *dlg,
						       GWEN_DIALOG_EVENTTYPE t,
						       const char *sender,
						       int intVal,
						       const char *charVal,
						       void *ptrVal);



GWENHYWFAR_API
GWEN_DIALOG *GWEN_Dialog_new(const char *dialogId);

GWENHYWFAR_API
void GWEN_Dialog_free(GWEN_DIALOG *dlg);

GWENHYWFAR_API
int GWEN_Dialog_ReadXml(GWEN_DIALOG *dlg, GWEN_XMLNODE *node);


/**
 * Inserts a sub-dialog into the given dialog. The widgets of the subdialog become
 * children of the main dialog below the widget referenced to by parentName.
 * Please take care that the subdialog doesn't contain widgets with the same name as
 * the main dialog.
 * This only works if @ref GWEN_Gui_ExecDialog has not been called yet!
 * Takes over ownership of the given subdialog.
 */
GWENHYWFAR_API
int GWEN_Dialog_AddSubDialog(GWEN_DIALOG *dlg,
			     const char *parentWidgetName,
			     GWEN_DIALOG *subdlg);


/**
 * Sets the signal handler of the dialog. Please note that this doesn't set the signal
 * handler of any sub-dialog.
 */
GWENHYWFAR_API
GWEN_DIALOG_SIGNALHANDLER GWEN_Dialog_SetSignalHandler(GWEN_DIALOG *dlg,
                                                       GWEN_DIALOG_SIGNALHANDLER fn);




GWENHYWFAR_API
int GWEN_Dialog_SetIntValue(GWEN_DIALOG *dlg,
			    const char *name,
			    int value,
			    int doSignal);

GWENHYWFAR_API
int GWEN_Dialog_GetIntValue(GWEN_DIALOG *dlg,
			    const char *name,
			    int defaultValue);

GWENHYWFAR_API
int GWEN_Dialog_SetCharValue(GWEN_DIALOG *dlg,
			     const char *name,
			     const char *value,
			     int doSignal);

GWENHYWFAR_API
const char *GWEN_Dialog_GetCharValue(GWEN_DIALOG *dlg,
				     const char *name,
				     const char *defaultValue);


GWENHYWFAR_API
int GWEN_Dialog_SetRange(GWEN_DIALOG *dlg,
			 const char *name,
			 int minValue,
			 int maxValue,
			 int doSignal);

GWENHYWFAR_API
int GWEN_Dialog_SetEnabled(GWEN_DIALOG *dlg,
			   const char *name,
			   int b,
			   int doSignal);


GWENHYWFAR_API
int GWEN_Dialog_GetEnabled(GWEN_DIALOG *dlg,
			   const char *name);

GWENHYWFAR_API
int GWEN_Dialog_AddChoice(GWEN_DIALOG *dlg,
			  const char *name,
			  const char *value,
			  int doSignal);

GWENHYWFAR_API
int GWEN_Dialog_ClearChoice(GWEN_DIALOG *dlg,
			    const char *name,
			    int doSignal);




#ifdef __cplusplus
}
#endif



/*@}*/


#endif
