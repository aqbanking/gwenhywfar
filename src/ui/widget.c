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


#include "widget_p.h"
#include "ui_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>
#include <time.h>
#include <string.h>
#include <ctype.h>



GWEN_LIST_FUNCTIONS(GWEN_WIDGET, GWEN_Widget)
GWEN_INHERIT_FUNCTIONS(GWEN_WIDGET)


static GWEN_TYPE_UINT32 GWEN_Widget_LastId=0;


GWEN_WIDGET *GWEN_Widget_new(GWEN_WIDGET *parent,
                             GWEN_TYPE_UINT32 flags,
                             const char *name,
                             const char *text,
                             int x, int y, int width, int height){
  GWEN_WIDGET *w;
  GWEN_EVENT *e;

  GWEN_NEW_OBJECT(GWEN_WIDGET, w);
  GWEN_INHERIT_INIT(GWEN_WIDGET, w);
  GWEN_LIST_INIT(GWEN_WIDGET, w);
  w->usage=1;
  w->eventHandler=GWEN_Widget__HandleEvent;

  if (GWEN_Widget_LastId==0)
    GWEN_Widget_LastId=time(0);
  w->id=++GWEN_Widget_LastId;

  w->children=GWEN_Widget_List_new();

  if (name)
    w->name=strdup(name);

  w->parent=parent;
  if (parent) {
    if (width==0)
      width=GWEN_Widget_GetWidth(parent);
    if (height==0)
      height=GWEN_Widget_GetHeight(parent);
  }
  else {
    if (width==0)
      width=COLS;
    if (height==0)
      height=LINES;
  }
  w->x=x;
  w->y=y;
  w->width=width;
  w->height=height;

  w->flags=flags;
  if (parent) {
    GWEN_WIDGET *pw;
    int nx, ny;

    nx=x;
    ny=y;
    pw=parent;
    while(pw) {
      nx+=pw->x;
      ny+=pw->y;
      pw=pw->parent;
    }

    DBG_NOTICE(0, "Creating window %d/%d, %d/%d",
               nx, ny, width, height);
    w->window=newwin(height, width,
                     ny,
                     nx);
    assert(w->window);

    //w->window=derwin(parent->window,
    //                 height, width,
    //                 y, x);
    GWEN_Widget_List_Add(w, w->parent->children);
    wrefresh(w->window);
  }
  else {
    GWEN_UI_AddRootWidget(w);
    DBG_NOTICE(0, "Creating root window %d/%d, %d/%d",
               x, y, width, height);
    w->window=newwin(height, width,
                     y, x);
    assert(w->window);
    refresh();
  }
  leaveok(w->window, FALSE);

  if (w->flags & GWEN_WIDGET_FLAGS_PANEL) {
    w->panel=new_panel(w->window);
  }

  keypad(w->window, (w->flags & GWEN_WIDGET_FLAGS_KEYPAD)?TRUE:FALSE);
  scrollok(w->window, FALSE);
  w->colour=GWEN_WidgetColour_Default;

  e=GWEN_EventCreated_new();
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
  }

  if (text)
    GWEN_Widget_SetText(w, text, GWEN_EventSetTextMode_Replace);
  return w;
}



void GWEN_Widget_Attach(GWEN_WIDGET *w){
  assert(w);
  w->usage++;
}



void GWEN_Widget_free(GWEN_WIDGET *w){
  if (w) {
    assert(w->usage);
    if ((--w->usage)==0) {
      GWEN_INHERIT_FINI(GWEN_WIDGET, w);
      GWEN_Widget_List_free(w->children);
      GWEN_LIST_FINI(GWEN_WIDGET, w);
      GWEN_FREE_OBJECT(w);
    }
  }
}



int GWEN_Widget_GetPhysicalX(const GWEN_WIDGET *w){
  assert(w);
  if (w->parent) {
    GWEN_WIDGET *pw;
    int nx;

    nx=w->x;
    pw=w->parent;
    while(pw) {
      nx+=pw->x;
      pw=pw->parent;
    }
    return nx;
  }
  return 0;
}



