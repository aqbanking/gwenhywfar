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


#include "messagebox_p.h"
#include <gwenhywfar/scrollwidget.h>
#include <gwenhywfar/button.h>
#include <gwenhywfar/window.h>
#include <gwenhywfar/textwidget.h>
#include <gwenhywfar/ui.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>





int GWEN_MessageBox(GWEN_WIDGET *parent,
                    const char *name,
                    const char *title,
                    const char *text,
                    int width,
                    const char *b1,
                    const char *b2,
                    const char *b3){
  GWEN_WIDGET *mw;
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_WIDGET *but1;
  GWEN_WIDGET *but2;
  GWEN_WIDGET *but3;
  GWEN_UI_RESULT res;
  GWEN_WIDGET *prevFocus;
  GWEN_TW_LINE_LIST *ll;

  int x,y;
  int ww, wh;
  int response;
  int theight;
  int twidth;
  int height;
  GWEN_TYPE_UINT32 scrollerFlags;

  but1=but2=but3=0;
  scrollerFlags=GWEN_WIDGET_FLAGS_DEFAULT;

  ll=GWEN_TextWidget_TextToLines(text, 0, width-1);
  if (!ll) {
    DBG_ERROR(0, "Bad text");
    return -1;
  }
  theight=GWEN_TWLine_List_GetCount(ll);
  if (theight==0)
    theight=1;
  height=theight+6;
  if (height>(GWEN_UI_GetLines()-2)) {
    scrollerFlags|=GWEN_WINDOW_FLAGS_VSLIDER;
    height=GWEN_UI_GetLines()-2;
  }

  twidth=GWEN_TextWidget_GetMaxWidth(ll);
  if (twidth==-1) {
    DBG_ERROR(0, "Bad text");
    return -1;
  }

  DBG_NOTICE(0, "Sizes: width=%d, height=%d, twidth=%d, theight=%d",
             width, height, twidth, theight);

  prevFocus=GWEN_UI_GetFocusWidget();
  if (parent) {
    x=(GWEN_Widget_GetWidth(parent)-width)/2;
    y=(GWEN_Widget_GetHeight(parent)-height)/2;
  }
  else {
    x=(GWEN_UI_GetCols()-width)/2;
    y=(GWEN_UI_GetLines()-height)/2;
  }

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WIDGET_FLAGS_PANEL |
                     GWEN_WINDOW_FLAGS_TITLE |
                     0,
                     "MessageWindow",
                     title,
		     x, y,
		     width, height);
  GWEN_Widget_SetColour(mw, GWEN_WidgetColour_Message);
  GWEN_Widget_SetColour(GWEN_Window_GetViewPort(mw),
			GWEN_WidgetColour_Message);

  ww=GWEN_Widget_GetWidth(GWEN_Window_GetViewPort(mw));
  wh=GWEN_Widget_GetHeight(GWEN_Window_GetViewPort(mw));

  w=GWEN_ScrollWidget_new(GWEN_Window_GetViewPort(mw),
                          scrollerFlags,
			  "ScrollWidget",
			  0, 1,
			  0,
			  wh-3);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Message);
  GWEN_Widget_SetColour(GWEN_ScrollWidget_GetViewPort(w),
			GWEN_WidgetColour_Message);

  tv=GWEN_TextWidget_new(GWEN_ScrollWidget_GetViewPort(w),
			 GWEN_WIDGET_FLAGS_DEFAULT |
			 GWEN_TEXTWIDGET_FLAGS_DATACHANGE |
			 ((scrollerFlags & GWEN_WINDOW_FLAGS_VSLIDER)?
			  (0):GWEN_TEXTWIDGET_FLAGS_FIXED),
			 "TextView",
			 0,
			 0, 0,
			 0, 0);
  GWEN_TextWidget_SetVirtualSize(tv, twidth, GWEN_TWLine_List_GetCount(ll));
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Message);
  GWEN_TextWidget_SetPreparedText(tv, ll, GWEN_EventSetTextMode_Replace);

  /* add buttons */

  if (b1 && b2 && b3) {
    /* three button mode */
  }
  else if (b1 && b2) {
    /* two button mode */
  }
  else if (b1) {
    int i;

    /* single button mode */
    i=strlen(b1);
    but1=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HCENTER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Button1",
                         b1,
                         GWEN_MSGBOX_CMD_BUT1, /* commandId */
                         (ww-(i+2))/2,
			 wh-1,
                         i+2, 1);
    GWEN_Widget_SetColour(but1, GWEN_WidgetColour_Button);
  }
  else {
    DBG_ERROR(0, "No buttons ??");
  }

  GWEN_Widget_Redraw(mw);
  GWEN_Widget_SetFocus(tv);

  response=0;
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      break;
    DBG_NOTICE(0, "Got this event:");
    GWEN_Event_Dump(e);
    if (GWEN_Event_GetType(e)==GWEN_EventType_Command) {
      GWEN_TYPE_UINT32 cmd;

      cmd=GWEN_EventCommand_GetCommandId(e);
      switch(cmd) {
      case GWEN_MSGBOX_CMD_BUT1: response=1; break;
      case GWEN_MSGBOX_CMD_BUT2: response=2; break;
      case GWEN_MSGBOX_CMD_BUT3: response=3; break;
      default:
        DBG_ERROR(0, "Unknown command \"%d\"", cmd);
        break;
      }
      GWEN_Widget_Close(mw); // DEBUG
    }
    else
      res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
    if (res==GWEN_UIResult_Finished ||
        res==GWEN_UIResult_Quit) {
      break;
    }
  }

  DBG_NOTICE(0, "Response was: %d", response);

  GWEN_Widget_free(mw);
  if (prevFocus)
    GWEN_Widget_SetFocus(prevFocus);
  return response;
}







