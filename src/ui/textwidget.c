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


#include "textwidget_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/bio_buffer.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>


#include <ncurses.h>



GWEN_LIST_FUNCTIONS(GWEN_TW_LINE, GWEN_TWLine)


GWEN_INHERIT(GWEN_WIDGET, GWEN_TEXTWIDGET);


GWEN_TW_LINE *GWEN_TWLine_new(GWEN_TYPE_UINT32 atts,
                              const char *s,
                              int len) {
  GWEN_TW_LINE *l;
  int rv;

  GWEN_NEW_OBJECT(GWEN_TW_LINE, l);
  GWEN_LIST_INIT(GWEN_TW_LINE, l);

  if (s && len) {
    rv=GWEN_TWLine_SetText(l, atts, s, len);
    assert(rv==0);
  }

  l->usage=1;
  return l;
}



int GWEN_TWLine_SetText(GWEN_TW_LINE *l,
                        GWEN_TYPE_UINT32 atts,
                        const char *s,
                        int len) {
  assert(l);

  l->length=0;
  assert(s);
  GWEN_Buffer_free(l->compressedText);
  l->compressedText=GWEN_Buffer_new(0, len, 0, 1);
  GWEN_Buffer_AppendBytes(l->compressedText, s, len);
  l->compressed=1;
  l->decompressed=0;
  l->startAttributes=atts;
  l->endAttributes=0;
  return 0;
}



void GWEN_TWLine_free(GWEN_TW_LINE *l){
  if (l) {
    assert(l->usage);
    if ((--l->usage)==0) {
      GWEN_LIST_FINI(GWEN_TW_LINE, l);
      GWEN_Buffer_free(l->compressedText);
      GWEN_Buffer_free(l->attributes);
      GWEN_Buffer_free(l->chars);
      GWEN_FREE_OBJECT(l);
    }
  }
}



void GWEN_TWLine_Attach(GWEN_TW_LINE *l){
  assert(l);
  l->usage++;
}




GWEN_WIDGET *GWEN_TextWidget_new(GWEN_WIDGET *parent,
                                 GWEN_TYPE_UINT32 flags,
                                 const char *name,
                                 const char *text,
                                 int x, int y, int width, int height){
  GWEN_WIDGET *w;
  GWEN_TEXTWIDGET *win;

  w=GWEN_Widget_new(parent,
                    flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS,
                    name, 0, x, y, width, height);
  GWEN_Widget_SetTypeName(w, "TextWidget");

  GWEN_NEW_OBJECT(GWEN_TEXTWIDGET, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w, win,
                       GWEN_TextWidget_freeData);
  win->lines=GWEN_TWLine_List_new();
  win->vwidth=GWEN_Widget_GetWidth(w);
  win->vheight=GWEN_Widget_GetHeight(w);
  win->flags=flags;

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_TextWidget_EventHandler);
  if (text)
    GWEN_TextWidget_SetText(w, text, GWEN_EventSetTextMode_Replace);
  return w;
}



void GWEN_TextWidget_freeData(void *bp, void *p) {
  GWEN_TEXTWIDGET *win;

  win=(GWEN_TEXTWIDGET*)p;
  GWEN_TWLine_List_free(win->lines);
  GWEN_FREE_OBJECT(win);
}



void GWEN_TextWidget_SetVirtualSize(GWEN_WIDGET *w,
                                    int vwidth,
                                    int vheight){
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);
  if (vwidth>=GWEN_Widget_GetWidth(w))
    win->vwidth=vwidth;
  if (vheight>=GWEN_Widget_GetHeight(w))
    win->vheight=vheight;
}



int GWEN_TextWidget_SetPreparedText(GWEN_WIDGET *w,
                                    GWEN_TW_LINE_LIST *ll,
                                    GWEN_EVENT_SETTEXT_MODE m) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  assert(ll);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  switch(m) {
  case GWEN_EventSetTextMode_Replace:
    GWEN_TWLine_List_free(win->lines);
    win->lines=ll;
    break;

  case GWEN_EventSetTextMode_Append:
    assert(win->lines);
    GWEN_TWLine_List_AddList(win->lines, ll);
    GWEN_TWLine_List_free(ll);
    break;

  case GWEN_EventSetTextMode_Insert:
    assert(win->lines);
    GWEN_TWLine_List_AddList(ll, win->lines);
    GWEN_TWLine_List_free(win->lines);
    win->lines=ll;
    break;
  default:
    DBG_ERROR(0, "Unknown settext mode %d", m);
    return -1;
  }

  return 0;
}



int GWEN_TextWidget_SetText(GWEN_WIDGET *w,
                            const char *text,
                            GWEN_EVENT_SETTEXT_MODE m) {
  GWEN_TEXTWIDGET *win;
  GWEN_TW_LINE_LIST *ll;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  ll=GWEN_TextWidget_TextToLines(text, 0, 0);
  if (!ll) {
    DBG_ERROR(0, "Bad text format");
    return -1;
  }
  return GWEN_TextWidget_SetPreparedText(w, ll, m);
}



int GWEN_TextWidget_ParseXMLTag(GWEN_WIDGET *w,
                                GWEN_XMLNODE *n,
                                GWEN_BUFFER *buf,
                                GWEN_TYPE_UINT32 startAtts,
                                GWEN_TYPE_UINT32 currentAtts,
                                int indent,
                                int *pos,
                                GWEN_TW_LINE_LIST *ll) {
  const char *p;
  GWEN_TYPE_UINT32 newAtts;
  int newIndent;
  int rv;
  GWEN_TEXTWIDGET *win;
  GWEN_XMLNODE *nn;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  DBG_NOTICE(0, "Parsing tag \"%s\" (%d)",
             GWEN_XMLNode_GetData(n), *pos);
  p=GWEN_XMLNode_GetData(n);
  if (!p) {
    DBG_ERROR(0, "Tag with no name ?");
    return -1;
  }

  newAtts=currentAtts;
  newIndent=indent;
  if (strcasecmp(p, "b")==0 ||
      strcasecmp(p, "strong")==0)
    newAtts|=GWEN_WIDGET_ATT_STANDOUT;
  else if (strcasecmp(p, "i")==0)
    newAtts|=GWEN_WIDGET_ATT_UNDERLINE;
  else if (strcasecmp(p, "p")==0)
    newIndent+=2;

  if (newAtts!=currentAtts) {
    GWEN_Buffer_AppendByte(buf, GWEN_WIDGET_ATT_ESC_CHAR);
    GWEN_Buffer_AppendByte(buf, newAtts & 0xff);
  }
  nn=GWEN_XMLNode_GetChild(n);
  if (nn) {
    rv=GWEN_TextWidget_ParseXMLSubNodes(w, nn, buf,
                                        startAtts, newAtts,
                                        newIndent,
                                        pos,
                                        ll);
  }
  else
    rv=1;
  if (newAtts!=currentAtts) {
    GWEN_Buffer_AppendByte(buf, GWEN_WIDGET_ATT_ESC_CHAR);
    GWEN_Buffer_AppendByte(buf, currentAtts & 0xff);
  }

  if (strcasecmp(p, "br")==0 ||
      strcasecmp(p, "p")==0 ||
      (strcasecmp(p, "gwen")==0 && GWEN_Buffer_GetUsedBytes(buf)) ||
      strcasecmp(p, "ul")==0 ||
      strcasecmp(p, "li")==0 ||
      strcasecmp(p, "ol")==0
     ) {
    GWEN_BUFFER *newbuf;
    GWEN_TW_LINE *l;
    int chg;

    newbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(buf)+128, 0, 1);
    GWEN_Buffer_Rewind(buf);

    if (GWEN_Text_EscapeToBufferTolerant2(buf, newbuf)) {
      DBG_INFO(0, "here");
      GWEN_Buffer_free(newbuf);
      return -1;
    }
    GWEN_Buffer_Rewind(newbuf);

    DBG_NOTICE(0, "New line");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(newbuf),
                        GWEN_Buffer_GetUsedBytes(newbuf),
                        0,
                        GWEN_LoggerLevelNotice);
    l=GWEN_TWLine_new(startAtts,
                      GWEN_Buffer_GetStart(newbuf),
                      GWEN_Buffer_GetUsedBytes(newbuf));
    if (GWEN_TextWidget_SelectLine(l)) {
      DBG_ERROR(0, "Error selecting new line");
      abort();
    }
    GWEN_TWLine_List_Add(l, ll);
    chg=0;
    if (win->dwidth<l->length) {
      win->dwidth=l->length;
      chg=1;
    }
    if (win->dheight<GWEN_TWLine_List_GetCount(ll)) {
      win->dheight=GWEN_TWLine_List_GetCount(ll);
      chg=1;
    }
    if (chg && (win->flags & GWEN_TEXTWIDGET_FLAGS_DATACHANGE))
      GWEN_Widget_ContentChange(w, win->dwidth, win->dheight);
    GWEN_Buffer_Reset(buf);
    GWEN_Buffer_free(newbuf);
    *pos=0;
  }


  return rv;
}



