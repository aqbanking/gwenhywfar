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

#include <gwenhywfar/widget.h>
#include <gwenhywfar/misc.h>


#define GWEN_TABLEWIDGET_FLAGS_BREAKLINES  0x00010000
#define GWEN_TABLEWIDGET_FLAGS_COLBORDER   0x00020000
#define GWEN_TABLEWIDGET_FLAGS_ROWBORDER   0x00040000


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








#endif


