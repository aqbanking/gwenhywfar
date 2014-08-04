/***************************************************************************
 begin       : Fri Apr 18 2014
 copyright   : (C) 2014 by Martin Preuss
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


#ifndef GWEN_PARSER_ELEMENT_FNS_H
#define GWEN_PARSER_ELEMENT_FNS_H


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/xml.h>



GWENHYWFAR_API GWEN_PARSER_ELEMENT *GWEN_ParserElement_fromXml(GWEN_XMLNODE *node);
GWENHYWFAR_API int GWEN_ParserElement_toXml(const GWEN_PARSER_ELEMENT *e, GWEN_XMLNODE *node);

GWENHYWFAR_API int GWEN_ParserElement_Tree_ReadXml(GWEN_PARSER_ELEMENT_TREE *et, GWEN_XMLNODE *node);
GWENHYWFAR_API int GWEN_ParserElement_Tree_ReadXmlFile(GWEN_PARSER_ELEMENT_TREE *et, const char *fname);

GWENHYWFAR_API int GWEN_ParserElement_Tree_WriteXml(const GWEN_PARSER_ELEMENT_TREE *et, GWEN_XMLNODE *node);
GWENHYWFAR_API int GWEN_ParserElement_Tree_WriteXmlFile(GWEN_PARSER_ELEMENT_TREE *et, const char *fname);


GWENHYWFAR_API void GWEN_ParserElement_Dump(const GWEN_PARSER_ELEMENT *e, int indent);
GWENHYWFAR_API void GWEN_ParserElement_DumpWithChildren(const GWEN_PARSER_ELEMENT *e, int indent);
GWENHYWFAR_API void GWEN_ParserElement_Tree_Dump(const GWEN_PARSER_ELEMENT_TREE *et, int indent);




#endif

