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


#include "dropdownbox_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/event.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/ui/button.h>
#include <gwenhywfar/ui/editbox.h>
#include <gwenhywfar/ui/scrollwidget.h>
#include <gwenhywfar/ui/textwidget.h>

#include <ctype.h>

#include <ncurses.h> /* for beep() and KEY_ defs */


GWEN_INHERIT(GWEN_WIDGET, GWEN_DROPDOWNBOX)



GWEN_WIDGET *GWEN_DropDownBox_new(GWEN_WIDGET *parent,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *name,
                                  int x, int y, int width, int height,
                                  const GWEN_STRINGLIST *sl){
  GWEN_WIDGET *w;
  GWEN_DROPDOWNBOX *win;
  GWEN_TYPE_UINT32 wflags;
  int wx, wy, ww, wh;
  GWEN_STRINGLISTENTRY *se;
  const char *p;

  wflags=(flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS &
          ~GWEN_WIDGET_FLAGS_FOCUSABLE &
          ~GWEN_WIDGET_FLAGS_HIGHLIGHT);

  w=GWEN_Widget_new(parent,
                    wflags,
                    name,
                    0,
                    x,
                    y,
                    width,
                    height);
  GWEN_Widget_SetTypeName(w, "DropDownBox");
  GWEN_NEW_OBJECT(GWEN_DROPDOWNBOX, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_DROPDOWNBOX, w, win,
                       GWEN_DropDownBox_freeData);

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  win->flags=flags;
  win->stringList=GWEN_StringList_dup(sl);
  if (GWEN_StringList_Count(win->stringList)==0)
    GWEN_StringList_AppendString(win->stringList, "", 0, 1);
  GWEN_Widget_SetEventHandler(w, GWEN_DropDownBox_EventHandler);

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

  se=GWEN_StringList_FirstEntry(sl);
  if (se)
    p=GWEN_StringListEntry_Data(se);
  else
    p="";
  win->wEdit=GWEN_EditBox_new(w,
                              GWEN_WIDGET_FLAGS_DEFAULT |
                              ((flags & GWEN_DROPDOWNBOX_FLAGS_EDIT)
                               ?GWEN_EDITBOX_FLAGS_EDIT:0),
                              "DropDownBoxEdit",
                              p,
                              wx, wy, ww-4, wh, ww-4);

  win->wButton=GWEN_Button_new(w,
                               GWEN_WIDGET_FLAGS_DEFAULT |
                               GWEN_WIDGET_FLAGS_BORDER |
                               GWEN_WIDGET_FLAGS_HIGHLIGHT,
                               "DropDownBoxButton",
                               "Y",
                               0,
                               ww-3, wy, 3, 1);
  /* subscribe to key events for the button */
  GWEN_Widget_Subscribe(win->wButton, GWEN_EventType_Key, w);
  GWEN_Widget_Subscribe(win->wEdit, GWEN_EventType_Key, w);

  return w;
}



void GWEN_DropDownBox_freeData(void *bp, void *p) {
  GWEN_DROPDOWNBOX *win;

  win=(GWEN_DROPDOWNBOX*)p;
  GWEN_StringList_free(win->stringList);

  GWEN_FREE_OBJECT(win);
}



