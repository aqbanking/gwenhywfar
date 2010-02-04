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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "dialog_p.h"
#include "widget_l.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>

#include <assert.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(GWEN_DIALOG)
GWEN_LIST_FUNCTIONS(GWEN_DIALOG, GWEN_Dialog)




GWEN_DIALOG *GWEN_Dialog_new(const char *dialogId) {
  GWEN_DIALOG *dlg;

  GWEN_NEW_OBJECT(GWEN_DIALOG, dlg);
  dlg->refCount=1;
  GWEN_INHERIT_INIT(GWEN_DIALOG, dlg);
  GWEN_LIST_INIT(GWEN_DIALOG, dlg);

  if (dialogId && *dialogId)
    dlg->dialogId=strdup(dialogId);

  dlg->widgets=GWEN_Widget_Tree_new();

  dlg->subDialogs=GWEN_Dialog_List_new();


  return dlg;
}



void GWEN_Dialog_free(GWEN_DIALOG *dlg) {
  if (dlg) {
    assert(dlg->refCount);
    if (dlg->refCount>1) {
      dlg->refCount--;
    }
    else {
      GWEN_Dialog_List_free(dlg->subDialogs);

      GWEN_INHERIT_FINI(GWEN_DIALOG, dlg);
      GWEN_LIST_FINI(GWEN_DIALOG, dlg);
      GWEN_Widget_Tree_free(dlg->widgets);
      free(dlg->dialogId);
      dlg->refCount=0;
      GWEN_FREE_OBJECT(dlg);
    }
  }
}



GWEN_DIALOG_SIGNALHANDLER GWEN_Dialog_SetSignalHandler(GWEN_DIALOG *dlg,
						       GWEN_DIALOG_SIGNALHANDLER fn) {
  GWEN_DIALOG_SIGNALHANDLER oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->signalHandler;
  dlg->signalHandler=fn;

  return oh;
}



int GWEN_Dialog_EmitSignal(GWEN_DIALOG *dlg,
			   GWEN_DIALOG_EVENTTYPE t,
			   const char *sender,
			   int intVal,
			   const char *charVal,
			   void *ptrVal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->signalHandler)
    return (dlg->signalHandler)(dlg, t, sender, intVal, charVal, ptrVal);
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No signal handler in dialog [%s]",
	     (dlg->dialogId)?(dlg->dialogId):"-unnamed-");
    return GWEN_DialogEvent_ResultNotHandled;
  }
}



int GWEN_Dialog_EmitSignalToAll(GWEN_DIALOG *dlg,
				GWEN_DIALOG_EVENTTYPE t,
				const char *sender,
				int intVal,
				const char *charVal,
				void *ptrVal) {
  int rv;
  GWEN_DIALOG *subdlg;

  assert(dlg);
  assert(dlg->refCount);

  if (dlg->signalHandler) {
    rv=(dlg->signalHandler)(dlg, t, sender, intVal, charVal, ptrVal);
    if (rv!=GWEN_DialogEvent_ResultHandled &&
	rv!=GWEN_DialogEvent_ResultNotHandled)
      return rv;
  }

  subdlg=GWEN_Dialog_List_First(dlg->subDialogs);
  while(subdlg) {
    rv=GWEN_Dialog_EmitSignalToAll(subdlg, t, sender, intVal, charVal, ptrVal);
    if (rv!=GWEN_DialogEvent_ResultHandled &&
	rv!=GWEN_DialogEvent_ResultNotHandled)
      return rv;
    subdlg=GWEN_Dialog_List_Next(subdlg);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int GWEN_Dialog_AddSubDialog(GWEN_DIALOG *dlg,
                             const char *parentName,
			     GWEN_DIALOG *subdlg) {
  GWEN_WIDGET *wparent;

  wparent=GWEN_Dialog_FindWidgetByName(dlg, parentName);
  if (wparent) {
    GWEN_WIDGET *cw;

    /* move all widgets from the sub dialog to the parent dialog */
    while( (cw=GWEN_Widget_Tree_GetFirst(subdlg->widgets)) ) {
      GWEN_Widget_Tree_Del(cw);
      GWEN_Widget_Tree_AddChild(wparent, cw);
    }

    /* store pointer to parent widget in dialog */
    subdlg->parentWidget=wparent;
    return 0;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Parent widget [%s] not found", parentName);
    return GWEN_ERROR_NOT_FOUND;
  }
}



int GWEN_Dialog__ReadXmlWidget(GWEN_DIALOG *dlg,
			       GWEN_WIDGET *wparent,
			       GWEN_XMLNODE *node) {
  GWEN_XMLNODE *n;
  GWEN_WIDGET *w;
  int rv;

  w=GWEN_Widget_new(dlg);
  rv=GWEN_Widget_ReadXml(w, node);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Widget_free(w);
    return rv;
  }

  if (wparent)
    GWEN_Widget_Tree_AddChild(wparent, w);
  else
    GWEN_Widget_Tree_Add(dlg->widgets, w);


  n=GWEN_XMLNode_FindFirstTag(node, "widget", NULL, NULL);
  while(n) {
    int rv;

    rv=GWEN_Dialog__ReadXmlWidget(dlg, w, n);
    if (rv<0)
      return rv;
    n=GWEN_XMLNode_FindNextTag(n, "widget", NULL, NULL);
  }

  return 0;
}



int GWEN_Dialog_ReadXml(GWEN_DIALOG *dlg, GWEN_XMLNODE *node) {
  int rv;

  assert(dlg);
  assert(dlg->refCount);

  assert(dlg->widgets);
  GWEN_Widget_Tree_Clear(dlg->widgets);

  rv=GWEN_Dialog__ReadXmlWidget(dlg, NULL, node);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Widget_Tree_free(dlg->widgets);
    dlg->widgets=NULL;
    return rv;
  }

  return 0;
}



