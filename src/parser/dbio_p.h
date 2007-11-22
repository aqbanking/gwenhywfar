/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Wed Mar 31 2004
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


/** @file gwenhywfar/dbio.h */

#ifndef GWENHYWFAR_DBIO_P_H
#define GWENHYWFAR_DBIO_P_H


#include "dbio_l.h"
#include <gwenhywfar/libloader.h>


typedef struct GWEN_DBIO_PLUGIN GWEN_DBIO_PLUGIN;
struct GWEN_DBIO_PLUGIN {
  GWEN_DBIO_PLUGIN_FACTORYFN factoryFn;
};
void GWENHYWFAR_CB GWEN_DBIO_Plugin_FreeData(void *bp, void *p);



struct GWEN_DBIO {
  GWEN_LIST_ELEMENT(GWEN_DBIO)
  GWEN_INHERIT_ELEMENT(GWEN_DBIO)
  /* No trailing semicolon here because this is a macro call */
  GWEN_DBIO_IMPORTFN importFn;
  GWEN_DBIO_EXPORTFN exportFn;
  GWEN_DBIO_CHECKFILEFN checkFileFn;

  char *name;
  char *descr;

  uint32_t usage;
};


#endif /* GWENHYWFAR_DBIO_P_H */


