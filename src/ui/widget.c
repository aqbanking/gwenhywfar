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
#include <gwenhywfar/messagebox.h>
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

  if (parent) {
    if (parent->state & GWEN_WIDGET_STATE_CLOSED) {
      DBG_ERROR(0, "Parent is closed, cannot create subwindow \"%s\"",
                name);
      return 0;
    }
  }

  GWEN_NEW_OBJECT(GWEN_WIDGET, w);
  GWEN_INHERIT_INIT(GWEN_WIDGET, w);
  GWEN_LIST_INIT(GWEN_WIDGET, w);
  w->usage=1;
  w->eventHandler=GWEN_Widget__HandleEvent;
  w->typeName=strdup("Widget");

  if (GWEN_Widget_LastId==0)
    GWEN_Widget_LastId=time(0);
  w->id=++GWEN_Widget_LastId;

  w->children=GWEN_Widget_List_new();

  if (name)
    w->name=strdup(name);

  w->parent=parent;
  if (parent) {
    if (width==0)
      width=GWEN_Widget_GetWidth(parent)-x;
    if (height==0)
      height=GWEN_Widget_GetHeight(parent)-y;
  }
  else {
    if (width==0)
      width=COLS-x;
    if (height==0)
      height=LINES-y;
  }
  w->x=x;
  w->y=y;
  w->width=width;
  w->height=height;
  w->state=GWEN_WIDGET_STATE_ACTIVE;
  w->cursorX=-1;
  w->cursorY=-1;

  DBG_NOTICE(0, "Creating widget \"%s\" %d/%d, %d/%d (%s)",
             name, x, y, width, height, text);

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
    //wrefresh(w->window);
  }
  else {
    GWEN_UI_AddRootWidget(w);
    DBG_NOTICE(0, "Creating root window %d/%d, %d/%d",
               x, y, width, height);
    w->window=newwin(height, width,
                     y, x);
    assert(w->window);
    //refresh();
  }
  leaveok(w->window, FALSE);

  w->panel=new_panel(w->window);
  update_panels();
  top_panel(w->panel);

  keypad(w->window, (w->flags & GWEN_WIDGET_FLAGS_KEYPAD)?TRUE:FALSE);
  scrollok(w->window, (w->flags & GWEN_WIDGET_FLAGS_SCROLL)?TRUE:FALSE);
  w->colour=GWEN_WidgetColour_Default;
  w->normalColour=GWEN_WidgetColour_Default;

  e=GWEN_EventCreated_new();
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
  }

  if (text) {
    DBG_NOTICE(0, "Setting text \"%s\"", text);
    GWEN_Widget_SetText(w, text, GWEN_EventSetTextMode_Replace);
  }
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
      GWEN_WIDGET *cw;

      cw=GWEN_Widget_List_First(w->children);
      while(cw) {
        cw->parent=0;
        cw=GWEN_Widget_List_Next(cw);
      }

      DBG_NOTICE(0, "Freeing widget \"%s\" [%08x]",
                 w->name, (unsigned int)w);

      GWEN_INHERIT_FINI(GWEN_WIDGET, w);

      GWEN_Widget_List_free(w->children);
      free(w->name);
      free(w->typeName);
      free(w->text);
      wclear(w->window);
      if (w->window) {
        DBG_WARN(0, "Widget still open");
        if (w->panel) {
          del_panel(w->panel);
          delwin(w->window);
          update_panels();
        }
        else {
          delwin(w->window);
        }
      }
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



GWEN_WIDGET *GWEN_Widget_GetFirstChild(const GWEN_WIDGET *w){
  assert(w);
  return GWEN_Widget_List_First(w->children);
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



const char *GWEN_Widget_GetTypeName(const GWEN_WIDGET *w){
  assert(w);
  return w->typeName;
}



void GWEN_Widget_SetTypeName(GWEN_WIDGET *w, const char *s){
  assert(w);
  if (s) {
    char *nn;

    nn=(char*)malloc(((w->typeName)?(strlen(w->typeName)+1):0)+
                     strlen(s)+1);
    *nn=0;
    if (w->typeName) {
      strcat(nn, w->typeName);
      strcat(nn, "/");
    }
    strcat(nn, s);
    free(w->typeName);
    w->typeName=nn;
  }
}



GWEN_TYPE_UINT32 GWEN_Widget_GetFlags(const GWEN_WIDGET *w){
  assert(w);
  return w->flags;
}



GWEN_TYPE_UINT32 GWEN_Widget_GetState(const GWEN_WIDGET *w){
  assert(w);
  return w->state;
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
      DBG_INFO(0, "Event handler want's to stop the loop (quit)");
      return GWEN_UIResult_Quit;
    }
    else if (rv==GWEN_UIResult_Finished) {
      DBG_INFO(0, "Event handler want's to stop the loop (finished)");
      return GWEN_UIResult_Finished;
    }
    else {
      return rv;
    }
  }

  return GWEN_UIResult_NotHandled;
}



