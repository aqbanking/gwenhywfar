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
#include <gwenhywfar/ui/scrollwidget.h>
#include <gwenhywfar/ui/button.h>
#include <gwenhywfar/ui/window.h>
#include <gwenhywfar/ui/textwidget.h>
#include <gwenhywfar/ui/ui.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>





int GWEN_MessageBox(GWEN_WIDGET *parent,
                    const char *name,
                    const char *title,
                    const char *text,
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
  int twidth;
  int width;
  int height;
  int buttonWidth;
  GWEN_TYPE_UINT32 scrollerFlags;

  but1=but2=but3=0;
  scrollerFlags=GWEN_WIDGET_FLAGS_DEFAULT;

  /* calculate minimum width */
  if (b1 && b2 && b3) {
    int i, i1, i2, i3;

    /* three button mode */
    i1=strlen(b1);
    i2=strlen(b2);
    i3=strlen(b3);
    i=i1;
    if (i2>i)
      i=i2;
    if (i3>i)
      i=i3;
    i+=2;
    buttonWidth=i;
    width=i+i+i+4;
  }
  else if (b1 && b2 && !b3) {
    int i, i1, i2;

    i1=strlen(b1);
    i2=strlen(b2);
    if (i1>i2)
      i=i1;
    else
      i=i2;
    i+=2;
    buttonWidth=i;
    width=i+i+3;
  }
  else if (b1 && !b2 && !b3) {
    width=strlen(b1)+2;
    buttonWidth=width;
  }
  else {
    DBG_ERROR(0, "Bad combination of buttons");
    return -1;
  }

  if (width<GWEN_MESSAGEBOX_MINWIDTH &&
      GWEN_UI_GetCols()>=GWEN_MESSAGEBOX_MINWIDTH)
    width=GWEN_MESSAGEBOX_MINWIDTH;
  if (width+2>GWEN_UI_GetCols()) {
    DBG_ERROR(0, "Buttons won't fit into screen");
    return -1;
  }

  ll=GWEN_TextWidget_TextToLines(text, 0, 0);
  if (!ll) {
    DBG_ERROR(0, "Bad text");
    return -1;
  }
  height=GWEN_TWLine_List_GetCount(ll)+6;
  if (height>(GWEN_UI_GetLines()-2)) {
    scrollerFlags|=GWEN_WINDOW_FLAGS_VSLIDER;
    height=GWEN_UI_GetLines()-2;
  }

  twidth=GWEN_TextWidget_GetMaxListWidth(ll);
  if (twidth>width)
    width=twidth;
  width+=2; /* take borders into account */

  if (width>GWEN_UI_GetCols()) {
    width=GWEN_UI_GetCols();
  }

  DBG_NOTICE(0, "ZZZ: Sizes: width=%d, height=%d, twidth=%d (%d)",
             width, height, twidth, -twidth);

  prevFocus=GWEN_UI_GetFocusWidget();

  x=(GWEN_UI_GetCols()-width)/2;
  y=(GWEN_UI_GetLines()-height)/2;

  if (x<0) x=0;
  if (y<0) y=0;

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WIDGET_FLAGS_MODAL |
                     GWEN_WIDGET_FLAGS_IGN_HELP |
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

  if (twidth<width-2) {
    tv=GWEN_TextWidget_new(GWEN_ScrollWidget_GetViewPort(w),
                           GWEN_WIDGET_FLAGS_DEFAULT |
                           GWEN_TEXTWIDGET_FLAGS_DATACHANGE |
                           ((scrollerFlags & GWEN_WINDOW_FLAGS_VSLIDER)?
                            (0):GWEN_TEXTWIDGET_FLAGS_FIXED),
                           "TextView",
                           0,
                           (width-twidth)/2, 0,
                           0, 0);
  }
  else {
    tv=GWEN_TextWidget_new(GWEN_ScrollWidget_GetViewPort(w),
                           GWEN_WIDGET_FLAGS_DEFAULT |
                           GWEN_TEXTWIDGET_FLAGS_DATACHANGE |
                           ((scrollerFlags & GWEN_WINDOW_FLAGS_VSLIDER)?
                            (0):GWEN_TEXTWIDGET_FLAGS_FIXED),
                           "TextView",
                           0,
                           0, 0,
                           0, 0);
  }
  GWEN_TextWidget_SetVirtualSize(tv, twidth, GWEN_TWLine_List_GetCount(ll));
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Message);
  GWEN_TextWidget_SetPreparedText(tv, ll, GWEN_EventSetTextMode_Replace);

  /* add buttons */

  if (b1 && b2 && b3) {
    int x1, x2, x3;

    /* three button mode */
    x1=(ww-(buttonWidth+buttonWidth+buttonWidth+1))/2;
    x2=x1+buttonWidth+1;
    x3=x2+buttonWidth+1;
    but1=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HCENTER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Button1",
                         b1,
                         GWEN_MSGBOX_CMD_BUT1, /* commandId */
                         x1,
                         wh-1,
                         buttonWidth, 1);
    GWEN_Widget_SetColour(but1, GWEN_WidgetColour_Button);

    but2=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HCENTER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Button2",
                         b2,
                         GWEN_MSGBOX_CMD_BUT2, /* commandId */
                         x2,
                         wh-1,
                         buttonWidth, 1);
    GWEN_Widget_SetColour(but2, GWEN_WidgetColour_Button);

    but3=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HCENTER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Button3",
                         b3,
                         GWEN_MSGBOX_CMD_BUT3, /* commandId */
                         x3,
                         wh-1,
                         buttonWidth, 1);
    GWEN_Widget_SetColour(but3, GWEN_WidgetColour_Button);
  }
  else if (b1 && b2) {
    int x1, x2;

    /* two button mode */
    x1=(ww-(buttonWidth+buttonWidth+1))/2;
    x2=x1+buttonWidth+1;
    but1=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HCENTER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Button1",
                         b1,
                         GWEN_MSGBOX_CMD_BUT1, /* commandId */
                         x1,
                         wh-1,
                         buttonWidth, 1);
    GWEN_Widget_SetColour(but1, GWEN_WidgetColour_Button);

    but2=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HCENTER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Button2",
                         b2,
                         GWEN_MSGBOX_CMD_BUT2, /* commandId */
                         x2,
                         wh-1,
                         buttonWidth, 1);
    GWEN_Widget_SetColour(but2, GWEN_WidgetColour_Button);

  }
  else if (b1) {
    /* single button mode */
    but1=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HCENTER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Button1",
                         b1,
                         GWEN_MSGBOX_CMD_BUT1, /* commandId */
                         (ww-buttonWidth)/2,
                         wh-1,
                         buttonWidth, 1);
    GWEN_Widget_SetColour(but1, GWEN_WidgetColour_Button);
  }
  else {
    DBG_ERROR(0, "No buttons ??");
  }

  GWEN_Widget_Redraw(mw);
  if (scrollerFlags & GWEN_WINDOW_FLAGS_VSLIDER || !but1)
    GWEN_Widget_SetFocus(tv);
  else
    GWEN_Widget_SetFocus(but1);

  response=0;
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e) {
      DBG_NOTICE(0, "ZZZ: No event");
      break;
    }
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
      GWEN_Event_free(e);
    }
    else {
      res=GWEN_UI_DispatchEvent(e);
      if (res==GWEN_UIResult_Quit ||
          res==GWEN_UIResult_Finished) {
        DBG_NOTICE(0, "ZZZ: Finished or quit");
        break;
      }
      GWEN_Event_free(e);
    }
  }

  DBG_NOTICE(0, "ZZZ: Response was: %d", response);

  GWEN_Widget_free(mw);
  if (prevFocus)
    GWEN_Widget_SetFocus(prevFocus);
  return response;
}







