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


#include "tablewidget_p.h"
#include <gwenhywfar/ui/textwidget.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>
#include <ctype.h>

#include <ncurses.h>



GWEN_INHERIT(GWEN_WIDGET, GWEN_TABLEWIDGET);


GWEN_LIST_FUNCTIONS(GWEN_TABLE_FIELD, GWEN_TableField);
GWEN_LIST_FUNCTIONS(GWEN_TABLE_COLUMN, GWEN_TableColumn);





GWEN_TABLE_FIELD *GWEN_TableField_new(int width, int height, char *text){
  GWEN_TABLE_FIELD *tf;

  GWEN_NEW_OBJECT(GWEN_TABLE_FIELD, tf);
  GWEN_LIST_INIT(GWEN_TABLE_FIELD, tf);

  if (text)
    tf->text=strdup(text);

  tf->width=width;
  tf->height=height;
  return tf;
}



void GWEN_TableField_free(GWEN_TABLE_FIELD *tf){
  if (tf) {
    free(tf->text);
    GWEN_LIST_FINI(GWEN_TABLE_FIELD, tf);
    GWEN_FREE_OBJECT(tf);
  }
}



GWEN_TABLE_COLUMN *GWEN_TableField_GetParent(const GWEN_TABLE_FIELD *tf){
  assert(tf);
  return tf->parent;
}



int GWEN_TableField_GetWidth(const GWEN_TABLE_FIELD *tf){
  assert(tf);
  return tf->width;
}



int GWEN_TableField_GetHeight(const GWEN_TABLE_FIELD *tf){
  assert(tf);
  return tf->height;
}



const char *GWEN_TableField_GetText(const GWEN_TABLE_FIELD *tf){
  assert(tf);
  return tf->text;
}



void GWEN_TableField_SetText(GWEN_TABLE_FIELD *tf,
                             const char *s){
  assert(tf);
  free(tf->text);
  if (s) tf->text=strdup(s);
  else tf->text=0;
}



void GWEN_TableField_Update(const GWEN_TABLE_FIELD *tf){
  assert(tf);

  DBG_NOTICE(0, "Updating table field");
  if (tf->text) {
    const char *p;
    const char *lastStart;
    const char *lastSpace;
    int y;


    DBG_NOTICE(0, "Field contains data");
    y=0;
    p=tf->text;
    while(*p) {
      GWEN_TW_LINE *lh;
      GWEN_WIDGET *tw;
      int wx;
      int wy;
      int i;
      int len;
      int breakLine;

      lastStart=p;
      lastSpace=0;
      i=0;
      breakLine=0;
      while(*p) {
        if (*p=='\n') {
          //DBG_INFO(0, "Line break");
          lastSpace=p;
          breakLine=1;
          DBG_NOTICE(0, "Breaking line");
          break;
        }
        else {
          if (i>tf->width) {
            //DBG_INFO(0, "Line too long, checking for break (%d)", i);
            if (!lastSpace) {
              DBG_NOTICE(0, "Field does not fit");
              lastSpace=p;
            }
            breakLine=1;
            DBG_NOTICE(0, "Breaking line (%d)", i);
            break;
          }
          else {
            if (isspace(*p)) {
              lastSpace=p;
            }
            i++;
          }
        }
        p++;
      }

      DBG_VERBOUS(0, "Writing line");
      if (y>=tf->height) {
        DBG_NOTICE(0, "Field line %d is not visible", y);
        return;
      }
      if (!tf->parent)
        return;
      if (!tf->parent->parent)
        return;
      tw=tf->parent->parent;

      wx=tf->x;
      wy=y+tf->y;

      lh=GWEN_TextWidget_LineOpen(tw, wy, 1);
      if (!lh) {
        DBG_NOTICE(0, "Error opening line %d", wy);
        return;
      }

      //DBG_NOTICE(0, "Setting borders: %d-%d",
      //           wx, wx+tf->width-1);
      GWEN_TextWidget_LineSetBorders(tw, lh, wx, wx+tf->width-1);
      GWEN_TextWidget_LineClear(tw, lh);
      GWEN_TextWidget_LineSetPos(tw, lh, wx);
      if (breakLine) {
        if (lastSpace)
          len=lastSpace-lastStart;
        else
          len=p-lastStart;
      }
      else {
        len=p-lastStart;
      }
      DBG_NOTICE(0, "Writing line: %d", len);
      if (tf->selected)
        GWEN_TextWidget_LineSetAttributes(tw, lh,
                                          (GWEN_WidgetColour_Chosen &0x0f));
      else
        GWEN_TextWidget_LineSetAttributes(tw, lh, 0x00);
      GWEN_TextWidget_LineWriteText(tw, lh, lastStart, len);
      if (!(GWEN_Widget_GetFlags(tf->parent->parent) &
            GWEN_TABLEWIDGET_FLAGS_COLBORDER))
        GWEN_TextWidget_LineRedraw(tw, lh);
      GWEN_TextWidget_LineClose(tw, lh, 0);

      y++;

      if (lastSpace) {
        p=lastSpace+1;
        lastSpace=0;
      }
      else if (*p)
        p++;
    } /* while */
  }

}



