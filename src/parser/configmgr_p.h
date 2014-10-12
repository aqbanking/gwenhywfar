/***************************************************************************
 begin       : Mon Aug 11 2008
 copyright   : (C) 2008 by Martin Preuss
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


#ifndef GWENHYWFAR_CONFIGMGR_P_H
#define GWENHYWFAR_CONFIGMGR_P_H

#include "configmgr_l.h"


struct GWEN_CONFIGMGR {
  GWEN_INHERIT_ELEMENT(GWEN_CONFIGMGR)

  GWEN_CONFIGMGR_GETGROUP_FN getGroupFn;
  GWEN_CONFIGMGR_SETGROUP_FN setGroupFn;

  GWEN_CONFIGMGR_LOCKGROUP_FN lockGroupFn;
  GWEN_CONFIGMGR_UNLOCKGROUP_FN unlockGroupFn;

  GWEN_CONFIGMGR_GETUNIQUEID_FN getUniqueIdFn;
  GWEN_CONFIGMGR_DELETEGROUP_FN deleteGroupFn;

  GWEN_CONFIGMGR_LISTGROUPS_FN listGroupsFn;
  GWEN_CONFIGMGR_LISTSUBGROUPS_FN listSubGroupsFn;

  char *url;
};



typedef struct GWEN_CONFIGMGR_PLUGIN GWEN_CONFIGMGR_PLUGIN;
struct GWEN_CONFIGMGR_PLUGIN {
  GWEN_CONFIGMGR_PLUGIN_FACTORYFN factoryFn;
};

void GWENHYWFAR_CB GWEN_ConfigMgr_Plugin_FreeData(void *bp, void *p);

GWEN_CONFIGMGR *GWEN_ConfigMgr_Plugin_Factory(GWEN_PLUGIN *pl,
    const char *url);



#endif