int GWEN_TextWidget_ParseXMLSubNodes(GWEN_WIDGET *w,
                                     GWEN_XMLNODE *n,
                                     GWEN_BUFFER *buf,
                                     GWEN_TYPE_UINT32 startAtts,
                                     GWEN_TYPE_UINT32 currentAtts,
                                     int indent,
                                     int *pos,
                                     GWEN_TW_LINE_LIST *ll){
  const char *p;
  int rv;
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  while(n) {
    int addSpace;

    switch(GWEN_XMLNode_GetType(n)) {
    case GWEN_XMLNodeTypeData:
      if (*pos==0) {
        int i;

        if (indent && currentAtts) {
          GWEN_Buffer_AppendByte(buf, GWEN_WIDGET_ATT_ESC_CHAR);
          GWEN_Buffer_AppendByte(buf, 0);
        }

        for (i=0; i<indent; i++) {
          if (win->flags & GWEN_TEXTWIDGET_FLAGS_BREAKLINES) {
            if (*pos>=win->vwidth) {
              GWEN_BUFFER *newbuf;
              GWEN_TW_LINE *l;
              int chg;
  
              newbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(buf)+128,
                                     0, 1);
              GWEN_Buffer_Rewind(buf);
              if (GWEN_Text_EscapeToBufferTolerant2(buf, newbuf)) {
                DBG_INFO(0, "here");
                GWEN_Buffer_free(newbuf);
                return -1;
              }
              GWEN_Buffer_Rewind(newbuf);
  
              l=GWEN_TWLine_new(startAtts,
                                GWEN_Buffer_GetStart(newbuf),
                                GWEN_Buffer_GetUsedBytes(newbuf));
              if (GWEN_TextWidget_SelectLine(l)) {
                DBG_ERROR(0, "Error selecting new line");
                abort();
              }
              GWEN_TWLine_List_Add(l, ll);
              chg=0;
              if (win->dwidth<l->length) {
                win->dwidth=l->length;
                chg=1;
              }
              if (win->dheight<GWEN_TWLine_List_GetCount(ll)) {
                win->dheight=GWEN_TWLine_List_GetCount(ll);
                chg=1;
              }
              if (chg && (win->flags & GWEN_TEXTWIDGET_FLAGS_DATACHANGE))
                GWEN_Widget_ContentChange(w, win->dwidth, win->dheight);
              GWEN_Buffer_Reset(buf);
              GWEN_Buffer_free(newbuf);
              *pos=0;
            }
          }
          GWEN_Buffer_AppendByte(buf, ' ');
          (*pos)++;
        }
        if (indent && currentAtts) {
          GWEN_Buffer_AppendByte(buf, GWEN_WIDGET_ATT_ESC_CHAR);
          GWEN_Buffer_AppendByte(buf, currentAtts & 0xff);
        }
        addSpace=0;
      }
      else {
        addSpace=1;
      }

      p=GWEN_XMLNode_GetData(n);
      if (p) {
        if (addSpace) {
          GWEN_Buffer_AppendByte(buf, ' ');
          *(pos++);
        }

        while(*p) {
          if (win->flags & GWEN_TEXTWIDGET_FLAGS_BREAKLINES) {
            if (*pos>=win->vwidth) {
              GWEN_BUFFER *newbuf;
              GWEN_TW_LINE *l;
              int chg;
          
              newbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(buf)+128,
                                     0, 1);
              GWEN_Buffer_Rewind(buf);
              if (GWEN_Text_EscapeToBufferTolerant2(buf, newbuf)) {
                DBG_INFO(0, "here");
                GWEN_Buffer_free(newbuf);
                return -1;
              }
              GWEN_Buffer_Rewind(newbuf);

              l=GWEN_TWLine_new(startAtts,
                                GWEN_Buffer_GetStart(newbuf),
                                GWEN_Buffer_GetUsedBytes(newbuf));
              if (GWEN_TextWidget_SelectLine(l)) {
                DBG_ERROR(0, "Error selecting new line");
                abort();
              }
              GWEN_TWLine_List_Add(l, ll);
              chg=0;
              if (win->dwidth<l->length) {
                win->dwidth=l->length;
                chg=1;
              }
              if (win->dheight<GWEN_TWLine_List_GetCount(ll)) {
                win->dheight=GWEN_TWLine_List_GetCount(ll);
                chg=1;
              }
              if (chg && (win->flags & GWEN_TEXTWIDGET_FLAGS_DATACHANGE))
                GWEN_Widget_ContentChange(w, win->dwidth, win->dheight);
              GWEN_Buffer_Reset(buf);
              GWEN_Buffer_free(newbuf);
              *pos=0;
            }
          }
          GWEN_Buffer_AppendByte(buf, *p);
          (*pos)++;
          p++;
        } /* while */
      }
      break;

    case GWEN_XMLNodeTypeTag:
      rv=GWEN_TextWidget_ParseXMLTag(w,
                                     n,
                                     buf,
                                     startAtts,
                                     currentAtts,
                                     indent,
                                     pos,
                                     ll);
      if (rv==-1) {
        DBG_INFO(0, "here");
        return rv;
      }
      else if (rv==1) {
      }
      break;
    default:
      break;
    }

    n=GWEN_XMLNode_Next(n);
  } /* while */

  return 0;
}



