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

#ifndef GWEN_UI_TEXTWIDGET_P_H
#define GWEN_UI_TEXTWIDGET_P_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/textwidget.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/buffer.h>


GWEN_LIST_FUNCTION_DEFS(GWEN_TW_LINE, GWEN_TWLine)

struct GWEN_TW_LINE {
  GWEN_LIST_ELEMENT(GWEN_TW_LINE)
  int length;
  char *text;
  GWEN_TYPE_UINT32 usage;
  GWEN_TYPE_UINT32 startAttributes;
  GWEN_TYPE_UINT32 endAttributes;

  /* vars for edit mode */
  int openCount;
  GWEN_BUFFER *attributes;
  GWEN_BUFFER *chars;
  int leftBorder;
  int rightBorder;
  int insertOn;
  int changed;
  int currentPos;
  GWEN_TYPE_UINT32 currentAtts;
};
GWEN_TW_LINE *GWEN_TWLine_new(GWEN_TYPE_UINT32 startAttributes,
                              const char *s);
void GWEN_TWLine_free(GWEN_TW_LINE *l);
void GWEN_TWLine_Attach(GWEN_TW_LINE *l);
int GWEN_TWLine_SetText(GWEN_TW_LINE *l, GWEN_TYPE_UINT32 startAttributes,
                        const char *s);



typedef struct GWEN_TEXTWIDGET GWEN_TEXTWIDGET;
struct GWEN_TEXTWIDGET {
  GWEN_WIDGET_EVENTHANDLER_FN previousHandler;
  GWEN_TW_LINE_LIST *lines;
  int top;
  int vheight;
  int left;
  int vwidth;
  int pos;
  int currentLine;
};
void GWEN_TextWidget_freeData(void *bp, void *p);

GWEN_UI_RESULT GWEN_TextWidget_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e);
int GWEN_TextWidget_WriteLine(GWEN_WIDGET *w, int x, int y);
int GWEN_TextWidget_WriteArea(GWEN_WIDGET *w,
                              int x, int y,
                              int width, int height);

int GWEN_TextWidget_ParseXMLSubNodes(GWEN_WIDGET *w,
                                     GWEN_XMLNODE *n,
                                     GWEN_BUFFER *buf,
                                     GWEN_TYPE_UINT32 startAtts,
                                     GWEN_TYPE_UINT32 currentAtts,
                                     int indent,
                                     int *pos,
                                     GWEN_TW_LINE_LIST *ll);

int GWEN_TextWidget_ParseXMLTag(GWEN_WIDGET *w,
                                GWEN_XMLNODE *n,
                                GWEN_BUFFER *buf,
                                GWEN_TYPE_UINT32 startAtts,
                                GWEN_TYPE_UINT32 currentAtts,
                                int indent,
                                int *pos,
                                GWEN_TW_LINE_LIST *ll);

int GWEN_TextWidget_SetText(GWEN_WIDGET *w,
                            const char *text,
                            GWEN_EVENT_SETTEXT_MODE m);

int GWEN_TextWidget_CondenseLineArea(GWEN_TW_LINE *l,
                                     int leftBorder,
                                     int rightBorder,
                                     GWEN_BUFFER *tbuf,
                                     GWEN_TYPE_UINT32 *atts);

void GWEN_TextWidget_Draw(GWEN_WIDGET *w);


#endif


