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
#include <time.h>
#include <string.h>

#include <gwenhywfar/widget.h>

#include <ncurses.h>


static GWEN_UI *GWEN_UI__ui=0;


int GWEN_UI_Begin() {
  if (GWEN_UI__ui) {
    GWEN_UI__ui->usage++;
    return 0;
  }
  initscr();
  start_color();
  cbreak();
  noecho();
  nonl();
  keypad(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  /* Initialize all the colors */
  init_pair(GWEN_WidgetColour_Default, COLOR_WHITE, COLOR_BLUE);
  init_pair(GWEN_WidgetColour_Message, COLOR_WHITE, COLOR_RED);
  init_pair(GWEN_WidgetColour_Unselected, COLOR_RED, COLOR_WHITE);
  init_pair(GWEN_WidgetColour_Selected, COLOR_RED, COLOR_CYAN);
  init_pair(GWEN_WidgetColour_Title, COLOR_BLUE, COLOR_WHITE);

  GWEN_NEW_OBJECT(GWEN_UI, GWEN_UI__ui);
  GWEN_UI__ui->rootWidgets=GWEN_Widget_List_new();
  GWEN_UI__ui->events=GWEN_Event_List_new();
  GWEN_UI__ui->usage=1;
  return 0;
}



int GWEN_UI_End() {
  assert(GWEN_UI__ui);
  if ((--GWEN_UI__ui->usage)==0) {
    GWEN_Event_List_free(GWEN_UI__ui->events);
    if (GWEN_Widget_List_GetCount(GWEN_UI__ui->rootWidgets)) {
      DBG_WARN(0, "There are still %d widgets",
               GWEN_Widget_List_GetCount(GWEN_UI__ui->rootWidgets));
    }
    GWEN_Widget_List_free(GWEN_UI__ui->rootWidgets);
    GWEN_FREE_OBJECT(GWEN_UI__ui);
    GWEN_UI__ui=0;
    endwin();
  }
  return 0;
}



void GWEN_UI_AddRootWidget(GWEN_WIDGET *w){
  assert(GWEN_UI__ui);

  GWEN_Widget_List_Add(w, GWEN_UI__ui->rootWidgets);
}


int GWEN_UI_SendEvent(GWEN_WIDGET *wRecipient,
                      GWEN_WIDGET *wSender,
                      GWEN_EVENT *e){
  assert(GWEN_UI__ui);
  assert(e);
  assert(wRecipient);

  if (wSender)
    GWEN_Event_SetSender(e, wSender);
  GWEN_Event_SetRecipient(e, wRecipient);
  GWEN_Event_List_Add(e, GWEN_UI__ui->events);
  return 0;
}



GWEN_EVENT *GWEN_UI_PeekNextEvent(GWEN_WIDGET *wRecipient) {
  GWEN_EVENT *e;

  assert(GWEN_UI__ui);
  e=GWEN_Event_List_First(GWEN_UI__ui->events);
  while(e) {
    if (wRecipient || wRecipient==GWEN_Event_GetRecipient(e))
      break;
    e=GWEN_Event_List_Next(e);
  } /* while */

  return e;
}



GWEN_EVENT *GWEN_UI_GetNextEvent(GWEN_WIDGET *wRecipient) {
  GWEN_EVENT *e;

  e=GWEN_UI_PeekNextEvent(wRecipient);
  if (e)
    GWEN_Event_List_Del(e);
  return e;
}



void GWEN_UI_SetFocus(GWEN_WIDGET *w) {
  assert(GWEN_UI__ui);

  if (GWEN_UI__ui->focusWidget) {
    GWEN_EVENT *e;

    e=GWEN_EventFocus_new(GWEN_EventFocusType_Lost);
    assert(e);
    if (GWEN_UI_SendEvent(GWEN_UI__ui->focusWidget, 0, e)) {
      DBG_ERROR(0, "Could not send event");
      GWEN_Event_free(e);
    }
    GWEN_Widget_free(GWEN_UI__ui->focusWidget);
  }

  GWEN_UI__ui->focusWidget=w;
  if (w) {
    GWEN_EVENT *e;

    GWEN_Widget_Attach(GWEN_UI__ui->focusWidget);

    e=GWEN_EventFocus_new(GWEN_EventFocusType_Got);
    assert(e);
    if (GWEN_UI_SendEvent(w, 0, e)) {
      DBG_ERROR(0, "Could not send event");
      GWEN_Event_free(e);
    }
  }
}



GWEN_UI_RESULT GWEN_UI_HandleEvents() {
  int quit;
  int handled;
  GWEN_EVENT *e;

  assert(GWEN_UI__ui);
  quit=0;
  handled=0;

  while(1) {
    GWEN_UI_RESULT res;
    GWEN_WIDGET *wRec;

    e=GWEN_Event_List_First(GWEN_UI__ui->events);
    if (!e)
      break;
    wRec=GWEN_Event_GetRecipient(e);
    assert(wRec);
    res=GWEN_Widget_HandleEvent(wRec, e);
    if (res==GWEN_UIResult_NotHandled) {
      GWEN_WIDGET *wParent;

      wParent=GWEN_Widget_GetParent(wRec);
      if (wParent) {
        res=GWEN_Widget_HandleEvent(wParent, e);
      }
    }

    switch(res) {
    case GWEN_UIResult_Handled:
      handled++;
      break;
    case GWEN_UIResult_Quit:
      quit=1;
      break;
    default:
      break;
    } /* switch */
    GWEN_Event_List_Del(e);
    GWEN_Event_free(e);
  } /* while e */

  doupdate();
  if (quit)
    return GWEN_UIResult_Quit;
  if (handled)
    return GWEN_UIResult_Handled;
  return GWEN_UIResult_NotHandled;
}



void GWEN_UI_Flush() {
  GWEN_UI_HandleEvents();
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
          if (GWEN_Widget_GetFlags(w) & GWEN_WIDGET_FLAGS_FOCUSABLE) {
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

        /* handle user interaction */
        if (!(GWEN_Widget_GetFlags(GWEN_UI__ui->focusWidget) &
              GWEN_WIDGET_FLAGS_NEEDCURSOR))
          move(0, 0);
        ch=getch();
        if (ch==ERR) {
          /* timeout */
          return GWEN_UIResult_Handled;
        }
        DBG_NOTICE(0, "Generating key event");
        e=GWEN_EventKey_new(ch);
        assert(e);
        if (GWEN_UI_SendEvent(GWEN_UI__ui->focusWidget, 0, e)) {
          DBG_ERROR(0, "Could not send event");
          GWEN_Event_free(e);
        }
      }
    }
  } /* while */

  return GWEN_UIResult_Quit;
}







