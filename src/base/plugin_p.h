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


#ifndef GWENHYWFAR_PLUGIN_P_H
#define GWENHYWFAR_PLUGIN_P_H

#include "plugin_l.h"
#include <gwenhywfar/stringlist.h>



struct GWEN_PLUGIN_MANAGER {
  GWEN_INHERIT_ELEMENT(GWEN_PLUGIN_MANAGER)
  GWEN_LIST_ELEMENT(GWEN_PLUGIN_MANAGER)
  char *name;
  GWEN_STRINGLIST *paths;
  GWEN_PLUGIN_LIST *plugins;
};



struct GWEN_PLUGIN {
  GWEN_INHERIT_ELEMENT(GWEN_PLUGIN)
  GWEN_LIST_ELEMENT(GWEN_PLUGIN)
  GWEN_PLUGIN_MANAGER *manager;
  char *name;
  char *fileName;
  GWEN_LIBLOADER *libLoader;
  GWEN_TYPE_UINT32 refCount;
};



void GWEN_Plugin_SetLibLoader(GWEN_PLUGIN *p, GWEN_LIBLOADER *ll);
GWEN_PLUGIN *GWEN_PluginManager__FindPlugin(GWEN_PLUGIN_MANAGER *pm,
                                            const char *s);






#endif


