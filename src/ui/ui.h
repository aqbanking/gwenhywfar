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

#ifndef GWEN_UI_UI_H
#define GWEN_UI_UI_H


typedef enum {
  GWEN_UIResult_Handled=0,
  GWEN_UIResult_NotHandled,
  GWEN_UIResult_Error,
  GWEN_UIResult_Finished,
  GWEN_UIResult_Quit
} GWEN_UI_RESULT;


#include <gwenhywfar/event.h>

int GWEN_UI_Begin();
int GWEN_UI_End();

void GWEN_UI_Flush();
GWEN_UI_RESULT GWEN_UI_Work();


GWEN_EVENT *GWEN_UI_PeekNextEvent();
GWEN_EVENT *GWEN_UI_GetNextEvent();
GWEN_UI_RESULT GWEN_UI_DispatchEvent(GWEN_EVENT *e);

GWEN_WIDGET *GWEN_UI_GetFocusWidget();



int GWEN_UI_GetCols();
int GWEN_UI_GetLines();


int GWEN_UI_Calculate_MinWidth(const char *text);
int GWEN_UI_Calculate_Height(const char *text, int width);


#endif





