/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
    copyright   : (C) 2004 by Martin Preuss
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


#include "checkbox_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/event.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/ui/button.h>
#include <gwenhywfar/ui/textwidget.h>

#include <ctype.h>

#include <ncurses.h> /* for beep() and KEY_ defs */


GWEN_INHERIT(GWEN_WIDGET, GWEN_CHECKBOX)



GWEN_WIDGET *GWEN_CheckBox_new(GWEN_WIDGET *parent,
                               GWEN_TYPE_UINT32 flags,
                               const char *name,
                               const char *text,
                               int x, int y, int width, int height){
  GWEN_WIDGET *w;
  GWEN_CHECKBOX *win;
  GWEN_TYPE_UINT32 wflags;
  int wx, wy, ww, wh;

  wflags=(flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS &
          ~GWEN_WIDGET_FLAGS_FOCUSABLE &
          ~GWEN_WIDGET_FLAGS_HIGHLIGHT);

  DBG_NOTICE(0, "ZZZ: CheckboxFlags are: %04x", wflags);
  w=GWEN_Widget_new(parent,
                    wflags,
                    name,
                    0,
                    x,
                    y,
                    width,
                    height);
  GWEN_Widget_SetTypeName(w, "CheckBox");
  GWEN_NEW_OBJECT(GWEN_CHECKBOX, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_CHECKBOX, w, win,
                       GWEN_CheckBox_freeData);

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  win->flags=flags;
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_CheckBox_EventHandler);

  ww=GWEN_Widget_GetWidth(w);
  wh=GWEN_Widget_GetHeight(w);
  wx=wy=0;
  if (flags & GWEN_WIDGET_FLAGS_BORDER) {
    wx++;
    ww-=2;
    if (wh>1) {
      wy++;
      wh-=2;
    }
  }

  DBG_NOTICE(0, "ZZZ: Creating button here: %d/%d",
             wx, wy);
  win->wButton=GWEN_Button_new(w,
                               GWEN_WIDGET_FLAGS_DEFAULT |
                               GWEN_WIDGET_FLAGS_BORDER |
                               GWEN_BUTTON_FLAGS_CHECKBOX |
                               GWEN_WIDGET_FLAGS_HIGHLIGHT,
                               "CheckBoxButton",
                               0,
                               0,
                               wx, wy, 3, 1);
  win->wText=GWEN_TextWidget_new(w,
                                 GWEN_WIDGET_FLAGS_DEFAULT &
                                 ~GWEN_WIDGET_FLAGS_FOCUSABLE &
                                 ~GWEN_WIDGET_FLAGS_HIGHLIGHT,
				 "CheckBoxText",
				 text,
				 wx+4, wy, ww-4, wh);
  return w;
}



void GWEN_CheckBox_freeData(void *bp, void *p) {
  GWEN_CHECKBOX *win;

  win=(GWEN_CHECKBOX*)p;
  GWEN_FREE_OBJECT(win);
}



GWEN_UI_RESULT GWEN_CheckBox_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_CHECKBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_CHECKBOX, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw:
    DBG_INFO(0, "Event: Draw(%s)", GWEN_Widget_GetName(w));
    win->previousHandler(w, e);
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Refresh:
    DBG_INFO(0, "Event: Refresh(%s)", GWEN_Widget_GetName(w));
    break;

  case GWEN_EventType_Update:
    DBG_INFO(0, "Event: Update(%s)", GWEN_Widget_GetName(w));
    GWEN_Widget_Refresh(w);
    return win->previousHandler(w, e);

  case GWEN_EventType_Key: {
    int key;

    DBG_NOTICE(0, "Event: Key(%s)", GWEN_Widget_GetName(w));
    key=GWEN_EventKey_GetKey(e);
    if (key==13 || key==32) {
      GWEN_EVENT *newE;

      newE=GWEN_EventKey_new(key);
      if (GWEN_Widget_SendEvent(win->wButton, w, newE)) {
        DBG_DEBUG(0, "Could not send event to button");
        GWEN_Event_free(newE);
      }
      return GWEN_UIResult_Handled;
    }
    return win->previousHandler(w, e);
  }

  case GWEN_EventType_SetColour: {
    GWEN_WIDGET_COLOUR col;

    DBG_INFO(0, "Event: SetColour(%s)", GWEN_Widget_GetName(w));
    col=GWEN_EventSetColour_GetColour(e);
    if (!(GWEN_Widget_GetState(w) & GWEN_WIDGET_STATE_HIGHLIGHT)) {
      win->previousHandler(w, e);
      DBG_NOTICE(0, "Setting colour: %d", col);
      GWEN_Widget_SetColour(win->wButton, col);
      GWEN_Widget_SetColour(win->wText, col);
      if (GWEN_Widget_Update(w))
        return GWEN_UIResult_Error;
    }
    else {
      DBG_NOTICE(0, "Storing colour: %d", col);
    }

  }

  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}



int GWEN_CheckBox_IsChecked(const GWEN_WIDGET *w){
  GWEN_CHECKBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_CHECKBOX, w);
  assert(win);
  return GWEN_Button_IsChecked(win->wButton);
}



void GWEN_CheckBox_SetChecked(GWEN_WIDGET *w, int b){
  GWEN_CHECKBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_CHECKBOX, w);
  assert(win);
  GWEN_Button_SetChecked(win->wButton, b);
}















