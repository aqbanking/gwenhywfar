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
#include "gui_l.h"

#include <aqbanking/banking.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>

#include <assert.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(GWEN_DIALOG)
GWEN_LIST_FUNCTIONS(GWEN_DIALOG, GWEN_Dialog)




GWEN_DIALOG *GWEN_Dialog_new(const char *dialogId) {
  GWEN_DIALOG *dlg;
  int rv;
  GWEN_DB_NODE *db=NULL;

  GWEN_NEW_OBJECT(GWEN_DIALOG, dlg);
  dlg->refCount=1;
  GWEN_INHERIT_INIT(GWEN_DIALOG, dlg);
  GWEN_LIST_INIT(GWEN_DIALOG, dlg);

  if (dialogId && *dialogId)
    dlg->dialogId=strdup(dialogId);

  dlg->widgets=GWEN_Widget_Tree_new();

  dlg->subDialogs=GWEN_Dialog_List_new();

  /* try to load preferences */
  rv=GWEN_Gui_ReadDialogPrefs(dialogId, NULL, &db);
  if (rv<0) {
    DBG_WARN(GWEN_LOGDOMAIN, "Could not read dialog preferences (%d)", rv);
    dlg->dbPreferences=GWEN_DB_Group_new("preferences");
  }
  else {
    dlg->dbPreferences=db;
  }

  return dlg;
}



void GWEN_Dialog_free(GWEN_DIALOG *dlg) {
  if (dlg) {
    assert(dlg->refCount);

    if (dlg->refCount>1) {
      dlg->refCount--;
    }
    else {
      int rv;

      /* try to save preferences */
      rv=GWEN_Gui_WriteDialogPrefs(dlg->dialogId, dlg->dbPreferences);
      if (rv<0) {
	DBG_WARN(GWEN_LOGDOMAIN, "Could not write dialog preferences (%d)", rv);
      }

      GWEN_Dialog_List_free(dlg->subDialogs);

      GWEN_INHERIT_FINI(GWEN_DIALOG, dlg);
      GWEN_LIST_FINI(GWEN_DIALOG, dlg);
      GWEN_Widget_Tree_free(dlg->widgets);
      free(dlg->dialogId);
      dlg->refCount=0;
      GWEN_DB_Group_free(dlg->dbPreferences);

      GWEN_FREE_OBJECT(dlg);
    }
  }
}



const char *GWEN_Dialog_GetId(const GWEN_DIALOG *dlg) {
  assert(dlg);
  assert(dlg->refCount);

  return dlg->dialogId;
}



uint32_t GWEN_Dialog_GetGuiId(const GWEN_DIALOG *dlg) {
  assert(dlg);
  assert(dlg->refCount);

  return dlg->guiId;
}



void GWEN_Dialog_SetGuiId(GWEN_DIALOG *dlg, uint32_t guiid) {
  assert(dlg);
  assert(dlg->refCount);

  dlg->guiId=guiid;
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
			   const char *sender) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->signalHandler)
    return (dlg->signalHandler)(dlg, t, sender);
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No signal handler in dialog [%s]",
	     (dlg->dialogId)?(dlg->dialogId):"-unnamed-");
    return GWEN_DialogEvent_ResultNotHandled;
  }
}