int GWEN_Widget_GetPhysicalY(const GWEN_WIDGET *w){
  assert(w);
  if (w->parent) {
    GWEN_WIDGET *pw;
    int ny;

    ny=w->y;
    pw=w->parent;
    while(pw) {
      ny+=pw->y;
      pw=pw->parent;
    }
    return ny;
  }
  return 0;
}



GWEN_WIDGET *GWEN_Widget_GetParent(const GWEN_WIDGET *w){
  assert(w);
  return w->parent;
}



int GWEN_Widget_GetX(const GWEN_WIDGET *w){
  assert(w);
  return w->x;
}



int GWEN_Widget_GetY(const GWEN_WIDGET *w){
  assert(w);
  return w->y;
}



int GWEN_Widget_GetWidth(const GWEN_WIDGET *w){
  assert(w);
  return w->width;
}



int GWEN_Widget_GetHeight(const GWEN_WIDGET *w){
  assert(w);
  return w->height;
}



const char *GWEN_Widget_GetText(const GWEN_WIDGET *w){
  assert(w);
  return w->text;
}



const char *GWEN_Widget_GetName(const GWEN_WIDGET *w){
  assert(w);
  return w->name;
}



GWEN_TYPE_UINT32 GWEN_Widget_GetFlags(const GWEN_WIDGET *w){
  assert(w);
  return w->flags;
}



GWEN_TYPE_UINT32 GWEN_Widget_GetId(const GWEN_WIDGET *w){
  assert(w);
  return w->id;
}



GWEN_WIDGET_EVENTHANDLER_FN GWEN_Widget_GetEventHandler(const GWEN_WIDGET *w){
  assert(w);
  return w->eventHandler;
}



void GWEN_Widget_SetEventHandler(GWEN_WIDGET *w,
                                 GWEN_WIDGET_EVENTHANDLER_FN f){
  assert(w);
  w->eventHandler=f;
}



int GWEN_Widget_SendEvent(GWEN_WIDGET *wRecipient,
                          GWEN_WIDGET *wSender,
                          GWEN_EVENT *e){
  assert(wSender);
  return GWEN_UI_SendEvent(wRecipient, wSender, e, 0);
}



GWEN_EVENT *GWEN_Widget_PeekNextEvent(GWEN_WIDGET *w) {
  assert(w);
  return GWEN_UI_PeekNextEvent(w);
}



GWEN_EVENT *GWEN_Widget_GetNextEvent(GWEN_WIDGET *w) {
  assert(w);
  return GWEN_UI_GetNextEvent(w);
}



GWEN_UI_RESULT GWEN_Widget_HandleEvent(GWEN_WIDGET *w,
                                       GWEN_EVENT *e){
  assert(w);
  if (w->eventHandler) {
    GWEN_UI_RESULT rv;

    rv=w->eventHandler(w, e);
    if (rv==GWEN_UIResult_Handled) {
      DBG_VERBOUS(0, "Event handled");
      return GWEN_UIResult_Handled;
    }
    else if (rv==GWEN_UIResult_Error) {
      DBG_INFO(0, "Error handling event");
      return GWEN_UIResult_Error;
    }
    else if (rv==GWEN_UIResult_Quit) {
      DBG_INFO(0, "Event handler want's to stop the loop");
      return GWEN_UIResult_Quit;
    }
  }

  return GWEN_UIResult_NotHandled;
}



