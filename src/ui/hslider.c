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


#include "hslider_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/event.h>
#include <gwenhywfar/buffer.h>


GWEN_INHERIT(GWEN_WIDGET, GWEN_HSLIDER)



GWEN_WIDGET *GWEN_HSlider_new(GWEN_WIDGET *parent,
                              GWEN_TYPE_UINT32 flags,
                              const char *name,
                              int x, int y, int width){
  GWEN_WIDGET *w;
  GWEN_HSLIDER *win;

  w=GWEN_Widget_new(parent,
                    flags,
                    name, 0,
                    x,
                    y,
                    width,
                    1);
  GWEN_Widget_SetTypeName(w, "HSlider");
  GWEN_NEW_OBJECT(GWEN_HSLIDER, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_HSLIDER, w, win,
                       GWEN_HSlider_freeData);

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_HSlider_EventHandler);

  return w;
}



void GWEN_HSlider_freeData(void *bp, void *p) {
  GWEN_HSLIDER *win;

  win=(GWEN_HSLIDER*)p;
  GWEN_FREE_OBJECT(win);
}



int GWEN_HSlider_Calculate(GWEN_WIDGET *w) {
  GWEN_HSLIDER *win;
  int k;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_HSLIDER, w);
  assert(win);

  k=GWEN_Widget_GetWidth(w);
  if (win->dsize) {
    win->slength=(double)(k*k)/
      (double)(win->dsize);
    if (win->slength==0)
      win->slength=1;
    win->spos=
      (double)(win->dpos*k)/
      (double)(win->dsize);

    /* check result */
    if (win->spos>=k)
      win->spos=k-1;
    if (win->spos+win->slength>k)
      win->slength=k-win->spos;

  }
  else {
    win->slength=k;
    win->spos=0;
  }

  DBG_NOTICE(0, "H-Slider: %d, %d", win->spos, win->slength);
  return 0;
}



int GWEN_HSlider_Draw(GWEN_WIDGET *w) {
  GWEN_HSLIDER *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_HSLIDER, w);
  assert(win);

  if (win->slength) {
    GWEN_BUFFER *mbuf;
    unsigned int i;

    mbuf=GWEN_Buffer_new(0, GWEN_Widget_GetWidth(w), 0, 1);
    GWEN_Widget_Clear(w, 0, 0, GWEN_EventClearMode_ToEOL);
    for (i=0; i<win->slength; i++) {
      GWEN_Buffer_AppendByte(mbuf, GWEN_WIDGET_CHAR_ESC_CHAR);
      GWEN_Buffer_AppendByte(mbuf, GWEN_WIDGET_CHAR_BLOCK);
    }

    GWEN_Widget_WriteAt(w,
                        win->spos, 0,
                        GWEN_Buffer_GetStart(mbuf),
                        GWEN_Buffer_GetUsedBytes(mbuf));
    GWEN_Buffer_free(mbuf);
  }
  return 0;
}



void GWEN_HSlider_Update(GWEN_WIDGET *w) {
  if (!GWEN_HSlider_Calculate(w))
    GWEN_HSlider_Draw(w);
}



GWEN_UI_RESULT GWEN_HSlider_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_HSLIDER *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_HSLIDER, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw:
    win->previousHandler(w, e);
    GWEN_HSlider_Update(w);
    return GWEN_UIResult_Handled;

  case GWEN_EventType_Scroll:
    if (GWEN_EventScroll_GetTodo(e)) {
      /* we should scroll */

    }
    else {
      /* buddy has scrolled, we should adapt */
      win->dpos+=GWEN_EventScroll_GetXBy(e);
      GWEN_Widget_Update(w);
      return win->previousHandler(w, e);
    }
    break;

  case GWEN_EventType_ContentChg:
    win->dsize=GWEN_EventContentChg_GetContentWidth(e);
    GWEN_Widget_Update(w);
    break;

  case GWEN_EventType_Highlight: {
    GWEN_WIDGET_COLOUR col;

    DBG_NOTICE(0, "Event: Highlight (%s)", GWEN_Widget_GetName(w));
    col=GWEN_EventHighlight_GetHi(e);
    if (col!=0) {
      DBG_NOTICE(0, "Setting colour %d", col);
      GWEN_Widget_SetColour(w, col);
    }
    break;
    if (col==0) {
      DBG_NOTICE(0, "Setting default colour");
      GWEN_Widget_SetColour(w, GWEN_WidgetColour_Button);
    }
    else {
      DBG_NOTICE(0, "Setting colour %d", col);
      GWEN_Widget_SetColour(w, col);
    }
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Refresh:
    break;

  case GWEN_EventType_Update:
    GWEN_HSlider_Update(w);
    GWEN_Widget_Refresh(w);
    return win->previousHandler(w, e);

  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}












