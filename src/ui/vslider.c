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


#include "vslider_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/event.h>
#include <gwenhywfar/buffer.h>


GWEN_INHERIT(GWEN_WIDGET, GWEN_VSLIDER)



GWEN_WIDGET *GWEN_VSlider_new(GWEN_WIDGET *parent,
                              GWEN_TYPE_UINT32 flags,
                              const char *name,
                              int x, int y, int height){
  GWEN_WIDGET *w;
  GWEN_VSLIDER *win;

  w=GWEN_Widget_new(parent,
		    flags,
		    name, 0,
		    x,
		    y,
		    1,
		    height);
  GWEN_Widget_SetTypeName(w, "VSlider");
  GWEN_NEW_OBJECT(GWEN_VSLIDER, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_VSLIDER, w, win,
                       GWEN_VSlider_freeData);

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_VSlider_EventHandler);

  return w;
}



void GWEN_VSlider_freeData(void *bp, void *p) {
  GWEN_VSLIDER *win;

  win=(GWEN_VSLIDER*)p;
  GWEN_FREE_OBJECT(win);
}



int GWEN_VSlider_Calculate(GWEN_WIDGET *w) {
  GWEN_VSLIDER *win;
  int k;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_VSLIDER, w);
  assert(win);

  k=GWEN_Widget_GetHeight(w);
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

  DBG_NOTICE(0, "V-Slider: %d, %d (%d, %d)",
             win->spos, win->slength,
             win->dpos, win->dsize);
  return 0;
}



int GWEN_VSlider_Draw(GWEN_WIDGET *w) {
  GWEN_VSLIDER *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_VSLIDER, w);
  assert(win);

  if (win->slength) {
    int i;
    int j;
    int k;
    static const char cbuf[3]={
      GWEN_WIDGET_CHAR_ESC_CHAR,
      GWEN_WIDGET_CHAR_BLOCK
    };

    k=GWEN_Widget_GetHeight(w);
    j=win->spos+win->slength;
    for (i=0; i<k; i++) {
      if (i<win->spos || i>=j) {
        GWEN_Widget_WriteAt(w,
                            0, i, " ", 1);
      }
      else {
        GWEN_Widget_WriteAt(w,
                            0, i, cbuf, 2);
      }
    }
  }
  GWEN_Widget_Refresh(w);
  return 0;
}



void GWEN_VSlider_Update(GWEN_WIDGET *w) {
  if (!GWEN_VSlider_Calculate(w))
    GWEN_VSlider_Draw(w);
}



GWEN_UI_RESULT GWEN_VSlider_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_VSLIDER *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_VSLIDER, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw:
    win->previousHandler(w, e);
    GWEN_VSlider_Update(w);
    return GWEN_UIResult_Handled;
    break;

  case GWEN_EventType_Scroll:
    if (GWEN_EventScroll_GetTodo(e)) {
      /* we should scroll */

    }
    else {
      /* buddy has scrolled, we should adapt */
      win->dpos+=GWEN_EventScroll_GetYBy(e);
      GWEN_Widget_Update(w);
      return win->previousHandler(w, e);
    }
    break;

  case GWEN_EventType_ContentChg:
    win->dsize=GWEN_EventContentChg_GetContentHeight(e);
    GWEN_Widget_Update(w);
    break;

  case GWEN_EventType_Update:
    GWEN_VSlider_Update(w);
    return win->previousHandler(w, e);
  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}