GWEN_UI_RESULT GWEN_Widget__HandleEvent(GWEN_WIDGET *w,
                                        GWEN_EVENT *e){
  const char *p;

  assert(w);

  if (GWEN_Event_GetRecipient(e)!=w) {
    /* do not handle events for children */
    DBG_NOTICE(0, "Not for me...");
    return GWEN_UIResult_NotHandled;
  }

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_SetText:
    DBG_INFO(0, "Event: SetText(%s)", w->name);
    p=GWEN_EventSetText_GetText(e);
    free(w->text);
    if (p) w->text=strdup(p);
    else w->text=0;
    if (GWEN_Widget_Update(w))
      return GWEN_UIResult_Error;
    return GWEN_UIResult_Handled;

  case GWEN_EventType_SetColour:
    DBG_INFO(0, "Event: SetColour(%s)", w->name);
    wbkgd(w->window, COLOR_PAIR(GWEN_EventSetColour_GetColour(e)));

    w->colour=GWEN_EventSetColour_GetColour(e);
    wattr_set(w->window, 0, w->colour, 0);
    if (GWEN_Widget_Update(w))
      return GWEN_UIResult_Error;
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Draw: {
    int x, y;

    DBG_INFO(0, "Event: Draw(%s)", w->name);
    x=0;
    y=0;
    if (w->text) {
      int length;

      length=strlen(w->text);
      if (w->flags & GWEN_WIDGET_FLAGS_HCENTER)
        x=(w->width-length)/2;
      if (w->flags & GWEN_WIDGET_FLAGS_VCENTER)
        y=w->height/2;
    }
    wbkgd(w->window, COLOR_PAIR(w->colour));
    wattr_set(w->window, 0, w->colour, 0);

    if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
      box(w->window, 0, 0);
      x++;
      y++;
    }
    if (w->text)
      mvwprintw(w->window, y, x, w->text);
    wrefresh(w->window);
    //doupdate();
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_ChgAtts: {
    GWEN_TYPE_UINT32 atts;
    int nattr;
    int set;

    DBG_INFO(0, "Event: ChgAtts(%s)", w->name);
    atts=GWEN_EventChgAtts_GetAtts(e);
    set=GWEN_EventChgAtts_GetSet(e);
    nattr=0;
    if (atts & GWEN_WIDGET_ATT_STANDOUT)
      nattr|=A_STANDOUT;
    if (atts & GWEN_WIDGET_ATT_UNDERLINE)
      nattr|=A_UNDERLINE;
    if (atts & GWEN_WIDGET_ATT_REVERSE)
      nattr|=A_REVERSE;
    if (set==1)
      wattron(w->window, nattr);
    else if (set==0)
      wattroff(w->window, nattr);
    else
      wattrset(w->window, nattr);

    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Highlight: {
    int x, y;
    int maxc;
    int len;
    GWEN_WIDGET_COLOUR hi;

    DBG_INFO(0, "Event: Highlight(%s)", w->name);
    x=GWEN_EventHighlight_GetX(e);
    y=GWEN_EventHighlight_GetY(e);
    len=GWEN_EventHighlight_GetLen(e);
    hi=GWEN_EventHighlight_GetHi(e);
    maxc=w->width;
    if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
      x++;
      y++;
      maxc-=2;
    }
    if (maxc>len)
      maxc=len;
    DBG_VERBOUS(0, "Highlighting %d, %d, %d bytes (true:%d)",
                x, y, len, hi);
    if (hi==0)
      hi=w->colour;
    mvwchgat(w->window, y, x, maxc, 0,
             hi, 0);
    touchline(w->window, y, 1);
    wrefresh(w->window);
    //doupdate();
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_WriteAt: {
    int x, y;
    int maxc;
    int len;
    const unsigned char *p;

    DBG_INFO(0, "Event: WriteAt(%s)", w->name);

    x=GWEN_EventWriteAt_GetX(e);
    y=GWEN_EventWriteAt_GetY(e);
    len=GWEN_EventWriteAt_GetLen(e);
    DBG_NOTICE(0, "Writing at %d, %d, %d bytes",
               x, y, len);

    maxc=w->width;
    if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
      x++;
      y++;
      maxc-=2;
    }
    if (maxc>len)
      maxc=len;
    p=(const unsigned char*)GWEN_EventWriteAt_GetText(e);
    //DBG_NOTICE(0, "Writing at %d, %d, %d bytes: \"%s\"",
    //           x, y, len, p);
    if (p) {
      int attrs;
      GWEN_BUFFER *mbuf;
      int c;

      mbuf=GWEN_Buffer_new(0, strlen(p), 0, 1);
      //DBG_NOTICE(0, "Dump:");
      //GWEN_Text_LogString(p , strlen(p), 0, GWEN_LoggerLevelNotice);
      if (GWEN_Text_UnescapeToBufferTolerant(p, mbuf)) {
        DBG_ERROR(0, "Bad string");
        GWEN_Buffer_free(mbuf);
        return GWEN_UIResult_Error;
      }
      GWEN_Buffer_Rewind(mbuf);
      //GWEN_Text_LogString(GWEN_Buffer_GetStart(mbuf),
      //                    GWEN_Buffer_GetUsedBytes(mbuf),
      //                    0,
      //                    GWEN_LoggerLevelNotice);
      wmove(w->window, y, x);
      wbkgdset(w->window, COLOR_PAIR(w->colour));
      wattr_set(w->window, 0, w->colour, 0);
      attrs=COLOR_PAIR(w->colour);
      while( (c=GWEN_Buffer_ReadByte(mbuf))!=-1) {
        if (maxc==0)
          break;
        if (c==GWEN_WIDGET_ATT_ESC_CHAR) {
          c=GWEN_Buffer_ReadByte(mbuf);
          if (c==-1) {
            DBG_ERROR(0, "Bad string");
            GWEN_Buffer_free(mbuf);
            return GWEN_UIResult_Error;
          }
          c=(unsigned char)c;
          DBG_VERBOUS(0, "Setting new attributes %02x", c);
          attrs=COLOR_PAIR(w->colour);
          if (c & GWEN_WIDGET_ATT_STANDOUT)
            attrs|=A_BOLD;
          if (c & GWEN_WIDGET_ATT_UNDERLINE)
            attrs|=A_UNDERLINE;
          if (c & GWEN_WIDGET_ATT_REVERSE)
            attrs|=A_REVERSE;
        }
        else if (c==GWEN_WIDGET_CHAR_ESC_CHAR) {
          chtype ch;

          c=GWEN_Buffer_ReadByte(mbuf);
          if (c==-1) {
            DBG_ERROR(0, "Bad string");
            GWEN_Buffer_free(mbuf);
            return GWEN_UIResult_Error;
          }
          c=(unsigned char)c;
          switch(c) {
          case GWEN_WIDGET_CHAR_VLINE: ch=ACS_VLINE; break;
          case GWEN_WIDGET_CHAR_HLINE: ch=ACS_HLINE; break;
          case GWEN_WIDGET_CHAR_ULCORNER: ch=ACS_ULCORNER; break;
          case GWEN_WIDGET_CHAR_URCORNER: ch=ACS_URCORNER; break;
          case GWEN_WIDGET_CHAR_LLCORNER: ch=ACS_LLCORNER; break;
          case GWEN_WIDGET_CHAR_LRCORNER: ch=ACS_LRCORNER; break;
          case GWEN_WIDGET_CHAR_LTEE: ch=ACS_LTEE; break;
          case GWEN_WIDGET_CHAR_RTEE: ch=ACS_RTEE; break;
          case GWEN_WIDGET_CHAR_TTEE: ch=ACS_TTEE; break;
          case GWEN_WIDGET_CHAR_BTEE: ch=ACS_BTEE; break;
          case GWEN_WIDGET_CHAR_BLOCK: ch=ACS_BLOCK; break;
          default: ch='?'; break;
          } /* switch */
          if (waddch(w->window, ch | attrs)==ERR) {
            DBG_VERBOUS(0, "Error writing to window (%02x)", c);
            break;
          }
          maxc--;
        }
        else if (isprint(c)) {
          if (waddch(w->window, c | attrs)==ERR) {
            DBG_VERBOUS(0, "Error writing to window (%02x)", c);
            break;
          }
          //if (waddch(w->window, c & 0x7f)==ERR) {
          //  DBG_INFO(0, "Error writing to window");
          //  break;
          //}
          maxc--;
        }
        else {
          DBG_NOTICE(0, "Unprintable character detected.");
          abort();
        }
      } /* while */
      GWEN_Buffer_free(mbuf);
      //wclrtoeol(w->window);
    }
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Scroll: {
    int v;

    DBG_INFO(0, "Event: Scroll(%s)", w->name);
    if (GWEN_EventScroll_GetTodo(e)) {
      v=GWEN_EventScroll_GetYBy(e);
      if (v) {
        wscrl(w->window, v);
        wrefresh(w->window);
        GWEN_Widget_Scrolled(w, 0, v);
        //doupdate();
        return GWEN_UIResult_Handled;
      }
    }
    else {
      DBG_VERBOUS(0, "Scrolling done, nothing to do here");
    }
    return GWEN_UIResult_NotHandled;
  }

  case GWEN_EventType_Clear: {
    int x, y;
    GWEN_EVENT_CLEAR_MODE m;

    DBG_INFO(0, "Event: Clear(%s)", w->name);
    x=GWEN_EventClear_GetX(e);
    y=GWEN_EventClear_GetY(e);
    m=GWEN_EventClear_GetMode(e);
    DBG_VERBOUS(0, "Clearing: %d/%d (%d)", x, y, m);
    wmove(w->window, y, x);
    switch(m) {
    case GWEN_EventClearMode_All:
      wclear(w->window);
      break;
    case GWEN_EventClearMode_ToEOL:
      wclrtoeol(w->window);
      break;
    case GWEN_EventClearMode_ToBottom:
      wclrtobot(w->window);
      break;
    default:
      DBG_DEBUG(0, "Unknown mode (%d)", m);
      return GWEN_UIResult_NotHandled;
    } /* switch */
    wrefresh(w->window);
    //doupdate();
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Update:
    DBG_INFO(0, "Event: Update(%s)", w->name);
    GWEN_Widget_Refresh(w);
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Refresh:
    DBG_INFO(0, "Event: Refresh(%s)", w->name);
    wrefresh(w->window);
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Move:
    DBG_INFO(0, "Event: Move(%s)", w->name);
    return GWEN_UIResult_NotHandled;

  case GWEN_EventType_Key: {
    int key;

    DBG_INFO(0, "Event: Key(%s)", w->name);
    key=GWEN_EventKey_GetKey(e);
    if (key==KEY_F(10)) {
      DBG_NOTICE(0, "Finished.");
      return GWEN_UIResult_Quit;
    }
    else if (key==KEY_F(5)) {
      DBG_NOTICE(0, "Updating");
      GWEN_Widget_Update(w);
    }
    else if (key==KEY_F(6)) {
      DBG_NOTICE(0, "Refreshing");
      GWEN_Widget_Refresh(w);
    }
    else if (key==KEY_F(7)) {
      DBG_NOTICE(0, "Redrawing");
      GWEN_Widget_Redraw(w);
    }
    return GWEN_UIResult_NotHandled;
  }

  case GWEN_EventType_Focus: {
    GWEN_EVENT_FOCUS_TYPE ft;

    DBG_INFO(0, "Event: Focus(%s)", w->name);
    ft=GWEN_EventFocus_GetFocusEventType(e);
    if (ft==GWEN_EventFocusType_Got)
      w->flags|=GWEN_WIDGET_FLAGS_HASFOCUS;
    else
      w->flags&=~GWEN_WIDGET_FLAGS_HASFOCUS;
    return GWEN_UIResult_Handled;
  }

  default:
    DBG_VERBOUS(0, "Event: Unknown(%s)", w->name);
    return GWEN_UIResult_NotHandled;
  } /* switch */

  /* event handled */
  return GWEN_UIResult_Handled;
}



int GWEN_Widget_SetText(GWEN_WIDGET *w, const char *t,
                        GWEN_EVENT_SETTEXT_MODE m){
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventSetText_new(t, m);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_Move(GWEN_WIDGET *w, int x, int y){
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventMove_new(x, y, w->x, w->y);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_Redraw(GWEN_WIDGET *w) {
  GWEN_EVENT *e;
  GWEN_WIDGET *subw;

  assert(w);
  e=GWEN_EventDraw_new();
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }

  /* redraw all children */
  subw=GWEN_Widget_List_First(w->children);
  while(subw) {
    GWEN_Widget_Redraw(subw);
    subw=GWEN_Widget_List_Next(subw);
  } /* while */

  return 0;
}



GWEN_WIDGET_COLOUR GWEN_Widget_GetColour(const GWEN_WIDGET *w){
  assert(w);
  return w->colour;
}



int GWEN_Widget_SetColour(GWEN_WIDGET *w, GWEN_WIDGET_COLOUR c) {
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventSetColour_new(c);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_SetFocus(GWEN_WIDGET *w){
  GWEN_UI_SetFocus(w);
  return 0;
}



int GWEN_Widget_Destroy(GWEN_WIDGET *w) {
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventDestroy_new();
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_Update(GWEN_WIDGET *w) {
  GWEN_EVENT *e;
  GWEN_WIDGET *subw;

  assert(w);
  e=GWEN_EventUpdate_new();
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }

  /* update all children */
  subw=GWEN_Widget_List_First(w->children);
  while(subw) {
    GWEN_Widget_Update(subw);
    subw=GWEN_Widget_List_Next(subw);
  } /* while */

  return 0;
}



int GWEN_Widget_Refresh(GWEN_WIDGET *w) {
  GWEN_EVENT *e;
  GWEN_WIDGET *subw;

  assert(w);
  e=GWEN_EventRefresh_new();
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }

  /* refresh all children */
  subw=GWEN_Widget_List_First(w->children);
  while(subw) {
    GWEN_Widget_Update(subw);
    subw=GWEN_Widget_List_Next(subw);
  } /* while */

  return 0;
}



int GWEN_Widget_Scroll(GWEN_WIDGET *w, int byX, int byY) {
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventScroll_new(byX, byY, 1);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_Scrolled(GWEN_WIDGET *w, int byX, int byY) {
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventScroll_new(byX, byY, 0);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_WriteAt(GWEN_WIDGET *w, int x, int y, const char *t, int len){
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventWriteAt_new(x, y, t, len);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_ChangeAtts(GWEN_WIDGET *w,
                           GWEN_TYPE_UINT32 atts,
                           int set) {
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventChgAtts_new(atts, set);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_Clear(GWEN_WIDGET *w, int x, int y, GWEN_EVENT_CLEAR_MODE m){
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventClear_new(x, y, m);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_Highlight(GWEN_WIDGET *w, int x, int y, int len,
                          GWEN_WIDGET_COLOUR colour){
 GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventHighlight_new(x, y, len, colour);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}



int GWEN_Widget_ContentChange(GWEN_WIDGET *w,
                              int contentWidth,
                              int contentHeight) {
  GWEN_EVENT *e;

  assert(w);
  e=GWEN_EventContentChg_new(contentWidth, contentHeight,
                             w->width, w->height);
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }
  return 0;
}






int GWEN_Widget_GetCursorX(const GWEN_WIDGET *w){
  assert(w);
  return w->cursorX;
}



void GWEN_Widget_SetCursorX(GWEN_WIDGET *w, int x){
  assert(w);
  w->cursorX=x;
}



int GWEN_Widget_GetCursorY(const GWEN_WIDGET *w){
  assert(w);
  return w->cursorY;
}



void GWEN_Widget_SetCursorY(GWEN_WIDGET *w, int y){
  assert(w);
  w->cursorY=y;
}



WINDOW *GWEN_Widget_GetWindow(const GWEN_WIDGET *w){
  assert(w);
  return w->window;
}





void GWEN_Widget_Dump(GWEN_WIDGET *w, int indent) {
  GWEN_WIDGET *sw;

  DBG_NOTICE(0, "Level %d: %s: Dims: %d/%d %d/%d",
             indent,
             w->name, w->x, w->y,
             w->width, w->height);
  sw=GWEN_Widget_List_First(w->children);
  while(sw) {
    GWEN_Widget_Dump(sw, indent+1);
    sw=GWEN_Widget_List_Next(sw);
  }
}



int GWEN_Widget_IsChildOf(GWEN_WIDGET *wc, GWEN_WIDGET *w) {
  assert(w);
  while(wc) {
    if (wc==w)
      return 1;
    wc=wc->parent;
  }

  return 0;
}



int GWEN_Widget_IsAncestorOf(GWEN_WIDGET *wc, GWEN_WIDGET *w) {
  assert(w);
  assert(wc);
  return GWEN_Widget_IsChildOf(w, wc);
  return 0;
}







