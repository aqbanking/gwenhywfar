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


#include "hline_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/event.h>
#include <gwenhywfar/buffer.h>


GWEN_INHERIT(GWEN_WIDGET, GWEN_HLINE)



GWEN_WIDGET *GWEN_HLine_new(GWEN_WIDGET *parent,
                            GWEN_TYPE_UINT32 flags,
                            const char *name,
                            int x, int y, int width, int height){
  GWEN_WIDGET *w;
  GWEN_HLINE *win;

  w=GWEN_Widget_new(parent,
                    flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS,
                    name, 0,
                    x,
                    y,
                    width,
                    height);
  GWEN_Widget_SetTypeName(w, "HLine");
  GWEN_NEW_OBJECT(GWEN_HLINE, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_HLINE, w, win,
                       GWEN_HLine_freeData);
  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  win->flags=flags;
  GWEN_Widget_SetEventHandler(w, GWEN_HLine_EventHandler);
  GWEN_Widget_SetCursorX(w, -1);
  GWEN_Widget_SetCursorY(w, -1);

  return w;
}



void GWEN_HLine_freeData(void *bp, void *p) {
  GWEN_HLINE *win;

  win=(GWEN_HLINE*)p;
  GWEN_FREE_OBJECT(win);
}




GWEN_UI_RESULT GWEN_HLine_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_HLINE *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_HLINE, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw: {
    win->previousHandler(w, e);
    GWEN_Widget_Update(w);
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Update:
    if (!(win->flags & GWEN_HLINE_FLAGS_EMPTY)) {
      GWEN_BUFFER *buf;
      int ww;

      ww=GWEN_Widget_GetWidth(w);
      buf=GWEN_Buffer_new(0, ww, 0, 1);
      while(ww--) {
        GWEN_Buffer_AppendByte(buf, GWEN_WIDGET_CHAR_ESC_CHAR);
        GWEN_Buffer_AppendByte(buf, GWEN_WIDGET_CHAR_HLINE);
      }
      GWEN_Widget_WriteAt(w, 0, 0,
                          GWEN_Buffer_GetStart(buf),
                          GWEN_Buffer_GetUsedBytes(buf));
      GWEN_Buffer_free(buf);
      GWEN_Widget_Refresh(w);
    }
    break;

  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}









