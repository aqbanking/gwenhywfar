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

#ifndef GWEN_UI_WINDOW_P_H
#define GWEN_UI_WINDOW_P_H

#include <gwenhywfar/ui/window.h>


typedef struct GWEN_WINDOW GWEN_WINDOW;
struct GWEN_WINDOW {
  GWEN_WIDGET *wTitle;
  GWEN_WIDGET *wView;
  GWEN_WIDGET *wHslider;
  GWEN_WIDGET *wVslider;
  GWEN_WIDGET_EVENTHANDLER_FN previousHandler;
};
void GWEN_Window_freeData(void *bp, void *p);

GWEN_UI_RESULT GWEN_Window_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e);





#endif





