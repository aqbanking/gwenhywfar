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


#include "scrollwidget_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include "hslider.h"
#include "vslider.h"


#include <ncurses.h> /* for beep, DEBUG */


GWEN_INHERIT(GWEN_WIDGET, GWEN_SCROLLWIN)



GWEN_WIDGET *GWEN_ScrollWidget_new(GWEN_WIDGET *parent,
                                   GWEN_TYPE_UINT32 flags,
                                   const char *name,
                                   int x, int y, int width, int height){
  GWEN_WIDGET *w;
  GWEN_SCROLLWIN *win;
  int xoffs, yoffs, woffs, hoffs;
  GWEN_TYPE_UINT32 sliderFlags;

  w=GWEN_Widget_new(parent,
                    flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS,
                    name, 0, x, y, width, height);
  GWEN_Widget_SetTypeName(w, "ScrollWidget");
  GWEN_NEW_OBJECT(GWEN_SCROLLWIN, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_SCROLLWIN, w, win,
                       GWEN_ScrollWidget_freeData);

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_ScrollWidget_EventHandler);

  sliderFlags=
    GWEN_WIDGET_FLAGS_DEFAULT |
    GWEN_WIDGET_FLAGS_HIGHLIGHT;

  if (flags & GWEN_SCROLLWIN_FLAGS_PASSIVE_SLIDERS)
    sliderFlags&=
      ~GWEN_WIDGET_FLAGS_FOCUSABLE &
      ~GWEN_WIDGET_FLAGS_HIGHLIGHT;
  width=GWEN_Widget_GetWidth(w);
  height=GWEN_Widget_GetHeight(w);
  xoffs=0;
  yoffs=0;
  woffs=0;
  hoffs=0;

  if (flags & GWEN_WIDGET_FLAGS_BORDER) {
    xoffs++;
    yoffs++;
    woffs+=2;
    hoffs+=2;
  }

  if (flags & GWEN_SCROLLWIN_FLAGS_HSLIDER) {
    DBG_NOTICE(0, "Creating horizontal slider");
    win->wHslider=GWEN_HSlider_new(w,
                                   sliderFlags,
                                   "HSlider",
                                   xoffs,
                                   height-
                                   ((flags & GWEN_WIDGET_FLAGS_BORDER)?2:1),
                                   width-woffs-1);
    hoffs++;
    GWEN_Widget_SetColour(win->wHslider, GWEN_WidgetColour_Button);
  }

  if (flags & GWEN_SCROLLWIN_FLAGS_VSLIDER) {
    DBG_NOTICE(0, "Creating vertical slider");
    win->wVslider=GWEN_VSlider_new(w,
                                   sliderFlags,
                                   "VSlider",
                                   width-
                                   ((flags & GWEN_WIDGET_FLAGS_BORDER)?2:1),
                                   yoffs,
                                   height-hoffs);
    woffs++;
    GWEN_Widget_SetColour(win->wVslider, GWEN_WidgetColour_Button);
  }

  DBG_NOTICE(0, "Creating viewport");
  win->wView=GWEN_Widget_new(w, 0, "View", 0,
                             xoffs, yoffs, width-woffs, height-hoffs);
  DBG_NOTICE(0, "Viewport is at %d/%d", xoffs, yoffs);
  return w;
}



void GWEN_ScrollWidget_freeData(void *bp, void *p) {
  GWEN_SCROLLWIN *win;

  win=(GWEN_SCROLLWIN*)p;
  GWEN_FREE_OBJECT(win);
}



GWEN_WIDGET *GWEN_ScrollWidget_GetViewPort(GWEN_WIDGET *w){
  GWEN_SCROLLWIN *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_SCROLLWIN, w);
  assert(win);

  return win->wView;
}



GWEN_UI_RESULT GWEN_ScrollWidget_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_SCROLLWIN *win;
  GWEN_WIDGET *wSender;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_SCROLLWIN, w);
  assert(win);
  assert(e);


  wSender=GWEN_Event_GetSender(e);
  if (wSender) {
    if (wSender==win->wView ||
        GWEN_Widget_IsChildOf(wSender, win->wView)) {
      /* event from view port received */
      switch(GWEN_Event_GetType(e)) {

      case GWEN_EventType_Scroll:
        if (!GWEN_EventScroll_GetTodo(e)) {
          int byX;
          int byY;

          byX=GWEN_EventScroll_GetXBy(e);
          byY=GWEN_EventScroll_GetYBy(e);

          /* view port has scrolled, we should adapt the sliders */
          if (win->wHslider && byX) {
            /* inform HSlider about horizontal changes */
            GWEN_Widget_Scrolled(win->wHslider, byX, byY);
          }

          if (win->wVslider && byY) {
            /* inform YSlider about vertical changes */
            GWEN_Widget_Scrolled(win->wVslider, byX, byY);
          }

          return GWEN_UIResult_Handled;
        }
        return win->previousHandler(w, e);
        break;

      case GWEN_EventType_ContentChg: {
        int cw, ch;
        GWEN_WIDGET *wSender;

        wSender=GWEN_Event_GetSender(e);
        cw=GWEN_EventContentChg_GetContentWidth(e);
        ch=GWEN_EventContentChg_GetContentHeight(e);
        if (win->wHslider)
          GWEN_Widget_ContentChange(win->wHslider, cw, ch);
        if (win->wVslider)
          GWEN_Widget_ContentChange(win->wVslider, cw, ch);
        return GWEN_UIResult_Handled;
      }

      default:
        break;
      } /* switch */
    }
    else if (wSender==win->wHslider) {
      /* event from horizontal slider */
    }
    else if (wSender==win->wVslider) {
      /* event from horizontal slider */
    }
  } /* if sender */


  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw:
    return win->previousHandler(w, e);

  case GWEN_EventType_ContentChg:
    break;

  case GWEN_EventType_Update:
    break;
  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}








