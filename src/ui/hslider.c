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


#include "hslider_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>


GWEN_INHERIT(GWEN_WIDGET, GWEN_HSLIDER)



GWEN_WIDGET *GWEN_HSlider_new(GWEN_WIDGET *parent,
                              GWEN_TYPE_UINT32 flags,
                              const char *name){
  GWEN_WIDGET *w;
  GWEN_HSLIDER *win;
  int xoffs;
  int yoffs;
  int woffs;

  assert(parent);
  xoffs=0;
  yoffs=0;
  woffs=0;
  if (flags & GWEN_WIDGET_FLAGS_BORDER) {
    xoffs++;
    yoffs++;
    woffs-=2;
  }

  w=GWEN_Widget_new(parent,
                    flags,
                    name, 0,
                    xoffs,
                    GWEN_Widget_GetHeight(parent)-1-yoffs,
                    GWEN_Widget_GetWidth(parent)-2-woffs,
                    1);
  GWEN_NEW_OBJECT(GWEN_HSLIDER, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_HSLIDER, w, win,
                       GWEN_HSlider_freeData);

  return w;
}



void GWEN_HSlider_freeData(void *bp, void *p) {
  GWEN_HSLIDER *win;

  win=(GWEN_HSLIDER*)p;
  GWEN_FREE_OBJECT(win);
}







