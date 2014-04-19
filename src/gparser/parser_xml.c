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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/parser_xml.h>
#include <gwenhywfar/debug.h>




int GWEN_ParserXml__Read(GWEN_PARSER_ELEMENT_TREE *et, GWEN_PARSER_ELEMENT *eParent, GWEN_XMLNODE *node) {
  GWEN_PARSER_ELEMENT *e=NULL;
  const char *s;

  switch(GWEN_XMLNode_GetType(node)) {
  case GWEN_XMLNodeTypeTag:
    e=GWEN_ParserElement_new();
    GWEN_ParserElement_SetType(e, GWEN_ParserElementType_Element);
    s=GWEN_XMLNode_GetData(node);
    if (s && *s)
      GWEN_ParserElement_SetName(e, s);
    /* TODO: Read attributes */
    break;
  case GWEN_XMLNodeTypeData:
    e=GWEN_ParserElement_new();
    GWEN_ParserElement_SetElementType(e, GWEN_ParserElementType_Data);
    s=GWEN_XMLNode_GetData(node);
    if (s && *s)
      GWEN_ParserElement_SetData(e, s);
    break;
  case GWEN_XMLNodeTypeComment:
    e=NULL;
    break;
  }

  if (e) {
    GWEN_XMLNODE *n;
    const char *name;

    name=GWEN_ParserElement_GetName(e);
    DBG_INFO(GWEN_LOGDOMAIN, "Adding element \"%s\" (%s)",
             name?name:"-?-",
             GWEN_ParserElementType_toString(GWEN_ParserElement_GetElementType(e)));

    if (eParent)
      GWEN_ParserElement_Tree_AddChild(eParent, e);
    else
      GWEN_ParserElement_Tree_Add(et, e);

    n=GWEN_XMLNode_GetChild(node);
    while(n) {
      int rv;

      rv=GWEN_ParserXml__Read(et, e, n);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }

      n=GWEN_XMLNode_Next(n);
    }
  }

  return 0;
}




int GWEN_ParserXml_Read(GWEN_PARSER_ELEMENT_TREE *et, GWEN_XMLNODE *node) {
  return GWEN_ParserXml__Read(et, NULL, node);
}



int GWEN_ParserXml_ReadFile(GWEN_PARSER_ELEMENT_TREE *et, const char *fname) {
  int rv;
  GWEN_XMLNODE *rootNode;
  GWEN_XMLNODE *n;

  rootNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(rootNode, fname, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_HANDLE_HEADERS);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(rootNode);
    return rv;
  }

  n=GWEN_XMLNode_GetChild(rootNode);
  while(n) {
    int rv;

    rv=GWEN_ParserXml_Read(et, n);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_XMLNode_free(rootNode);
      return rv;
    }
    n=GWEN_XMLNode_Next(n);
  }

  GWEN_XMLNode_free(rootNode);
  return 0;
}






