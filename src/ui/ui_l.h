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

#ifndef GWEN_UI_UI_L_H
#define GWEN_UI_UI_L_H


#include <gwenhywfar/ui.h>
#include <gwenhywfar/widget.h>




void GWEN_UI_AddRootWidget(GWEN_WIDGET *w);
void GWEN_UI_SetFocus(GWEN_WIDGET *w);

int GWEN_UI_FocusToNext(GWEN_WIDGET *w);

GWEN_EVENT *GWEN_UI_PeekNextEvent(GWEN_WIDGET *wRecipient);
GWEN_EVENT *GWEN_UI_GetNextEvent(GWEN_WIDGET *wRecipient);

int GWEN_UI_SendEvent(GWEN_WIDGET *wSender,
                      GWEN_WIDGET *wRecipient,
                      GWEN_EVENT *e,
                      int withPriority);

void GWEN_UI_DumpWidgets();


#endif



