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

#ifndef GWEN_UI_HLINE_P_H
#define GWEN_UI_HLINE_P_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/widget.h>
#include <gwenhywfar/ui/hline.h>

typedef struct GWEN_HLINE GWEN_HLINE;


struct GWEN_HLINE {
  GWEN_WIDGET_EVENTHANDLER_FN previousHandler;
  GWEN_TYPE_UINT32 flags;
};
void GWEN_HLine_freeData(void *bp, void *p);


GWEN_UI_RESULT GWEN_HLine_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e);

#endif