int GWEN_TableField_Calculate_MinWidth(const GWEN_TABLE_FIELD *tf){
  int m;

  assert(tf);

  m=0;
  if (tf->text) {
    const char *p;
    int cm;

    cm=0;
    p=tf->text;
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



int GWEN_TableField_Calculate_Height(const GWEN_TABLE_FIELD *tf, int width){
  int m;
  int i;

  assert(tf);

  m=0;
  if (tf->text) {
    const char *p;
    const char *lastSpace;

    lastSpace=0;
    i=0;
    p=tf->text;
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
            DBG_INFO(0, "Field does not fit");
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










GWEN_TABLE_COLUMN *GWEN_TableColumn_new(int width){
  GWEN_TABLE_COLUMN *tc;

  GWEN_NEW_OBJECT(GWEN_TABLE_COLUMN, tc);
  GWEN_LIST_INIT(GWEN_TABLE_COLUMN, tc);
  tc->fields=GWEN_TableField_List_new();
  tc->width=width;
  return tc;
}



void GWEN_TableColumn_free(GWEN_TABLE_COLUMN *tc){
  if (tc) {
    GWEN_TableField_List_free(tc->fields);
    GWEN_LIST_FINI(GWEN_TABLE_COLUMN, tc);
    GWEN_FREE_OBJECT(tc);
  }
}



void GWEN_TableColumn_Update(GWEN_TABLE_COLUMN *tc){
  GWEN_TABLE_FIELD *tf;

  DBG_NOTICE(0, "Updating table column (%d fields)",
             GWEN_TableField_List_GetCount(tc->fields));
  tf=GWEN_TableField_List_First(tc->fields);
  while(tf) {
    GWEN_TableField_Update(tf);
    tf=GWEN_TableField_List_Next(tf);
  }
}



void GWEN_TableColumn_AddField(GWEN_TABLE_COLUMN *tc, GWEN_TABLE_FIELD *tf){
  GWEN_TABLEWIDGET *win;

  assert(tc);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, tc->parent);
  assert(win);

  assert(tf);

  GWEN_TableField_List_Add(tf, tc->fields);
  tf->y=tc->y+tc->height;
  tf->x=tc->x;
  tc->height+=tf->height;
  if (win->flags & GWEN_TABLEWIDGET_FLAGS_ROWBORDER) {
    tf->y++;
    tc->height++;
  }
  if (win->theight<tc->height) {
    win->theight=tc->height;
    GWEN_Widget_ContentChange(tc->parent, win->twidth, win->theight);
  }

  tf->parent=tc;
}



int GWEN_TableColumn_GetWidth(const GWEN_TABLE_COLUMN *tc){
  assert(tc);
  return tc->width;
}



int GWEN_TableColumn_GetHeight(const GWEN_TABLE_COLUMN *tc){
  assert(tc);
  return tc->height;
}



GWEN_WIDGET *GWEN_TableColumn_GetParent(const GWEN_TABLE_COLUMN *tc){
  assert(tc);
  return tc->parent;
}



GWEN_TABLE_FIELD_LIST*
GWEN_TableColumn_GetFields(const GWEN_TABLE_COLUMN *tc){
  assert(tc);
  return tc->fields;
}
























GWEN_WIDGET *GWEN_TableWidget_new(GWEN_WIDGET *parent,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *name,
                                  int x, int y,
                                  int width, int height){
  GWEN_WIDGET *w;
  GWEN_TABLEWIDGET *win;
  GWEN_TYPE_UINT32 xflags;

  xflags=flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS;
  w=GWEN_TextWidget_new(parent,
                        xflags,
                        name, 0, x, y, width, height);
  GWEN_Widget_SetTypeName(w, "TableWidget");

  GWEN_NEW_OBJECT(GWEN_TABLEWIDGET, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w, win,
                       GWEN_TableWidget_freeData);
  win->flags=flags;
  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_TableWidget_EventHandler);

  win->columns=GWEN_TableColumn_List_new();

  return w;
}




void GWEN_TableWidget_freeData(void *bp, void *p){
  GWEN_TABLEWIDGET *win;

  win=(GWEN_TABLEWIDGET*)p;
  GWEN_TableColumn_List_free(win->columns);

  GWEN_FREE_OBJECT(win);
}



void GWEN_TableWidget__AddColumn(GWEN_WIDGET *w, GWEN_TABLE_COLUMN *tc) {
  GWEN_TABLEWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);
  assert(tc);

  GWEN_TableColumn_List_Add(tc, win->columns);
  tc->y=win->ty;
  tc->x=win->tx+win->twidth;
  win->twidth+=tc->width;
  if (win->flags & GWEN_TABLEWIDGET_FLAGS_COLBORDER &&
      tc->x) {
    tc->x++;
    win->twidth++;
  }
  tc->parent=w;
  GWEN_Widget_ContentChange(w, win->twidth, win->theight);
}




GWEN_TABLE_FIELD *GWEN_TableWidget_LocateField(GWEN_WIDGET *w,
                                               int x, int y, int crea) {
  GWEN_TABLE_COLUMN *tc;
  GWEN_TABLE_FIELD *tf;
  int i;
  GWEN_TABLEWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);

  i=x;
  tc=GWEN_TableColumn_List_First(win->columns);
  while(tc && i--) tc=GWEN_TableColumn_List_Next(tc);
  if (!tc) {
    if (!crea) {
      DBG_NOTICE(0, "Column %d not found", x);
      return 0;
    }
    else {
      i++;
      DBG_NOTICE(0, "Column %d not found, creating %d", x, i);
      while(i--) {
        tc=GWEN_TableColumn_new(GWEN_TABLECOLUMN_DEFAULT_WIDTH);
        GWEN_TableWidget__AddColumn(w, tc);
      }
      win->changed=1;
    }
  }

  i=y;
  tf=GWEN_TableField_List_First(tc->fields);
  while(tf && i--) tf=GWEN_TableField_List_Next(tf);
  if (!tf) {
    if (!crea) {
      return 0;
    }
    else {
      i++;
      DBG_NOTICE(0, "Field not found, creating %d", i);
      while(i--) {
        tf=GWEN_TableField_new(tc->width, 1, "");
        GWEN_TableColumn_AddField(tc, tf);
        tf->height=1;
      }
      win->changed=1;
    }
  }

  return tf;
}