GWEN_WIDGET *GWEN_Dialog_FindWidgetByName(GWEN_DIALOG *dlg, const char *name) {
  GWEN_WIDGET *w;

  assert(dlg);
  assert(dlg->refCount);
  assert(dlg->widgets);

  if (dlg->parentWidget)
    w=dlg->parentWidget;
  else
    w=GWEN_Widget_Tree_GetFirst(dlg->widgets);

  while(w) {
    const char *s;

    s=GWEN_Widget_GetName(w);
    if (s && *s && strcasecmp(s, name)==0)
      break;
    w=GWEN_Widget_Tree_GetBelow(w);
  }

  return w;
}



GWEN_WIDGET *GWEN_Dialog_FindWidgetByImplData(GWEN_DIALOG *dlg, void *ptr) {
  GWEN_WIDGET *w;

  assert(dlg);
  assert(dlg->refCount);
  assert(dlg->widgets);

  if (dlg->parentWidget)
    w=dlg->parentWidget;
  else
    w=GWEN_Widget_Tree_GetFirst(dlg->widgets);

  while(w) {
    if (ptr==GWEN_Widget_GetImplData(w))
      break;
    w=GWEN_Widget_Tree_GetBelow(w);
  }

  return w;
}



GWEN_WIDGET_TREE *GWEN_Dialog_GetWidgets(const GWEN_DIALOG *dlg) {
  assert(dlg);
  assert(dlg->refCount);
  assert(dlg->widgets);

  return dlg->widgets;
}



GWEN_DIALOG_SETINTVALUE_FN GWEN_Dialog_SetSetIntValueFn(GWEN_DIALOG *dlg,
							GWEN_DIALOG_SETINTVALUE_FN fn) {
  GWEN_DIALOG_SETINTVALUE_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->setIntValueFn;
  dlg->setIntValueFn=fn;
  return oh;
}



GWEN_DIALOG_GETINTVALUE_FN GWEN_Dialog_SetGetIntValueFn(GWEN_DIALOG *dlg,
							GWEN_DIALOG_GETINTVALUE_FN fn) {
  GWEN_DIALOG_GETINTVALUE_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->getIntValueFn;
  dlg->getIntValueFn=fn;
  return oh;
}



GWEN_DIALOG_SETCHARVALUE_FN GWEN_Dialog_SetSetCharValueFn(GWEN_DIALOG *dlg,
							  GWEN_DIALOG_SETCHARVALUE_FN fn) {
  GWEN_DIALOG_SETCHARVALUE_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->setCharValueFn;
  dlg->setCharValueFn=fn;
  return oh;
}



