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


#include "editbox_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/event.h>
#include <gwenhywfar/buffer.h>

#include <ctype.h>

#include <ncurses.h> /* for beep() and KEY_ defs */


GWEN_INHERIT(GWEN_WIDGET, GWEN_EDITBOX)



GWEN_WIDGET *GWEN_EditBox_new(GWEN_WIDGET *parent,
                              GWEN_TYPE_UINT32 flags,
                              const char *name,
                              const char *text,
                              int x, int y, int width, int height,
                              int maxLen){
  GWEN_WIDGET *w;
  GWEN_EDITBOX *win;
  GWEN_TYPE_UINT32 wflags;

  wflags=(flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS);
  wflags|=
    GWEN_WIDGET_FLAGS_NEEDCURSOR |
    GWEN_WIDGET_FLAGS_HIGHLIGHT;

  DBG_NOTICE(0, "EditFlags are: %04x", wflags);
  w=GWEN_TextWidget_new(parent,
                        wflags,
                        name,
                        text,
                        x,
                        y,
                        width,
                        height);
  GWEN_Widget_SetTypeName(w, "EditBox");
  GWEN_NEW_OBJECT(GWEN_EDITBOX, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_EDITBOX, w, win,
                       GWEN_EditBox_freeData);

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  win->flags=flags;
  win->maxLen=maxLen;
  if (flags & GWEN_EDITBOX_FLAGS_CLEARALL)
    win->clearAllFlag=1;
  if (flags & GWEN_EDITBOX_FLAGS_INSERT)
    win->insertMode=1;
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_EditBox_EventHandler);

  return w;
}



void GWEN_EditBox_freeData(void *bp, void *p) {
  GWEN_EDITBOX *win;

  win=(GWEN_EDITBOX*)p;
  GWEN_FREE_OBJECT(win);
}



int GWEN_EditBox_EnsureLine(GWEN_WIDGET *w, int y) {
  GWEN_EDITBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_EDITBOX, w);
  assert(win);

  if (!win->currLine || win->currY!=y) {
    if (win->currLine)
      GWEN_TextWidget_LineClose(w, win->currLine, 0);
    win->currY=y;
    win->currLine=GWEN_TextWidget_LineOpen(w, y,
                                           (win->flags &
                                            GWEN_EDITBOX_FLAGS_DYNAMIC));
    if (!win->currLine)
      return -1;
  }
  if (!win->currLine)
    return -1;
  if (win->maxLen)
    GWEN_TextWidget_LineSetBorders(w, win->currLine, 0, win->maxLen-1);
  else
    GWEN_TextWidget_LineSetBorders(w, win->currLine, 0,
                                   GWEN_TextWidget_GetVirtualWidth(w)-1);
  GWEN_TextWidget_LineSetInsert(w, win->currLine, win->insertMode);
  return 0;
}



void GWEN_EditBox_AdjustCursor(GWEN_WIDGET *w) {
  int x, y;
  GWEN_EDITBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_EDITBOX, w);
  assert(win);

  x=win->currX-GWEN_TextWidget_GetLeft(w);
  y=win->currY-GWEN_TextWidget_GetTop(w);
  if (win->flags & GWEN_WIDGET_FLAGS_BORDER) {
    x++;
    if (GWEN_Widget_GetHeight(w)>1)
      y++;
  }
  GWEN_Widget_SetCursorX(w, x);
  GWEN_Widget_SetCursorY(w, y);
  DBG_NOTICE(0, "Cursor at: %d/%d", x, y);
}