void GWEN_TextWidget_Draw(GWEN_WIDGET *w){
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  DBG_NOTICE(0, "Writing lines: %d, %d: %d, %d",
             win->left,
             win->top,
             GWEN_Widget_GetWidth(w),
             GWEN_Widget_GetHeight(w));
  GWEN_TextWidget__DrawArea(w,
                            win->left, GWEN_Widget_GetWidth(w),
                            win->top, GWEN_Widget_GetHeight(w));
}



GWEN_UI_RESULT GWEN_TextWidget_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_SetText: {
    const char *p;
    GWEN_EVENT_SETTEXT_MODE m;

    DBG_NOTICE(0, "Event: SetText(%s)", GWEN_Widget_GetName(w));
    p=GWEN_EventSetText_GetText(e);
    m=GWEN_EventSetText_GetMode(e);
    if (p) {
      GWEN_TextWidget_SetText(w, p, m);
    }
    return GWEN_UIResult_Handled;
  }
  case GWEN_EventType_Draw: {
    DBG_NOTICE(0, "Event: Draw(%s)", GWEN_Widget_GetName(w));
    win->previousHandler(w, e);
    GWEN_TextWidget_Draw(w);
    if ((GWEN_Widget_GetState(w) & GWEN_WIDGET_STATE_HASFOCUS) &&
        (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)) {
      GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                            GWEN_Widget_GetWidth(w),
                            GWEN_WidgetColour_Selected);
    }
    GWEN_Widget_SetCursorX(w, 0);
    GWEN_Widget_SetCursorY(w, win->pos-win->top);
    GWEN_Widget_Refresh(w);
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Highlight: {
    int x, y;
    GWEN_WIDGET_COLOUR hi;

    DBG_NOTICE(0, "Event: Highlight(%s)", GWEN_Widget_GetName(w));
    if (!(win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT))
      return win->previousHandler(w, e);

    x=GWEN_EventHighlight_GetX(e);
    y=GWEN_EventHighlight_GetY(e);
    GWEN_Widget_SetCursorX(w, x);
    GWEN_Widget_SetCursorY(w, y);
    DBG_NOTICE(0, "Highlight %d/%d, top=%d", x, y, win->top);
    y+=win->top;
    hi=GWEN_EventHighlight_GetHi(e);
    win->previousHandler(w, e);
    if (!hi) {
      if (GWEN_Widget_GetFlags(w) & GWEN_WIDGET_FLAGS_BORDER) {
        x++;
        y++;
      }
      DBG_NOTICE(0, "Rewriting line %d/%d", x, y);
      GWEN_TextWidget__DrawArea(w,
                                win->left, GWEN_Widget_GetWidth(w),
                                y, 1);
      GWEN_Widget_Refresh(w);
    }
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Scroll: {
    DBG_NOTICE(0, "Event: Scroll(%s)", GWEN_Widget_GetName(w));
    if (win->flags & GWEN_TEXTWIDGET_FLAGS_FIXED) {
      DBG_NOTICE(0, "Fixed window, can not scroll");
      return GWEN_UIResult_Handled;
    }
    if (GWEN_EventScroll_GetTodo(e)) {
      int byX;
      int byY;

      DBG_NOTICE(0, "\nScrolling...\n");
      byX=GWEN_EventScroll_GetXBy(e);
      byY=GWEN_EventScroll_GetYBy(e);
      if (!byX && !byY)
        return GWEN_UIResult_Handled;

      if (byX) {
        /* handle horizontal scrolling */
        if (byX<0) {
          /* scroll backwards */
          if (win->left+byX<0)
            byX=-win->left;
        }
        else {
          /* scroll forward */
          if ((win->left+byX)>=win->vwidth)
            byX=win->vwidth-win->left-1;
        }
      }
      if (byY) {
        /* handle vertical scrolling */
        if (byY<0) {
          /* scroll backwards */
          if (win->top+byY<0)
            byY=-win->top;
        }
        else {
          /* scroll forward */
          DBG_NOTICE(0, "TOP: %d, byY: %d, vheight: %d",
                     win->top, byY, win->vheight);

          if (win->top+byY>=GWEN_TWLine_List_GetCount(win->lines)-1) {
            byY=GWEN_TWLine_List_GetCount(win->lines)-1-win->top-1;
            DBG_NOTICE(0, "byY corrected");
          }

          if ((win->top+byY)>=win->vheight) {
            byY=win->vheight-win->top-1;
            DBG_NOTICE(0, "Corrected byY: %d", byY);
          }
        }
      }
      if (byX)
        win->left+=byX;
      if (byY)
        win->top+=byY;
      if (byX || byY) {
        //GWEN_Widget_Redraw(w);
        GWEN_TextWidget_Draw(w);
        if ((GWEN_Widget_GetState(w) & GWEN_WIDGET_STATE_HASFOCUS) &&
            (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)) {
          GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                                GWEN_Widget_GetWidth(w),
                                GWEN_WidgetColour_Selected);
        }
        GWEN_Widget_Scrolled(w, byX, byY);
      }
      return GWEN_UIResult_Handled;
    }
    else {
      return win->previousHandler(w, e);
    }
  }

  case GWEN_EventType_Key: {
    int key;

    DBG_NOTICE(0, "Event: Key(%s)", GWEN_Widget_GetName(w));
    key=GWEN_EventKey_GetKey(e);
    if (key==KEY_DOWN) {
      if (win->flags & GWEN_TEXTWIDGET_FLAGS_FIXED) {
        DBG_NOTICE(0, "Fixed window, can not scroll");
        return GWEN_UIResult_Handled;
      }
      if (win->flags & GWEN_TEXTWIDGET_FLAGS_LINEMODE) {
        if (win->pos+1>=
            GWEN_TWLine_List_GetCount(win->lines)) {
          DBG_INFO(0, "Already at bottom of the list");
          beep();
        }
        else {
          /* new position is valid, select it */
          if ((win->pos+1-win->top)<win->vheight) {
            /* remove highlight */
            if (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
              GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                                    GWEN_Widget_GetWidth(w),
                                    0);
            win->pos++;
            if ((win->pos-win->top)>=GWEN_Widget_GetHeight(w)) {
              /* scroll up */
              GWEN_Widget_Scroll(w, 0, 1);
            }
            else {
              if (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
                GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                                      GWEN_Widget_GetWidth(w),
                                      GWEN_WidgetColour_Selected);
            }
          }
          GWEN_Widget_SetCursorX(w, 0);
          GWEN_Widget_SetCursorY(w, win->pos-win->top);
        }
      }
      else {
        /* not in linemode */
        if (win->top+1>=GWEN_TWLine_List_GetCount(win->lines)) {
          beep();
        }
        else {
          GWEN_Widget_Scroll(w, 0, 1);
        }
      }
      return GWEN_UIResult_Handled;
    }
    else if (key==KEY_UP) {
      if (win->flags & GWEN_TEXTWIDGET_FLAGS_FIXED) {
        DBG_NOTICE(0, "Fixed window, can not scroll");
        return GWEN_UIResult_Handled;
      }
      if (win->flags & GWEN_TEXTWIDGET_FLAGS_LINEMODE) {
        /* in line mode */
        if (win->pos) {
          if (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
            GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                                  GWEN_Widget_GetWidth(w),
                                  0);
          win->pos--;
          if (win->pos<win->top) {
            GWEN_Widget_Scroll(w, 0, -1);
          }
          else
            if (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
              GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                                    GWEN_Widget_GetWidth(w),
                                    GWEN_WidgetColour_Selected);
          GWEN_Widget_SetCursorX(w, 0);
          GWEN_Widget_SetCursorY(w, win->pos-win->top);
        }
        else {
          beep();
        }
      }
      else {
        if (win->top) {
          GWEN_Widget_Scroll(w, 0, -1);
        }
        else {
          beep();
        }
      }
      return GWEN_UIResult_Handled;
    }
    if (key==KEY_F(9)) {
      FILE *f;
      GWEN_TW_LINE *l;

      beep();
      f=fopen("gwen-lines.dump", "w+");
      if (!f) {
        DBG_ERROR(0, "fopen: %s", strerror(errno));
        return GWEN_UIResult_Handled;
      }

      l=GWEN_TWLine_List_First(win->lines);
      while(l) {
        if (!l->compressed) {
          if (GWEN_TextWidget_CompressLine(l)) {
            DBG_NOTICE(0, "Could not compress line");
            fprintf(f, "--- error while compressing ---\n");
          }
        }

        if (l->compressed) {
          assert(l->compressedText);
          if (GWEN_Buffer_GetUsedBytes(l->compressedText)) {
            if (1!=fwrite(GWEN_Buffer_GetStart(l->compressedText),
                          GWEN_Buffer_GetUsedBytes(l->compressedText),
                          1, f)) {
              DBG_ERROR(0, "fwrite: %s", strerror(errno));
              fclose(f);
              return GWEN_UIResult_Handled;
            }
          }
          else {
            DBG_NOTICE(0, "Empty line");
            fprintf(f, "--- empty line --");
          }
          fprintf(f, "\n");
        }
        else {
          DBG_NOTICE(0, "Empty line");
          fprintf(f, "--- not compressed ---\n");
        }
        l=GWEN_TWLine_List_Next(l);
      }
      if (fclose(f)) {
        DBG_ERROR(0, "fclose: %s", strerror(errno));
        return GWEN_UIResult_Handled;
      }
      beep();
      return GWEN_UIResult_Handled;
    }

    if (key==KEY_F(2)) {
      FILE *f;
      GWEN_TW_LINE *l;
      int i;
      int j;

      f=fopen("gwen-chars.dump", "w+");
      if (!f) {
        DBG_ERROR(0, "fopen: %s", strerror(errno));
        return GWEN_UIResult_Handled;
      }

      i=GWEN_TWLine_List_GetCount(win->lines);
      for (j=0; j<i; j++) {
        l=GWEN_TextWidget_LineOpen(w, j, 0);
        if (l) {
          if (GWEN_Buffer_GetUsedBytes(l->chars)) {
            if (1!=fwrite(GWEN_Buffer_GetStart(l->chars),
                          GWEN_Buffer_GetUsedBytes(l->chars),
                          1, f)) {
              DBG_ERROR(0, "fwrite: %s", strerror(errno));
              GWEN_TextWidget_LineClose(w, l, 0);
              break;
            }
          }
          else {
            fprintf(f, "--- empty line ---");
          }
          fprintf(f, "\n");
          GWEN_TextWidget_LineClose(w, l, 0);
        }
      }
      if (fclose(f)) {
        DBG_ERROR(0, "fclose: %s", strerror(errno));
        return GWEN_UIResult_Handled;
      }
      beep();
      return GWEN_UIResult_Handled;
    }

    if (key==KEY_F(8)) {
      GWEN_TextWidget_Draw(w);
      beep();
      return GWEN_UIResult_Handled;
    }

    return win->previousHandler(w, e);
  }

  case GWEN_EventType_Focus: {
    GWEN_EVENT_FOCUS_TYPE ft;

    DBG_NOTICE(0, "Event: Focus(%s)", GWEN_Widget_GetName(w));
    win->previousHandler(w, e);
    ft=GWEN_EventFocus_GetFocusEventType(e);
    if (ft==GWEN_EventFocusType_Got) {
      if (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
        GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                              GWEN_Widget_GetWidth(w),
                              GWEN_WidgetColour_Selected);
    }
    else {
      if (win->flags & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
        GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                              GWEN_Widget_GetWidth(w),
                              0);
    }
    return GWEN_UIResult_Handled;
  }
  default:
    return win->previousHandler(w, e);
  } /* switch */

  return win->previousHandler(w, e);
}




