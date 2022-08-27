/***************************************************************************
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


#ifndef GWENHYWFAR_PLUGIN_L_H
#define GWENHYWFAR_PLUGIN_L_H

#include "plugin.h"
#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTION_DEFS(GWEN_PLUGIN, GWEN_Plugin)
GWEN_LIST_FUNCTION_DEFS(GWEN_PLUGIN_MANAGER, GWEN_PluginManager)


int GWEN_Plugin_ModuleInit(void);
int GWEN_Plugin_ModuleFini(void);

void GWEN_Plugin_Attach(GWEN_PLUGIN *p);




#endif