int GWEN_Dialog_EmitSignalToAll(GWEN_DIALOG *dlg,
				GWEN_DIALOG_EVENTTYPE t,
				const char *sender) {
  int rv;
  GWEN_DIALOG *subdlg;

  assert(dlg);
  assert(dlg->refCount);

  if (dlg->signalHandler) {
    rv=(dlg->signalHandler)(dlg, t, sender);
    if (rv!=GWEN_DialogEvent_ResultHandled &&
	rv!=GWEN_DialogEvent_ResultNotHandled)
      return rv;
  }

  subdlg=GWEN_Dialog_List_First(dlg->subDialogs);
  while(subdlg) {
    rv=GWEN_Dialog_EmitSignalToAll(subdlg, t, sender);
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



int GWEN_Dialog_ReadXmlFile(GWEN_DIALOG *dlg, const char *fname) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nDialog;
  int rv;

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(n, fname,
		       GWEN_XML_FLAGS_DEFAULT |
		       GWEN_XML_FLAGS_HANDLE_HEADERS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(n);
    return rv;
  }

  nDialog=GWEN_XMLNode_FindFirstTag(n, "dialog", NULL, NULL);
  if (nDialog==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Dialog element not found in XML file [%s]", fname);
    GWEN_XMLNode_free(n);
    return rv;
  }

  rv=GWEN_Dialog_ReadXml(dlg, nDialog);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(n);
    return rv;
  }

  GWEN_XMLNode_free(n);
  return 0;
}



GWEN_WIDGET *GWEN_Dialog_FindWidgetByName(const GWEN_DIALOG *dlg, const char *name) {
  GWEN_WIDGET *w;

  assert(dlg);
  assert(dlg->refCount);
  assert(dlg->widgets);

  if (dlg->parentWidget)
    w=dlg->parentWidget;
  else
    w=GWEN_Widget_Tree_GetFirst(dlg->widgets);

  /* empty name always corresponds to the root */
  if (name==NULL || *name==0)
    return w;

  while(w) {
    const char *s;

    s=GWEN_Widget_GetName(w);
    if (s && *s && strcasecmp(s, name)==0)
      break;
    w=GWEN_Widget_Tree_GetBelow(w);
  }

  return w;
}



GWEN_WIDGET *GWEN_Dialog_FindWidgetByImplData(const GWEN_DIALOG *dlg, int index, const void *ptr) {
  GWEN_WIDGET *w;

  assert(dlg);
  assert(dlg->refCount);
  assert(dlg->widgets);

  if (dlg->parentWidget)
    w=dlg->parentWidget;
  else
    w=GWEN_Widget_Tree_GetFirst(dlg->widgets);

  while(w) {
    if (ptr==GWEN_Widget_GetImplData(w, index))
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



GWEN_DIALOG_SETINTPROPERTY_FN GWEN_Dialog_SetSetIntPropertyFn(GWEN_DIALOG *dlg,
							      GWEN_DIALOG_SETINTPROPERTY_FN fn) {
  GWEN_DIALOG_SETINTPROPERTY_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->setIntPropertyFn;
  dlg->setIntPropertyFn=fn;
  return oh;
}



GWEN_DIALOG_GETINTPROPERTY_FN GWEN_Dialog_SetGetIntPropertyFn(GWEN_DIALOG *dlg,
							      GWEN_DIALOG_GETINTPROPERTY_FN fn) {
  GWEN_DIALOG_GETINTPROPERTY_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->getIntPropertyFn;
  dlg->getIntPropertyFn=fn;
  return oh;
}



GWEN_DIALOG_SETCHARPROPERTY_FN GWEN_Dialog_SetSetCharPropertyFn(GWEN_DIALOG *dlg,
								GWEN_DIALOG_SETCHARPROPERTY_FN fn) {
  GWEN_DIALOG_SETCHARPROPERTY_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->setCharPropertyFn;
  dlg->setCharPropertyFn=fn;
  return oh;
}



GWEN_DIALOG_GETCHARPROPERTY_FN GWEN_Dialog_SetGetCharPropertyFn(GWEN_DIALOG *dlg,
								GWEN_DIALOG_GETCHARPROPERTY_FN fn) {
  GWEN_DIALOG_GETCHARPROPERTY_FN oh;

  assert(dlg);
  assert(dlg->refCount);

  oh=dlg->getCharPropertyFn;
  dlg->getCharPropertyFn=fn;
  return oh;
}






int GWEN_Dialog_SetIntProperty(GWEN_DIALOG *dlg,
			       const char *name,
			       GWEN_DIALOG_PROPERTY prop,
			       int index,
			       int value,
			       int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->setIntPropertyFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->setIntPropertyFn(dlg, w, prop, index, value, doSignal);
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



int GWEN_Dialog_GetIntProperty(GWEN_DIALOG *dlg,
			       const char *name,
			       GWEN_DIALOG_PROPERTY prop,
			       int index,
			       int defaultProperty) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->getIntPropertyFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->getIntPropertyFn(dlg, w, prop, index, defaultProperty);
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



int GWEN_Dialog_SetCharProperty(GWEN_DIALOG *dlg,
				const char *name,
				GWEN_DIALOG_PROPERTY prop,
				int index,
				const char *value,
				int doSignal) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->setCharPropertyFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->setCharPropertyFn(dlg, w, prop, index, value, doSignal);
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



const char *GWEN_Dialog_GetCharProperty(GWEN_DIALOG *dlg,
					const char *name,
                                        GWEN_DIALOG_PROPERTY prop,
					int index,
					const char *defaultProperty) {
  assert(dlg);
  assert(dlg->refCount);

  if (dlg->getCharPropertyFn) {
    GWEN_WIDGET *w;

    w=GWEN_Dialog_FindWidgetByName(dlg, name);
    if (w)
      return dlg->getCharPropertyFn(dlg, w, prop, index, defaultProperty);
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Widget [%s] not found", name);
      return defaultProperty;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Function pointer not set");
    return defaultProperty;
  }
}



int GWEN_Dialog_RemoveWidget(GWEN_DIALOG *dlg, const char *name) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    GWEN_Widget_Tree_Del(w);
    GWEN_Widget_free(w);
    return 0;
  }

  return GWEN_ERROR_NOT_FOUND;
}


uint32_t GWEN_Dialog_GetWidgetFlags(const GWEN_DIALOG *dlg, const char *name) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    return GWEN_Widget_GetFlags(w);
  }

  return 0;
}



