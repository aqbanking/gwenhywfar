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

#ifndef GWEN_UI_LOADER_P_H
#define GWEN_UI_LOADER_P_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/widget.h>
#include <gwenhywfar/ui/loader.h>



int GWEN_UILoader__ParseWidget(GWEN_XMLNODE *n,
                               GWEN_DB_NODE *db,
                               int *x, int *y,
                               int width, int height);

int GWEN_UILoader__ParseVGroup(GWEN_XMLNODE *n,
                               GWEN_DB_NODE *db,
                               int *x, int *y,
                               int width, int height);
int GWEN_UILoader__ParseHGroup(GWEN_XMLNODE *n,
                               GWEN_DB_NODE *db,
                               int *x, int *y,
                               int width, int height);

int GWEN_UILoader__AdjustWidgetsPos(GWEN_DB_NODE *db,
                                    int x, int y);


GWEN_WIDGET *GWEN_UILoader_LoadButton(GWEN_WIDGET *parent,
                                      GWEN_DB_NODE *db,
                                      GWEN_XMLNODE *n);
GWEN_WIDGET *GWEN_UILoader_LoadWindow(GWEN_WIDGET *parent,
                                      GWEN_DB_NODE *db,
                                      GWEN_XMLNODE *n);
GWEN_WIDGET *GWEN_UILoader_LoadEditBox(GWEN_WIDGET *parent,
                                       GWEN_DB_NODE *db,
                                       GWEN_XMLNODE *n);
GWEN_WIDGET *GWEN_UILoader_LoadCheckBox(GWEN_WIDGET *parent,
                                        GWEN_DB_NODE *db,
                                        GWEN_XMLNODE *n);
GWEN_WIDGET *GWEN_UILoader_LoadDropDownBox(GWEN_WIDGET *parent,
                                           GWEN_DB_NODE *db,
                                           GWEN_XMLNODE *n);
GWEN_WIDGET *GWEN_UILoader_LoadTextWidget(GWEN_WIDGET *parent,
                                          GWEN_DB_NODE *db,
                                          GWEN_XMLNODE *n);
GWEN_WIDGET *GWEN_UILoader_LoadTableWidget(GWEN_WIDGET *parent,
                                           GWEN_DB_NODE *db,
                                           GWEN_XMLNODE *n);







#endif





