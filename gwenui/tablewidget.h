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

#ifndef GWEN_UI_TABLEWIDGET_H
#define GWEN_UI_TABLEWIDGET_H

#include <gwenhywfar/ui/widget.h>
#include <gwenhywfar/misc.h>


#define GWEN_TABLEWIDGET_FLAGS_BREAKLINES  0x00010000
#define GWEN_TABLEWIDGET_FLAGS_COLBORDER   0x00020000
#define GWEN_TABLEWIDGET_FLAGS_ROWBORDER   0x00040000
#define GWEN_TABLEWIDGET_FLAGS_LINEMODE    0x00080000
#define GWEN_TABLEWIDGET_FLAGS_FIXED       0x00100000
#define GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT   0x00200000
#define GWEN_TABLEWIDGET_FLAGS_SELECTION   0x00400000


typedef struct GWEN_TABLE_FIELD GWEN_TABLE_FIELD;
typedef struct GWEN_TABLE_ROW GWEN_TABLE_ROW;
typedef struct GWEN_TABLE_COLUMN GWEN_TABLE_COLUMN;

GWEN_LIST_FUNCTION_DEFS(GWEN_TABLE_FIELD, GWEN_TableField);
GWEN_LIST_FUNCTION_DEFS(GWEN_TABLE_COLUMN, GWEN_TableColumn);



GWEN_WIDGET *GWEN_TableWidget_new(GWEN_WIDGET *parent,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *name,
                                  int x, int y,
                                  int width, int height);

void GWEN_TableWidget_SetText(GWEN_WIDGET *w,
                              int x, int y, const char *text);
const char *GWEN_TableWidget_GetText(GWEN_WIDGET *w, int x, int y);


int GWEN_TableWidget_AddColumn(GWEN_WIDGET *w, int width);
int GWEN_TableWidget_GetColumns(const GWEN_WIDGET *w);

int GWEN_TableWidget_Clear(GWEN_WIDGET *w);


#endif