void GWEN_Dialog_SetWidgetFlags(GWEN_DIALOG *dlg, const char *name, uint32_t fl) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    GWEN_Widget_SetFlags(w, fl);
  }
}



void GWEN_Dialog_AddWidgetFlags(GWEN_DIALOG *dlg, const char *name, uint32_t fl) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    GWEN_Widget_AddFlags(w, fl);
  }
}



void GWEN_Dialog_SubWidgetFlags(GWEN_DIALOG *dlg, const char *name, uint32_t fl) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    GWEN_Widget_SubFlags(w, fl);
  }
}



int GWEN_Dialog_GetWidgetColumns(const GWEN_DIALOG *dlg, const char *name) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    return GWEN_Widget_GetColumns(w);
  }

  return -1;
}



void GWEN_Dialog_SetWidgetColumns(GWEN_DIALOG *dlg, const char *name, int i) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    GWEN_Widget_SetColumns(w, i);
  }
}



int GWEN_Dialog_GetWidgetRows(const GWEN_DIALOG *dlg, const char *name) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    return GWEN_Widget_GetRows(w);
  }

  return -1;
}



void GWEN_Dialog_SetWidgetRows(GWEN_DIALOG *dlg, const char *name, int i) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    GWEN_Widget_SetRows(w, i);
  }
}



void GWEN_Dialog_SetWidgetText(GWEN_DIALOG *dlg, const char *name, const char *t) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    GWEN_Widget_SetText(w, 0, t);
  }
}



const char *GWEN_Dialog_GetWidgetText(const GWEN_DIALOG *dlg, const char *name) {
  GWEN_WIDGET *w;

  w=GWEN_Dialog_FindWidgetByName(dlg, name);
  if (w) {
    return GWEN_Widget_GetText(w, 0);
  }

  return NULL;
}






GWEN_DB_NODE *GWEN_Dialog_GetPreferences(const GWEN_DIALOG *dlg) {
  assert(dlg);
  assert(dlg->refCount);

  return dlg->dbPreferences;
}