GWEN_UI_RESULT GWEN_DropDownBox_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_DROPDOWNBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_DROPDOWNBOX, w);
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

    key=GWEN_EventKey_GetKey(e);
    if (GWEN_Event_DueToSubscription(e)) {
      DBG_NOTICE(0, "Informed due to subscription");
      if (GWEN_Event_GetRecipient(e)==win->wButton ||
          GWEN_Event_GetRecipient(e)==win->wEdit) {
        /* key event for the button */
        if (key==13) {
          /* open drop down list */
          GWEN_DropDownBox_ShowList(w);
        }
      }
      return GWEN_UIResult_Handled;
    }
    else {
      DBG_NOTICE(0, "Event: Key(%s)", GWEN_Widget_GetName(w));
      if (key==13 || key==32) {
        GWEN_EVENT *newE;

        newE=GWEN_EventKey_new(key);
        assert(newE);
        if (GWEN_Widget_SendEvent(win->wButton, w, newE)) {
          DBG_DEBUG(0, "Could not send event to button");
          GWEN_Event_free(newE);
        }
        return GWEN_UIResult_Handled;
      }
      return win->previousHandler(w, e);
    }
  }

  case GWEN_EventType_SetText: {
    const char *t;
    int pos;

    win->previousHandler(w, e);
    t=GWEN_EventSetText_GetText(e);
    pos=-1;
    if (t) {
      GWEN_STRINGLISTENTRY *se;
      int i;

      se=GWEN_StringList_FirstEntry(win->stringList);
      while(se) {
        const char *p;

        p=GWEN_StringListEntry_Data(se);
        if (p) {
          if (strcasecmp(p, t)==0) {
            pos=i;
          }
        }
        i++;
        se=GWEN_StringListEntry_Next(se);
      }
    } /* while */
    if (pos!=-1) {
      GWEN_EVENT *newE;

      free(win->chosenText);
      win->chosenText=0;
      win->chosenText=strdup(t);
      GWEN_Widget_SetText(win->wEdit, t,
                          GWEN_EventSetText_GetMode(e));
      newE=GWEN_EventChosen_new(t, 0, 0);
      assert(newE);
      if (GWEN_Widget_SendEvent(w, w, newE)) {
        DBG_DEBUG(0, "Could not send event to button");
        GWEN_Event_free(newE);
      }
    }
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_SetColour: {
    GWEN_WIDGET_COLOUR col;

    DBG_INFO(0, "Event: SetColour(%s)", GWEN_Widget_GetName(w));
    col=GWEN_EventSetColour_GetColour(e);
    if (!(GWEN_Widget_GetState(w) & GWEN_WIDGET_STATE_HIGHLIGHT)) {
      win->previousHandler(w, e);
      DBG_NOTICE(0, "Setting colour: %d", col);
      GWEN_Widget_SetColour(win->wButton, col);
      GWEN_Widget_SetColour(win->wEdit, col);
      if (GWEN_Widget_Update(w))
        return GWEN_UIResult_Error;
    }
  }

  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}



