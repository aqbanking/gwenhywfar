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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <gwenhywfar/debug.h>

#include "event_p.h"


GWEN_LIST_FUNCTIONS(GWEN_EVENT, GWEN_Event)
GWEN_INHERIT_FUNCTIONS(GWEN_EVENT)

static GWEN_TYPE_UINT32 GWEN_EVENT__lastId=0;


GWEN_EVENT *GWEN_Event_new(GWEN_EVENT_TYPE t) {
  GWEN_EVENT *e;

  GWEN_NEW_OBJECT(GWEN_EVENT, e);
  GWEN_INHERIT_INIT(GWEN_EVENT, e);
  GWEN_LIST_INIT(GWEN_EVENT, e);
  e->type=t;
  e->id=++GWEN_EVENT__lastId;
  return e;
}



void GWEN_Event_free(GWEN_EVENT *e){
  if (e) {
    GWEN_INHERIT_FINI(GWEN_EVENT, e);
    GWEN_Widget_free(e->recipient);
    GWEN_Widget_free(e->sender);
    GWEN_LIST_FINI(GWEN_EVENT, e);
    GWEN_FREE_OBJECT(e);
  }
}



void GWEN_Event_SetSender(GWEN_EVENT *e, GWEN_WIDGET *w){
  assert(e);
  GWEN_Widget_free(e->sender);
  e->sender=w;
  if (w)
    GWEN_Widget_Attach(e->sender);
}



void GWEN_Event_SetRecipient(GWEN_EVENT *e, GWEN_WIDGET *w){
  assert(e);
  GWEN_Widget_free(e->recipient);
  e->recipient=w;
  if (w)
    GWEN_Widget_Attach(e->recipient);
  e->posted=time(0);
}



GWEN_EVENT_TYPE GWEN_Event_GetType(const GWEN_EVENT *e){
  assert(e);
  return e->type;
}



GWEN_WIDGET *GWEN_Event_GetSender(const GWEN_EVENT *e){
  assert(e);
  return e->sender;
}



GWEN_WIDGET *GWEN_Event_GetRecipient(const GWEN_EVENT *e){
  assert(e);
  return e->recipient;
}



const char *GWEN_Event_TypeName(GWEN_EVENT_TYPE t) {
  const char *s;

  switch(t) {
  case GWEN_EventType_Key: s="Key"; break;
  case GWEN_EventType_SetText: s="SetText"; break;
  case GWEN_EventType_Move: s="Move"; break;
  case GWEN_EventType_Draw: s="Draw"; break;
  case GWEN_EventType_SetColour: s="SetColour"; break;
  case GWEN_EventType_Focus: s="Focus"; break;
  case GWEN_EventType_Created: s="Created"; break;
  case GWEN_EventType_Destroy: s="Destroy"; break;
  case GWEN_EventType_Update: s="Update"; break;
  case GWEN_EventType_Refresh: s="Refresh"; break;
  case GWEN_EventType_Scroll: s="Scroll"; break;
  case GWEN_EventType_WriteAt: s="WriteAt"; break;
  case GWEN_EventType_ChgAtts: s="ChgAtts"; break;
  case GWEN_EventType_Clear: s="Clear"; break;
  case GWEN_EventType_Highlight: s="Highlight"; break;
  case GWEN_EventType_None: s="None"; break;
  default: s="Unknown"; break;
  }

  return s;
}



void GWEN_Event_Dump(const GWEN_EVENT *e){
  assert(e);

  DBG_NOTICE(0, "Event[%02d] %9.9s at %04x (sender=\"%s\", recipient=\"%s\")",
             e->id,
             GWEN_Event_TypeName(e->type),
             (unsigned int)e->posted,
             (e->sender)?GWEN_Widget_GetName(e->sender):"<none>",
             (e->recipient)?GWEN_Widget_GetName(e->recipient):"<none>");
}








GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_SETTEXT);

