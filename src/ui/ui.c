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


#include "ui_p.h"
#include "event_l.h"
#include "widget_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/ui/widget.h>
#include <time.h>
#include <string.h>
#include <ctype.h>


#include <ncurses.h>
#include <panel.h>


static GWEN_UI *GWEN_UI__ui=0;


int GWEN_UI_Begin() {
  if (GWEN_UI__ui) {
    GWEN_UI__ui->usage++;
    DBG_NOTICE(0, "GUI already in use (%d)", GWEN_UI__ui->usage);
    return 0;
  }
  initscr();
  start_color();
  cbreak();
  noecho();
  nonl();
  raw();
  keypad(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  /* Initialize all the colors */
  init_pair(GWEN_WidgetColour_Default, COLOR_WHITE, COLOR_BLUE);
  init_pair(GWEN_WidgetColour_Message, COLOR_WHITE, COLOR_RED);
  init_pair(GWEN_WidgetColour_Unselected, COLOR_RED, COLOR_WHITE);
  init_pair(GWEN_WidgetColour_Selected, COLOR_RED, COLOR_CYAN);
  init_pair(GWEN_WidgetColour_Chosen, COLOR_MAGENTA, COLOR_WHITE);
  init_pair(GWEN_WidgetColour_Title, COLOR_BLUE, COLOR_WHITE);
  init_pair(GWEN_WidgetColour_Button, COLOR_BLACK, COLOR_WHITE);

  GWEN_NEW_OBJECT(GWEN_UI, GWEN_UI__ui);
  GWEN_UI__ui->rootWidgets=GWEN_Widget_List_new();
  GWEN_UI__ui->events=GWEN_Event_List_new();
  GWEN_UI__ui->newEvents=GWEN_Event_List_new();
  GWEN_UI__ui->usage=1;
  return 0;
}



int GWEN_UI_End() {
  assert(GWEN_UI__ui);
  if ((--GWEN_UI__ui->usage)==0) {
    DBG_NOTICE(0, "Closing GUI mode");
    GWEN_Widget_free(GWEN_UI__ui->focusWidget);
    GWEN_Event_List_free(GWEN_UI__ui->events);
    GWEN_Event_List_free(GWEN_UI__ui->newEvents);
    if (GWEN_Widget_List_GetCount(GWEN_UI__ui->rootWidgets)) {
      DBG_WARN(0, "There are still %d widgets",
               GWEN_Widget_List_GetCount(GWEN_UI__ui->rootWidgets));
      GWEN_UI_DumpWidgets();
    }
    GWEN_Widget_List_free(GWEN_UI__ui->rootWidgets);
    GWEN_FREE_OBJECT(GWEN_UI__ui);
    GWEN_UI__ui=0;
    endwin();
  }
  else {
    DBG_NOTICE(0, "GUI still in use (%d)", GWEN_UI__ui->usage);
  }
  return 0;
}



void GWEN_UI_AddRootWidget(GWEN_WIDGET *w){
  assert(GWEN_UI__ui);

  GWEN_Widget_List_Add(w, GWEN_UI__ui->rootWidgets);
}



int GWEN_UI_SendEvent(GWEN_WIDGET *wRecipient,
                      GWEN_WIDGET *wSender,
                      GWEN_EVENT *e,
                      int withPriority){
  GWEN_EVENT_TYPE t;
  int delGraEvents;
  int delSameEvents;

  assert(GWEN_UI__ui);
  assert(e);
  assert(wRecipient);

  if (wSender)
    GWEN_Event_SetSender(e, wSender);
  GWEN_Event_SetRecipient(e, wRecipient);

  if (GWEN_Widget_GetState(wRecipient) & GWEN_WIDGET_STATE_CLOSED) {
    DBG_NOTICE(0, "Recipient already closed, not sending event");
    GWEN_Event_Dump(e);
    abort();
    return -1;
  }

  t=GWEN_Event_GetType(e);
  delGraEvents=0;
  delSameEvents=0;

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw:
  case GWEN_EventType_Update:
  case GWEN_EventType_Refresh:
  case GWEN_EventType_ContentChg:
    delSameEvents=1;
    break;
  case GWEN_EventType_Destroy:
    delGraEvents=1;
    break;
  default:
    break;
  }

  if (delSameEvents && (withPriority || GWEN_UI__ui->currentEvent) &&
      t!=GWEN_EventType_ContentChg) {
    GWEN_EVENT *le;

    /* do not add event if it already exists */
    le=GWEN_Event_List_First(GWEN_UI__ui->events);
    while(le) {
      if (wRecipient==GWEN_Event_GetRecipient(le) ||
          GWEN_Widget_IsAncestorOf(GWEN_Event_GetRecipient(le),
                                   wRecipient)) {
        if (GWEN_Event_GetType(le)==t) {
          DBG_NOTICE(0, "Not adding event event");
          GWEN_Event_free(e);
          return 0;
        }
      } /* if match */
      le=GWEN_Event_List_Next(le);
    } /* while */
  } /* if delSameEvent */

  if (delGraEvents || delSameEvents) {
    GWEN_EVENT *le;

    /* remove all previous events of that type */
    le=GWEN_Event_List_First(GWEN_UI__ui->newEvents);
    while(le) {
      GWEN_EVENT *nextE;

      nextE=GWEN_Event_List_Next(le);
      if (wRecipient==GWEN_Event_GetRecipient(le) ||
          GWEN_Widget_IsChildOf(GWEN_Event_GetRecipient(le),
                                wRecipient)) {
        if (delGraEvents) {
          switch(GWEN_Event_GetType(le)) {
          case GWEN_EventType_Draw:
          case GWEN_EventType_Update:
          case GWEN_EventType_Refresh:
          case GWEN_EventType_WriteAt:
          case GWEN_EventType_Clear:
            DBG_NOTICE(0, "Removing event:");
            GWEN_Event_Dump(le);
            GWEN_Event_List_Del(le);
            GWEN_Event_free(le);
            break;
          default:
            break;
          }
        }
        else if (delSameEvents) {
          if (GWEN_Event_GetType(le)==t) {
            DBG_NOTICE(0, "Removing same event:");
            //GWEN_Event_Dump(le);
            GWEN_Event_List_Del(le);
            GWEN_Event_free(le);
          }
        }
      }
      le=nextE;
    }
  }

  if ((withPriority || GWEN_UI__ui->currentEvent) &&
      t!=GWEN_EventType_ContentChg) {
    DBG_NOTICE(0, "Adding as new event");
    GWEN_Event_List_Add(e, GWEN_UI__ui->newEvents);
  }
  else {
    DBG_NOTICE(0, "Adding as next event");
    GWEN_Event_List_Add(e, GWEN_UI__ui->events);
  }

  if (0) {
    GWEN_EVENT *le;

    DBG_NOTICE(0, "Enqueued this event:");
    GWEN_Event_Dump(e);

    le=GWEN_Event_List_First(GWEN_UI__ui->events);
    while(le) {

      GWEN_Event_Dump(le);
      le=GWEN_Event_List_Next(le);
    }
  }

  return 0;
}



