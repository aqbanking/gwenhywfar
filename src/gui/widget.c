/***************************************************************************
    begin       : Wed Jan 20 2010
    copyright   : (C) 2025 by Martin Preuss
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

#define DISABLE_DEBUGLOG


#include "widget_p.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/dialog_be.h>

#include <assert.h>
#include <ctype.h>



GWEN_TREE_FUNCTIONS(GWEN_WIDGET, GWEN_Widget)
GWEN_INHERIT_FUNCTIONS(GWEN_WIDGET)




GWEN_WIDGET *GWEN_Widget_new(GWEN_DIALOG *dlg)
{
  GWEN_WIDGET *w;

  GWEN_NEW_OBJECT(GWEN_WIDGET, w);
  w->refCount=1;
  GWEN_INHERIT_INIT(GWEN_WIDGET, w);
  GWEN_TREE_INIT(GWEN_WIDGET, w);

  w->dialog=dlg;

  return w;
}



void GWEN_Widget_free(GWEN_WIDGET *w)
{
  if (w) {
    assert(w->refCount);
    if (w->refCount>1) {
      w->refCount--;
    }
    else {
      int i;

      GWEN_TREE_FINI(GWEN_WIDGET, w);
      GWEN_INHERIT_FINI(GWEN_WIDGET, w);
      free(w->name);
      for (i=0; i<GWEN_WIDGET_TEXTCOUNT; i++)
        free(w->text[i]);
      free(w->iconFile);
      free(w->imageFile);
      w->refCount=0;
      GWEN_FREE_OBJECT(w);
    }
  }
}




GWEN_DIALOG *GWEN_Widget_GetDialog(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);

  return w->dialog;
}



GWEN_DIALOG *GWEN_Widget_GetTopDialog(const GWEN_WIDGET *w)
{
  GWEN_DIALOG *dlg;
  GWEN_DIALOG *pdlg;

  assert(w);
  assert(w->refCount);

  dlg=w->dialog;
  if (dlg) {
    while ((pdlg=GWEN_Dialog_GetParentDialog(dlg)))
      dlg=pdlg;

    return w->dialog;
  }
  return NULL;
}



void *GWEN_Widget_GetImplData(const GWEN_WIDGET *w, int index)
{
  assert(w);
  assert(w->refCount);
  if (index<GWEN_WIDGET_IMPLDATACOUNT)
    return w->impl_data[index];
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Index out of range");
    return NULL;
  }
}



void GWEN_Widget_SetImplData(GWEN_WIDGET *w, int index, void *ptr)
{
  assert(w);
  assert(w->refCount);
  if (index<GWEN_WIDGET_IMPLDATACOUNT)
    w->impl_data[index]=ptr;
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Index out of range");
  }
}



uint32_t GWEN_Widget_GetFlags(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->flags;
}



void GWEN_Widget_SetFlags(GWEN_WIDGET *w, uint32_t fl)
{
  assert(w);
  assert(w->refCount);
  w->flags=fl;
}



void GWEN_Widget_AddFlags(GWEN_WIDGET *w, uint32_t fl)
{
  assert(w);
  assert(w->refCount);
  w->flags|=fl;
}



void GWEN_Widget_SubFlags(GWEN_WIDGET *w, uint32_t fl)
{
  assert(w);
  assert(w->refCount);
  w->flags&=~fl;
}



GWEN_WIDGET_TYPE GWEN_Widget_GetType(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->wtype;
}



void GWEN_Widget_SetType(GWEN_WIDGET *w, GWEN_WIDGET_TYPE t)
{
  assert(w);
  assert(w->refCount);
  w->wtype=t;
}



int GWEN_Widget_GetColumns(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->columns;
}



void GWEN_Widget_SetColumns(GWEN_WIDGET *w, int i)
{
  assert(w);
  assert(w->refCount);
  w->columns=i;
}



int GWEN_Widget_GetRows(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->rows;
}



void GWEN_Widget_SetRows(GWEN_WIDGET *w, int i)
{
  assert(w);
  assert(w->refCount);
  w->rows=i;
}



int GWEN_Widget_GetGroupId(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->groupId;
}



void GWEN_Widget_SetGroupId(GWEN_WIDGET *w, int i)
{
  assert(w);
  assert(w->refCount);
  w->groupId=i;
}



int GWEN_Widget_GetWidth(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->width;
}



void GWEN_Widget_SetWidth(GWEN_WIDGET *w, int i)
{
  assert(w);
  assert(w->refCount);
  w->width=i;
}



int GWEN_Widget_GetHeight(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->height;
}



void GWEN_Widget_SetHeight(GWEN_WIDGET *w, int i)
{
  assert(w);
  assert(w->refCount);
  w->height=i;
}



const char *GWEN_Widget_GetText(const GWEN_WIDGET *w, int idx)
{
  assert(w);
  assert(w->refCount);
  if (idx<0 || idx>=GWEN_WIDGET_TEXTCOUNT)
    return NULL;
  return w->text[idx];
}



void GWEN_Widget_SetText(GWEN_WIDGET *w, int idx, const char *s)
{
  assert(w);
  assert(w->refCount);

  if (idx>=0 && idx<GWEN_WIDGET_TEXTCOUNT) {
    free(w->text[idx]);
    if (s)
      w->text[idx]=strdup(s);
    else
      w->text[idx]=NULL;
  }
}



const char *GWEN_Widget_GetName(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->name;
}



void GWEN_Widget_SetName(GWEN_WIDGET *w, const char *s)
{
  assert(w);
  assert(w->refCount);
  free(w->name);
  if (s)
    w->name=strdup(s);
  else
    w->name=NULL;
}



const char *GWEN_Widget_GetIconFileName(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->iconFile;
}



void GWEN_Widget_SetIconFileName(GWEN_WIDGET *w, const char *s)
{
  assert(w);
  assert(w->refCount);
  free(w->iconFile);
  if (s)
    w->iconFile=strdup(s);
  else
    w->iconFile=NULL;
}



const char *GWEN_Widget_GetImageFileName(const GWEN_WIDGET *w)
{
  assert(w);
  assert(w->refCount);
  return w->imageFile;
}



void GWEN_Widget_SetImageFileName(GWEN_WIDGET *w, const char *s)
{
  assert(w);
  assert(w->refCount);
  free(w->imageFile);
  if (s)
    w->imageFile=strdup(s);
  else
    w->imageFile=NULL;
}





GWEN_WIDGET_TYPE GWEN_Widget_Type_fromString(const char *s)
{
  if (s && *s) {
    if (strcasecmp(s, "unknown")==0)
      return GWEN_Widget_TypeUnknown;
    else if (strcasecmp(s, "none")==0)
      return GWEN_Widget_TypeNone;
    else if (strcasecmp(s, "label")==0)
      return GWEN_Widget_TypeLabel;
    else if (strcasecmp(s, "pushButton")==0)
      return GWEN_Widget_TypePushButton;
    else if (strcasecmp(s, "lineEdit")==0)
      return GWEN_Widget_TypeLineEdit;
    else if (strcasecmp(s, "textEdit")==0)
      return GWEN_Widget_TypeTextEdit;
    else if (strcasecmp(s, "comboBox")==0)
      return GWEN_Widget_TypeComboBox;
    else if (strcasecmp(s, "radioButton")==0)
      return GWEN_Widget_TypeRadioButton;
    else if (strcasecmp(s, "progressBar")==0)
      return GWEN_Widget_TypeProgressBar;
    else if (strcasecmp(s, "groupBox")==0)
      return GWEN_Widget_TypeGroupBox;
    else if (strcasecmp(s, "hSpacer")==0)
      return GWEN_Widget_TypeHSpacer;
    else if (strcasecmp(s, "vSpacer")==0)
      return GWEN_Widget_TypeVSpacer;
    else if (strcasecmp(s, "hLayout")==0)
      return GWEN_Widget_TypeHLayout;
    else if (strcasecmp(s, "vLayout")==0)
      return GWEN_Widget_TypeVLayout;
    else if (strcasecmp(s, "gridLayout")==0)
      return GWEN_Widget_TypeGridLayout;
    else if (strcasecmp(s, "listBox")==0)
      return GWEN_Widget_TypeListBox;
    else if (strcasecmp(s, "dialog")==0)
      return GWEN_Widget_TypeDialog;
    else if (strcasecmp(s, "tabBook")==0)
      return GWEN_Widget_TypeTabBook;
    else if (strcasecmp(s, "tabPage")==0)
      return GWEN_Widget_TypeTabPage;
    else if (strcasecmp(s, "widgetStack")==0)
      return GWEN_Widget_TypeWidgetStack;
    else if (strcasecmp(s, "checkBox")==0)
      return GWEN_Widget_TypeCheckBox;
    else if (strcasecmp(s, "scrollArea")==0)
      return GWEN_Widget_TypeScrollArea;
    else if (strcasecmp(s, "hLine")==0)
      return GWEN_Widget_TypeHLine;
    else if (strcasecmp(s, "vLine")==0)
      return GWEN_Widget_TypeVLine;
    else if (strcasecmp(s, "textBrowser")==0)
      return GWEN_Widget_TypeTextBrowser;
    else if (strcasecmp(s, "spinBox")==0)
      return GWEN_Widget_TypeSpinBox;
    else if (strcasecmp(s, "hSplitter")==0)
      return GWEN_Widget_TypeHSplitter;
    else if (strcasecmp(s, "vSplitter")==0)
      return GWEN_Widget_TypeVSplitter;
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown widget type [%s]", s);
    }
  }
  return GWEN_Widget_TypeUnknown;
}



const char *GWEN_Widget_Type_toString(GWEN_WIDGET_TYPE t)
{
  switch (t) {
  case GWEN_Widget_TypeNone:
    return "none";
  case GWEN_Widget_TypeLabel:
    return "label";
  case GWEN_Widget_TypePushButton:
    return "pushButton";
  case GWEN_Widget_TypeLineEdit:
    return "lineEdit";
  case GWEN_Widget_TypeTextEdit:
    return "textEdit";
  case GWEN_Widget_TypeComboBox:
    return "comboBox";
  case GWEN_Widget_TypeRadioButton:
    return "radioButton";
  case GWEN_Widget_TypeProgressBar:
    return "progressBar";
  case GWEN_Widget_TypeGroupBox:
    return "groupBox";
  case GWEN_Widget_TypeHSpacer:
    return "hSpacer";
  case GWEN_Widget_TypeVSpacer:
    return "vSpacer";
  case GWEN_Widget_TypeHLayout:
    return "hLayout";
  case GWEN_Widget_TypeVLayout:
    return "vLayout";
  case GWEN_Widget_TypeGridLayout:
    return "gridLayout";
  case GWEN_Widget_TypeListBox:
    return "listBox";
  case GWEN_Widget_TypeDialog:
    return "dialog";
  case GWEN_Widget_TypeTabBook:
    return "tabBook";
  case GWEN_Widget_TypeTabPage:
    return "tabPage";
  case GWEN_Widget_TypeWidgetStack:
    return "widgetStack";
  case GWEN_Widget_TypeCheckBox:
    return "checkBox";
  case GWEN_Widget_TypeScrollArea:
    return "scrollArea";
  case GWEN_Widget_TypeHLine:
    return "hLine";
  case GWEN_Widget_TypeVLine:
    return "vLine";
  case GWEN_Widget_TypeTextBrowser:
    return "textBrowser";
  case GWEN_Widget_TypeSpinBox:
    return "spinBox";
  case GWEN_Widget_TypeHSplitter:
    return "hSplitter";
  case GWEN_Widget_TypeVSplitter:
    return "vSplitter";
  case GWEN_Widget_TypeUnknown:
    return "unknown";
  }

  return "unknown";
}



uint32_t GWEN_Widget_Flags_fromString(const char *s)
{
  uint32_t fl=0;

  if (s && *s) {
    char *copy;
    char *p;

    copy=strdup(s);
    p=copy;

    while (*p) {
      char *wstart;

      /* skip blanks */
      while (*p && isspace(*p))
        p++;
      /* save start of word */
      wstart=p;

      /* find end of word */
      while (*p && !(isspace(*p) || *p==','))
        p++;
      if (*p)
        /* set blank or comma to 0, advance pointer */
        *(p++)=0;

      /* parse flags */
      if (strcasecmp(wstart, "fillX")==0)
        fl|=GWEN_WIDGET_FLAGS_FILLX;
      else if (strcasecmp(wstart, "fillY")==0)
        fl|=GWEN_WIDGET_FLAGS_FILLY;
      else if (strcasecmp(wstart, "readOnly")==0)
        fl|=GWEN_WIDGET_FLAGS_READONLY;
      else if (strcasecmp(wstart, "password")==0)
        fl|=GWEN_WIDGET_FLAGS_PASSWORD;
      else if (strcasecmp(wstart, "default")==0)
        fl|=GWEN_WIDGET_FLAGS_DEFAULT_WIDGET;
      else if (strcasecmp(wstart, "decorShrinkable")==0)
        fl|=GWEN_WIDGET_FLAGS_DECOR_SHRINKABLE;
      else if (strcasecmp(wstart, "decorStretchable")==0)
        fl|=GWEN_WIDGET_FLAGS_DECOR_STRETCHABLE;
      else if (strcasecmp(wstart, "decorMinimize")==0)
        fl|=GWEN_WIDGET_FLAGS_DECOR_MINIMIZE;
      else if (strcasecmp(wstart, "decorMaximize")==0)
        fl|=GWEN_WIDGET_FLAGS_DECOR_MAXIMIZE;
      else if (strcasecmp(wstart, "decorClose")==0)
        fl|=GWEN_WIDGET_FLAGS_DECOR_CLOSE;
      else if (strcasecmp(wstart, "decorMenu")==0)
        fl|=GWEN_WIDGET_FLAGS_DECOR_MENU;
      else if (strcasecmp(wstart, "fixedWidth")==0)
        fl|=GWEN_WIDGET_FLAGS_FIXED_WIDTH;
      else if (strcasecmp(wstart, "fixedHeight")==0)
        fl|=GWEN_WIDGET_FLAGS_FIXED_HEIGHT;
      else if (strcasecmp(wstart, "equalWidth")==0)
        fl|=GWEN_WIDGET_FLAGS_EQUAL_WIDTH;
      else if (strcasecmp(wstart, "equalHeight")==0)
        fl|=GWEN_WIDGET_FLAGS_EQUAL_HEIGHT;
      else if (strcasecmp(wstart, "justifyLeft")==0)
        fl|=GWEN_WIDGET_FLAGS_JUSTIFY_LEFT;
      else if (strcasecmp(wstart, "justifyRight")==0)
        fl|=GWEN_WIDGET_FLAGS_JUSTIFY_RIGHT;
      else if (strcasecmp(wstart, "justifyTop")==0)
        fl|=GWEN_WIDGET_FLAGS_JUSTIFY_TOP;
      else if (strcasecmp(wstart, "justifyBottom")==0)
        fl|=GWEN_WIDGET_FLAGS_JUSTIFY_BOTTOM;
      else if (strcasecmp(wstart, "justifyCenterX")==0)
        fl|=GWEN_WIDGET_FLAGS_JUSTIFY_CENTERX;
      else if (strcasecmp(wstart, "justifyCenterY")==0)
        fl|=GWEN_WIDGET_FLAGS_JUSTIFY_CENTERY;
      else if (strcasecmp(wstart, "noWordWrap")==0)
        fl|=GWEN_WIDGET_FLAGS_NO_WORDWRAP;
      else if (strcasecmp(wstart, "frameSunken")==0)
        fl|=GWEN_WIDGET_FLAGS_FRAME_SUNKEN;
      else if (strcasecmp(wstart, "frameRaised")==0)
        fl|=GWEN_WIDGET_FLAGS_FRAME_RAISED;
      else if (strcasecmp(wstart, "frameThick")==0)
        fl|=GWEN_WIDGET_FLAGS_FRAME_THICK;
      else if (strcasecmp(wstart, "frameGroove")==0)
        fl|=GWEN_WIDGET_FLAGS_FRAME_GROOVE;
    }
    if (copy)
      free(copy);
  }

  return fl;
}



