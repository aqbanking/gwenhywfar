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

#ifndef GWEN_UI_EVENT_H
#define GWEN_UI_EVENT_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

typedef struct GWEN_EVENT GWEN_EVENT;


typedef enum {
  GWEN_EventSetTextMode_Replace=0,
  GWEN_EventSetTextMode_Append,
  GWEN_EventSetTextMode_Insert
} GWEN_EVENT_SETTEXT_MODE;


typedef enum {
  GWEN_EventFocusType_Got=0,
  GWEN_EventFocusType_Lost
} GWEN_EVENT_FOCUS_TYPE;


typedef enum {
  GWEN_EventClearMode_All=0,
  GWEN_EventClearMode_ToEOL,
  GWEN_EventClearMode_ToBottom
} GWEN_EVENT_CLEAR_MODE;


typedef enum {
  GWEN_EventType_None=0,
  GWEN_EventType_Key,
  GWEN_EventType_SetText,
  GWEN_EventType_Move,
  GWEN_EventType_Draw,
  GWEN_EventType_SetColour,
  GWEN_EventType_Focus,
  GWEN_EventType_Created,
  GWEN_EventType_Destroy,
  GWEN_EventType_Update,
  GWEN_EventType_Refresh,
  GWEN_EventType_Scroll,
  GWEN_EventType_WriteAt,
  GWEN_EventType_ChgAtts,
  GWEN_EventType_Clear,
  GWEN_EventType_Highlight,
  GWEN_EventType_ContentChg,
  GWEN_EventType_Command,
  GWEN_EventType_Close,
  GWEN_EventType_Closed,
  GWEN_EventType_LastClosed,
  GWEN_EventType_Selected,
  GWEN_EventType_Chosen
} GWEN_EVENT_TYPE;


#include <gwenhywfar/ui/widget.h>


GWEN_LIST_FUNCTION_DEFS(GWEN_EVENT, GWEN_Event)
GWEN_INHERIT_FUNCTION_DEFS(GWEN_EVENT)



GWEN_EVENT_TYPE GWEN_Event_GetType(const GWEN_EVENT *e);
GWEN_WIDGET *GWEN_Event_GetSender(const GWEN_EVENT *e);
GWEN_WIDGET *GWEN_Event_GetRecipient(const GWEN_EVENT *e);
const char *GWEN_Event_GetEventTypeName(const GWEN_EVENT *e);

int GWEN_Event_DueToSubscription(const GWEN_EVENT *e);


GWEN_EVENT *GWEN_Event_new(GWEN_EVENT_TYPE t);

void GWEN_Event_free(GWEN_EVENT *e);

GWEN_EVENT_TYPE GWEN_Event_GetType(const GWEN_EVENT *e);
GWEN_WIDGET *GWEN_Event_GetSender(const GWEN_EVENT *e);


GWEN_EVENT *GWEN_EventSetText_new(const char *text,
                                  GWEN_EVENT_SETTEXT_MODE m);
const char *GWEN_EventSetText_GetText(const GWEN_EVENT *e);
GWEN_EVENT_SETTEXT_MODE GWEN_EventSetText_GetMode(const GWEN_EVENT *e);


GWEN_EVENT *GWEN_EventWriteAt_new(int x, int y,
                                  const char *text,
                                  int len);
const char *GWEN_EventWriteAt_GetText(const GWEN_EVENT *e);
int GWEN_EventWriteAt_GetX(const GWEN_EVENT *e);
int GWEN_EventWriteAt_GetY(const GWEN_EVENT *e);
int GWEN_EventWriteAt_GetLen(const GWEN_EVENT *e);




GWEN_EVENT *GWEN_EventMove_new(int x, int y,
                               int oldx, int oldy);
int GWEN_EventMove_GetX(const GWEN_EVENT *e);
int GWEN_EventMove_GetY(const GWEN_EVENT *e);
int GWEN_EventMove_GetOldX(const GWEN_EVENT *e);
int GWEN_EventMove_GetOldY(const GWEN_EVENT *e);



GWEN_EVENT *GWEN_EventDraw_new();


GWEN_EVENT *GWEN_EventCreated_new();