GWEN_EVENT *GWEN_UI_PeekNextEvent() {
  GWEN_EVENT *e;

  assert(GWEN_UI__ui);

  if (GWEN_UI__ui->currentEvent) {
    GWEN_EVENT_LIST *tmpel;

    GWEN_Event_List_AddList(GWEN_UI__ui->newEvents, GWEN_UI__ui->events);
    tmpel=GWEN_UI__ui->newEvents;
    GWEN_UI__ui->newEvents=GWEN_UI__ui->events;
    GWEN_UI__ui->events=tmpel;
    GWEN_UI__ui->currentEvent=0;
    GWEN_UI_Flush();
  }

  e=GWEN_Event_List_First(GWEN_UI__ui->events);
  if (e) {
    return e;
  }
  else {
    if (GWEN_UI_HandleKeyEvents(0)==0) {
      e=GWEN_Event_List_First(GWEN_UI__ui->events);
      if (e) {
        return e;
      }
    }
  }
  return 0;

}



GWEN_EVENT *GWEN_UI_GetNextEvent() {
  GWEN_EVENT *e;

  if (GWEN_UI__ui->currentEvent) {
    GWEN_EVENT_LIST *tmpel;

    GWEN_Event_List_AddList(GWEN_UI__ui->newEvents, GWEN_UI__ui->events);
    tmpel=GWEN_UI__ui->newEvents;
    GWEN_UI__ui->newEvents=GWEN_UI__ui->events;
    GWEN_UI__ui->events=tmpel;
    GWEN_UI__ui->currentEvent=0;
    GWEN_UI_Flush();
  }

  e=GWEN_Event_List_First(GWEN_UI__ui->events);
  if (e) {
    GWEN_Event_List_Del(e);
    return e;
  }
  else {
    int rv;

    rv=GWEN_UI_HandleKeyEvents(1);

    if (rv==0) {
      e=GWEN_Event_List_First(GWEN_UI__ui->events);
      if (e) {
        GWEN_Event_List_Del(e);
        return e;
      }
    }
    else if (rv==-1) {
      DBG_DEBUG(0, "Error");
      return 0;
    }
    else {
      DBG_DEBUG(0, "No next event (even no key event)");
      return 0;
    }

  }
  return 0;
}