int GWEN_Widget_ReadXml(GWEN_WIDGET *w, GWEN_XMLNODE *node)
{
  const char *s;

  s=GWEN_XMLNode_GetProperty(node, "name", NULL);
  if (s && *s)
    GWEN_Widget_SetName(w, s);

  s=GWEN_XMLNode_GetProperty(node, "type", "unknown");
  if (s && *s) {
    w->wtype=GWEN_Widget_Type_fromString(s);
    if (w->wtype==GWEN_Widget_TypeUnknown) {
      DBG_ERROR(GWEN_LOGDOMAIN, "unknown type in node");
      GWEN_XMLNode_Dump(node, 2);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "type property missing in node");
    return GWEN_ERROR_BAD_DATA;
  }

  s=GWEN_XMLNode_GetProperty(node, "flags", NULL);
  if (s && *s)
    w->flags=GWEN_Widget_Flags_fromString(s);

  s=GWEN_XMLNode_GetProperty(node, "columns", NULL);
  if (s && *s) {
    if (1!=sscanf(s, "%d", &(w->columns))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Value [%s] is not an integer", s);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  s=GWEN_XMLNode_GetProperty(node, "rows", NULL);
  if (s && *s) {
    if (1!=sscanf(s, "%d", &(w->rows))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Value [%s] is not an integer", s);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  s=GWEN_XMLNode_GetProperty(node, "width", NULL);
  if (s && *s) {
    if (1!=sscanf(s, "%d", &(w->width))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Value [%s] is not an integer", s);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  s=GWEN_XMLNode_GetProperty(node, "height", NULL);
  if (s && *s) {
    if (1!=sscanf(s, "%d", &(w->height))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Value [%s] is not an integer", s);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  s=GWEN_XMLNode_GetProperty(node, "text", NULL);
  if (s && *s)
    GWEN_Widget_SetText(w, 0, GWEN_Dialog_TranslateString(w->dialog, s));

  s=GWEN_XMLNode_GetProperty(node, "icon", NULL);
  if (s && *s)
    GWEN_Widget_SetIconFileName(w, s);

  s=GWEN_XMLNode_GetProperty(node, "image", NULL);
  if (s && *s)
    GWEN_Widget_SetImageFileName(w, s);

  s=GWEN_XMLNode_GetProperty(node, "groupId", NULL);
  if (s && *s) {
    if (1!=sscanf(s, "%d", &(w->groupId))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Value [%s] is not an integer", s);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  return 0;
}



GWEN_WIDGET_SETINTPROPERTY_FN GWEN_Widget_SetSetIntPropertyFn(GWEN_WIDGET *w,
                                                              GWEN_WIDGET_SETINTPROPERTY_FN fn)
{
  GWEN_WIDGET_SETINTPROPERTY_FN of;

  assert(w);
  assert(w->refCount);

  of=w->setIntPropertyFn;
  w->setIntPropertyFn=fn;
  return of;
}



GWEN_WIDGET_GETINTPROPERTY_FN GWEN_Widget_SetGetIntPropertyFn(GWEN_WIDGET *w,
                                                              GWEN_WIDGET_GETINTPROPERTY_FN fn)
{
  GWEN_WIDGET_GETINTPROPERTY_FN of;

  assert(w);
  assert(w->refCount);

  of=w->getIntPropertyFn;
  w->getIntPropertyFn=fn;
  return of;
}



GWEN_WIDGET_SETCHARPROPERTY_FN GWEN_Widget_SetSetCharPropertyFn(GWEN_WIDGET *w,
                                                                GWEN_WIDGET_SETCHARPROPERTY_FN fn)
{
  GWEN_WIDGET_SETCHARPROPERTY_FN of;

  assert(w);
  assert(w->refCount);

  of=w->setCharPropertyFn;
  w->setCharPropertyFn=fn;
  return of;
}



GWEN_WIDGET_GETCHARPROPERTY_FN GWEN_Widget_SetGetCharPropertyFn(GWEN_WIDGET *w,
                                                                GWEN_WIDGET_GETCHARPROPERTY_FN fn)
{
  GWEN_WIDGET_GETCHARPROPERTY_FN of;

  assert(w);
  assert(w->refCount);

  of=w->getCharPropertyFn;
  w->getCharPropertyFn=fn;
  return of;
}



GWEN_WIDGET_ADDCHILDGUIWIDGET_FN GWEN_Widget_SetAddChildGuiWidgetFn(GWEN_WIDGET *w,
                                                                    GWEN_WIDGET_ADDCHILDGUIWIDGET_FN fn)
{
  GWEN_WIDGET_ADDCHILDGUIWIDGET_FN of;

  assert(w);
  assert(w->refCount);

  of=w->addChildGuiWidgetFn;
  w->addChildGuiWidgetFn=fn;
  return of;
}



int GWEN_Widget_SetIntProperty(GWEN_WIDGET *w,
                               GWEN_DIALOG_PROPERTY prop,
                               int index,
                               int value,
                               int doSignal)
{
  assert(w);
  assert(w->refCount);

  if (w->setIntPropertyFn)
    return w->setIntPropertyFn(w, prop, index, value, doSignal);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Widget_GetIntProperty(GWEN_WIDGET *w,
                               GWEN_DIALOG_PROPERTY prop,
                               int index,
                               int defaultValue)
{
  assert(w);
  assert(w->refCount);

  if (w->getIntPropertyFn)
    return w->getIntPropertyFn(w, prop, index, defaultValue);
  else
    return defaultValue;
}



int GWEN_Widget_SetCharProperty(GWEN_WIDGET *w,
                                GWEN_DIALOG_PROPERTY prop,
                                int index,
                                const char *value,
                                int doSignal)
{
  assert(w);
  assert(w->refCount);

  if (w->setCharPropertyFn)
    return w->setCharPropertyFn(w, prop, index, value, doSignal);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



const char *GWEN_Widget_GetCharProperty(GWEN_WIDGET *w,
                                        GWEN_DIALOG_PROPERTY prop,
                                        int index,
                                        const char *defaultValue)
{
  assert(w);
  assert(w->refCount);

  if (w->getCharPropertyFn)
    return w->getCharPropertyFn(w, prop, index, defaultValue);
  else
    return defaultValue;
}



int GWEN_Widget_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild)
{
  assert(w);
  assert(w->refCount);

  if (w->addChildGuiWidgetFn)
    return w->addChildGuiWidgetFn(w, wChild);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}