GWEN_EVENT *GWEN_EventSetText_new(const char *text,
                                  GWEN_EVENT_SETTEXT_MODE m){
  GWEN_EVENT *e;
  GWEN_EVENT_SETTEXT *et;

  e=GWEN_Event_new(GWEN_EventType_SetText);
  GWEN_NEW_OBJECT(GWEN_EVENT_SETTEXT, et);
  if (text)
    et->text=strdup(text);
  et->mode=m;

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_SETTEXT,
                       e, et,
                       GWEN_EventSetText_freeData);
  return e;
}



void GWEN_EventSetText_freeData(void *bp, void *p) {
  GWEN_EVENT_SETTEXT *et;

  et=(GWEN_EVENT_SETTEXT*)p;
  free(et->text);
  GWEN_FREE_OBJECT(et);
}



const char *GWEN_EventSetText_GetText(const GWEN_EVENT *e){
  GWEN_EVENT_SETTEXT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_SETTEXT, e);
  assert(et);

  return et->text;
}



GWEN_EVENT_SETTEXT_MODE GWEN_EventSetText_GetMode(const GWEN_EVENT *e){
  GWEN_EVENT_SETTEXT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_SETTEXT, e);
  assert(et);

  return et->mode;
}




GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_WRITEAT);

GWEN_EVENT *GWEN_EventWriteAt_new(int x, int y,
                                  const char *text,
                                  int len){
  GWEN_EVENT *e;
  GWEN_EVENT_WRITEAT *et;

  e=GWEN_Event_new(GWEN_EventType_WriteAt);
  GWEN_NEW_OBJECT(GWEN_EVENT_WRITEAT, et);
  et->x=x;
  et->y=y;
  if (len==0)
    et->len=strlen(text);
  else
    et->len=len;
  if (text)
    et->text=strdup(text);

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_WRITEAT,
                       e, et,
                       GWEN_EventWriteAt_freeData);
  return e;
}



void GWEN_EventWriteAt_freeData(void *bp, void *p) {
  GWEN_EVENT_WRITEAT *et;

  et=(GWEN_EVENT_WRITEAT*)p;
  free(et->text);
  GWEN_FREE_OBJECT(et);
}



const char *GWEN_EventWriteAt_GetText(const GWEN_EVENT *e){
  GWEN_EVENT_WRITEAT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_WRITEAT, e);
  assert(et);

  return et->text;
}



int GWEN_EventWriteAt_GetX(const GWEN_EVENT *e){
  GWEN_EVENT_WRITEAT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_WRITEAT, e);
  assert(et);

  return et->x;
}



int GWEN_EventWriteAt_GetY(const GWEN_EVENT *e){
  GWEN_EVENT_WRITEAT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_WRITEAT, e);
  assert(et);

  return et->y;
}



int GWEN_EventWriteAt_GetLen(const GWEN_EVENT *e){
  GWEN_EVENT_WRITEAT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_WRITEAT, e);
  assert(et);

  return et->len;
}









GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_MOVE);

GWEN_EVENT *GWEN_EventMove_new(int x, int y,
                               int oldx, int oldy){
  GWEN_EVENT *e;
  GWEN_EVENT_MOVE *et;

  e=GWEN_Event_new(GWEN_EventType_Move);
  GWEN_NEW_OBJECT(GWEN_EVENT_MOVE, et);
  et->x=x;
  et->y=y;
  et->oldx=oldx;
  et->oldy=oldy;

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_MOVE,
                       e, et,
                       GWEN_EventMove_freeData);
  return e;
}



void GWEN_EventMove_freeData(void *bp, void *p) {
  GWEN_EVENT_MOVE *et;

  et=(GWEN_EVENT_MOVE*)p;
  GWEN_FREE_OBJECT(et);
}


int GWEN_EventMove_GetX(const GWEN_EVENT *e){
  GWEN_EVENT_MOVE *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_MOVE, e);
  assert(et);
  return et->x;
}


