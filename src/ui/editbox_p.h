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

#ifndef GWEN_UI_EDITBOX_P_H
#define GWEN_UI_EDITBOX_P_H

#include <gwenhywfar/ui/editbox.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/widget.h>
#include <gwenhywfar/ui/textwidget.h>



typedef struct GWEN_EDITBOX GWEN_EDITBOX;
struct GWEN_EDITBOX {
  GWEN_WIDGET_EVENTHANDLER_FN previousHandler;
  GWEN_TYPE_UINT32 flags;
  int currX;
  int currY;
  GWEN_TW_LINE *currLine;
  int insertMode;
  int clearAllFlag;
  int maxLen;
};
void GWEN_EditBox_freeData(void *bp, void *p);



int GWEN_EditBox_EnsureLine(GWEN_WIDGET *w, int y);
void GWEN_EditBox_AdjustCursor(GWEN_WIDGET *w);
GWEN_UI_RESULT GWEN_EditBox_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e);



#endif