GWEN_UI_RESULT GWEN_EditBox_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_EDITBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_EDITBOX, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw:
    DBG_INFO(0, "Event: Draw(%s)", GWEN_Widget_GetName(w));
    win->previousHandler(w, e);
    GWEN_EditBox_AdjustCursor(w);
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Refresh:
    DBG_INFO(0, "Event: Refresh(%s)", GWEN_Widget_GetName(w));
    break;

  case GWEN_EventType_Update:
    DBG_INFO(0, "Event: Update(%s)", GWEN_Widget_GetName(w));
    GWEN_Widget_Refresh(w);
    return win->previousHandler(w, e);

  case GWEN_EventType_Scroll:
    DBG_NOTICE(0, "Event: Scroll(%s)", GWEN_Widget_GetName(w));
    win->previousHandler(w, e);
    if (GWEN_EventScroll_GetTodo(e)) {
      GWEN_EditBox_AdjustCursor(w);
    }
    return GWEN_UIResult_Handled;

  case GWEN_EventType_SetText:
    win->currX=0;
    win->currY=0;
    if (win->currLine)
      GWEN_TextWidget_LineClose(w, win->currLine, 0);
    win->currLine=0;
    break;

  case GWEN_EventType_Key: {
    int key;

    DBG_INFO(0, "Event: Key(%s)", GWEN_Widget_GetName(w));
    key=GWEN_EventKey_GetKey(e);

    if (key==KEY_RIGHT) {
      int ww;

      ww=GWEN_Widget_GetWidth(w);
      if (win->flags & GWEN_WIDGET_FLAGS_BORDER)
        ww-=2;
      if (win->currX+1<ww &&
          (win->maxLen==0 || win->currX+1<win->maxLen)) {
        win->currX++;
        GWEN_TextWidget_EnsureVisible(w, win->currX, win->currY,
                                      GWEN_Widget_GetWidth(w), 1);
        GWEN_EditBox_AdjustCursor(w);
      }
      else {
        beep();
        return GWEN_UIResult_Handled;
      }
      return GWEN_UIResult_Handled;
    }
    else if (key==KEY_LEFT) {
      win->clearAllFlag=0;
      if (win->currX==0) {
        beep();
        return GWEN_UIResult_Handled;
      }
      win->currX--;
      GWEN_EditBox_AdjustCursor(w);
      return GWEN_UIResult_Handled;
    }
    else if (key==KEY_DOWN) {
      win->clearAllFlag=0;
      if (win->flags & GWEN_EDITBOX_FLAGS_MULTILINE) {
        if (win->currY+1>=GWEN_Widget_GetHeight(w)) {
          return GWEN_UIResult_Handled;
        }
        win->currY++;
        GWEN_TextWidget_EnsureVisible(w, win->currX, win->currY,
                                      GWEN_Widget_GetWidth(w), 1);
        GWEN_EditBox_AdjustCursor(w);
        return GWEN_UIResult_Handled;
      }
    }
    else if (key==KEY_UP) {
      win->clearAllFlag=0;
      if (win->flags & GWEN_EDITBOX_FLAGS_MULTILINE) {
        if (win->currY==0) {
          return GWEN_UIResult_Handled;
        }
        win->currY--;
        GWEN_EditBox_AdjustCursor(w);
        return GWEN_UIResult_Handled;
      }
    }
    else if (key==GWEN_UI_KEY_INSERT) {
      if (!(win->flags & GWEN_EDITBOX_FLAGS_EDIT))
        return GWEN_UIResult_Handled;
      win->clearAllFlag=0;
      win->insertMode=!win->insertMode;
      beep();
      return GWEN_UIResult_Handled;
    }
    else if (key==GWEN_UI_KEY_DELETE) {
      if (!(win->flags & GWEN_EDITBOX_FLAGS_EDIT))
        return GWEN_UIResult_Handled;
      if (win->clearAllFlag)
        GWEN_TextWidget_LineClear(w, win->currLine);
      win->clearAllFlag=0;
      if (GWEN_EditBox_EnsureLine(w, win->currY)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      GWEN_TextWidget_EnsureVisible(w, win->currX, win->currY,
                                    win->maxLen, 1);
      if (GWEN_TextWidget_LineSetPos(w, win->currLine, win->currX)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      if (GWEN_TextWidget_LineDelete(w, win->currLine, 1)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      if (GWEN_TextWidget_LineRedraw(w, win->currLine)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      return GWEN_UIResult_Handled;
    }
    else if (key==KEY_BACKSPACE) {
      if (!(win->flags & GWEN_EDITBOX_FLAGS_EDIT))
        return GWEN_UIResult_Handled;
      win->clearAllFlag=0;
      if (GWEN_EditBox_EnsureLine(w, win->currY)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      GWEN_TextWidget_EnsureVisible(w, win->currX, win->currY,
                                    win->maxLen, 1);
      if (win->currX==0)
        return GWEN_UIResult_Handled;

      win->currX--;
      if (GWEN_TextWidget_LineSetPos(w, win->currLine, win->currX)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      if (GWEN_TextWidget_LineDelete(w, win->currLine, 1)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      if (GWEN_TextWidget_LineRedraw(w, win->currLine)) {
        beep();
        DBG_NOTICE(0, "here");
        return GWEN_UIResult_Handled;
      }
      GWEN_EditBox_AdjustCursor(w);
      return GWEN_UIResult_Handled;
    }
    else if (isprint(key)) {
      char buffer[2];
      int ww;

      if (!(win->flags & GWEN_EDITBOX_FLAGS_EDIT))
        return GWEN_UIResult_Handled;
      if (GWEN_EditBox_EnsureLine(w, win->currY)) {
        return GWEN_UIResult_Handled;
      }

      GWEN_TextWidget_EnsureVisible(w, win->currX, win->currY,
                                    win->maxLen, 1);
      if (GWEN_TextWidget_LineSetPos(w, win->currLine, win->currX)) {
        return GWEN_UIResult_Handled;
      }
      if (win->clearAllFlag)
        GWEN_TextWidget_LineClear(w, win->currLine);
      win->clearAllFlag=0;
      /* validate character */
      if (win->dataType==GWEN_EditBoxType_Int) {
        if (!isdigit(key)) {
          beep();
          return GWEN_UIResult_Handled;
        }
      }
      buffer[0]=key;
      buffer[1]=0;
      if (GWEN_TextWidget_LineWriteText(w, win->currLine, buffer, 1)) {
        return GWEN_UIResult_Handled;
      }

      ww=GWEN_Widget_GetWidth(w);
      if (win->flags & GWEN_WIDGET_FLAGS_BORDER)
        ww-=2;
      if (win->currX+1<ww &&
          (win->maxLen==0 || win->currX+1<win->maxLen)) {
        win->currX++;
        if (win->maxLen)
          GWEN_TextWidget_EnsureVisible(w, win->currX, win->currY,
                                        win->maxLen, 1);
        else
          GWEN_TextWidget_EnsureVisible(w, win->currX, win->currY,
                                        GWEN_Widget_GetWidth(w), 1);
      }
      if (GWEN_TextWidget_LineRedraw(w, win->currLine)) {
        beep();
        return GWEN_UIResult_Handled;
      }
      GWEN_EditBox_AdjustCursor(w);
      return GWEN_UIResult_Handled;
    }
    return win->previousHandler(w, e);
  }

  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}



GWEN_BUFFER *GWEN_EditBox_GetText(const GWEN_WIDGET *w, int asAscii){
  GWEN_EDITBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_EDITBOX, w);
  assert(win);

  return GWEN_TextWidget_GetText(w, asAscii);
}



void GWEN_EditBox_SetDataType(GWEN_WIDGET *w, GWEN_EDITBOX_TYPE d){
  GWEN_EDITBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_EDITBOX, w);
  assert(win);

  win->dataType=d;
}