GWEN_UI_RESULT GWEN_Widget__HandleEvent(GWEN_WIDGET *w,
                                        GWEN_EVENT *e){
  const char *p;

  assert(w);
  assert(w->usage);


  if (GWEN_Event_GetType(e)==GWEN_EventType_Key) {
    int key;

    DBG_NOTICE(0, "Event: Key(%s)", w->name);
    key=GWEN_EventKey_GetKey(e);
    if (key==KEY_F(1)) {
      DBG_NOTICE(0, "Help-Key pressed");
      if (w->flags & GWEN_WIDGET_FLAGS_IGN_HELP) {
        DBG_NOTICE(0, "Ignoring help string");
        return GWEN_UIResult_Handled;
      }
      if (w->helpText) {
        DBG_NOTICE(0, "Showing help-screen");
        GWEN_MessageBox(w,
                        "HelpWindow",
                        "Help",
                        w->helpText,
                        "Ok", 0, 0);
        return GWEN_UIResult_Handled;
      }
      return GWEN_UIResult_NotHandled;
    }
  }

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

  case GWEN_EventType_SetColour: {
    GWEN_WIDGET_COLOUR col;

    DBG_INFO(0, "Event: SetColour(%s)", w->name);
    col=GWEN_EventSetColour_GetColour(e);
    w->normalColour=col;
    if (!(w->state & GWEN_WIDGET_STATE_HIGHLIGHT)) {
      DBG_NOTICE(0, "Setting colour: %d", col);
      w->colour=col;
      wbkgd(w->window, COLOR_PAIR(col));
      wattr_set(w->window, 0, w->colour, 0);
      if (GWEN_Widget_Update(w))
        return GWEN_UIResult_Error;
    }
    else {
      DBG_NOTICE(0, "Storing colour: %d", col);
    }
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Draw: {
    int x, y;

    DBG_INFO(0, "Event: Draw(%s)", w->name);
    x=0;
    y=0;
    if (w->text) {
      int length;
      int nw, nh;

      if (w->flags & GWEN_WIDGET_FLAGS_BORDER) nw=w->width-2;
      else nw=w->width;
      if (w->flags & GWEN_WIDGET_FLAGS_BORDER && w->height!=1) nh=w->height-2;
      else nh=w->height;
      length=strlen(w->text);
      if (w->flags & GWEN_WIDGET_FLAGS_HCENTER)
        x=(nw-length)/2;
      if (w->flags & GWEN_WIDGET_FLAGS_VCENTER)
        y=nh/2;
    }
    wbkgd(w->window, COLOR_PAIR(w->colour));
    wattr_set(w->window, 0, w->colour, 0);

    if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
      if (w->height>1) {
        box(w->window, 0, 0);
        x++;
        y++;
      }
      else if (w->height==1) {
        wmove(w->window, 0, 0);
        waddch(w->window, '[');
        wmove(w->window, 0, w->width-1);
        waddch(w->window, ']');
        x++;
      }
    }
    if (w->text) {
      DBG_NOTICE(0, "Setting text \"%s\"", w->text);
      mvwprintw(w->window, y, x, w->text);
    }
    GWEN_Widget_Refresh(w);
    //wrefresh(w->window);
    //doupdate();
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Close: {
    DBG_INFO(0, "Event: Close(%s)", w->name);
    if (GWEN_Event_GetRecipient(e)==w) {
      if (w->state & GWEN_WIDGET_STATE_CLOSED) {
        DBG_NOTICE(0, "Widget already *is* closed");
        return GWEN_UIResult_NotHandled;
      }
      w->state|=GWEN_WIDGET_STATE_CLOSED;
      if (w->parent) {
        GWEN_EVENT *eClosed;

        /* send "Closed" event to parent.
         * Since events are attached to both sender and recipient of an event
         * this widget will not be deleted now, even though the function
         * GWEN_Widget_free() is called below. That will just decrement the
         * usage counter of this widget. When the "closed" event is handled
         * and freed this widget will finally be deleted.
         */
        eClosed=GWEN_EventClosed_new();
        assert(eClosed);
        if (GWEN_Widget_SendEvent(w->parent, w, eClosed)) {
          DBG_INFO(0, "Could not send event");
          GWEN_Event_free(eClosed);
        }
      }
      wclear(w->window);
      del_panel(w->panel);
      delwin(w->window);
      w->window=0;
      w->panel=0;
      update_panels();
      //refresh();
      return GWEN_UIResult_Handled;
    }
    else
      return GWEN_UIResult_Finished;
  }

  case GWEN_EventType_Closed: {
    GWEN_WIDGET *wSender;

    DBG_INFO(0, "Event: Closed(%s)", w->name);
    wSender=GWEN_Event_GetSender(e);
    if (wSender) {
      if (GWEN_Widget_IsChildOf(wSender, w)) {
        GWEN_WIDGET *cw;
        int actives;

        /* a child of ours closed */
        actives=0;
        cw=GWEN_Widget_List_First(w->children);
        while(cw) {
          if (cw!=wSender) {
            if (!(w->state & GWEN_WIDGET_STATE_CLOSED))
              actives++;
          }
          cw=GWEN_Widget_List_Next(cw);
        }
        if (!actives) {
          GWEN_EVENT *eLast;

          /* last child closed */
          eLast=GWEN_EventLastClosed_new();
          assert(eLast);
          if (GWEN_Widget_SendEvent(w, w, eLast)) {
            DBG_INFO(0, "Could not send event");
            GWEN_Event_free(eLast);
          }
        }
        return GWEN_UIResult_Handled;
      }
    }
    return GWEN_UIResult_NotHandled;
  }

  case GWEN_EventType_LastClosed:
    DBG_INFO(0, "Event: LastClosed(%s)", w->name);
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Command:
    DBG_INFO(0, "Event: Command(%s)", w->name);
    switch(GWEN_EventCommand_GetCommandId(e)) {
    case GWEN_WIDGET_CMD_CLOSE:
      GWEN_Widget_Close(w);
      return GWEN_UIResult_Handled;
    default:
      break;
    }
    return GWEN_UIResult_NotHandled;

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
    int i;

    DBG_INFO(0, "Event: Highlight(%s)", w->name);
    x=GWEN_EventHighlight_GetX(e);
    y=GWEN_EventHighlight_GetY(e);
    len=GWEN_EventHighlight_GetLen(e);
    hi=GWEN_EventHighlight_GetHi(e);
    maxc=w->width;
    if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
      if (w->height>1)
        y++;
      x++;
      maxc-=2;
    }
    if (maxc>len)
      maxc=len;
    DBG_NOTICE(0, "ZZZ: Highlighting %d, %d, %d bytes (true:%d)",
               x, y, len, hi);
    if (hi==0)
      hi=w->colour;

    for (i=0; i<maxc; i++) {
      chtype oldc;

      oldc=(mvwinch(w->window, y, x+i) & ~A_COLOR);
      oldc|=COLOR_PAIR(hi);
      mvwaddch(w->window, y, x+i, oldc);
    }
    GWEN_Widget_Refresh(w);
    //wrefresh(w->window);
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
    DBG_NOTICE(0, "ZZZ: Writing at %d, %d, %d bytes",
               x, y, len);

    maxc=w->width;
    if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
      x++;
      if (w->height>1)
        y++;
      maxc-=2;
    }
    p=(const unsigned char*)GWEN_EventWriteAt_GetText(e);

    if (p) {
      int attrs;
      int c;

      DBG_NOTICE(0, "Writing this at %d/%d [%d, %d]:",
                 x, y, len, maxc);
      GWEN_Text_LogString(p, len, 0, GWEN_LoggerLevelNotice);
      wmove(w->window, y, x);
      wbkgdset(w->window, COLOR_PAIR(w->colour));
      wattr_set(w->window, 0, w->colour, 0);
      attrs=COLOR_PAIR(w->colour);
      while(len) {
        c=*p;
        if (maxc==0)
          break;

        if (c==GWEN_WIDGET_ATT_ESC_CHAR) {
          if (!--len) {
            DBG_ERROR(0, "Bad string");
            abort();
            return GWEN_UIResult_Error;
          }
          p++;
          c=*p;
          DBG_NOTICE(0, "Setting new attributes %02x", c);
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

          if (!--len) {
            DBG_ERROR(0, "Bad string");
            abort();
            return GWEN_UIResult_Error;
          }
          p++;
          c=*p;
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
          default: ch='?'; DBG_NOTICE(0, "Bad character \"%04x\"", c); break;
          } /* switch */
          if (waddch(w->window, ch | attrs)==ERR) {
            DBG_VERBOUS(0, "Error writing to window (%02x)", c);
            break;
          }
          maxc--;
        }
        else if (isprint(c)) {
          DBG_NOTICE(0, "Writing char %02x", c);
          if (waddch(w->window, c | attrs)==ERR) {
            DBG_VERBOUS(0, "Error writing to window (%02x)", c);
            break;
          }
          maxc--;
        }
        else {
          DBG_NOTICE(0, "Unprintable character detected.");
          abort();
        }
        x++;
        p++;
        len--;
      } /* while */
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
        //wrefresh(w->window);
        GWEN_Widget_Refresh(w);
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
    if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
      x++;
      if (w->height>1)
        y++;
    }
    m=GWEN_EventClear_GetMode(e);
    DBG_NOTICE(0, "Clearing dims: %d/%d (%d) [%s]", x, y, m, w->name);
    switch(m) {
    case GWEN_EventClearMode_All:
      wmove(w->window, y, x);
      wclear(w->window);
      if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
        if (w->height>1) {
          box(w->window, 0, 0);
        }
        else if (w->height==1) {
          wmove(w->window, 0, 0);
          waddch(w->window, '[');
          wmove(w->window, 0, w->width-1);
          waddch(w->window, ']');
        }
      }
      break;
    case GWEN_EventClearMode_ToEOL:
      if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
        int len;

        len=w->width-2-x;
        x++;
        while(len-->0)
          mvwaddch(w->window, y, x++, ' ');
      }
      else {
        wmove(w->window, y, x);
        wclrtoeol(w->window);
      }
      break;
    case GWEN_EventClearMode_ToBottom:
      wmove(w->window, y, x);
      wclrtobot(w->window);
      if (w->flags & GWEN_WIDGET_FLAGS_BORDER) {
        if (w->height>1) {
          box(w->window, 0, 0);
        }
        else if (w->height==1) {
          wmove(w->window, 0, 0);
          waddch(w->window, '[');
          wmove(w->window, 0, w->width-1);
          waddch(w->window, ']');
        }
      }
      break;
    default:
      DBG_DEBUG(0, "Unknown mode (%d)", m);
      return GWEN_UIResult_NotHandled;
    } /* switch */
    GWEN_Widget_Refresh(w);
    //wrefresh(w->window);
    //doupdate();
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Update:
    DBG_INFO(0, "Event: Update(%s)", w->name);
    GWEN_Widget_Refresh(w);
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Refresh:
    DBG_INFO(0, "Event: Refresh(%s)", w->name);
    //wrefresh(w->window);
    update_panels();
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Move:
    DBG_INFO(0, "Event: Move(%s)", w->name);
    return GWEN_UIResult_NotHandled;

  case GWEN_EventType_Key: {
    int key;

    DBG_INFO(0, "Event: Key(%s)", w->name);
    key=GWEN_EventKey_GetKey(e);
    if (key==KEY_F(5)) {
      DBG_NOTICE(0, "Updating");
      //doupdate();
      update_panels();
    }
    else if (key==KEY_F(6)) {
      DBG_NOTICE(0, "Refreshing");
      //wrefresh(w->window);
      GWEN_Widget_Refresh(w);
    }
    else if (key==KEY_F(7)) {
      DBG_NOTICE(0, "Redrawing");
      beep();
      GWEN_Widget_Redraw(w);
    }
    else if (key==KEY_F(11)) {
      beep();
      GWEN_UI_DumpWidgets();
    }
    if (key==KEY_F(12)) {
      GWEN_UI_End();
      DBG_NOTICE(0, "Emergency quit");
      exit(2);
    }
    else if (key==9) {
      if (GWEN_UI_FocusToNext(w)) {
        beep();
        return GWEN_UIResult_Handled;
      }
    }
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Focus: {
    GWEN_EVENT_FOCUS_TYPE ft;

    DBG_INFO(0, "Event: Focus(%s)", w->name);
    ft=GWEN_EventFocus_GetFocusEventType(e);
    if (ft==GWEN_EventFocusType_Got) {
      w->state|=GWEN_WIDGET_STATE_HASFOCUS;
      if (w->flags & GWEN_WIDGET_FLAGS_HIGHLIGHT) {
        w->normalColour=w->colour;
        w->colour=GWEN_WidgetColour_Selected;
        wbkgd(w->window, COLOR_PAIR(w->colour));
        wattr_set(w->window, 0, w->colour, 0);
        w->state|=GWEN_WIDGET_STATE_HIGHLIGHT;
        GWEN_Widget_Refresh(w);
        //wrefresh(w->window);
      }
      else {
        w->state&=~GWEN_WIDGET_STATE_HIGHLIGHT;
      }
    }
    else {
      DBG_NOTICE(0, "Focus lost");
      w->state&=~GWEN_WIDGET_STATE_HASFOCUS;
      if (w->state & GWEN_WIDGET_STATE_HIGHLIGHT) {
        w->state&=~GWEN_WIDGET_STATE_HIGHLIGHT;
        w->colour=w->normalColour;
        wbkgd(w->window, COLOR_PAIR(w->colour));
        wattr_set(w->window, 0, w->colour, 0);
        GWEN_Widget_Refresh(w);
        //wrefresh(w->window);
      }
    }
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
    GWEN_Widget_Refresh(subw);
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

  if (t) {
    GWEN_BUFFER *mb;
    int i;

    mb=GWEN_Buffer_new(0, len, 0, 1);
    for (i=0; i<len; i++) {
      if (!isprint(t[i]))
        GWEN_Buffer_AppendByte(mb, '?');
      else
        GWEN_Buffer_AppendByte(mb, t[i]);
    }
    DBG_NOTICE(0, "Writing this: \"%s\"",
               GWEN_Buffer_GetStart(mb));
    if (strstr(GWEN_Buffer_GetStart(mb), "<gwen>")) {
      DBG_NOTICE(0, "What ??");
      abort();
    }
    GWEN_Buffer_free(mb);
  }

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
  GWEN_BUFFER *m;

  m=GWEN_Buffer_new(0, indent, 0, 1);
  if (indent) {
    int i;

    for (i=0; i<indent; i++)
      GWEN_Buffer_AppendByte(m, '-');
  }

  DBG_NOTICE(0, "%s %s[%s]: Dims: %d/%d %d/%d [%d]",
             indent?GWEN_Buffer_GetStart(m):"",
             w->name,
             w->typeName,
             w->x, w->y,
             w->width, w->height, w->usage);
  GWEN_Buffer_free(m);
  sw=GWEN_Widget_List_First(w->children);
  while(sw) {
    GWEN_Widget_Dump(sw, indent+1);
    sw=GWEN_Widget_List_Next(sw);
  }
}



int GWEN_Widget_IsChildOf(GWEN_WIDGET *wc, GWEN_WIDGET *w) {
  assert(w);
  while(wc) {
    assert(wc->usage);
    assert(w->usage);
    if (wc==w)
      return 1;
    wc=wc->parent;
  }

  return 0;
}



int GWEN_Widget_IsAncestorOf(GWEN_WIDGET *wc, GWEN_WIDGET *w) {
  assert(w);
  assert(wc);
  assert(w->usage);
  assert(wc->usage);
  return GWEN_Widget_IsChildOf(w, wc);
  return 0;
}



int GWEN_Widget_Close(GWEN_WIDGET *w) {
  GWEN_EVENT *e;
  GWEN_WIDGET *subw;

  assert(w);

  if (w->state & GWEN_WIDGET_STATE_CLOSED) {
    DBG_ERROR(0, "Widget \"%s\" is already closed", w->name);
    return -1;
  }

  /* close all children */
  subw=GWEN_Widget_List_First(w->children);
  while(subw) {
    GWEN_Widget_Close(subw);
    subw=GWEN_Widget_List_Next(subw);
  } /* while */

  e=GWEN_EventClose_new();
  assert(e);
  if (GWEN_Widget_SendEvent(w, w, e)) {
    DBG_INFO(0, "Could not send event");
    GWEN_Event_free(e);
    return -1;
  }

  return 0;
}



GWEN_UI_RESULT GWEN_Widget_Run(GWEN_WIDGET *w) {
  GWEN_UI_RESULT res;

  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      return GWEN_UIResult_NotHandled;
    DBG_NOTICE(0, "Got this event:");
    GWEN_Event_Dump(e);
    res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
    if (res==GWEN_UIResult_Finished ||
        res==GWEN_UIResult_Quit) {
      return res;
    }
  }
}



const char *GWEN_Widget_GetHelpText(const GWEN_WIDGET *w){
  assert(w);
  return w->helpText;
}



void GWEN_Widget_SetHelpText(GWEN_WIDGET *w, const char *s){
  assert(w);
  free(w->helpText);
  if (s) w->helpText=strdup(s);
  else w->helpText=0;
}