int GWEN_UI_HandleKeyEvents(int wt) {
  GWEN_EVENT *e;

  if (GWEN_UI__ui->focusWidget) {
    if (GWEN_Widget_GetState(GWEN_UI__ui->focusWidget) &
        GWEN_WIDGET_STATE_CLOSED) {
      GWEN_Widget_free(GWEN_UI__ui->focusWidget);
      GWEN_UI__ui->focusWidget=0;
    }
  }
  if (GWEN_UI__ui->focusWidget==0) {
    /* find a widget which may receive the focus */
    if (!GWEN_UI_FocusToNext()) {
      DBG_ERROR(0, "No focusable widget found");
      return -1;
    }
    return 1;
  }
  else {
    int ch;
    int cx;
    int cy;
    int i;

    /* handle user interaction */
    DBG_NOTICE(0, "Focus widget: %s",
               GWEN_Widget_GetName(GWEN_UI__ui->focusWidget));
    cx=GWEN_Widget_GetCursorX(GWEN_UI__ui->focusWidget);
    cy=GWEN_Widget_GetCursorY(GWEN_UI__ui->focusWidget);
    if (cx!=-1 && cy!=-1)
      move(cy+GWEN_Widget_GetPhysicalY(GWEN_UI__ui->focusWidget),
           cx+GWEN_Widget_GetPhysicalX(GWEN_UI__ui->focusWidget));
    else {
      move(0, 0);
    }
    update_panels();
    refresh();
    doupdate();

    nodelay(GWEN_Widget_GetWindow(GWEN_UI__ui->focusWidget), TRUE);
    i=0;
    while ( i++<5) {
      ch=getch();
      if (ch==ERR)
        break;
      DBG_NOTICE(0, "Generating key event for key %04x", ch);
      e=GWEN_EventKey_new(ch);
      assert(e);
      if (GWEN_UI_SendEvent(GWEN_UI__ui->focusWidget, 0, e, 0)) {
        DBG_ERROR(0, "Could not send event");
        GWEN_Event_free(e);
      }
      nodelay(GWEN_Widget_GetWindow(GWEN_UI__ui->focusWidget), FALSE);
      return 0;
    }
    nodelay(GWEN_Widget_GetWindow(GWEN_UI__ui->focusWidget), FALSE);

    if (!wt)
      return 1;

    ch=getch();
    if (ch==ERR) {
      /* timeout */
      return 1;
    }
    DBG_NOTICE(0, "Generating key event for %04x", ch);
    e=GWEN_EventKey_new(ch);
    assert(e);
    if (GWEN_UI_SendEvent(GWEN_UI__ui->focusWidget, 0, e, 0)) {
      DBG_ERROR(0, "Could not send event");
      GWEN_Event_free(e);
    }
    return 0;
  }
}



