/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
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

#ifndef GWENHYWFAR_XML_L_H
#define GWENHYWFAR_XML_L_H

#include <gwenhywfar/xml.h>


/** The type of a property of a tag (in XML notation this would be
    called the attribute of an element). */
typedef struct GWEN__XMLPROPERTY GWEN_XMLPROPERTY;


GWEN_XMLPROPERTY *GWEN_XMLNode_GetFirstProperty(const GWEN_XMLNODE *n);
GWEN_XMLPROPERTY *GWEN_XMLNode_GetNextProperty(const GWEN_XMLNODE *n,
                                               const GWEN_XMLPROPERTY *pr);

const char *GWEN_XMLProperty_GetName(const GWEN_XMLPROPERTY *pr);
const char *GWEN_XMLProperty_GetValue(const GWEN_XMLPROPERTY *pr);

#endif

