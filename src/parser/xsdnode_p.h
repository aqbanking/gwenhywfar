/***************************************************************************
 begin       : Wed Feb 27 2008
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

#ifndef GWENHYWFAR_XSDNODE_P_H
#define GWENHYWFAR_XSDNODE_P_H

#include "xsdnode_l.h"


struct GWEN_XSD_NODE {
  GWEN_INHERIT_ELEMENT(GWEN_XSD_NODE);
  GWEN_LIST_ELEMENT(GWEN_XSD_NODE);

  GWEN_XSD_NODETYPE nodeType;
  char *name;

  char *defaultValue;
  char *fixedValue;

  int minOccurrs;
  int maxOccurrs;

  uint32_t flags;

  GWEN_XSD_NODE *parent;
  GWEN_XSD_NODE_LIST *children;

  GWEN_XSDNODE_READ_FN readFn;
  GWEN_XSDNODE_WRITE_FN writeFn;
};


#endif