void GWEN_DropDownBox_ShowList(GWEN_WIDGET *w){
  GWEN_WIDGET *wList;
  GWEN_WIDGET *wScroller;
  int x, y, ww, wh;
  GWEN_DROPDOWNBOX *win;
  GWEN_BUFFER *buf;
  GWEN_STRINGLISTENTRY *se;
  int first;
  GWEN_TYPE_UINT32 scrollerFlags;
  GWEN_WIDGET *prevFocus;
  int focusLost;
  int i;
  int pos;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_DROPDOWNBOX, w);
  assert(win);

  prevFocus=GWEN_UI_GetFocusWidget();
  ww=GWEN_Widget_GetWidth(win->wEdit);
  x=GWEN_Widget_GetPhysicalX(w);

  wh=GWEN_StringList_Count(win->stringList)+2;
  if (wh>GWEN_UI_GetLines())
    wh=GWEN_UI_GetLines();

  y=GWEN_Widget_GetPhysicalY(win->wEdit)+1;
  DBG_NOTICE(0, "ZZZ: Dimensions (%d/%d, %d/%d)",
             x, y, ww, wh);
  if (y+wh>GWEN_UI_GetLines()) {
    wh=5;
    DBG_NOTICE(0, "ZZZ: Adjusted (%d/%d, %d/%d)",
               x, y, ww, wh);
  }

  if (y+wh>GWEN_UI_GetLines()) {
    /* bottom of list is too low, can we adjust y ? */
    if (y>wh) {
      /* yes, we can, so do it */
      DBG_NOTICE(0, "ZZZ: Adjusted (%d/%d, %d/%d)",
                 x, y, ww, wh);
      y-=(y-wh);
    }
    else {
      /* nope, we can't, is it enough to shorten the height ? */
      wh=GWEN_UI_GetLines()-y;
      if (wh<5 && y>=5) {
        /* height is too small, adjust Y as well */
        y-=5;
        wh=5;
        DBG_NOTICE(0, "ZZZ: Adjusted (%d/%d, %d/%d)",
                   x, y, ww, wh);
      }
      else {
        /* very small screen, use all we can */
        y=0;
        wh=GWEN_UI_GetLines();
        DBG_NOTICE(0, "ZZZ: Adjusted (%d/%d, %d/%d)",
                   x, y, ww, wh);
      }
    }
  }

  DBG_NOTICE(0, "ZZZ: Dimensions of list: %d/%d, %d/%d",
             x, y, ww, wh);

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf, "<gwen>");

  se=GWEN_StringList_FirstEntry(win->stringList);
  first=1;
  i=0;
  pos=0;
  while(se) {
    const char *p;

    p=GWEN_StringListEntry_Data(se);
    if (p) {
      if (win->chosenText) {
        DBG_NOTICE(0, "VVV: Checking %s against %s (pos=%d)",
                   p, win->chosenText,
                   pos);
        if (strcasecmp(p, win->chosenText)==0) {
          pos=i;
          DBG_NOTICE(0, "VVV: Found matching text (pos=%d)", pos);
        }
      }
      if (!first)
        GWEN_Buffer_AppendString(buf, "<br>");
      GWEN_Buffer_AppendString(buf, p);
    }
    first=0;
    i++;
    se=GWEN_StringListEntry_Next(se);
  } /* while */
  GWEN_Buffer_AppendString(buf, "</gwen>");

  if (GWEN_StringList_Count(win->stringList)>wh-2)
    scrollerFlags=GWEN_SCROLLWIN_FLAGS_VSLIDER;
  else
    scrollerFlags=0;
  wScroller=GWEN_ScrollWidget_new(0,
                                  GWEN_WIDGET_FLAGS_DEFAULT |
                                  GWEN_WIDGET_FLAGS_BORDER |
                                  scrollerFlags,
                                  "DropDownScroller",
                                  x, y, ww, wh);
  wList=GWEN_TextWidget_new(GWEN_ScrollWidget_GetViewPort(wScroller),
                            GWEN_WIDGET_FLAGS_DEFAULT |
                            GWEN_TEXTWIDGET_FLAGS_LINEMODE |
                            GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT,
                            "DropDownList",
                            GWEN_Buffer_GetStart(buf),
                            0, 0, 0, 0);
  GWEN_Buffer_free(buf);
  GWEN_Widget_Subscribe(wList, GWEN_EventType_Focus,
                        wScroller);

  GWEN_Widget_Redraw(wScroller);
  GWEN_TextWidget_SetPos(wList, pos);
  GWEN_TextWidget_EnsureVisible(wList, 0, pos,
                                GWEN_Widget_GetWidth(wList), 1);
  GWEN_Widget_SetFocus(wList);

  focusLost=0;
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e) {
      break;
    }
    if (GWEN_Event_GetType(e)==GWEN_EventType_Chosen) {
      const char *p;

      p=GWEN_EventChosen_GetText(e);
      free(win->chosenText);
      win->chosenText=0;
      if (p) {
        win->chosenText=strdup(p);
      }
      else win->chosenText=0;
      GWEN_Widget_Close(wScroller);
      GWEN_Event_free(e);
    }
    else if (GWEN_Event_GetType(e)==GWEN_EventType_Focus) {
      if (GWEN_EventFocus_GetFocusEventType(e)==GWEN_EventFocusType_Lost){
        GWEN_Widget_Close(wScroller);
        GWEN_UI_Flush();
        free(win->chosenText);
        win->chosenText=0;
        focusLost=1;
        break;
      }
    }
    else {
      GWEN_UI_DispatchEvent(e);
      GWEN_Event_free(e);
    }
  }

  if (win->chosenText && !focusLost) {
    GWEN_Widget_SetText(w, win->chosenText, // DEBUG
                        GWEN_EventSetTextMode_Replace);
  }

  GWEN_Widget_free(wScroller);
  if (prevFocus && !focusLost)
    GWEN_Widget_SetFocus(prevFocus);
}



GWEN_BUFFER *GWEN_DropDownBox_GetText(const GWEN_WIDGET *w, int asAscii){
  GWEN_DROPDOWNBOX *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_DROPDOWNBOX, w);
  assert(win);

  return GWEN_EditBox_GetText(win->wEdit, asAscii);
}









