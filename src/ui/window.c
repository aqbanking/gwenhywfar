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


#include "window_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>


GWEN_INHERIT(GWEN_WIDGET, GWEN_WINDOW)



GWEN_WIDGET *GWEN_Window_new(GWEN_WIDGET *parent,
                             GWEN_TYPE_UINT32 flags,
                             const char *name,
                             const char *title,
                             int x, int y, int width, int height){
  GWEN_WIDGET *w;
  GWEN_WINDOW *win;
  int xoffs, yoffs, woffs, hoffs;

  w=GWEN_Widget_new(parent, flags, name, 0, x, y, width, height);
  GWEN_NEW_OBJECT(GWEN_WINDOW, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_WINDOW, w, win,
                       GWEN_Window_freeData);
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
  if (flags & GWEN_WINDOW_FLAGS_TITLE) {
    win->wTitle=GWEN_Widget_new(w,
                                GWEN_WIDGET_FLAGS_HCENTER,
                                "Title", title,
                                xoffs, yoffs,
                                width-woffs, 1);
    GWEN_Widget_SetColour(win->wTitle, GWEN_WidgetColour_Title);
    yoffs++;
    hoffs++;
  }
  win->wView=GWEN_Widget_new(w, 0, "View", 0,
                             xoffs, yoffs, width-woffs, height-hoffs);
  DBG_NOTICE(0, "Viewport is at %d/%d", xoffs, yoffs);
  GWEN_Widget_Redraw(w);
  return w;
}



void GWEN_Window_freeData(void *bp, void *p) {
  GWEN_WINDOW *win;

  win=(GWEN_WINDOW*)p;
  GWEN_FREE_OBJECT(win);
}



GWEN_WIDGET *GWEN_Window_GetViewPort(GWEN_WIDGET *w){
  GWEN_WINDOW *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_WINDOW, w);
  assert(win);

  return win->wView;
}











