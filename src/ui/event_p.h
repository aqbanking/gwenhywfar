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


#ifndef GWEN_UI_EVENT_P_H
#define GWEN_UI_EVENT_P_H


#include "event_l.h"
#include <time.h>


struct GWEN_EVENT_SUBSCRIPTION {
  GWEN_LIST_ELEMENT(GWEN_EVENT_SUBSCRIPTION)
  GWEN_EVENT_TYPE type;
  GWEN_WIDGET *widget;
};


struct GWEN_EVENT {
  GWEN_LIST_ELEMENT(GWEN_EVENT)
  GWEN_INHERIT_ELEMENT(GWEN_EVENT)
  GWEN_EVENT_TYPE type;
  GWEN_WIDGET *sender;
  GWEN_WIDGET *recipient;
  time_t posted;
  GWEN_TYPE_UINT32 id;
  int subscriptionMark;
  GWEN_TYPE_UINT32 usage;
};



typedef struct GWEN_EVENT_SETTEXT GWEN_EVENT_SETTEXT;
struct GWEN_EVENT_SETTEXT {
  char *text;
  GWEN_EVENT_SETTEXT_MODE mode;
};
void GWEN_EventSetText_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_MOVE GWEN_EVENT_MOVE;
struct GWEN_EVENT_MOVE {
  int x, y, oldx, oldy;
};
void GWEN_EventMove_freeData(void *bp, void *p);


typedef struct GWEN_EVENT_KEY GWEN_EVENT_KEY;
struct GWEN_EVENT_KEY {
  int key;
};
void GWEN_EventKey_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_SETCOLOUR GWEN_EVENT_SETCOLOUR;
struct GWEN_EVENT_SETCOLOUR {
  GWEN_WIDGET_COLOUR colour;
};
void GWEN_EventSetColour_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_FOCUS GWEN_EVENT_FOCUS;
struct GWEN_EVENT_FOCUS {
  GWEN_EVENT_FOCUS_TYPE focusEventType;
};
void GWEN_EventFocus_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_SCROLL GWEN_EVENT_SCROLL;
struct GWEN_EVENT_SCROLL {
  int xBy, yBy, todo;
};
void GWEN_EventScroll_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_WRITEAT GWEN_EVENT_WRITEAT;
struct GWEN_EVENT_WRITEAT {
  int x, y;
  char *text;
  int len;
  GWEN_WIDGET_COLOUR colour;
};
void GWEN_EventWriteAt_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_CHGATTS GWEN_EVENT_CHGATTS;
struct GWEN_EVENT_CHGATTS {
  GWEN_TYPE_UINT32 atts;
  int set;
};
void GWEN_EventChgAtts_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_CLEAR GWEN_EVENT_CLEAR;
struct GWEN_EVENT_CLEAR {
  int x, y;
  GWEN_EVENT_CLEAR_MODE mode;
};
void GWEN_EventClear_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_HIGHLIGHT GWEN_EVENT_HIGHLIGHT;
struct GWEN_EVENT_HIGHLIGHT {
  int x, y;
  int len;
  GWEN_WIDGET_COLOUR hi;
};
void GWEN_EventHighlight_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_CONTENTCHG GWEN_EVENT_CONTENTCHG;
struct GWEN_EVENT_CONTENTCHG {
  int cwidth, cheight;
  int width, height;
};
void GWEN_EventContentChg_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_COMMAND GWEN_EVENT_COMMAND;
struct GWEN_EVENT_COMMAND {
  GWEN_TYPE_UINT32 commandId;
};
void GWEN_EventCommand_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_SELECTED GWEN_EVENT_SELECTED;
struct GWEN_EVENT_SELECTED {
  char *text;
  int x, y;
  int state;
};
void GWEN_EventSelected_freeData(void *bp, void *p);



typedef struct GWEN_EVENT_CHOSEN GWEN_EVENT_CHOSEN;
struct GWEN_EVENT_CHOSEN {
  char *text;
  int x, y;
};
void GWEN_EventChosen_freeData(void *bp, void *p);


typedef struct GWEN_EVENT_CHECKED GWEN_EVENT_CHECKED;
struct GWEN_EVENT_CHECKED {
  int status;
};
void GWEN_EventChecked_freeData(void *bp, void *p);


#endif