int GWEN_EventMove_GetY(const GWEN_EVENT *e){
  GWEN_EVENT_MOVE *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_MOVE, e);
  assert(et);
  return et->y;
}


int GWEN_EventMove_GetOldX(const GWEN_EVENT *e){
  GWEN_EVENT_MOVE *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_MOVE, e);
  assert(et);
  return et->oldx;
}


int GWEN_EventMove_GetOldY(const GWEN_EVENT *e){
  GWEN_EVENT_MOVE *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_MOVE, e);
  assert(et);
  return et->oldy;
}







GWEN_EVENT *GWEN_EventDraw_new(){
  return GWEN_Event_new(GWEN_EventType_Draw);
}




GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_KEY);


GWEN_EVENT *GWEN_EventKey_new(int key){
  GWEN_EVENT *e;
  GWEN_EVENT_KEY *et;

  e=GWEN_Event_new(GWEN_EventType_Key);
  GWEN_NEW_OBJECT(GWEN_EVENT_KEY, et);
  et->key=key;

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_KEY,
                       e, et,
                       GWEN_EventKey_freeData);
  return e;
}



void GWEN_EventKey_freeData(void *bp, void *p) {
  GWEN_EVENT_KEY *et;

  et=(GWEN_EVENT_KEY*)p;
  GWEN_FREE_OBJECT(et);
}



int GWEN_EventKey_GetKey(const GWEN_EVENT *e){
  GWEN_EVENT_KEY *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_KEY, e);
  assert(et);
  return et->key;
}




GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_SETCOLOUR);


GWEN_EVENT *GWEN_EventSetColour_new(GWEN_WIDGET_COLOUR colour){
  GWEN_EVENT *e;
  GWEN_EVENT_SETCOLOUR *et;

  e=GWEN_Event_new(GWEN_EventType_SetColour);
  GWEN_NEW_OBJECT(GWEN_EVENT_SETCOLOUR, et);
  et->colour=colour;

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_SETCOLOUR,
                       e, et,
                       GWEN_EventSetColour_freeData);
  return e;
}



void GWEN_EventSetColour_freeData(void *bp, void *p) {
  GWEN_EVENT_SETCOLOUR *et;

  et=(GWEN_EVENT_SETCOLOUR*)p;
  GWEN_FREE_OBJECT(et);
}



GWEN_WIDGET_COLOUR GWEN_EventSetColour_GetColour(const GWEN_EVENT *e){
  GWEN_EVENT_SETCOLOUR *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_SETCOLOUR, e);
  assert(et);
  return et->colour;
}




GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_FOCUS);


GWEN_EVENT *GWEN_EventFocus_new(GWEN_EVENT_FOCUS_TYPE ft){
  GWEN_EVENT *e;
  GWEN_EVENT_FOCUS *et;

  e=GWEN_Event_new(GWEN_EventType_Focus);
  GWEN_NEW_OBJECT(GWEN_EVENT_FOCUS, et);
  et->focusEventType=ft;

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_FOCUS,
                       e, et,
                       GWEN_EventFocus_freeData);
  return e;
}



void GWEN_EventFocus_freeData(void *bp, void *p) {
  GWEN_EVENT_FOCUS *et;

  et=(GWEN_EVENT_FOCUS*)p;
  GWEN_FREE_OBJECT(et);
}



GWEN_EVENT_FOCUS_TYPE GWEN_EventFocus_GetFocusEventType(const GWEN_EVENT *e){
  GWEN_EVENT_FOCUS *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_FOCUS, e);
  assert(et);
  return et->focusEventType;
}







GWEN_EVENT *GWEN_EventCreated_new(){
  return GWEN_Event_new(GWEN_EventType_Created);
}





GWEN_EVENT *GWEN_EventDestroy_new(){
  return GWEN_Event_new(GWEN_EventType_Destroy);
}





GWEN_EVENT *GWEN_EventUpdate_new(){
  return GWEN_Event_new(GWEN_EventType_Update);
}