void GWEN_UI_SetFocus(GWEN_WIDGET *w) {
  assert(GWEN_UI__ui);

  if (GWEN_UI__ui->focusWidget) {
    GWEN_EVENT *e;

    e=GWEN_EventFocus_new(GWEN_EventFocusType_Lost);
    assert(e);
    if (GWEN_UI_SendEvent(GWEN_UI__ui->focusWidget, 0, e, 0)) {
      DBG_ERROR(0, "Could not send event");
      GWEN_Event_free(e);
    }
    DBG_NOTICE(0, "Freeing widget \"%s\"",
               GWEN_Widget_GetName(GWEN_UI__ui->focusWidget));
    GWEN_Widget_free(GWEN_UI__ui->focusWidget);
    GWEN_UI__ui->focusWidget=0;
  }

  if (w)
    w=GWEN_UI_GetDeepestFocusable(w);
  if (w) {
    GWEN_EVENT *e;

    GWEN_UI__ui->focusWidget=w;
    GWEN_Widget_Attach(GWEN_UI__ui->focusWidget);

    e=GWEN_EventFocus_new(GWEN_EventFocusType_Got);
    assert(e);
    if (GWEN_UI_SendEvent(w, 0, e, 0)) {
      DBG_ERROR(0, "Could not send event");
      GWEN_Event_free(e);
    }
  }
}



GWEN_UI_RESULT GWEN_UI_DispatchEvent(GWEN_EVENT *e) {
  GWEN_UI_RESULT res;
  GWEN_WIDGET *wRec;
  GWEN_EVENT_LIST *tmpel;

  wRec=GWEN_Event_GetRecipient(e);
  assert(wRec);

  GWEN_UI__ui->currentEvent=e;

  if (GWEN_Widget_GetState(wRec) & GWEN_WIDGET_STATE_CLOSED) {
    DBG_NOTICE(0, "Recipient already closed, dismissing event");
    return GWEN_UIResult_Error;
  }

  DBG_NOTICE(0, "Sending event to:");
  GWEN_Event_Dump(e);
  GWEN_Widget_Dump(wRec, 1);
  res=GWEN_Widget_HandleEvent(wRec, e);
  if (res==GWEN_UIResult_NotHandled) {
    GWEN_WIDGET *wParent;

    DBG_NOTICE(0, "Sending to parent");
    wParent=GWEN_Widget_GetParent(wRec);
    while(wParent) {
      DBG_NOTICE(0, "Really sending to parent");
      GWEN_Event_Dump(e);
      GWEN_Widget_Dump(wRec, 1);
      res=GWEN_Widget_HandleEvent(wParent, e);
      DBG_NOTICE(0, "Response is: %d", res);
      if (res!=GWEN_UIResult_NotHandled)
        break;
      wParent=GWEN_Widget_GetParent(wParent);
    }
  }
  GWEN_Event_List_AddList(GWEN_UI__ui->newEvents, GWEN_UI__ui->events);
  tmpel=GWEN_UI__ui->newEvents;
  GWEN_UI__ui->newEvents=GWEN_UI__ui->events;
  GWEN_UI__ui->events=tmpel;
  GWEN_UI__ui->currentEvent=0;
  return res;
}



GWEN_WIDGET *GWEN_UI_GetFocusWidget() {
  assert(GWEN_UI__ui);
  return GWEN_UI__ui->focusWidget;
}



GWEN_UI_RESULT GWEN_UI_HandleEvents() {
  int quit;
  int finished;
  int handled;
  GWEN_EVENT *e;

  assert(GWEN_UI__ui);
  quit=0;
  finished=0;
  handled=0;

  while(1) {
    GWEN_UI_RESULT res;

    e=GWEN_Event_List_First(GWEN_UI__ui->events);
    if (!e)
      break;
    GWEN_Event_List_Del(e);
    res=GWEN_UI_DispatchEvent(e);

    switch(res) {
    case GWEN_UIResult_Handled:
      handled++;
      break;
    case GWEN_UIResult_Quit:
      quit=1;
      break;
    case GWEN_UIResult_Finished:
      finished=1;
      break;
    default:
      break;
    } /* switch */
    GWEN_Event_free(e);
  } /* while e */

  //doupdate();
  update_panels();
  if (quit)
    return GWEN_UIResult_Quit;
  if (finished)
    return GWEN_UIResult_Finished;
  if (handled)
    return GWEN_UIResult_Handled;
  return GWEN_UIResult_NotHandled;
}



void GWEN_UI_Flush() {
  GWEN_UI_HandleEvents();
  update_panels();
  doupdate();
}