GWEN_EVENT *GWEN_EventDestroy_new();


GWEN_EVENT *GWEN_EventUpdate_new();


GWEN_EVENT *GWEN_EventRefresh_new();


GWEN_EVENT *GWEN_EventClose_new();

GWEN_EVENT *GWEN_EventClosed_new();


GWEN_EVENT *GWEN_EventLastClosed_new();


GWEN_EVENT *GWEN_EventLastClosed_new();


GWEN_EVENT *GWEN_EventKey_new(int key);
int GWEN_EventKey_GetKey(const GWEN_EVENT *e);



GWEN_EVENT *GWEN_EventChgAtts_new(GWEN_TYPE_UINT32 atts, int set);
GWEN_TYPE_UINT32 GWEN_EventChgAtts_GetAtts(const GWEN_EVENT *e);
int GWEN_EventChgAtts_GetSet(const GWEN_EVENT *e);



GWEN_EVENT *GWEN_EventSetColour_new(GWEN_WIDGET_COLOUR c);
GWEN_WIDGET_COLOUR GWEN_EventSetColour_GetColour(const GWEN_EVENT *e);




GWEN_EVENT *GWEN_EventFocus_new(GWEN_EVENT_FOCUS_TYPE ft);
GWEN_EVENT_FOCUS_TYPE GWEN_EventFocus_GetFocusEventType(const GWEN_EVENT *e);



GWEN_EVENT *GWEN_EventScroll_new(int xBy, int yBy, int todo);
int GWEN_EventScroll_GetXBy(GWEN_EVENT *e);
int GWEN_EventScroll_GetYBy(GWEN_EVENT *e);
int GWEN_EventScroll_GetTodo(GWEN_EVENT *e);



GWEN_EVENT *GWEN_EventClear_new(int x, int y, GWEN_EVENT_CLEAR_MODE m);
int GWEN_EventClear_GetX(const GWEN_EVENT *e);
int GWEN_EventClear_GetY(const GWEN_EVENT *e);
GWEN_EVENT_CLEAR_MODE GWEN_EventClear_GetMode(const GWEN_EVENT *e);


GWEN_EVENT *GWEN_EventHighlight_new(int x, int y,
                                    int len,
                                    GWEN_WIDGET_COLOUR hi);
const char *GWEN_EventHighlight_GetText(const GWEN_EVENT *e);
int GWEN_EventHighlight_GetX(const GWEN_EVENT *e);
int GWEN_EventHighlight_GetY(const GWEN_EVENT *e);
int GWEN_EventHighlight_GetLen(const GWEN_EVENT *e);
GWEN_WIDGET_COLOUR GWEN_EventHighlight_GetHi(const GWEN_EVENT *e);



GWEN_EVENT *GWEN_EventContentChg_new(int contentWidth, int contentHeight,
                                     int width, int height);
int GWEN_EventContentChg_GetContentWidth(const GWEN_EVENT *e);
int GWEN_EventContentChg_GetContentHeight(const GWEN_EVENT *e);
int GWEN_EventContentChg_GetWidth(const GWEN_EVENT *e);
int GWEN_EventContentChg_GetHeight(const GWEN_EVENT *e);


GWEN_EVENT *GWEN_EventCommand_new(GWEN_TYPE_UINT32 commandId);
GWEN_TYPE_UINT32 GWEN_EventCommand_GetCommandId(const GWEN_EVENT *e);


GWEN_EVENT *GWEN_EventSelected_new(const char *text, int x, int y);
const char *GWEN_EventSelected_GetText(const GWEN_EVENT *e);
int GWEN_EventSelected_GetX(const GWEN_EVENT *e);
int GWEN_EventSelected_GetY(const GWEN_EVENT *e);


GWEN_EVENT *GWEN_EventChosen_new(const char *text, int x, int y);
const char *GWEN_EventChosen_GetText(const GWEN_EVENT *e);
int GWEN_EventChosen_GetX(const GWEN_EVENT *e);
int GWEN_EventChosen_GetY(const GWEN_EVENT *e);


void GWEN_Event_Dump(const GWEN_EVENT *e);


#endif