void GWEN_TableWidget_SetText(GWEN_WIDGET *w,
                              int x, int y, const char *text) {
  GWEN_TABLE_FIELD *tf;
  int nh;
  GWEN_TABLEWIDGET *win;

  DBG_NOTICE(0, "Setting text of field %d/%d to %s", x, y, text);
  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);

  tf=GWEN_TableWidget_LocateField(w, x, y, 1);
  assert(tf);

  GWEN_TableField_SetText(tf, text);

  nh=GWEN_TableField_Calculate_Height(tf, tf->width);
  if (nh<1) tf->height=1;
  else tf->height=nh;
  win->changed=1;
  GWEN_Widget_Changed(w);
}



const char *GWEN_TableWidget_GetText(GWEN_WIDGET *w, int x, int y) {
  GWEN_TABLE_FIELD *tf;

  tf=GWEN_TableWidget_LocateField(w, x, y, 0);
  if (tf)
    return tf->text;
  return 0;
}



void GWEN_TableWidget_DrawBorder(GWEN_WIDGET *w, int y1, int y2){
  GWEN_TABLE_COLUMN *tc;
  GWEN_TABLEWIDGET *win;

  DBG_NOTICE(0, "Drawing border");
  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);

  if (win->flags & GWEN_TABLEWIDGET_FLAGS_COLBORDER) {
    unsigned int i;
    /*unsigned -- see below*/ char vert[3]={
      GWEN_WIDGET_CHAR_ESC_CHAR,
      GWEN_WIDGET_CHAR_VLINE,
      0};

    for (i=y1; i<y2; i++) {
      GWEN_TW_LINE *lh;
      int x;
      int first;

      lh=GWEN_TextWidget_LineOpen(w, i, 1);
      if (!lh) {
        DBG_INFO(0, "Could not update table entry");
        return;
      }
      //DBG_NOTICE(0, "Setting borders: %d, %d", 0, win->twidth-1);
      GWEN_TextWidget_LineSetBorders(w, lh, 0, win->twidth-1);
      GWEN_TextWidget_LineSetInsert(w, lh, 0);
      tc=GWEN_TableColumn_List_First(win->columns);
      first=1;
      while(tc) {
        if (first)
          first=0;
        else {
          x=tc->x-1;
          if (GWEN_TextWidget_LineSetPos(w, lh, x)) {
            DBG_NOTICE(0, "Could not set pos %d", x);
          }
	  /* The 'vert' argument is expected to be 'char*', not
	     'unsigned char*', therefore the definition above has
	     been changed. */
          GWEN_TextWidget_LineWriteText(w, lh,
                                        vert, 0);
        }
        tc=GWEN_TableColumn_List_Next(tc);
      }
      GWEN_TextWidget_LineRedraw(w, lh);
      GWEN_TextWidget_LineClose(w, lh, 0);
    }
  }

}



