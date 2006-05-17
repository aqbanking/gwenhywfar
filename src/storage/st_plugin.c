/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: csv_p.h 120 2003-12-03 23:29:33Z aquamaniac $
 begin       : Thu Oct 30 2003
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "st_plugin_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


GWEN_INHERIT(GWEN_PLUGIN, GWEN_STO_PLUGIN)



GWEN_PLUGIN *GWEN_StoPlugin_new(GWEN_PLUGIN_MANAGER *pm,
                                const char *modName,
                                const char *fileName) {
  GWEN_PLUGIN *pl;
  GWEN_STO_PLUGIN *xpl;

  pl=GWEN_Plugin_new(pm, modName, fileName);
  GWEN_NEW_OBJECT(GWEN_STO_PLUGIN, xpl);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, GWEN_STO_PLUGIN, pl, xpl,
                       GWEN_StoPlugin_FreeData);

  return pl;
}



void GWEN_StoPlugin_FreeData(void *bp, void *p) {
  GWEN_STO_PLUGIN *xpl;

  xpl=(GWEN_STO_PLUGIN*) p;
  GWEN_FREE_OBJECT(xpl);
}



void GWEN_StoPlugin_SetFactoryFn(GWEN_PLUGIN *pl,
                                 GWEN_STO_PLUGIN_FACTORY_FN f) {
  GWEN_STO_PLUGIN *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_STO_PLUGIN, pl);
  assert(xpl);

  xpl->factoryFn=f;
}



GWEN_STO_STORAGE* GWEN_StoPlugin_Factory(GWEN_PLUGIN *pl,
                                         const char *address) {
  GWEN_STO_PLUGIN *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_STO_PLUGIN, pl);
  assert(xpl);

  assert(xpl->factoryFn);
  return xpl->factoryFn(pl, address);
}