GWEN_EVENT *GWEN_EventRefresh_new(){
  return GWEN_Event_new(GWEN_EventType_Refresh);
}






GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_SCROLL);

GWEN_EVENT *GWEN_EventScroll_new(int xBy, int yBy, int todo){
  GWEN_EVENT *e;
  GWEN_EVENT_SCROLL *et;

  e=GWEN_Event_new(GWEN_EventType_Scroll);
  GWEN_NEW_OBJECT(GWEN_EVENT_SCROLL, et);

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_SCROLL,
                       e, et,
                       GWEN_EventScroll_freeData);
  et->xBy=xBy;
  et->yBy=yBy;
  et->todo=todo;
  return e;
}



void GWEN_EventScroll_freeData(void *bp, void *p){
  GWEN_EVENT_SCROLL *et;

  et=(GWEN_EVENT_SCROLL*)p;
  GWEN_FREE_OBJECT(et);
}



int GWEN_EventScroll_GetXBy(GWEN_EVENT *e){
  GWEN_EVENT_SCROLL *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_SCROLL, e);
  assert(et);

  return et->xBy;
}



int GWEN_EventScroll_GetYBy(GWEN_EVENT *e){
  GWEN_EVENT_SCROLL *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_SCROLL, e);
  assert(et);

  return et->yBy;
}



int GWEN_EventScroll_GetTodo(GWEN_EVENT *e){
  GWEN_EVENT_SCROLL *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_SCROLL, e);
  assert(et);

  return et->todo;
}







GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_CHGATTS);


GWEN_EVENT *GWEN_EventChgAtts_new(GWEN_TYPE_UINT32 atts, int set){
  GWEN_EVENT *e;
  GWEN_EVENT_CHGATTS *et;

  e=GWEN_Event_new(GWEN_EventType_ChgAtts);
  GWEN_NEW_OBJECT(GWEN_EVENT_CHGATTS, et);

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_CHGATTS,
                       e, et,
                       GWEN_EventChgAtts_freeData);
  et->atts=atts;
  et->set=set;
  return e;
}



void GWEN_EventChgAtts_freeData(void *bp, void *p){
  GWEN_EVENT_CHGATTS *et;

  et=(GWEN_EVENT_CHGATTS*)p;
  GWEN_FREE_OBJECT(et);
}



GWEN_TYPE_UINT32 GWEN_EventChgAtts_GetAtts(const GWEN_EVENT *e){
  GWEN_EVENT_CHGATTS *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CHGATTS, e);
  assert(et);

  return et->atts;
}



int GWEN_EventChgAtts_GetSet(const GWEN_EVENT *e){
  GWEN_EVENT_CHGATTS *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CHGATTS, e);
  assert(et);

  return et->set;
}





GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_CLEAR);

GWEN_EVENT *GWEN_EventClear_new(int x, int y, GWEN_EVENT_CLEAR_MODE m){
  GWEN_EVENT *e;
  GWEN_EVENT_CLEAR *et;

  e=GWEN_Event_new(GWEN_EventType_Clear);
  GWEN_NEW_OBJECT(GWEN_EVENT_CLEAR, et);

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_CLEAR,
                       e, et,
                       GWEN_EventClear_freeData);
  et->x=x;
  et->y=y;
  et->mode=m;
  return e;

}



void GWEN_EventClear_freeData(void *bp, void *p){
  GWEN_EVENT_CLEAR *et;

  et=(GWEN_EVENT_CLEAR*)p;
  GWEN_FREE_OBJECT(et);

}



int GWEN_EventClear_GetX(const GWEN_EVENT *e){
  GWEN_EVENT_CLEAR *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CLEAR, e);
  assert(et);
  return et->x;
}



int GWEN_EventClear_GetY(const GWEN_EVENT *e){
  GWEN_EVENT_CLEAR *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CLEAR, e);
  assert(et);
  return et->y;
}



