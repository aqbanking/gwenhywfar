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

#include <gwenhywfar/tablewidget.h>
#include <gwenhywfar/textwidget.h>


#define GWEN_TABLECOLUMN_DEFAULT_WIDTH 8

struct GWEN_TABLE_FIELD {
  GWEN_LIST_ELEMENT(GWEN_TABLE_FIELD);
  int x; /* updated by parents as soon as the position is known */
  int y; /* updated by parents as soon as the position is known */
  int width;
  int height;
  char *text;
  GWEN_TABLE_COLUMN *parent;
};



struct GWEN_TABLE_COLUMN {
  GWEN_LIST_ELEMENT(GWEN_TABLE_COLUMN);
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
                                int yes);




#endif