int GWEN_TextWidget_DecompressLine(GWEN_TW_LINE *l){
  GWEN_TYPE_UINT32 currAtts;
  unsigned int i;
  int lastWasEsc;
  int lastEscape;
  const unsigned char *p;

  assert(l);

  GWEN_Buffer_free(l->chars);
  GWEN_Buffer_free(l->attributes);
  l->attributes=GWEN_Buffer_new(0, 80, 0, 1);
  l->chars=GWEN_Buffer_new(0, 80, 0, 1);

  if (!l->compressed) {
    l->decompressed=1;
    return 0;
  }

  currAtts=l->startAttributes;

  p=GWEN_Buffer_GetStart(l->compressedText);
  i=GWEN_Buffer_GetUsedBytes(l->compressedText);
  lastWasEsc=0;
  lastEscape=0;
  while(i--) {
    int c;

    c=*p;
    if (lastWasEsc) {
      if (lastEscape==GWEN_WIDGET_ATT_ESC_CHAR)
        currAtts=c;
      else if (lastEscape==GWEN_WIDGET_CHAR_ESC_CHAR) {
        GWEN_Buffer_AppendByte(l->attributes,
                               currAtts | GWEN_WIDGET_ATT_CHAR);
        GWEN_Buffer_AppendByte(l->chars, c);
      }
      lastWasEsc=0;
    }
    else {
      if (c==GWEN_WIDGET_ATT_ESC_CHAR) {
        lastWasEsc=1;
        lastEscape=c;
      }
      else if (c==GWEN_WIDGET_CHAR_ESC_CHAR) {
        lastWasEsc=1;
        lastEscape=c;
      }
      else {
        lastWasEsc=0;
        GWEN_Buffer_AppendByte(l->attributes, currAtts);
        GWEN_Buffer_AppendByte(l->chars, c);
      }
    }
    p++;
  } /* while */

  if (lastWasEsc) {
    DBG_ERROR(0, "Bad string");
    GWEN_Buffer_free(l->attributes); l->attributes=0;
    GWEN_Buffer_free(l->chars); l->chars=0;
    return -1;
  }

  l->changed=0;
  l->decompressed=1;
  l->length=GWEN_Buffer_GetUsedBytes(l->chars);
  return 0;
}