GWEN_EVENT_CLEAR_MODE GWEN_EventClear_GetMode(const GWEN_EVENT *e){
  GWEN_EVENT_CLEAR *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CLEAR, e);
  assert(et);
  return et->mode;
}








GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_HIGHLIGHT);

GWEN_EVENT *GWEN_EventHighlight_new(int x, int y,
                                    int len,
                                    GWEN_WIDGET_COLOUR hi){
  GWEN_EVENT *e;
  GWEN_EVENT_HIGHLIGHT *et;

  e=GWEN_Event_new(GWEN_EventType_Highlight);
  GWEN_NEW_OBJECT(GWEN_EVENT_HIGHLIGHT, et);
  et->x=x;
  et->y=y;
  et->len=len;
  et->hi=hi;

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_HIGHLIGHT,
                       e, et,
                       GWEN_EventHighlight_freeData);
  return e;
}



void GWEN_EventHighlight_freeData(void *bp, void *p) {
  GWEN_EVENT_HIGHLIGHT *et;

  et=(GWEN_EVENT_HIGHLIGHT*)p;
  GWEN_FREE_OBJECT(et);
}



int GWEN_EventHighlight_GetX(const GWEN_EVENT *e){
  GWEN_EVENT_HIGHLIGHT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_HIGHLIGHT, e);
  assert(et);

  return et->x;
}



int GWEN_EventHighlight_GetY(const GWEN_EVENT *e){
  GWEN_EVENT_HIGHLIGHT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_HIGHLIGHT, e);
  assert(et);

  return et->y;
}



int GWEN_EventHighlight_GetLen(const GWEN_EVENT *e){
  GWEN_EVENT_HIGHLIGHT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_HIGHLIGHT, e);
  assert(et);

  return et->len;
}



GWEN_WIDGET_COLOUR GWEN_EventHighlight_GetHi(const GWEN_EVENT *e){
  GWEN_EVENT_HIGHLIGHT *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_HIGHLIGHT, e);
  assert(et);

  return et->hi;
}





GWEN_INHERIT(GWEN_EVENT, GWEN_EVENT_CONTENTCHG);


GWEN_EVENT *GWEN_EventContentChg_new(int contentWidth, int contentHeight,
                                     int width, int height){
  GWEN_EVENT *e;
  GWEN_EVENT_CONTENTCHG *et;

  e=GWEN_Event_new(GWEN_EventType_ContentChg);
  GWEN_NEW_OBJECT(GWEN_EVENT_CONTENTCHG, et);
  et->width=width;
  et->height=height;
  et->cwidth=contentWidth;
  et->cheight=contentHeight;

  GWEN_INHERIT_SETDATA(GWEN_EVENT, GWEN_EVENT_CONTENTCHG,
                       e, et,
                       GWEN_EventContentChg_freeData);
  return e;
}



void GWEN_EventContentChg_freeData(void *bp, void *p){
  GWEN_EVENT_CONTENTCHG *et;

  et=(GWEN_EVENT_CONTENTCHG*)p;
  GWEN_FREE_OBJECT(et);
}



int GWEN_EventContentChg_GetContentWidth(const GWEN_EVENT *e){
  GWEN_EVENT_CONTENTCHG *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CONTENTCHG, e);
  assert(et);

  return et->cwidth;
}



int GWEN_EventContentChg_GetContentHeight(const GWEN_EVENT *e){
  GWEN_EVENT_CONTENTCHG *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CONTENTCHG, e);
  assert(et);

  return et->cheight;
}



int GWEN_EventContentChg_GetWidth(const GWEN_EVENT *e){
  GWEN_EVENT_CONTENTCHG *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CONTENTCHG, e);
  assert(et);

  return et->width;
}



int GWEN_EventContentChg_GetHeight(const GWEN_EVENT *e){
  GWEN_EVENT_CONTENTCHG *et;

  assert(e);
  et=GWEN_INHERIT_GETDATA(GWEN_EVENT, GWEN_EVENT_CONTENTCHG, e);
  assert(et);

  return et->height;
}
