GWEN_UI_RESULT GWEN_UI_Work() {
  int quit;

  quit=0;
  while(!quit) {
    GWEN_UI_RESULT res;

    res=GWEN_UI_HandleEvents();
    if (res==GWEN_UIResult_Quit)
      quit=1;
    else {
      GWEN_EVENT *e;

      if (GWEN_UI__ui->focusWidget==0) {
        GWEN_WIDGET *w;
        int found;

        /* find a widget which may receive the focus */
        found=0;
        w=GWEN_Widget_List_First(GWEN_UI__ui->rootWidgets);
        while(w) {
          if ((GWEN_Widget_GetFlags(w) & GWEN_WIDGET_FLAGS_FOCUSABLE) &&
              !(GWEN_Widget_GetState(w) & GWEN_WIDGET_STATE_CLOSED)) {
            DBG_NOTICE(0, "Setting focus to window \"%s\"",
                       GWEN_Widget_GetName(w));
            GWEN_UI_SetFocus(w);
            found=1;
            break;
          }
          w=GWEN_Widget_List_Next(w);
        } /* while */
        if (!found) {
          DBG_ERROR(0, "No focusable widget found");
          return GWEN_UIResult_Error;
        }
      }
      else {
        int ch;
        int cx;
        int cy;

        /* handle user interaction */
        DBG_NOTICE(0, "Focus widget: %s",
                   GWEN_Widget_GetName(GWEN_UI__ui->focusWidget));
        cx=GWEN_Widget_GetCursorX(GWEN_UI__ui->focusWidget);
        cy=GWEN_Widget_GetCursorY(GWEN_UI__ui->focusWidget);
        if (cx!=-1 && cy!=-1)
          move(cy+GWEN_Widget_GetPhysicalY(GWEN_UI__ui->focusWidget),
               cx+GWEN_Widget_GetPhysicalX(GWEN_UI__ui->focusWidget));
        else {
          move(0, 0);
        }
        refresh();
        ch=getch();
        if (ch==ERR) {
          /* timeout */
          return GWEN_UIResult_Handled;
        }
        DBG_NOTICE(0, "Generating key event");
        e=GWEN_EventKey_new(ch);
        assert(e);
        if (GWEN_UI_SendEvent(GWEN_UI__ui->focusWidget, 0, e, 0)) {
          DBG_ERROR(0, "Could not send event");
          GWEN_Event_free(e);
        }
      }
    }
  } /* while */

  return GWEN_UIResult_Quit;
}




GWEN_WIDGET *GWEN_UI_GetDeepestFocusable(GWEN_WIDGET *w){
  GWEN_WIDGET *wnext;
  GWEN_WIDGET *wfocus;
  GWEN_TYPE_UINT32 wf, ws;

  wf=GWEN_Widget_GetFlags(w);
  ws=GWEN_Widget_GetState(w);

  DBG_NOTICE(0, "Checking this widget: %s", GWEN_Widget_GetName(w));

  if (!(ws & GWEN_WIDGET_STATE_ACTIVE) ||
      (ws & GWEN_WIDGET_STATE_CLOSED)) {
    DBG_NOTICE(0, "Widget \"%s\" inactive or closed",
               GWEN_Widget_GetName(w));
    return 0;
  }

  wnext=GWEN_Widget_GetFirstChild(w);
  while (wnext) {
    GWEN_TYPE_UINT32 wf, ws;

    wf=GWEN_Widget_GetFlags(wnext);
    ws=GWEN_Widget_GetState(wnext);
    if ((ws & GWEN_WIDGET_STATE_ACTIVE) &&
        !(ws & GWEN_WIDGET_STATE_CLOSED)) {
      wfocus=GWEN_UI_GetDeepestFocusable(wnext);
      if (wfocus) {
        DBG_NOTICE(0, "Widget \"%s\" found", GWEN_Widget_GetName(wfocus));
        return wfocus;
      }
      if ((wf & GWEN_WIDGET_FLAGS_FOCUSABLE) &&
          !(ws & GWEN_WIDGET_STATE_HASFOCUS))
        return wnext;
    }

    wnext=GWEN_Widget_List_Next(wnext);
  } /* while */

  if ((wf & GWEN_WIDGET_FLAGS_FOCUSABLE) &&
      !(ws & GWEN_WIDGET_STATE_HASFOCUS))
    return w;

  return 0;
}



