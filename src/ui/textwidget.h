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

#ifndef GWEN_UI_TEXTWIDGET_H
#define GWEN_UI_TEXTWIDGET_H

#include <gwenhywfar/widget.h>
#include <gwenhywfar/xml.h>


typedef struct GWEN_TW_LINE GWEN_TW_LINE;
GWEN_LIST_FUNCTION_DEFS(GWEN_TW_LINE, GWEN_TWLine)


#define GWEN_TEXTWIDGET_FLAGS_BREAKLINES  0x00010000
#define GWEN_TEXTWIDGET_FLAGS_LINEMODE    0x00020000
#define GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT   0x00040000
#define GWEN_TEXTWIDGET_FLAGS_DATACHANGE  0x00080000
#define GWEN_TEXTWIDGET_FLAGS_FIXED       0x00100000


GWEN_WIDGET *GWEN_TextWidget_new(GWEN_WIDGET *parent,
                                 GWEN_TYPE_UINT32 flags,
                                 const char *name,
                                 const char *text,
                                 int x, int y,
                                 int width, int height);

void GWEN_TextWidget_SetVirtualSize(GWEN_WIDGET *w,
                                    int vwidth,
                                    int vheight);


GWEN_TW_LINE *GWEN_TextWidget_LineOpen(GWEN_WIDGET *w, int n, int cre);
int GWEN_TextWidget_LineClose(GWEN_WIDGET *w,
                              GWEN_TW_LINE *l,
                              int force);
int GWEN_TextWidget_LineSetBorders(GWEN_WIDGET *w,
                                   GWEN_TW_LINE *l,
                                   int leftBorder,
                                   int rightBorder);

int GWEN_TextWidget_LineSetInsert(GWEN_WIDGET *w,
                                  GWEN_TW_LINE *l,
                                  int insert);
int GWEN_TextWidget_LineSetAttributes(GWEN_WIDGET *w,
                                      GWEN_TW_LINE *l,
                                      GWEN_TYPE_UINT32 atts);
int GWEN_TextWidget_LineSetPos(GWEN_WIDGET *w, GWEN_TW_LINE *l, int pos);
int GWEN_TextWidget_LineClear(GWEN_WIDGET *w, GWEN_TW_LINE *l);
int GWEN_TextWidget_LineRedraw(GWEN_WIDGET *w, GWEN_TW_LINE *l);
int GWEN_TextWidget_LineWriteText(GWEN_WIDGET *w,
                                  GWEN_TW_LINE *l,
                                  const char *text,
                                  int len);

int GWEN_TextWidget_EnsureVisible(GWEN_WIDGET *w,
                                  int x, int y,
                                  int width, int height);

GWEN_TW_LINE_LIST *GWEN_TextWidget_XMLToLines(GWEN_XMLNODE *n,
                                              int leftBorder,
                                              int rightBorder);
GWEN_TW_LINE_LIST *GWEN_TextWidget_TextToLines(const char *s,
                                               int leftBorder,
                                               int rightBorder);

int GWEN_TextWidget_SetPreparedText(GWEN_WIDGET *w,
                                    GWEN_TW_LINE_LIST *ll,
                                    GWEN_EVENT_SETTEXT_MODE m);

int GWEN_TextWidget_GetMaxWidth(GWEN_TW_LINE_LIST *ll);



#endif


