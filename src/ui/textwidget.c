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


GWEN_TW_LINE *GWEN_TWLine_new(GWEN_TYPE_UINT32 startAttributes,
                              const char *s) {
  GWEN_TW_LINE *l;
  int rv;

  GWEN_NEW_OBJECT(GWEN_TW_LINE, l);
  GWEN_LIST_INIT(GWEN_TW_LINE, l);

  rv=GWEN_TWLine_SetText(l, startAttributes, s);
  assert(rv==0);

  l->usage=1;
  return l;
}



int GWEN_TWLine_SetText(GWEN_TW_LINE *l,
                        GWEN_TYPE_UINT32 startAttributes,
                        const char *s) {
  GWEN_TYPE_UINT32 endAttributes;

  assert(l);

  l->length=0;
  free(l->text);
  l->text=0;
  if (s) {
    const char *p;
    unsigned int i;
    int error;
    int lastWasEsc;
    int lastEscape;
    unsigned int length;

    p=s;
    i=strlen(p);
    lastWasEsc=0;
    length=0;
    error=0;
    while(*p) {
      int c;

      if (*p=='%') {
        int j;

        if (i<3) {
          DBG_ERROR(0, "Bad string");
          error++;
          break;
        }
        i-=3;
        p++;
        j=toupper(*p)-'0';
        if (j>9)
          j-=7;
        if (j<0 || j>15) {
          DBG_ERROR(0, "Bad string");
          error++;
          break;
        }
        c=j<<4;
        p++;
        j=toupper(*p)-'0';
        if (j>9)
          j-=7;
        if (j<0 || j>15) {
          DBG_ERROR(0, "Bad string");
          error++;
          break;
        }
        c+=j;
      }
      else {
        c=*p;
        i--;
      }

      if (lastWasEsc) {
        if (lastEscape==GWEN_WIDGET_ATT_ESC_CHAR)
          endAttributes=c;
        else if (lastEscape==GWEN_WIDGET_CHAR_ESC_CHAR)
          length++;
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
          length++;
        }
      }
      p++;
    } /* while */
    if (lastWasEsc) {
      DBG_ERROR(0, "Bad string");
      error++;
    }

    assert(error==0);
    l->length=length;
    l->text=strdup(s);
    l->startAttributes=startAttributes;
    l->endAttributes=endAttributes;
  } /* if s */

  return 0;
}