GWEN_WIDGET *GWEN_UI_GetDeepestFocusableBackwards(GWEN_WIDGET *w){
  GWEN_WIDGET *wnext;
  GWEN_WIDGET *wfocus;
  GWEN_TYPE_UINT32 wf, ws;

  wf=GWEN_Widget_GetFlags(w);
  ws=GWEN_Widget_GetState(w);

  DBG_NOTICE(0, "Checking this widget: %s", GWEN_Widget_GetName(w));

  if (!(ws & GWEN_WIDGET_STATE_ACTIVE) ||
      (ws & GWEN_WIDGET_STATE_CLOSED)) {
    DBG_NOTICE(0, "Widget \"%s\" inactive or closed",
               GWEN_Widget_GetName(w));
    return 0;
  }

  wnext=GWEN_Widget_GetFirstChild(w);
  if (wnext) {
    while(GWEN_Widget_List_Next(wnext))
      wnext=GWEN_Widget_List_Next(wnext);
  }
  while (wnext) {
    GWEN_TYPE_UINT32 wf, ws;

    wf=GWEN_Widget_GetFlags(wnext);
    ws=GWEN_Widget_GetState(wnext);
    if ((ws & GWEN_WIDGET_STATE_ACTIVE) &&
        !(ws & GWEN_WIDGET_STATE_CLOSED)) {
      wfocus=GWEN_UI_GetDeepestFocusableBackwards(wnext);
      if (wfocus) {
        DBG_NOTICE(0, "Widget \"%s\" found", GWEN_Widget_GetName(wfocus));
        return wfocus;
      }
      if ((wf & GWEN_WIDGET_FLAGS_FOCUSABLE) &&
          !(ws & GWEN_WIDGET_STATE_HASFOCUS))
        return wnext;
    }

    wnext=GWEN_Widget_List_Previous(wnext);
  } /* while */

  if ((wf & GWEN_WIDGET_FLAGS_FOCUSABLE) &&
      !(ws & GWEN_WIDGET_STATE_HASFOCUS))
    return w;

  return 0;
}



GWEN_WIDGET *GWEN_UI__FocusToNext(GWEN_WIDGET *wlevel){
  GWEN_WIDGET *wfocus;

  assert(wlevel);
  while(wlevel) {
    GWEN_WIDGET *wnext;

    DBG_NOTICE(0, "Checking this widget: %s", GWEN_Widget_GetName(wlevel));
    wnext=GWEN_Widget_List_Next(wlevel);
    if (!wnext) {
      DBG_NOTICE(0, "No next widget");
    }
    while(wnext) {
      DBG_NOTICE(0, "Checking this widget: %s", GWEN_Widget_GetName(wnext));
      wfocus=GWEN_UI_GetDeepestFocusable(wnext);
      if (wfocus) {
        return wfocus;
      }

      wnext=GWEN_Widget_List_Next(wnext);
    }
    if (GWEN_Widget_GetFlags(wlevel) & GWEN_WIDGET_FLAGS_MODAL)
      wlevel=0;
    else
      wlevel=GWEN_Widget_GetParent(wlevel);
  }

  DBG_NOTICE(0, "No next widget to focus to");
  return 0;
}



int GWEN_UI_FocusToNext(){
  GWEN_WIDGET *wfocus;

  wfocus=0;
  if (GWEN_UI__ui->focusWidget)
    wfocus=GWEN_UI__FocusToNext(GWEN_UI__ui->focusWidget);
  if (!wfocus) {
    GWEN_WIDGET *wc;

    /* get first focus */
    wc=0;
    /* get root (or modal widget above) */
    if (GWEN_UI__ui->focusWidget) {
      wc=GWEN_Widget_GetParent(GWEN_UI__ui->focusWidget);
      while(wc) {
        if (GWEN_Widget_GetFlags(wc) & GWEN_WIDGET_FLAGS_MODAL)
          /* found root */
          break;
        wc=GWEN_Widget_GetParent(wc);
      }
    }
    if (!wc)
      wc=GWEN_Widget_List_First(GWEN_UI__ui->rootWidgets);
    while(wc) {
      wfocus=GWEN_UI_GetDeepestFocusable(wc);
      if (wfocus)
        break;
      wc=GWEN_Widget_List_Next(wc);
    }
  }

  if (!wfocus)
    return 1;

  GWEN_UI_SetFocus(wfocus);
  return 0;
}



