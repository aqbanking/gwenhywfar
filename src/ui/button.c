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


#include "button_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/event.h>
#include <gwenhywfar/buffer.h>

#include <ncurses.h> /* for keydefs */


GWEN_INHERIT(GWEN_WIDGET, GWEN_BUTTON)



GWEN_WIDGET *GWEN_Button_new(GWEN_WIDGET *parent,
                             GWEN_TYPE_UINT32 flags,
                             const char *name,
                             const char *text,
                             GWEN_TYPE_UINT32 commandId,
                             int x, int y, int width, int height){
  GWEN_WIDGET *w;
  GWEN_BUTTON *win;

  w=GWEN_Widget_new(parent,
                    flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS,
                    name, text,
                    x,
                    y,
                    width,
                    height);
  GWEN_Widget_SetTypeName(w, "Button");
  GWEN_NEW_OBJECT(GWEN_BUTTON, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_BUTTON, w, win,
                       GWEN_Button_freeData);
  win->flags=flags;
  win->commandId=commandId;
  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_Button_EventHandler);
  GWEN_Widget_SetCursorX(w, -1);
  GWEN_Widget_SetCursorY(w, -1);

  return w;
}



void GWEN_Button_freeData(void *bp, void *p) {
  GWEN_BUTTON *win;

  win=(GWEN_BUTTON*)p;
  GWEN_FREE_OBJECT(win);
}




GWEN_UI_RESULT GWEN_Button_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_BUTTON *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_BUTTON, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw: {
    break;
  }

  case GWEN_EventType_Update:
    break;

  case GWEN_EventType_Key: {
    int key;

    DBG_NOTICE(0, "Event: Key(%s)", GWEN_Widget_GetName(w));
    key=GWEN_EventKey_GetKey(e);
    if (key==13 || key==32) {
      const char *c;

      if (win->flags & GWEN_BUTTON_FLAGS_CHECKBOX) {
        win->isChecked=!win->isChecked;
        if (win->isChecked)
          c="X";
        else
          c=" ";
        GWEN_Widget_WriteAt(w, 0, 0, c, strlen(c));
        GWEN_Widget_Refresh(w);
      }
      else {
        GWEN_EVENT *e;

        assert(w);
        beep();
        e=GWEN_EventCommand_new(win->commandId);
        assert(e);
        if (GWEN_Widget_SendEvent(w, w, e)) {
          DBG_INFO(0, "Could not send event");
          GWEN_Event_free(e);
        }
      }
      return GWEN_UIResult_Handled;
    }
    break;
  }

  case GWEN_EventType_Highlight:
    DBG_NOTICE(0, "Button: Highlighting");
    return win->previousHandler(w, e);

  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}



int GWEN_Button_IsChecked(const GWEN_WIDGET *w){
  GWEN_BUTTON *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_BUTTON, w);
  assert(win);
  return win->isChecked;
}



void GWEN_Button_SetChecked(GWEN_WIDGET *w, int b){
  GWEN_BUTTON *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_BUTTON, w);
  assert(win);
  win->isChecked=b;
}