int GWEN_TextWidget_CompressLine(GWEN_TW_LINE *l) {
  unsigned char currAtts;
  unsigned int i;
  const char *pAtts;
  const char *pChars;
  unsigned int len;

  assert(l->decompressed);
  len=GWEN_Buffer_GetUsedBytes(l->chars);
  pChars=GWEN_Buffer_GetStart(l->chars);
  pAtts=GWEN_Buffer_GetStart(l->attributes);
  GWEN_Buffer_free(l->compressedText);
  l->compressedText=GWEN_Buffer_new(0, len, 0, 1);
  currAtts=0xff;
  for (i=0; i<len; i++) {
    if (currAtts!=(*pAtts & ~GWEN_WIDGET_ATT_CHAR)) {
      GWEN_Buffer_AppendByte(l->compressedText, GWEN_WIDGET_ATT_ESC_CHAR);
      GWEN_Buffer_AppendByte(l->compressedText, *pAtts);
      currAtts=(*pAtts & ~GWEN_WIDGET_ATT_CHAR);
    }
    if (*pAtts & GWEN_WIDGET_ATT_CHAR) {
      GWEN_Buffer_AppendByte(l->compressedText, GWEN_WIDGET_CHAR_ESC_CHAR);
      GWEN_Buffer_AppendByte(l->compressedText, *pChars);
    }
    else {
      GWEN_Buffer_AppendByte(l->compressedText, *pChars);
    }
    pChars++;
    pAtts++;
  }

  l->endAttributes=currAtts;
  l->compressed=1;
  return 0;
}



int GWEN_TextWidget_SelectLine(GWEN_TW_LINE *l) {
  assert(l);
  if (l->decompressed)
    return 0;

  if (GWEN_TextWidget_DecompressLine(l)) {
    DBG_ERROR(0, "Error decompressing line");
    return -1;
  }

  return 0;
}



GWEN_TW_LINE *GWEN_TextWidget_LineOpen(GWEN_WIDGET *w, int n, int crea) {
  GWEN_TW_LINE *l;
  GWEN_TW_LINE *lastl;
  GWEN_TEXTWIDGET *win;
  int j;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  j=n;
  l=GWEN_TWLine_List_First(win->lines);
  lastl=l;
  while(l && j) {
    lastl=l;
    l=GWEN_TWLine_List_Next(l);
    j--;
  }
  if (!l) {
    if (crea) {
      int k;
      GWEN_TYPE_UINT32 atts;

      j++;
      DBG_INFO(0, "Line %d not found, create %d lines", n, j);
      atts=0;
      if (lastl) {
        if (GWEN_TextWidget_SelectLine(lastl)) {
          DBG_ERROR(0, "Bad last line");
          return 0;
        }
        atts=lastl->endAttributes;
      }

      /* create it */
      for (k=0; k<j; k++) {
        l=GWEN_TWLine_new(atts, " ", 1);
        GWEN_TWLine_List_Add(l, win->lines);
      }
      if (win->dheight<GWEN_TWLine_List_GetCount(win->lines)) {
        win->dheight=GWEN_TWLine_List_GetCount(win->lines);
        if (win->flags & GWEN_TEXTWIDGET_FLAGS_DATACHANGE)
          GWEN_Widget_ContentChange(w, win->dwidth, win->dheight);
      }
    }
    else {
      DBG_INFO(0, "Line %d not found", n);
      return 0;
    }
  }

  if (GWEN_TextWidget_SelectLine(l)) {
    DBG_ERROR(0, "Bad last line");
    return 0;
  }

  GWEN_TWLine_Attach(l);
  return l;
}




int GWEN_TextWidget_LineClose(GWEN_WIDGET *w,
                              GWEN_TW_LINE *l,
                              int force) {
  assert(l);
  GWEN_TWLine_free(l);
  return 0;
}



int GWEN_TextWidget_LineSetBorders(GWEN_WIDGET *w,
                                   GWEN_TW_LINE *l,
                                   int leftBorder,
                                   int rightBorder) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (rightBorder<leftBorder)
    return -1;

  if (GWEN_TextWidget_SelectLine(l)) {
    DBG_NOTICE(0, "Error selecting line");
    return -1;
  }

  if (leftBorder>l->length) {
    int i;

    i=leftBorder-l->length;
    while(i--) {
      GWEN_Buffer_AppendByte(l->chars, ' ');
      GWEN_Buffer_AppendByte(l->attributes, 0);
      l->length++;
    }
  }
  if (rightBorder>l->length) {
    int i;

    i=rightBorder-l->length;
    while(i--) {
      GWEN_Buffer_AppendByte(l->chars, ' ');
      GWEN_Buffer_AppendByte(l->attributes, 0);
      l->length++;
    }
  }
  l->leftBorder=leftBorder;
  l->rightBorder=rightBorder;

  return 0;
}



int GWEN_TextWidget_LineSetInsert(GWEN_WIDGET *w,
                                  GWEN_TW_LINE *l,
                                  int insert) {
  l->insertOn=insert;

  return 0;
}



int GWEN_TextWidget_LineSetAttributes(GWEN_WIDGET *w,
                                      GWEN_TW_LINE *l,
                                      GWEN_TYPE_UINT32 atts) {
  l->currentAtts=atts;

  return 0;
}



int GWEN_TextWidget_LineSetPos(GWEN_WIDGET *w,
                               GWEN_TW_LINE *l,
                               int pos) {
  l->currentPos=pos;
  return 0;
}



int GWEN_TextWidget_LineClear(GWEN_WIDGET *w,
                              GWEN_TW_LINE *l) {
  GWEN_TEXTWIDGET *win;
  unsigned int i;
  char *pAtts;
  char *pChars;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (GWEN_TextWidget_SelectLine(l)) {
    DBG_NOTICE(0, "Error selecting line");
    return -1;
  }

  pChars=GWEN_Buffer_GetStart(l->chars);
  pAtts=GWEN_Buffer_GetStart(l->attributes);
  pChars+=l->leftBorder;
  pAtts+=l->leftBorder;
  i=l->rightBorder-l->leftBorder+1;

  while(i--) {
    *pChars++=' ';
    *pAtts++=0;
  }

  l->changed=1;
  l->compressed=0;
  return 0;
}