GWEN_DIALOG_GETCHARVALUE_FN GWEN_Dialog_SetGetCharValueFn(GWEN_DIALOG *dlg,
							  GWEN_DIALOG_GETCHARVALUE_FN fn) {
  GWEN_DIALOG_GETCHARVALUE_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->getCharValueFn;
  dlg->getCharValueFn=fn;
  return oh;
}



GWEN_DIALOG_SETRANGE_FN GWEN_Dialog_SetSetRangeFn(GWEN_DIALOG *dlg,
						  GWEN_DIALOG_SETRANGE_FN fn) {
  GWEN_DIALOG_SETRANGE_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->setRangeFn;
  dlg->setRangeFn=fn;
  return oh;
}



GWEN_DIALOG_SETENABLED_FN GWEN_Dialog_SetSetEnabledFn(GWEN_DIALOG *dlg,
						      GWEN_DIALOG_SETENABLED_FN fn) {
  GWEN_DIALOG_SETENABLED_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->setEnabledFn;
  dlg->setEnabledFn=fn;
  return oh;
}



GWEN_DIALOG_GETENABLED_FN GWEN_Dialog_SetGetEnabledFn(GWEN_DIALOG *dlg,
						      GWEN_DIALOG_GETENABLED_FN fn) {
  GWEN_DIALOG_GETENABLED_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->getEnabledFn;
  dlg->getEnabledFn=fn;
  return oh;
}



GWEN_DIALOG_ADDCHOICE_FN GWEN_Dialog_SetAddChoiceFn(GWEN_DIALOG *dlg,
						    GWEN_DIALOG_ADDCHOICE_FN fn) {
  GWEN_DIALOG_ADDCHOICE_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->addChoiceFn;
  dlg->addChoiceFn=fn;
  return oh;
}



GWEN_DIALOG_CLRCHOICE_FN GWEN_Dialog_SetClearChoiceFn(GWEN_DIALOG *dlg,
						      GWEN_DIALOG_CLRCHOICE_FN fn) {
  GWEN_DIALOG_CLRCHOICE_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->clearChoiceFn;
  dlg->clearChoiceFn=fn;
  return oh;
}











int GWEN_Dialog_SetIntValue(GWEN_DIALOG *dlg,
			    const char *name,
			    int value,
			    int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->setIntValueFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->setIntValueFn(dlg, w, value, doSignal);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



int GWEN_Dialog_GetIntValue(GWEN_DIALOG *dlg,
			    const char *name,
			    int defaultValue) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->getIntValueFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->getIntValueFn(dlg, w, defaultValue);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



int GWEN_Dialog_SetCharValue(GWEN_DIALOG *dlg,
			     const char *name,
			     const char *value,
			     int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->setCharValueFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->setCharValueFn(dlg, w, value, doSignal);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



const char *GWEN_Dialog_GetCharValue(GWEN_DIALOG *dlg,
				     const char *name,
				     const char *defaultValue) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->getCharValueFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->getCharValueFn(dlg, w, defaultValue);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return defaultValue;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return defaultValue;
  }
}



int GWEN_Dialog_SetRange(GWEN_DIALOG *dlg,
			 const char *name,
			 int minValue,
			 int maxValue,
			 int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->setRangeFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->setRangeFn(dlg, w, minValue, maxValue, doSignal);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



int GWEN_Dialog_SetEnabled(GWEN_DIALOG *dlg,
			   const char *name,
			   int b,
			   int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->setEnabledFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->setEnabledFn(dlg, w, b, doSignal);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



int GWEN_Dialog_GetEnabled(GWEN_DIALOG *dlg,
			   const char *name) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->getEnabledFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->getEnabledFn(dlg, w);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



int GWEN_Dialog_AddChoice(GWEN_DIALOG *dlg,
			  const char *name,
			  const char *value,
			  int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->addChoiceFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->addChoiceFn(dlg, w, value, doSignal);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



int GWEN_Dialog_ClearChoice(GWEN_DIALOG *dlg,
			    const char *name,
			    int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->clearChoiceFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->clearChoiceFn(dlg, w, doSignal);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}