void GWEN_TWLine_free(GWEN_TW_LINE *l){
  if (l) {
    assert(l->usage);
    if ((--l->usage)==0) {
      GWEN_LIST_FINI(GWEN_TW_LINE, l);
      GWEN_Buffer_free(l->attributes);
      GWEN_Buffer_free(l->chars);
      free(l->text);
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
                    name, text, x, y, width, height);

  GWEN_NEW_OBJECT(GWEN_TEXTWIDGET, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w, win,
                       GWEN_TextWidget_freeData);
  win->lines=GWEN_TWLine_List_new();
  win->vwidth=width;
  win->vheight=height;

  win->previousHandler=GWEN_Widget_GetEventHandler(w);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(w, GWEN_TextWidget_EventHandler);
  return w;
}



void GWEN_TextWidget_freeData(void *bp, void *p) {
  GWEN_TEXTWIDGET *win;

  win=(GWEN_TEXTWIDGET*)p;
  GWEN_TWLine_List_free(win->lines);
  GWEN_FREE_OBJECT(win);
}



int GWEN_TextWidget_WriteLine(GWEN_WIDGET *w, int x, int y) {
  GWEN_TEXTWIDGET *win;
  GWEN_TW_LINE *l;
  int i;
  const char *p;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (y<win->top) {
    DBG_NOTICE(0, "Line %d is invisible", y);
    return 0;
  }

  i=y;
  l=GWEN_TWLine_List_First(win->lines);
  while(l && i--) l=GWEN_TWLine_List_Next(l);
  if (!l) {
    DBG_NOTICE(0, "Line %d not found ", y);
    GWEN_Widget_Clear(w, 0, y-win->top, GWEN_EventClearMode_ToBottom);
    return 1;
  }

  DBG_NOTICE(0, "Found line %d", y);
  if (l->text[0]==0) {
    GWEN_Widget_Clear(w, 0, y-win->top, GWEN_EventClearMode_ToEOL);
    return 0;
  }

  p=l->text;
  if (p) {
    GWEN_TYPE_UINT32 startAttributes;
    unsigned int i;
    int error;
    int lastWasEsc;
    int lastEscape;
    startAttributes=l->startAttributes;
    const char *startPos;
    int length;
    int pos;

    startPos=0;
    pos=0;
    i=strlen(p);
    lastWasEsc=0;
    length=0;
    while(*p) {
      int c;

      if (pos==win->left && !startPos)
        startPos=p;

      if (*p=='%') {
        int j;

        if (i<3) {
          DBG_ERROR(0, "Bad string");
          error++;
          break;
        }
        i-=3;
        p++;
        j=toupper(*p)-'0';
        if (j>9)
          j-=7;
        if (j<0 || j>15) {
          DBG_ERROR(0, "Bad string");
          error++;
          break;
        }
        c=j<<4;
        p++;
        j=toupper(*p)-'0';
        if (j>9)
          j-=7;
        if (j<0 || j>15) {
          DBG_ERROR(0, "Bad string");
          error++;
          break;
        }
        c+=j;
        p++;
      }
      else {
        c=*p;
        i--;
        p++;
      }

      if (lastWasEsc) {
        if (lastEscape==GWEN_WIDGET_ATT_ESC_CHAR) {
          if (pos<win->left)
            startAttributes=c;
        }
        else if (lastEscape==GWEN_WIDGET_CHAR_ESC_CHAR) {
          pos++;
          if (pos>=win->left) {
            length++;
            if (length==GWEN_Widget_GetWidth(w))
              break;
          }
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
          pos++;
          if (pos>=win->left) {
            length++;
            if (length==GWEN_Widget_GetWidth(w))
              break;
          }
        }

      }
    } /* while */
    if (lastWasEsc) {
      DBG_ERROR(0, "Bad string");
      error++;
    }
    if (!length) {
      GWEN_Widget_Clear(w, 0, y-win->top, GWEN_EventClearMode_ToEOL);
      return 0;
    }
    GWEN_Widget_ChangeAtts(w, startAttributes, 1);
    if (!*p) {
      DBG_NOTICE(0, "String ends.");
    }
    DBG_NOTICE(0, "Writing this (i=%d, left=%d, length=%d):",
               i, win->left, length);
    DBG_NOTICE(0, "Positions: x=%d, y=%d (top=%d)",
               0, y-win->top, win->top);
    GWEN_Text_LogString(startPos, p-startPos, 0, GWEN_LoggerLevelNotice);
    GWEN_Widget_WriteAt(w, 0, y-win->top, startPos, p-startPos);
    GWEN_Widget_Clear(w, length, y-win->top, GWEN_EventClearMode_ToEOL);
  } /* if p */
  else {
    DBG_NOTICE(0, "No text");
  }
  return 0;
}



int GWEN_TextWidget_WriteArea(GWEN_WIDGET *w,
                              int x, int y,
                              int width, int height) {
  GWEN_TEXTWIDGET *win;
  int i;
  int rv;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  /* validate */
  if (y<win->top) {
    i=win->top-y;
    if (height<=i)
      return 0;
    height-=i;
    y=win->top;
  }

  if (x<win->left) {
    i=win->left-x;
    if (width<=i)
      return 0;
    width-=i;
    x=win->left;
  }

  if (width>GWEN_Widget_GetWidth(w))
    width=GWEN_Widget_GetWidth(w);
  if (height>GWEN_Widget_GetHeight(w))
    height=GWEN_Widget_GetHeight(w);

  for (i=0; i<height; i++) {
    DBG_VERBOUS(0, "Writing line %d (->%d)", i, y+i);
    rv=GWEN_TextWidget_WriteLine(w, x, y+i);
    if (rv==-1) {
      DBG_INFO(0, "Error writing line %d (->%d)", i, y+i);
      break;
    }
    else if (rv==1)
      break;
  } /* for */

  return 0;
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



int GWEN_TextWidget_SetText(GWEN_WIDGET *w,
                            const char *text,
                            GWEN_EVENT_SETTEXT_MODE m) {
  GWEN_TEXTWIDGET *win;
  GWEN_XMLNODE *xmlNode;
  GWEN_XMLNODE *n;
  GWEN_BUFFEREDIO *bio;
  GWEN_BUFFER *buf;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (m==GWEN_EventSetTextMode_Replace && !text) {
    GWEN_TWLine_List_Clear(win->lines);
    return 0;
  }

  buf=GWEN_Buffer_new(0, strlen(text), 0, 1);
  GWEN_Buffer_AppendString(buf, text);
  GWEN_Buffer_Rewind(buf);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 1);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);
  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  if (GWEN_XML_Parse(xmlNode, bio, GWEN_XML_FLAGS_DEFAULT)) {
    DBG_INFO(0, "here");
    GWEN_BufferedIO_free(bio);
    GWEN_XMLNode_free(xmlNode);
    return -1;
  }
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  if (m==GWEN_EventSetTextMode_Replace)
    GWEN_TWLine_List_Clear(win->lines);

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  if (n) {
    GWEN_BUFFER *lineBuf;
    int pos;
    GWEN_TW_LINE_LIST *ll;
    int rv;

    lineBuf=GWEN_Buffer_new(0, 256, 0, 1);
    pos=0;
    if (m!=GWEN_EventSetTextMode_Insert)
      ll=win->lines;
    else
      ll=GWEN_TWLine_List_new();
    rv=GWEN_TextWidget_ParseXMLTag(w,
                                   n,
                                   lineBuf,
                                   0,
                                   0,
                                   0,
                                   &pos,
                                   ll);
    if (rv) {
      if (m==GWEN_EventSetTextMode_Insert)
        GWEN_TWLine_List_free(ll);
      GWEN_Buffer_free(lineBuf);
      GWEN_XMLNode_free(xmlNode);
      return -1;
    }
  }

  return 0;
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
    l=GWEN_TWLine_new(startAtts, GWEN_Buffer_GetStart(newbuf));
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
    if (chg)
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
          if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_BREAKLINES) {
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
  
              l=GWEN_TWLine_new(startAtts, GWEN_Buffer_GetStart(newbuf));
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
              if (chg)
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
          if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_BREAKLINES) {
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

              l=GWEN_TWLine_new(startAtts, GWEN_Buffer_GetStart(newbuf));
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
              if (chg)
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



void GWEN_TextWidget_Draw(GWEN_WIDGET *w) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  GWEN_TextWidget_WriteArea(w,
                            win->left,
                            win->top,
                            GWEN_Widget_GetWidth(w),
                            GWEN_Widget_GetHeight(w));
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
    GWEN_TextWidget_Draw(w);
    if ((GWEN_Widget_GetFlags(w) & GWEN_WIDGET_FLAGS_HASFOCUS) &&
        (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)) {
      GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                            GWEN_Widget_GetWidth(w),
                            GWEN_WidgetColour_Selected);
    }
    GWEN_Widget_SetCursorX(w, 0);
    GWEN_Widget_SetCursorY(w, win->pos-win->top);
    win->previousHandler(w, e);
    GWEN_Widget_Refresh(w);
    return GWEN_UIResult_Handled;
  }

  case GWEN_EventType_Highlight: {
    int x, y;
    GWEN_WIDGET_COLOUR hi;

    DBG_NOTICE(0, "Event: Highlight(%s)", GWEN_Widget_GetName(w));
    if (!(GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT))
      return win->previousHandler(w, e);

    x=GWEN_EventHighlight_GetX(e);
    y=GWEN_EventHighlight_GetY(e);
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
      GWEN_TextWidget_WriteLine(w, x, y);
      GWEN_Widget_Refresh(w);
      return GWEN_UIResult_Handled;
    }
    else {
      return GWEN_UIResult_Handled;
    }
  }

  case GWEN_EventType_Scroll: {
    DBG_NOTICE(0, "Event: Scroll(%s)", GWEN_Widget_GetName(w));
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

          if (win->top+byY>=GWEN_TWLine_List_GetCount(win->lines)) {
            byY=GWEN_TWLine_List_GetCount(win->lines)-win->top-1;
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
      if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_LINEMODE) {
        if (win->pos+1>=
            GWEN_TWLine_List_GetCount(win->lines)) {
          DBG_INFO(0, "Already at bottom of the list");
          beep();
        }
        else {
          /* new position is valid, select it */
          if ((win->pos+1-win->top)<win->vheight) {
            /* remove highlight */
            if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
              GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                                    GWEN_Widget_GetWidth(w),
                                    0);
            win->pos++;
            if ((win->pos-win->top)>=GWEN_Widget_GetHeight(w)) {
              /* scroll up */
              GWEN_Widget_Scroll(w, 0, 1);
            }
            else {
              if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
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
      if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_LINEMODE) {
        /* in line mode */
        if (win->pos) {
          if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
            GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                                  GWEN_Widget_GetWidth(w),
                                  0);
          win->pos--;
          if (win->pos<win->top) {
            GWEN_Widget_Scroll(w, 0, -1);
          }
          else
            if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
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

      f=fopen("gwen-lines.dump", "w+");
      if (!f) {
        DBG_ERROR(0, "fopen: %s", strerror(errno));
        return GWEN_UIResult_Handled;
      }

      l=GWEN_TWLine_List_First(win->lines);
      while(l) {
        if (l->text)
          fprintf(f, "%s\n", l->text);
        else {
          DBG_NOTICE(0, "Empty line");
          fprintf(f, "---empty---\n");
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
          if (1!=fwrite(GWEN_Buffer_GetStart(l->chars),
                        GWEN_Buffer_GetUsedBytes(l->chars),
                        1, f)) {
            DBG_ERROR(0, "fwrite: %s", strerror(errno));
            GWEN_TextWidget_LineClose(w, l, 0);
            break;
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
      if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
        GWEN_Widget_Highlight(w, 0, win->pos-win->top,
                              GWEN_Widget_GetWidth(w),
                              GWEN_WidgetColour_Selected);
    }
    else {
      if (GWEN_Widget_GetFlags(w) & GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT)
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




GWEN_TW_LINE *GWEN_TextWidget_LineOpen(GWEN_WIDGET *w, int n, int crea) {
  GWEN_TW_LINE *l;
  GWEN_TW_LINE *lastl;
  GWEN_TEXTWIDGET *win;
  int j;
  GWEN_TYPE_UINT32 currAtts;
  unsigned int i;
  int error;
  int lastWasEsc;
  int lastEscape;
  const char *p;

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
      if (lastl)
        atts=lastl->endAttributes;

      /* create it */
      for (k=0; k<j; k++) {
        l=GWEN_TWLine_new(atts, "");
        GWEN_TWLine_List_Add(l, win->lines);
      }
      if (win->dheight<GWEN_TWLine_List_GetCount(win->lines)) {
        win->dheight=GWEN_TWLine_List_GetCount(win->lines);
        GWEN_Widget_ContentChange(w, win->dwidth, win->dheight);
      }
    }
    else {
      DBG_INFO(0, "Line %d not found", n);
      return 0;
    }
  }

  if (l->openCount) {
    l->openCount++;
    GWEN_TWLine_Attach(l);
    return l;
  }

  p=l->text;
  if (!p) {
    DBG_INFO(0, "No text");
    return 0;
  }

  l->attributes=GWEN_Buffer_new(0, l->length, 0, 1);
  l->chars=GWEN_Buffer_new(0, l->length, 0, 1);

  currAtts=l->startAttributes;

  assert(l);

  i=strlen(p);
  lastWasEsc=0;
  error=0;
  while(*p) {
    int c;

    if (*p=='%') {
      int j;

      if (i<3) {
        DBG_ERROR(0, "Bad string");
        error++;
        break;
      }
      i-=3;
      p++;
      j=toupper(*p)-'0';
      if (j>9)
        j-=7;
      if (j<0 || j>15) {
        DBG_ERROR(0, "Bad string");
        error++;
        break;
      }
      c=j<<4;
      p++;
      j=toupper(*p)-'0';
      if (j>9)
        j-=7;
      if (j<0 || j>15) {
        DBG_ERROR(0, "Bad string");
        error++;
        break;
      }
      c+=j;
    }
    else {
      c=*p;
      i--;
    }

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
    error++;
  }

  assert(error==0);
  l->openCount=1;
  l->changed=0;

  GWEN_TWLine_Attach(l);
  return l;
}



int GWEN_TextWidget_CondenseLineArea(GWEN_TW_LINE *l,
                                     int leftBorder,
                                     int rightBorder,
                                     GWEN_BUFFER *tbuf,
                                     GWEN_TYPE_UINT32 *atts) {
  unsigned char currAtts;
  unsigned int i;
  const char *pAtts;
  const char *pChars;
  unsigned int len;

  len=rightBorder-leftBorder+1;
  if (len>GWEN_Buffer_GetUsedBytes(l->chars))
    len=GWEN_Buffer_GetUsedBytes(l->chars)-leftBorder;

  pChars=GWEN_Buffer_GetStart(l->chars)+leftBorder;
  pAtts=GWEN_Buffer_GetStart(l->attributes)+leftBorder;
  currAtts=0xff;
  for (i=0; i<len; i++) {
    if (currAtts!=(*pAtts & ~GWEN_WIDGET_ATT_CHAR)) {
      char numbuf[4];

      GWEN_Buffer_AppendString(tbuf, "%ff%");
      snprintf(numbuf, sizeof(numbuf), "%02x", *pAtts);
      GWEN_Buffer_AppendString(tbuf, numbuf);
      currAtts=(*pAtts & ~GWEN_WIDGET_ATT_CHAR);
    }
    if (*pAtts & GWEN_WIDGET_CHAR_ESC_CHAR) {
      char numbuf[4];

      GWEN_Buffer_AppendString(tbuf, "%fe%");
      snprintf(numbuf, sizeof(numbuf), "%02x", *pChars);
      GWEN_Buffer_AppendString(tbuf, numbuf);
    }
    else {
      GWEN_Buffer_AppendByte(tbuf, *pChars);
    }
    pChars++;
    pAtts++;
  }
  if (atts)
    *atts=currAtts;

  return 0;
}




int GWEN_TextWidget_LineClose(GWEN_WIDGET *w,
                              GWEN_TW_LINE *l,
                              int force) {
  GWEN_TEXTWIDGET *win;
  GWEN_BUFFER *tbuf;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (!l->openCount)
    return 0;

  l->openCount--;
  if (l->openCount && !force)
    return 0;

  l->openCount=0;
  if (!l->changed) {
    GWEN_Buffer_free(l->attributes); l->attributes=0;
    GWEN_Buffer_free(l->chars); l->chars=0;
    return 0;
  }

  tbuf=GWEN_Buffer_new(0, 2*GWEN_Buffer_GetUsedBytes(l->chars), 0, 1);
  if (GWEN_TextWidget_CondenseLineArea(l,
                                       0,
                                       GWEN_Buffer_GetUsedBytes(l->chars)-1,
                                       tbuf,
                                       &(l->endAttributes))) {
    DBG_ERROR(0, "Error condensing buffer");
  }
  else {
    free(l->text);
    l->text=strdup(GWEN_Buffer_GetStart(tbuf));
  }
  l->length=GWEN_Buffer_GetUsedBytes(l->chars);
  if (win->dwidth<l->length) {
    win->dwidth=l->length;
    GWEN_Widget_ContentChange(w, win->dwidth, win->dheight);
  }

  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_free(l->attributes); l->attributes=0;
  GWEN_Buffer_free(l->chars); l->chars=0;
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

  if (leftBorder>l->length) {
    int i;

    i=leftBorder-l->length;
    while(i--) {
      GWEN_Buffer_AppendByte(l->chars, ' ');
      GWEN_Buffer_AppendByte(l->attributes, 0);
    }
  }
  if (rightBorder>l->length) {
    int i;

    i=rightBorder-l->length;
    while(i--) {
      GWEN_Buffer_AppendByte(l->chars, ' ');
      GWEN_Buffer_AppendByte(l->attributes, 0);
    }
  }
  l->leftBorder=leftBorder;
  l->rightBorder=rightBorder;

  return 0;
}



int GWEN_TextWidget_LineSetInsert(GWEN_WIDGET *w,
                                  GWEN_TW_LINE *l,
                                  int insert) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  l->insertOn=insert;

  return 0;
}



int GWEN_TextWidget_LineSetAttributes(GWEN_WIDGET *w,
                                      GWEN_TW_LINE *l,
                                      GWEN_TYPE_UINT32 atts) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  l->currentAtts=atts;

  return 0;
}



int GWEN_TextWidget_LineSetPos(GWEN_WIDGET *w,
                               GWEN_TW_LINE *l,
                               int pos) {
  GWEN_TEXTWIDGET *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

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

  if (!l->openCount)
    return -1;

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
  return 0;
}



int GWEN_TextWidget_LineRedraw(GWEN_WIDGET *w, GWEN_TW_LINE *l) {
  GWEN_TEXTWIDGET *win;
  int x, y, len;
  int left, right;
  GWEN_TW_LINE *tmpl;
  GWEN_BUFFER *tbuf;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_TEXTWIDGET, w);
  assert(win);

  if (!l->openCount)
    return -1;

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

  if (y<win->top) {
    DBG_NOTICE(0, "Line not visible (above)");
    return 0;
  }

  if (y>=(win->top+GWEN_Widget_GetHeight(w))) {
    DBG_NOTICE(0, "Line not visible (below)");
    return 0;
  }

  left=l->leftBorder;
  if (left<win->left)
    left=win->left;

  right=l->rightBorder;
  if (right-left>=GWEN_Widget_GetHeight(w))
    right=GWEN_Widget_GetWidth(w)+left;

  len=right-left+1;
  if (len<1)
    return 0;

  y-=win->top;
  x=left-win->left;

  DBG_NOTICE(0, "Update dims: %d/%d, %d (%d, %d)",
             x, y, len, left, right);
  tbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(l->chars), 0, 1);
  if (GWEN_TextWidget_CondenseLineArea(l,
                                       left,
                                       right,
                                       tbuf,
                                       0)) {
    DBG_ERROR(0, "Error condensing buffer");
  }

  GWEN_Widget_WriteAt(w, x, y, GWEN_Buffer_GetStart(tbuf), len);
  GWEN_Widget_Refresh(w);
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
    p++;
  }
  if (blnks<len) {
    DBG_INFO(0, "Line full");
    return -1;
  }

  pChars+=l->currentPos;
  pAtts+=l->currentPos;

  if (blnks<l->rightBorder-l->currentPos+1) {
    /* move data behind out of the way */
    p=pChars+=i;
    memmove(p, pChars, i);
    p=pAtts+=len;
    memmove(p, pAtts, i);
  }

  /* actually write into the buffer */
  l->currentPos+=i;
  while(i--) {
    *pChars++=*text++;
    *pAtts++=l->currentAtts;
  }

  l->changed=1;
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

  if (!l->openCount) {
    DBG_INFO(0, "Line not open");
    return -1;
  }

  if (l->currentPos>l->rightBorder || l->currentPos<l->leftBorder) {
    DBG_INFO(0, "Position outside border");
    return -1;
  }

  if (len==0)
    len=strlen(text);
  if (!len)
    return 0;

  if (l->insertOn)
    return GWEN_TextWidget_LineWriteText_INS(w, l, text, len);
  else
    return GWEN_TextWidget_LineWriteText_OV(w, l, text, len);
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

  if (x+xoffs>=(win->left+ww))
    xoffs=(x-(win->left+ww)+width);    /* scroll to the right */


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
    DBG_NOTICE(0, "X Does not fit (vwidth=%d)", win->vwidth);
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










