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


#ifndef GWENHYWFAR_PLUGINDESCR_H
#define GWENHYWFAR_PLUGINDESCR_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>

#ifdef __cplusplus
extern "C" {
#endif

GWENHYWFAR_API
typedef struct GWEN_PLUGIN_DESCRIPTION GWEN_PLUGIN_DESCRIPTION;

GWEN_LIST_FUNCTION_DEFS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription);
GWEN_LIST2_FUNCTION_DEFS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription);


GWENHYWFAR_API
void GWEN_PluginDescription_free(GWEN_PLUGIN_DESCRIPTION *pd);

GWENHYWFAR_API
void GWEN_PluginDescription_free(GWEN_PLUGIN_DESCRIPTION *pd);

GWENHYWFAR_API
void GWEN_PluginDescription_freeAll(GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl);

GWENHYWFAR_API
const char *GWEN_PluginDescription_GetName(const GWEN_PLUGIN_DESCRIPTION *pd);

GWENHYWFAR_API
const char *GWEN_PluginDescription_GetType(const GWEN_PLUGIN_DESCRIPTION *pd);

GWENHYWFAR_API
const char*
  GWEN_PluginDescription_GetShortDescr(const GWEN_PLUGIN_DESCRIPTION *pd);

GWENHYWFAR_API
const char*
  GWEN_PluginDescription_GetAuthor(const GWEN_PLUGIN_DESCRIPTION *pd);

GWENHYWFAR_API
const char*
  GWEN_PluginDescription_GetVersion(const GWEN_PLUGIN_DESCRIPTION *pd);

GWENHYWFAR_API
const char*
  GWEN_PluginDescription_GetLongDescr(const GWEN_PLUGIN_DESCRIPTION *pd);


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_PLUGINDESCR_H */