GWEN_WIDGET *GWEN_UI__FocusToPrevious(GWEN_WIDGET *wlevel){
  GWEN_WIDGET *wfocus;

  assert(wlevel);
  while(wlevel) {
    GWEN_WIDGET *wnext;

    DBG_NOTICE(0, "Checking this widget: %s", GWEN_Widget_GetName(wlevel));
    wnext=GWEN_Widget_List_Previous(wlevel);
    if (!wnext) {
      DBG_NOTICE(0, "No previous widget");
    }
    while(wnext) {
      DBG_NOTICE(0, "Checking this widget: %s", GWEN_Widget_GetName(wnext));
      wfocus=GWEN_UI_GetDeepestFocusableBackwards(wnext);
      if (wfocus) {
        return wfocus;
      }

      wnext=GWEN_Widget_List_Previous(wnext);
    }
    if (GWEN_Widget_GetFlags(wlevel) & GWEN_WIDGET_FLAGS_MODAL)
      wlevel=0;
    else
      wlevel=GWEN_Widget_GetParent(wlevel);
  }

  DBG_NOTICE(0, "No previous widget to focus to");
  return 0;
}



int GWEN_UI_FocusToPrevious(){
  GWEN_WIDGET *wfocus;

  wfocus=0;
  if (GWEN_UI__ui->focusWidget)
    wfocus=GWEN_UI__FocusToPrevious(GWEN_UI__ui->focusWidget);
  if (!wfocus) {
    GWEN_WIDGET *wc;

    /* get last focus */
    wc=0;
    /* get root (or modal widget above) */
    if (GWEN_UI__ui->focusWidget) {
      wc=GWEN_Widget_GetParent(GWEN_UI__ui->focusWidget);
      while(wc) {
        if (GWEN_Widget_GetFlags(wc) & GWEN_WIDGET_FLAGS_MODAL)
          /* found root */
          break;
        wc=GWEN_Widget_GetParent(wc);
      }
    }
    if (!wc)
      wc=GWEN_Widget_List_Last(GWEN_UI__ui->rootWidgets);
    while(wc) {
      wfocus=GWEN_UI_GetDeepestFocusableBackwards(wc);
      if (wfocus)
        break;
      wc=GWEN_Widget_List_Previous(wc);
    }
  }

  if (!wfocus)
    return 1;

  GWEN_UI_SetFocus(wfocus);
  return 0;
}



void GWEN_UI_DumpWidgets(){
  GWEN_WIDGET *w;

  if (!GWEN_UI__ui) {
    DBG_NOTICE(0, "Not initialized.");
    return;
  }

  w=GWEN_Widget_List_First(GWEN_UI__ui->rootWidgets);
  while(w) {
    GWEN_Widget_Dump(w, 0);
    w=GWEN_Widget_List_Next(w);
  }
}



int GWEN_UI_Calculate_MinWidth(const char *text){
  int m;

  m=0;
  if (text) {
    const char *p;
    int cm;

    cm=0;
    p=text;
    while(*p) {
      if (*p=='\n' || isspace(*p)) {
        m=(m>cm)?m:cm;
        cm=0;
      }
      else
        cm++;
      p++;
    } /* while */
  }

  return m;
}



int GWEN_UI_Calculate_Height(const char *text, int width){
  int m;
  int i;

  m=0;
  if (text) {
    const char *p;
    const char *lastSpace;

    lastSpace=0;
    i=0;
    p=text;
    while(*p) {
      if (*p=='\n') {
        lastSpace=0;
        i=0;
        m++;
      }
      else {
        if (isspace(*p)) {
          lastSpace=p;
        }
        if (i>=width) {
          if (!lastSpace) {
            DBG_INFO(0, "Text does not fit");
            return -1;
          }
          m++;
          p=lastSpace;
          lastSpace=0;
          i=0;
        }
      }
      p++;
    } /* while */
  }

  return m;
}



int GWEN_UI_GetCols(){
  return COLS;
}



int GWEN_UI_GetLines(){
  return LINES;
}