int GWEN_TextWidget__DrawLine(GWEN_WIDGET *w, GWEN_TW_LINE *l,
                              int x, int y, int len) {
  GWEN_TEXTWIDGET *win;
  const unsigned char *p;
  const unsigned char *startPtr;
  int i, j, pos;
  int lastWasEsc, lastEsc = GWEN_WIDGET_ATT_ESC_CHAR;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  DBG_NOTICE(0, "DrawLine: x=%d, y=%d, len=%d", x, y, len);

  if (y<win->top) {
    DBG_NOTICE(0, "Line not visible (above)");
    return 0;
  }

  if (y>=(win->top+GWEN_Widget_GetHeight(w))) {
    DBG_NOTICE(0, "Line not visible (below)");
    return 0;
  }

  if (x<win->left) {
    len-=(win->left-x);
    x=win->left;
  }

  if (x-win->left+len>GWEN_Widget_GetWidth(w)) {
    DBG_NOTICE(0, "Shortening len from %d to %d",
               len, GWEN_Widget_GetWidth(w)-(x-win->left));
    len=GWEN_Widget_GetWidth(w)-(x-win->left);
  }

  if (len<1) {
    DBG_NOTICE(0, "Not updating dims: %d/%d, %d",
               x, y, len);
    return 0;
  }

  if (!l->compressed) {
    DBG_NOTICE(0, "Compressing text");
    if (GWEN_TextWidget_CompressLine(l)) {
      DBG_ERROR(0, "Error compressing line");
      abort();
    }
  }

  p=GWEN_Buffer_GetStart(l->compressedText);
  j=GWEN_Buffer_GetUsedBytes(l->compressedText);
  DBG_NOTICE(0, "Line length: %d bytes", j);
  GWEN_Text_LogString(p, j, 0, GWEN_LoggerLevelNotice);

  i=len;
  startPtr=0;
  pos=0;
  lastWasEsc=0;
  while(i && j) {
    if (pos==x && !startPtr) {
      startPtr=p;
    }

    if (lastWasEsc) {
      lastWasEsc=0;
      if (lastEsc==GWEN_WIDGET_ATT_ESC_CHAR) {
      }
      else if (lastEsc==GWEN_WIDGET_CHAR_ESC_CHAR) {
        pos++;
        if (startPtr)
          i--;
      }
    }
    else {
      if (*p==GWEN_WIDGET_ATT_ESC_CHAR) {
        lastWasEsc=1;
        lastEsc=GWEN_WIDGET_ATT_ESC_CHAR;
      }
      else if (*p==GWEN_WIDGET_CHAR_ESC_CHAR) {
        lastWasEsc=1;
        lastEsc=GWEN_WIDGET_CHAR_ESC_CHAR;
      }
      else {
        lastWasEsc=0;
        pos++;
        if (startPtr)
          i--;
      }
    }
    j--;
    p++;
  }

  assert(lastWasEsc==0);

  if (startPtr) {
    DBG_NOTICE(0, "ZZZ: Updating dims: %d/%d, %d(%d) [%d, %d]",
               x, y, len, p-startPtr, win->left, win->top);
    GWEN_Text_LogString(startPtr, p-startPtr, 0, GWEN_LoggerLevelNotice);
    GWEN_Widget_WriteAt(w,
                        x-win->left, y-win->top,
                        startPtr, p-startPtr);
    if (i) {
      DBG_NOTICE(0, "ZZZ: Clearing rest of line [%d, %d]",
                 p-startPtr,
                 i);
      GWEN_Widget_Clear(w, pos+1, y-win->top,
                        GWEN_EventClearMode_ToEOL);
    }
  }
  else {
    DBG_NOTICE(0, "ZZZ: Clearing line %d", y);
    GWEN_Widget_WriteAt(w,
                        win->left, y-win->top,
                        " ", 1);
    GWEN_Widget_Clear(w, win->left, y-win->top,
                      GWEN_EventClearMode_ToEOL);
  }

  GWEN_Widget_Refresh(w);
  return 0;
}



int GWEN_TextWidget__DrawArea(GWEN_WIDGET *w,
                              int x, int len,
                              int y, int height) {
  int i;
  GWEN_TW_LINE *l;
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (y<win->top) {
    height-=win->top-y;
    y=win->top;
  }

  if (height<1)
    return 0;

  l=GWEN_TWLine_List_First(win->lines);
  i=y;
  while(i && l) {
    l=GWEN_TWLine_List_Next(l);
    i--;
  }

  while(l && (y-win->top)<height) {
    DBG_NOTICE(0, "Drawing line %d (%d - %d)",
               y, x, x+len-1);
    if (GWEN_TextWidget__DrawLine(w, l, x, y, len)) {
      DBG_ERROR(0, "Error wiriting line %d (%d - %d)",
                y, x, x+len-1);
    }
    y++;
    l=GWEN_TWLine_List_Next(l);
  }

  if ((y-win->top)<height) {
    DBG_NOTICE(0, "Clearing rest of screen");
    GWEN_Widget_Clear(w, win->left, y-win->top,
                      GWEN_EventClearMode_ToBottom);
  }

  return 0;
}



int GWEN_TextWidget_LineRedraw(GWEN_WIDGET *w, GWEN_TW_LINE *l) {
  GWEN_TEXTWIDGET *win;
  int x = 0, y, len;
  int left, right;
  GWEN_TW_LINE *tmpl;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (!l->changed) {
    DBG_NOTICE(0, "Line unchanged");
    return 0;
  }

  tmpl=GWEN_TWLine_List_First(win->lines);
  y=0;
  while(tmpl && tmpl!=l) {
    tmpl=GWEN_TWLine_List_Next(tmpl);
    y++;
  }
  if (!tmpl) {
    DBG_INFO(0, "Line not found in stack");
    return -1;
  }

  left=l->leftBorder;
  if (left<win->left)
    left=win->left;

  right=l->rightBorder;
  if ((right-win->left)>=GWEN_Widget_GetWidth(w))
    right=win->left+GWEN_Widget_GetWidth(w)-1;

  len=right-left+1;
  if (len<1) {
    DBG_NOTICE(0, "Not updating dims: %d/%d, %d (%d, %d) [%d, %d]",
               x, y, len, left, right,
               l->leftBorder, l->rightBorder);
    return 0;
  }
  y -= win->top;
  x = left-win->left;

  DBG_NOTICE(0, "Update dims: %d/%d, %d (%d, %d)",
             x, y, len, left, right);

  if (GWEN_TextWidget__DrawLine(w, l, x+win->left, y+win->top, len)) {
    DBG_ERROR(0, "Error drawing line %d/%d (%d bytes)", x, y, len);
    return -1;
  }

  return 0;
}



int GWEN_TextWidget_LineWriteText_OV(GWEN_WIDGET *w,
                                     GWEN_TW_LINE *l,
                                     const char *text,
                                     int len) {
  GWEN_TEXTWIDGET *win;
  unsigned int i;
  unsigned char *pAtts;
  unsigned char *pChars;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  assert(text);

  pChars=GWEN_Buffer_GetStart(l->chars);
  pAtts=GWEN_Buffer_GetStart(l->attributes);
  pChars+=l->currentPos;
  pAtts+=l->currentPos;
  i=l->rightBorder-l->leftBorder+1;
  if (i>len)
    i=len;

  l->currentPos+=i;
  while(i--) {
    if ((unsigned char)(*text)==GWEN_WIDGET_CHAR_ESC_CHAR) {
      if (!i)
        break;
      *pAtts++=(l->currentAtts | GWEN_WIDGET_ATT_CHAR);
      *pChars++=*(++text);
      text++;
      i--;
    }
    else if ((unsigned char)(*text)==GWEN_WIDGET_ATT_ESC_CHAR) {
      if (!i)
        break;
      l->currentAtts=*(++text);
      text++;
      i--;
    }
    else {
      *pChars++=*text++;
      *pAtts++=l->currentAtts;
    }
  }

  l->changed=1;
  l->compressed=0;
  return 0;
}



