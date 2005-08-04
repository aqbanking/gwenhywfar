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

#ifndef GWEN_UI_TABLEWIDGET_P_H
#define GWEN_UI_TABLEWIDGET_P_H

#include <gwenhywfar/ui/tablewidget.h>
#include <gwenhywfar/ui/textwidget.h>


#define GWEN_TABLECOLUMN_DEFAULT_WIDTH 8

struct GWEN_TABLE_FIELD {
  GWEN_LIST_ELEMENT(GWEN_TABLE_FIELD)
  int x; /* updated by parents as soon as the position is known */
  int y; /* updated by parents as soon as the position is known */
  int width;
  int height;
  int selected;
  char *text;
  GWEN_TABLE_COLUMN *parent;
};



struct GWEN_TABLE_COLUMN {
  GWEN_LIST_ELEMENT(GWEN_TABLE_COLUMN)
  int x;
  int y;
  int width;
  int height;
  GWEN_WIDGET *parent;
  GWEN_TABLE_FIELD_LIST *fields;
};



typedef struct GWEN_TABLEWIDGET GWEN_TABLEWIDGET;
struct GWEN_TABLEWIDGET {
  GWEN_WIDGET_EVENTHANDLER_FN previousHandler;
  GWEN_TABLE_COLUMN_LIST *columns;

  int tx;
  int ty;
  int twidth;
  int theight;

  int mx;
  int my;
  int top;
  int left;
  int changed;

  GWEN_TYPE_UINT32 flags;
};
void GWEN_TableWidget_freeData(void *bp, void *p);

GWEN_UI_RESULT GWEN_TableWidget_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e);


int GWEN_TableField_Calculate_MinWidth(const GWEN_TABLE_FIELD *tf);
int GWEN_TableField_Calculate_Height(const GWEN_TABLE_FIELD *tf, int width);
GWEN_TABLE_FIELD *GWEN_TableWidget_LocateField(GWEN_WIDGET *w,
                                               int x, int y, int crea);
void GWEN_TableWidget_Highlight(GWEN_WIDGET *w,
                                GWEN_TABLE_FIELD *tf,
                                GWEN_WIDGET_COLOUR colour);

void GWEN_TableWidget_DrawBorder(GWEN_WIDGET *w, int y1, int y2);

void GWEN_TableWidget__AddColumn(GWEN_WIDGET *w, GWEN_TABLE_COLUMN *tc);


GWEN_TABLE_FIELD *GWEN_TableField_new(int width, int height, char *text);
void GWEN_TableField_free(GWEN_TABLE_FIELD *tf);

GWEN_TABLE_COLUMN *GWEN_TableField_GetParent(const GWEN_TABLE_FIELD *tf);
int GWEN_TableField_GetWidth(const GWEN_TABLE_FIELD *tf);
int GWEN_TableField_GetHeight(const GWEN_TABLE_FIELD *tf);
const char *GWEN_TableField_GetText(const GWEN_TABLE_FIELD *tf);
void GWEN_TableField_SetText(GWEN_TABLE_FIELD *tf,
                             const char *s);
void GWEN_TableField_Update(const GWEN_TABLE_FIELD *tf);





GWEN_TABLE_COLUMN *GWEN_TableColumn_new(int width);
void GWEN_TableColumn_free(GWEN_TABLE_COLUMN *tc);
int GWEN_TableColumn_GetWidth(const GWEN_TABLE_COLUMN *tc);
int GWEN_TableColumn_GetHeight(const GWEN_TABLE_COLUMN *tc);
GWEN_WIDGET *GWEN_TableColumn_GetParent(const GWEN_TABLE_COLUMN *tc);
GWEN_TABLE_FIELD_LIST*
  GWEN_TableColumn_GetFields(const GWEN_TABLE_COLUMN *tc);
void GWEN_TableColumn_Update(GWEN_TABLE_COLUMN *tc);
void GWEN_TableColumn_AddField(GWEN_TABLE_COLUMN *tc, GWEN_TABLE_FIELD *tf);



void GWEN_TableWidget_Refresh(GWEN_WIDGET *w);


#endif