void GWEN_TableWidget_Update(GWEN_WIDGET *w){
  GWEN_TABLE_COLUMN *tc;
  GWEN_TABLEWIDGET *win;

  DBG_NOTICE(0, "Updating table");
  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);

  tc=GWEN_TableColumn_List_First(win->columns);
  while(tc) {
    GWEN_TableColumn_Update(tc);
    tc=GWEN_TableColumn_List_Next(tc);
  }
  GWEN_TableWidget_DrawBorder(w, 0, win->theight);
  win->changed=0;
}



void GWEN_TableWidget_Refresh(GWEN_WIDGET *w){
  GWEN_TABLE_COLUMN *tc;
  GWEN_TABLEWIDGET *win;

  DBG_NOTICE(0, "Refreshing table");
  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);

  if (!win->changed)
    return;

  tc=GWEN_TableColumn_List_First(win->columns);
  while(tc) {
    GWEN_TableColumn_Update(tc);
    tc=GWEN_TableColumn_List_Next(tc);
  }
  GWEN_TableWidget_DrawBorder(w, 0, win->theight);
  win->changed=0;
}

















void GWEN_TableWidget_Highlight(GWEN_WIDGET *w,
                                GWEN_TABLE_FIELD *tf,
                                GWEN_WIDGET_COLOUR colour){
  if (tf->width && tf->height && tf->parent) {
    int i;
    int x;
    int y;
    int ww;
    GWEN_TABLEWIDGET *win;

    assert(w);
    win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
    assert(win);

    x=tf->x;
    if ((x+tf->width-1)<win->left) {
      return;
    }
    if (x>win->left+GWEN_Widget_GetWidth(w))
      return;
    x-=win->left;
    ww=tf->width;

    y=tf->y+tf->parent->y+win->ty;

    if (win->flags & GWEN_TABLEWIDGET_FLAGS_LINEMODE) {
      x=0;
      ww=GWEN_Widget_GetWidth(w);
    }

    for (i=0; i<tf->height; i++) {
      if ((y+i)>=win->top &&
          (y+i<(win->top+GWEN_Widget_GetHeight(w)))) {
        //DBG_NOTICE(0, "Highlighting %d/%d, %d  (%d) [%08x]",
        //           x, y+i-win->top, tf->width, colour, (unsigned int)tf);
        GWEN_Widget_Highlight(w, x, y+i-win->top, ww, colour);
      }
      else {
        //DBG_INFO(0, "Line %d is not visible (top=%d)", y+i, win->top);
      }
    }
  }
}