int GWEN_TextWidget_LineWriteText_INS(GWEN_WIDGET *w,
                                      GWEN_TW_LINE *l,
                                      const char *text,
                                      int len) {
  GWEN_TEXTWIDGET *win;
  unsigned int i;
  char *pAtts;
  char *pChars;
  char *p;
  int blnks;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  assert(text);

  i=l->rightBorder-l->leftBorder+1;
  if (i>len)
    i=len;

  pChars=GWEN_Buffer_GetStart(l->chars);
  pAtts=GWEN_Buffer_GetStart(l->attributes);

  /* check whether there is enough space at the end... */
  blnks=0;
  p=pChars+l->rightBorder;
  while(p>=(pChars+l->currentPos) && *p && isspace(*p)) {
    blnks++;
    p--;
  }
  DBG_NOTICE(0, "ZZZ: Blanks: %d", blnks);
  if (blnks<len) {
    DBG_INFO(0, "Line full");
    return -1;
  }

  pChars+=l->currentPos;
  pAtts+=l->currentPos;

  if (blnks<l->rightBorder-l->currentPos+1) {
    /* move data behind out of the way */
    DBG_NOTICE(0, "ZZZ: Moving %d bytes from %d",
               l->rightBorder-l->currentPos+1-blnks,
               l->currentPos);
    p=pChars+i;
    memmove(p, pChars, l->rightBorder-l->currentPos+1-blnks);
    p=pAtts+i;
    memmove(p, pAtts, l->rightBorder-l->currentPos+1-blnks);
  }

  /* actually write into the buffer */
  while(i--) {
    *pChars++=*text++;
    *pAtts++=l->currentAtts;
  }
  l->currentPos+=i;

  l->changed=1;
  l->compressed=0;
  return 0;
}



int GWEN_TextWidget_LineWriteText(GWEN_WIDGET *w,
                                  GWEN_TW_LINE *l,
                                  const char *text,
                                  int len) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  assert(text);
  assert(len>=0);

  if (l->rightBorder && l->currentPos>l->rightBorder) {
    DBG_INFO(0, "Position outside border (right)");
    return -1;
  }
  if (l->leftBorder && l->currentPos<l->leftBorder) {
    DBG_INFO(0, "Position outside border (left)");
    return -1;
  }

  if (GWEN_TextWidget_SelectLine(l)) {
    DBG_NOTICE(0, "Error selecting line");
    return -1;
  }

  if (len==0)
    len=strlen(text);
  if (!len)
    return 0;

  if (l->insertOn) {
    DBG_NOTICE(0, "ZZZ: InsertMode");
    return GWEN_TextWidget_LineWriteText_INS(w, l, text, len);
  }
  else {
    DBG_NOTICE(0, "ZZZ: OverwriteMode");
    return GWEN_TextWidget_LineWriteText_OV(w, l, text, len);
  }
}



int GWEN_TextWidget_LineDelete(GWEN_WIDGET *w, GWEN_TW_LINE *l, int n){
  GWEN_TEXTWIDGET *win;
  int len;
  char *pAtts;
  char *pChars;
  int i;

  assert(l);
  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  len=((l->rightBorder-l->currentPos)+1)-n;

  pChars=GWEN_Buffer_GetStart(l->chars);
  pAtts=GWEN_Buffer_GetStart(l->attributes);

  if (len!=n && len>0) {
    DBG_NOTICE(0, "ZZZ: Moving %d bytes from %d to %d",
               len, l->currentPos+n, l->currentPos);
    /* need to copy */
    memmove(pChars+l->currentPos, pChars+l->currentPos+n,
            len);
    memmove(pAtts+l->currentPos, pAtts+l->currentPos+n,
            len);
  }

  pChars+=l->currentPos+len;
  pAtts+=l->currentPos+len;
  for (i=0; i<n; i++) {
    *(pChars++)=' ';
    *(pAtts++)=0;
  }

  l->changed=1;
  l->compressed=0;
  return 0;
}



int GWEN_TextWidget_EnsureVisible(GWEN_WIDGET *w,
                                  int x, int y,
                                  int width, int height) {
  GWEN_TEXTWIDGET *win;
  int xoffs;
  int yoffs;
  int ww, wh;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  DBG_NOTICE(0, "Asking to ensure this: %d/%d, %d/%d",
             x, y, width, height);
  ww=GWEN_Widget_GetWidth(w);
  wh=GWEN_Widget_GetHeight(w);

  if (width>ww)
    return -1;
  if (height>wh)
    return -1;

  xoffs=0;
  yoffs=0;

  if (x<win->left)
    xoffs=-(win->left-x);               /* scroll to the left */
  else {
    /* x is already visible, is it enough ? */
    if ((x+width)>(win->left+ww))
      xoffs=((x+width)-(win->left+ww));
  }

  if (x+xoffs>=(win->left+ww)) {
    DBG_NOTICE(0, "New X would be outside right border (%d, %d, %d)",
               x, xoffs, win->left);
    xoffs=(x+xoffs)-(win->left+ww);
    //xoffs=(x-(win->left+ww)+width);    /* scroll to the right */
  }

  if (y<win->top)
    yoffs=-(win->top-y);
  else {
    /* x is already visible, is it enough ? */
    if ((y+height)>(win->top+wh))
      yoffs=((y+height)-(win->top+wh));
  }

  if (y+yoffs>=(win->top+wh))
    yoffs=(y-(win->top+ww)+height);


  if (y<win->top)
    yoffs=-(win->top-y);
  else if (y>=(win->top+wh))              /* scroll up */
    yoffs=(y-(win->top+wh)+height);     /* scroll down */

  DBG_NOTICE(0, "Scrolling by: %d, %d", xoffs, yoffs);

  if ((win->left+xoffs+ww)>win->vwidth) {
    DBG_NOTICE(0, "X Does not fit into virtual size (vwidth=%d)",
               win->vwidth);
    return -1;
  }

  if ((win->top+yoffs+wh)>win->vheight) {
    DBG_NOTICE(0, "Y Does not fit");
    return -1;
  }

  if (xoffs || yoffs) {
    GWEN_Widget_Scroll(w, xoffs, yoffs);
  }

  return 0;
}















GWEN_TW_LINE *GWEN_TextWidget__NewLine(GWEN_TYPE_UINT32 startAtts,
                                       int indent,
                                       int rightBorder) {
  GWEN_TW_LINE *l;
  int z;

  l=GWEN_TWLine_new(startAtts, 0, 0);
  if (GWEN_TextWidget_SelectLine(l)) {
    DBG_NOTICE(0, "Error selecting line");
    GWEN_TWLine_free(l);
    return 0;
  }

  /* indent */
  for (z=0; z<indent; z++) {
    GWEN_Buffer_AppendByte(l->chars, ' ');
    GWEN_Buffer_AppendByte(l->attributes, 0);
  }
  l->rightBorder=rightBorder;

  return l;
}



