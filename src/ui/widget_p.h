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

#ifndef GWEN_UI_WIDGET_P_H
#define GWEN_UI_WIDGET_P_H


#include "widget_l.h"
#include "event_l.h"
#include <panel.h>

#include <ncurses.h>



struct GWEN_WIDGET {
  /* data administration */
  GWEN_LIST_ELEMENT(GWEN_WIDGET)
  GWEN_INHERIT_ELEMENT(GWEN_WIDGET)

  char *name;
  char *typeName;

  GWEN_TYPE_UINT32 id;
  GWEN_WIDGET_LIST *children;
  GWEN_WIDGET *parent;
  GWEN_TYPE_UINT32 usage;

  /* window data */
  int x;
  int y;
  int width;
  int height;
  GWEN_WIDGET_COLOUR colour;
  GWEN_WIDGET_COLOUR normalColour;

  int cursorX;
  int cursorY;

  /* content data */
  char *text;

  GWEN_TYPE_UINT32 flags;
  GWEN_TYPE_UINT32 state;
  GWEN_WIDGET_EVENTHANDLER_FN eventHandler;
  GWEN_WIDGET_RUN_FN runFn;

  GWEN_EVENT_SUBSCRIPTION_LIST *subscribers;
  GWEN_EVENT_SUBSCRIPTION_LIST *subscriptions;

  /* additional data */
  char *helpText;
  char *dbVarName;

  /* ncurses data */
  WINDOW *window;
  PANEL *panel;

};


GWEN_EVENT *GWEN_Widget_PeekNextEvent(GWEN_WIDGET *w);
GWEN_EVENT *GWEN_Widget_GetNextEvent(GWEN_WIDGET *w);
GWEN_UI_RESULT GWEN_Widget__HandleEvent(GWEN_WIDGET *w,
                                        GWEN_EVENT *e);
void GWEN_Widget_UnsubscribeFromAll(GWEN_WIDGET *w);



#endif