GWEN_UI_RESULT GWEN_TableWidget_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e){
  GWEN_TABLEWIDGET *win;
  GWEN_UI_RESULT rv;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);
  assert(e);

  rv=GWEN_UIResult_Handled;

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_SetText: {
    DBG_NOTICE(0, "Event: SetText(%s)", GWEN_Widget_GetName(w));
    rv=win->previousHandler(w, e);
    break;
  }
  case GWEN_EventType_Draw: {
    GWEN_TABLE_FIELD *tf;

    DBG_NOTICE(0, "Event: Draw(%s)", GWEN_Widget_GetName(w));
    win->previousHandler(w, e);
    GWEN_TableWidget_Update(w);
    tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
    if (GWEN_Widget_GetState(w) & GWEN_WIDGET_STATE_HASFOCUS && tf) {
      if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
        GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
    }
    if (tf) {
      GWEN_EVENT *newE;

      newE=GWEN_EventActivated_new(tf->text, win->mx, win->my);
      assert(newE);
      if (GWEN_Widget_SendEvent(w, w, newE)) {
        DBG_INFO(0, "Could not send event");
        GWEN_Event_free(newE);
      }
    }

    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Highlight: {
    GWEN_WIDGET_COLOUR hi;

    DBG_NOTICE(0, "Event: Highlight(%s)", GWEN_Widget_GetName(w));

    hi=GWEN_EventHighlight_GetHi(e);
    win->previousHandler(w, e);

    if (!hi) {
      int x, y;
      int len;
      GWEN_TABLE_COLUMN *tc;
      int updated;

      x=GWEN_EventHighlight_GetX(e);
      y=GWEN_EventHighlight_GetY(e);
      len=GWEN_EventHighlight_GetLen(e);
      y+=win->top;

      updated=0;
      tc=GWEN_TableColumn_List_First(win->columns);
      while(tc) {
        GWEN_TABLE_FIELD *tf;

        if (tc->x>=x && (tc->x+tc->width)<=(x+len)) {
          tf=GWEN_TableField_List_First(tc->fields);
          while(tf) {
            if (y>=tf->y && y<(tf->y+tf->height)) {
              /* found Y line */
              GWEN_TableField_Update(tf);
              updated=1;
              break;
            }
            tf=GWEN_TableField_List_Next(tf);
          }
        }
        tc=GWEN_TableColumn_List_Next(tc);
      }

      if (updated)
        GWEN_Widget_Refresh(w);
      return GWEN_UIResult_Handled;
    }
    else {
      return win->previousHandler(w, e);
    }
    break;
  }

  case GWEN_EventType_Scroll: {
    DBG_NOTICE(0, "Event: Scroll(%s)", GWEN_Widget_GetName(w));

    if (!GWEN_EventScroll_GetTodo(e)) {
      int byX;
      int byY;

      win->previousHandler(w, e);

      byX=GWEN_EventScroll_GetXBy(e);
      byY=GWEN_EventScroll_GetYBy(e);
      if (!byX && !byY)
        return GWEN_UIResult_Handled;

      win->top+=byY;
      win->left+=byX;

      //DBG_NOTICE(0, "Scrolled: x=%d, y=%d (top=%d, left=%d)",
      //           byX, byY, win->top, win->left);
      GWEN_TableWidget_DrawBorder(w, 0, win->theight);
      if (GWEN_Widget_GetState(w) & GWEN_WIDGET_STATE_HASFOCUS) {
        GWEN_TABLE_FIELD *tf;
        tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
        if (tf) {
          if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
            GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
          GWEN_Widget_SetCursorX(w, tf->x-win->left);
          GWEN_Widget_SetCursorY(w, tf->y-win->top);
        }
      }
      return GWEN_UIResult_NotHandled;
    }
    return win->previousHandler(w, e);
    break;
  }

  case GWEN_EventType_Update: {
    DBG_NOTICE(0, "Event: Update(%s)", GWEN_Widget_GetName(w));
    GWEN_TableWidget_Refresh(w);
    return win->previousHandler(w, e);
  }

  case GWEN_EventType_Refresh:
    GWEN_TableWidget_Refresh(w);
    return win->previousHandler(w, e);

  case GWEN_EventType_Key: {
    int key;

    DBG_NOTICE(0, "Event: Key(%s)", GWEN_Widget_GetName(w));
    key=GWEN_EventKey_GetKey(e);
    if (key==KEY_DOWN) {
      GWEN_TABLE_FIELD *tf;
      GWEN_TABLE_FIELD *ntf;
      GWEN_EVENT *newE;

      tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
      ntf=GWEN_TableWidget_LocateField(w, win->mx, win->my+1,
                                       !(win->flags &
                                         GWEN_TABLEWIDGET_FLAGS_FIXED));
      if (!ntf || !tf) {
        return GWEN_UIResult_Handled;
      }
      if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
        GWEN_TableWidget_Highlight(w, tf, 0);
      if (!GWEN_TextWidget_EnsureVisible(w,
                                         ntf->x,
                                         ntf->y,
                                         ntf->width,
                                         ntf->height)) {
        if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
          GWEN_TableWidget_Highlight(w, ntf, GWEN_WidgetColour_Selected);
        win->my++;
        newE=GWEN_EventActivated_new(ntf->text, win->mx, win->my);
        assert(newE);
        if (GWEN_Widget_SendEvent(w, w, newE)) {
          DBG_INFO(0, "Could not send event");
          GWEN_Event_free(newE);
        }
        GWEN_Widget_SetCursorX(w, ntf->x-win->left);
        GWEN_Widget_SetCursorY(w, ntf->y-win->top);
        //GWEN_Widget_Refresh(w);
      }
      else {
        if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
          GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
      }
      return GWEN_UIResult_Handled;
    }

    else if (key==KEY_UP) {
      GWEN_TABLE_FIELD *tf;

      if (win->my) {
        tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
        if (tf) {
          if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
            GWEN_TableWidget_Highlight(w, tf, 0);
        }
        tf=GWEN_TableWidget_LocateField(w, win->mx, win->my-1,
                                        !(win->flags &
                                          GWEN_TABLEWIDGET_FLAGS_FIXED));
        if (!tf) {
          return GWEN_UIResult_Handled;
        }
        if (!GWEN_TextWidget_EnsureVisible(w,
                                           tf->x,
                                           tf->y,
                                           tf->width,
                                           tf->height)) {
          GWEN_EVENT *newE;

          if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
            GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
          win->my--;
          newE=GWEN_EventActivated_new(tf->text, win->mx, win->my);
          assert(newE);
          if (GWEN_Widget_SendEvent(w, w, newE)) {
            DBG_INFO(0, "Could not send event");
            GWEN_Event_free(newE);
          }
	  GWEN_Widget_SetCursorX(w, tf->x-win->left);
          GWEN_Widget_SetCursorY(w, tf->y-win->top);
          //GWEN_Widget_Refresh(w);
	}
	else {
	  if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
	    GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
	}
      }
      else {
      }
      return GWEN_UIResult_Handled;
    }

    else if (key==KEY_RIGHT) {
      GWEN_TABLE_FIELD *tf;
      GWEN_TABLE_FIELD *ntf;
      GWEN_EVENT *newE;

      tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
      ntf=GWEN_TableWidget_LocateField(w, win->mx+1, win->my,
                                       !(win->flags &
                                         GWEN_TABLEWIDGET_FLAGS_FIXED));
      if (!ntf || !tf) {
        return GWEN_UIResult_Handled;
      }
      if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
        GWEN_TableWidget_Highlight(w, tf, 0);
      if (!GWEN_TextWidget_EnsureVisible(w,
                                         ntf->x,
                                         ntf->y,
                                         ntf->width,
                                         ntf->height)) {
        if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
          GWEN_TableWidget_Highlight(w, ntf, GWEN_WidgetColour_Selected);
        win->mx++;

        newE=GWEN_EventActivated_new(ntf->text, win->mx, win->my);
        assert(newE);
        if (GWEN_Widget_SendEvent(w, w, newE)) {
          DBG_INFO(0, "Could not send event");
          GWEN_Event_free(newE);
        }

        GWEN_Widget_SetCursorX(w, ntf->x-win->left);
        GWEN_Widget_SetCursorY(w, ntf->y-win->top);
        //GWEN_Widget_Refresh(w);
      }
      else {
        if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
          GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
      }
      return GWEN_UIResult_Handled;
    }

    else if (key==KEY_LEFT) {
      GWEN_TABLE_FIELD *tf;
      GWEN_EVENT *newE;

      if (win->mx) {
        tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
        if (tf) {
          if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
            GWEN_TableWidget_Highlight(w, tf, 0);
        }
        tf=GWEN_TableWidget_LocateField(w, win->mx-1, win->my,
                                        !(win->flags &
                                          GWEN_TABLEWIDGET_FLAGS_FIXED));
        if (!tf) {
          return GWEN_UIResult_Handled;
        }
        if (!GWEN_TextWidget_EnsureVisible(w,
                                           tf->x,
                                           tf->y,
                                           tf->width,
                                           tf->height)) {
          if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
            GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
          win->mx--;
          newE=GWEN_EventActivated_new(tf->text, win->mx, win->my);
          assert(newE);
          if (GWEN_Widget_SendEvent(w, w, newE)) {
            DBG_INFO(0, "Could not send event");
            GWEN_Event_free(newE);
          }
          GWEN_Widget_SetCursorX(w, tf->x-win->left);
          GWEN_Widget_SetCursorY(w, tf->y-win->top);
          //GWEN_Widget_Refresh(w);
        }
        else {
          if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
            GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
        }
      }
      else {
      }
      return GWEN_UIResult_Handled;
    }
    else if (key==32 && (win->flags & GWEN_TABLEWIDGET_FLAGS_SELECTION)) {
      GWEN_TABLE_FIELD *tf;

      tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
      if (tf) {
        GWEN_EVENT *newE;

        tf->selected=!tf->selected;
        GWEN_TableField_Update(tf);
        newE=GWEN_EventSelected_new(tf->text, win->mx, win->my,
                                    tf->selected);
        assert(newE);
        if (GWEN_Widget_SendEvent(w, w, newE)) {
          DBG_INFO(0, "Could not send event");
          GWEN_Event_free(newE);
        }
        GWEN_Widget_Refresh(w);
      }
      return GWEN_UIResult_Handled;
    }
    else if (key==13) {
      GWEN_TABLE_FIELD *tf;
      int x;

      if (win->flags & GWEN_TABLEWIDGET_FLAGS_LINEMODE)
        x=0;
      else
        x=win->mx;
      tf=GWEN_TableWidget_LocateField(w, x, win->my, 0);
      if (tf) {
        GWEN_EVENT *newE;

        newE=GWEN_EventChosen_new(tf->text, x, win->my);
        assert(newE);
        if (GWEN_Widget_SendEvent(w, w, newE)) {
          DBG_INFO(0, "Could not send event");
          GWEN_Event_free(newE);
        }
      }
      return GWEN_UIResult_Handled;
    }
    else if (key==KEY_F(11)) {
      GWEN_TABLE_COLUMN *tc;
      int x;

      DBG_NOTICE(0, "Table:");
      tc=GWEN_TableColumn_List_First(win->columns);
      x=0;
      while(tc) {
        GWEN_TABLE_FIELD *tf;
        int y;

        y=0;
        DBG_NOTICE(0, "Column: %d: %d", x, tc->width);
        tf=GWEN_TableField_List_First(tc->fields);
        while(tf) {
          DBG_NOTICE(0, "Field: %3d/%3d %3d/%3d: %s",
                     x, y, tf->width, tf->height, tf->text);
          tf=GWEN_TableField_List_Next(tf);
          y++;
        }
        tc=GWEN_TableColumn_List_Next(tc);
        x++;
      }
    }

    rv=win->previousHandler(w, e);
    break;
  }

  case GWEN_EventType_Focus: {
    GWEN_EVENT_FOCUS_TYPE ft;
    GWEN_TABLE_FIELD *tf;

    DBG_NOTICE(0, "Event: Focus(%s)", GWEN_Widget_GetName(w));
    win->previousHandler(w, e);
    ft=GWEN_EventFocus_GetFocusEventType(e);

    tf=GWEN_TableWidget_LocateField(w, win->mx, win->my, 0);
    if (tf) {
      if (ft==GWEN_EventFocusType_Got) {
	GWEN_EVENT *newE;

	if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
	  GWEN_TableWidget_Highlight(w, tf, GWEN_WidgetColour_Selected);
	newE=GWEN_EventActivated_new(tf->text, win->mx, win->my);
	assert(newE);
	if (GWEN_Widget_SendEvent(w, w, newE)) {
	  DBG_INFO(0, "Could not send event");
	  GWEN_Event_free(newE);
	}
      }
      else {
        if (win->flags & GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT)
          GWEN_TableWidget_Highlight(w, tf, 0);
      }
    }
    return GWEN_UIResult_Handled;
  }
  default:
    rv=win->previousHandler(w, e);
    break;
  } /* switch */

  return rv;
}



int GWEN_TableWidget_AddColumn(GWEN_WIDGET *w, int width){
  GWEN_TABLEWIDGET *win;
  GWEN_TABLE_COLUMN *tc;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);

  tc=GWEN_TableColumn_new(width);
  GWEN_TableWidget__AddColumn(w, tc);

  GWEN_Widget_Changed(w);
  return 0;
}



int GWEN_TableWidget_GetColumns(const GWEN_WIDGET *w) {
  GWEN_TABLEWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);

  return GWEN_TableColumn_List_GetCount(win->columns);
}



int GWEN_TableWidget_Clear(GWEN_WIDGET *w){
  GWEN_TABLEWIDGET *win;
  GWEN_TABLE_COLUMN *tc;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TABLEWIDGET, w);
  assert(win);

  tc=GWEN_TableColumn_List_First(win->columns);
  while(tc) {
    GWEN_TableField_List_Clear(tc->fields);
    tc->height=0;
    tc=GWEN_TableColumn_List_Next(tc);
  }
  win->top=0;
  win->left=0;
  win->mx=0;
  win->my=0;
  win->ty=0;
  win->theight=0;
  win->changed=1;
  GWEN_TextWidget_Clear(w);
  return 0;
}






