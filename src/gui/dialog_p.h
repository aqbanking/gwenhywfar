/***************************************************************************
    begin       : Wed Jan 20 2010
    copyright   : (C) 2010 by Martin Preuss
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


#ifndef GWENHYWFAR_GUI_DIALOG_P_H
#define GWENHYWFAR_GUI_DIALOG_P_H

#include <gwenhywfar/dialog_be.h>


struct GWEN_DIALOG {
  GWEN_INHERIT_ELEMENT(GWEN_DIALOG)
  GWEN_LIST_ELEMENT(GWEN_DIALOG)

  char *dialogId;

  uint32_t flags;
  GWEN_WIDGET_TREE *widgets;

  GWEN_DIALOG_SIGNALHANDLER signalHandler;

  GWEN_DIALOG_SETINTPROPERTY_FN setIntPropertyFn;
  GWEN_DIALOG_GETINTPROPERTY_FN getIntPropertyFn;
  GWEN_DIALOG_SETCHARPROPERTY_FN setCharPropertyFn;
  GWEN_DIALOG_GETCHARPROPERTY_FN getCharPropertyFn;

  GWEN_DIALOG_LIST *subDialogs;
  GWEN_WIDGET *parentWidget;

  uint32_t refCount;
};



static
int GWEN_Dialog__ReadXmlWidget(GWEN_DIALOG *dlg,
			       GWEN_WIDGET *wparent,
			       GWEN_XMLNODE *node);


#endif