GWEN_TW_LINE_LIST *GWEN_TextWidget_TextToLines(const char *s,
                                               int leftBorder,
                                               int rightBorder) {
  GWEN_XMLNODE *xmlNode;
  GWEN_BUFFEREDIO *bio;
  GWEN_BUFFER *buf;
  GWEN_TW_LINE_LIST *ll;

  buf=GWEN_Buffer_new(0, strlen(s), 0, 1);
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_Rewind(buf);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 1);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);
  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "gwen");
  if (GWEN_XML_Parse(xmlNode, bio, GWEN_XML_FLAGS_DEFAULT)) {
    DBG_NOTICE(0, "here");
    GWEN_BufferedIO_free(bio);
    GWEN_XMLNode_free(xmlNode);
    return 0;
  }
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  ll=GWEN_TextWidget_XMLToLines(xmlNode, leftBorder, rightBorder);
  GWEN_XMLNode_free(xmlNode);
  return ll;
}




GWEN_TW_LINE_LIST *GWEN_TextWidget_XMLToLines(GWEN_XMLNODE *n,
                                              int leftBorder,
                                              int rightBorder) {
  GWEN_TW_LINE_LIST *ll;
  GWEN_TW_LINE *l;
  int rv;

  ll=GWEN_TWLine_List_new();
  l=GWEN_TextWidget__NewLine(0, leftBorder, rightBorder);
  if (!l) {
    GWEN_TWLine_List_free(ll);
    return 0;
  }
  rv=GWEN_TextWidget__ParseXMLTag(n, 0, leftBorder, ll, &l);
  if (rv==-1) {
    DBG_NOTICE(0, "here");
    GWEN_TWLine_List_free(ll);
    return 0;
  }
  if (GWEN_Buffer_GetUsedBytes(l->chars))
    GWEN_TWLine_List_Add(l, ll);
  else
    GWEN_TWLine_free(l);

  return ll;
}



int GWEN_TextWidget__ParseXMLTag(GWEN_XMLNODE *n,
                                 GWEN_TYPE_UINT32 currentAtts,
                                 int indent,
                                 GWEN_TW_LINE_LIST *ll,
                                 GWEN_TW_LINE **l) {
  const char *p;
  GWEN_TYPE_UINT32 newAtts;
  int newIndent;
  int rv;
  GWEN_XMLNODE *nn;

  p=GWEN_XMLNode_GetData(n);
  if (!p) {
    DBG_ERROR(0, "Tag with no name ?");
    return -1;
  }

  DBG_NOTICE(0, "Parsing tag \"%s\"", p);

  newAtts=currentAtts;
  newIndent=indent;
  if (strcasecmp(p, "b")==0 ||
      strcasecmp(p, "strong")==0)
    newAtts|=GWEN_WIDGET_ATT_STANDOUT;
  else if (strcasecmp(p, "i")==0)
    newAtts|=GWEN_WIDGET_ATT_UNDERLINE;
  else if (strcasecmp(p, "p")==0)
    newIndent+=2;

  nn=GWEN_XMLNode_GetChild(n);
  if (nn) {
    rv=GWEN_TextWidget__ParseXMLSubNodes(nn,
                                         newAtts,
                                         newIndent,
                                         ll,
                                         l);
    DBG_NOTICE(0, "rv was: %d", rv);
  }
  else
    rv=1;

  if (strcasecmp(p, "br")==0 ||
      strcasecmp(p, "p")==0 ||
      (strcasecmp(p, "gwen")==0) ||
      strcasecmp(p, "ul")==0 ||
      strcasecmp(p, "li")==0 ||
      strcasecmp(p, "ol")==0
     ) {
    DBG_NOTICE(0, "New line");
    GWEN_TWLine_List_Add(*l, ll);
    *l=GWEN_TextWidget__NewLine(currentAtts, indent, (*l)->rightBorder);
    if (!*l) {
      DBG_INFO(0, "here");
      return -1;
    }
  }

  return rv;
}



int GWEN_TextWidget__ParseXMLSubNodes(GWEN_XMLNODE *n,
                                      GWEN_TYPE_UINT32 currentAtts,
                                      int indent,
                                      GWEN_TW_LINE_LIST *ll,
                                      GWEN_TW_LINE **l){
  const char *p;
  int rv;

  while(n) {
    if (GWEN_Buffer_GetUsedBytes((*l)->chars)>indent) {
      GWEN_Buffer_AppendByte((*l)->chars, ' ');
      GWEN_Buffer_AppendByte((*l)->attributes, 0);
    }

    switch(GWEN_XMLNode_GetType(n)) {
    case GWEN_XMLNodeTypeData:
      p=GWEN_XMLNode_GetData(n);
      while(*p) {
        GWEN_Buffer_AppendByte((*l)->attributes, currentAtts);
        GWEN_Buffer_AppendByte((*l)->chars, *p);

        if ((*l)->rightBorder &&
            (GWEN_Buffer_GetUsedBytes((*l)->chars)>=(*l)->rightBorder)) {
          DBG_NOTICE(0, "New line");
          GWEN_TWLine_List_Add((*l), ll);
          *l=GWEN_TextWidget__NewLine(currentAtts, indent,
                                      (*l)->rightBorder);
          if (!(*l)) {
            DBG_INFO(0, "here");
            return -1;
          }
        }
        p++;
      }
      break;

    case GWEN_XMLNodeTypeTag:
      rv=GWEN_TextWidget__ParseXMLTag(n, currentAtts, indent, ll, l);
      if (rv==-1) {
        DBG_INFO(0, "here");
        return rv;
      }
      else if (rv==1) {
      }
      break;

    default:
      break;
    }

    n=GWEN_XMLNode_Next(n);
  } /* while */

  return 0;
}



int GWEN_TextWidget_GetMaxWidth(GWEN_TW_LINE_LIST *ll) {
  GWEN_TW_LINE *l;
  int i;

  i=0;
  l=GWEN_TWLine_List_First(ll);
  while(l) {
    if (GWEN_TextWidget_SelectLine(l)) {
      return -1;
    }
    DBG_NOTICE(0, "ZZZ: Width: %d bytes",
               GWEN_Buffer_GetUsedBytes(l->chars));
    if (i<GWEN_Buffer_GetUsedBytes(l->chars))
      i=GWEN_Buffer_GetUsedBytes(l->chars);
    l=GWEN_TWLine_List_Next(l);
  }

  DBG_NOTICE(0, "ZZZ: Highest width: %d bytes",
            i);
  return i;
}



int GWEN_TextWidget_GetLeft(const GWEN_WIDGET *w){
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);
  return win->left;
}


int GWEN_TextWidget_GetTop(const GWEN_WIDGET *w){
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);
  return win->top;
}



int GWEN_TextWidget_GetVirtualWidth(const GWEN_WIDGET *w){
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);
  return win->vwidth;
}



int GWEN_TextWidget_GetVirtualHeight(const GWEN_WIDGET *w){
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);
  return win->vheight;
}


















