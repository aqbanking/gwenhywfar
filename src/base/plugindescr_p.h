/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 12 2003
    copyright   : (C) 2003 by Martin Preuss
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


#ifndef GWENHYWFAR_PLUGINDESCR_P_H
#define GWENHYWFAR_PLUGINDESCR_P_H

#include <gwenhywfar/plugindescr.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/xml.h>


struct GWEN_PLUGIN_DESCRIPTION {
  GWEN_LIST_ELEMENT(GWEN_PLUGIN_DESCRIPTION);

  char *name;
  char *type;
  char *shortDescr;
  char *author;
  char *version;
  char *longDescr;
  int isActive;
  GWEN_XMLNODE *xmlNode;
};

GWEN_PLUGIN_DESCRIPTION *GWEN_PluginDescription_new(GWEN_XMLNODE *node);

GWEN_PLUGIN_DESCRIPTION*
GWEN_PluginDescription_List2_freeAll_cb(GWEN_PLUGIN_DESCRIPTION *pd);


#endif /* GWENHYWFAR_PLUGINDESCR_P_H */
