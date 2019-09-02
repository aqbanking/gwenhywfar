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

#ifndef GWENHYWFAR_XSDNODE_L_H
#define GWENHYWFAR_XSDNODE_L_H

#include <gwenhywfar/xsdnode.h>


typedef int GWENHYWFAR_CB (*GWEN_XSDNODE_READ_FN)(GWEN_XSD_NODE *xsdNode,
                                                  GWEN_XMLNODE *xmlNode,
                                                  GWEN_DB_NODE *db);
typedef int GWENHYWFAR_CB (*GWEN_XSDNODE_WRITE_FN)(GWEN_XSD_NODE *xsdNode,
                                                   GWEN_XMLNODE *xmlNode,
                                                   GWEN_DB_NODE *db);

GWEN_XSDNODE_READ_FN GWEN_XsdNode_SetReadFn(GWEN_XSD_NODE *xsdNode,
    GWEN_XSDNODE_READ_FN fn);
GWEN_XSDNODE_WRITE_FN GWEN_XsdNode_SetWriteFn(GWEN_XSD_NODE *xsdNode,
    GWEN_XSDNODE_WRITE_FN fn);

#endif

