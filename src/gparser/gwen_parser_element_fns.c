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


#include "gwen_parser_element_fns.h"




GWEN_PARSER_ELEMENT *GWEN_ParserElement_fromXml(GWEN_XMLNODE *node) {
  GWEN_PARSER_ELEMENT *e;
  const char *s;
  int i;

  e=GWEN_ParserElement_new();

  s=GWEN_XMLNode_GetProperty(node, "elementType", "element");
  if (s && *s) {
    GWEN_PARSER_ELEMENT_TYPE et;

    et=GWEN_ParserElementType_fromString(s);
    if (et==GWEN_ParserElementType_Unknown) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid element type \"%s\"", s);
      GWEN_ParserElement_free(e);
      return NULL;
    }
    e->elementType=et;
  }

  s=GWEN_XMLNode_GetProperty(node, "name", NULL);
  if (s && *s)
    GWEN_ParserElement_SetName(e, s);

  s=GWEN_XMLNode_GetProperty(node, "type", NULL);
  if (s && *s)
    GWEN_ParserElement_SetType(e, s);

  s=GWEN_XMLNode_GetProperty(node, "dbName", NULL);
  if (s && *s)
    GWEN_ParserElement_SetDbName(e, s);

  i=GWEN_XMLNode_GetIntProperty(node, "minOccurs", 1);
  GWEN_ParserElement_SetMinOccurs(e, i);

  i=GWEN_XMLNode_GetIntProperty(node, "maxOccurs", 1);
  GWEN_ParserElement_SetMaxOccurs(e, i);

  i=GWEN_XMLNode_GetIntProperty(node, "minSize", 0);
  GWEN_ParserElement_SetMinSize(e, i);

  i=GWEN_XMLNode_GetIntProperty(node, "maxSize", -1);
  GWEN_ParserElement_SetMaxSize(e, i);

  return e;
}



int GWEN_ParserElement_toXml(const GWEN_PARSER_ELEMENT *e, GWEN_XMLNODE *node) {

  GWEN_XMLNode_SetProperty(node, "name", GWEN_ParserElementType_toString(e->elementType));

  if (e->name)
    GWEN_XMLNode_SetProperty(node, "name", e->name);

  if (e->type)
    GWEN_XMLNode_SetProperty(node, "type", e->type);

  if (e->dbName)
    GWEN_XMLNode_SetProperty(node, "dbName", e->dbName);

  GWEN_XMLNode_SetIntProperty(node, "minOccurs", e->minOccurs);
  GWEN_XMLNode_SetIntProperty(node, "maxOccurs", e->maxOccurs);

  GWEN_XMLNode_SetIntProperty(node, "minSize", e->minSize);
  GWEN_XMLNode_SetIntProperty(node, "maxSize", e->maxSize);

  return 0;
}





int GWEN_ParserElement_Tree__ReadXml(GWEN_PARSER_ELEMENT_TREE *et, GWEN_PARSER_ELEMENT *eParent, GWEN_XMLNODE *node) {
  GWEN_PARSER_ELEMENT *e;
  GWEN_XMLNODE *n;

  e=GWEN_ParserElement_fromXml(node);
  if (e==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Adding element \"%s\" (%s)",
           (e->name)?(e->name):"-?-",
           GWEN_ParserElementType_toString(e->elementType));

  if (eParent)
    GWEN_ParserElement_Tree_AddChild(eParent, e);
  else
    GWEN_ParserElement_Tree_Add(et, e);

  /* read sub-elements */
  n=GWEN_XMLNode_FindFirstTag(node, "element", NULL, NULL);
  while(n) {
    int rv;

    rv=GWEN_ParserElement_Tree__ReadXml(et, e, n);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    n=GWEN_XMLNode_FindNextTag(n, "element", NULL, NULL);
  }

  return 0;
}



int GWEN_ParserElement_Tree_ReadXml(GWEN_PARSER_ELEMENT_TREE *et, GWEN_XMLNODE *node) {
  return GWEN_ParserElement_Tree__ReadXml(et, NULL, node);
}





int GWEN_ParserElement_Tree_ReadXmlFile(GWEN_PARSER_ELEMENT_TREE *et, const char *fname) {
  int rv;
  GWEN_XMLNODE *rootNode;
  GWEN_XMLNODE *n;

  rootNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(rootNode, fname, GWEN_XML_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(rootNode);
    return rv;
  }

  n=GWEN_XMLNode_FindFirstTag(rootNode, "gwen-parser-def", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "element", NULL, NULL);
    while(nn) {
      int rv;
  
      rv=GWEN_ParserElement_Tree_ReadXml(et, nn);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_XMLNode_free(rootNode);
        return rv;
      }

      nn=GWEN_XMLNode_FindNextTag(nn, "element", NULL, NULL);
    }
  }

  GWEN_XMLNode_free(rootNode);
  return 0;
}




int GWEN_ParserElement_Tree__WriteXml(const GWEN_PARSER_ELEMENT *e, GWEN_XMLNODE *node) {
  const GWEN_PARSER_ELEMENT *eChild;
  GWEN_XMLNODE *n;
  int rv;

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "element");
  rv=GWEN_ParserElement_toXml(e, n);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(n);
    return rv;
  }

  eChild=GWEN_ParserElement_Tree_GetFirstChild(e);
  while(eChild) {
    rv=GWEN_ParserElement_Tree__WriteXml(eChild, n);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_XMLNode_free(n);
      return rv;
    }
    eChild=GWEN_ParserElement_Tree_GetNext(eChild);
  }

  GWEN_XMLNode_AddChild(node, n);
  return 0;
}



int GWEN_ParserElement_Tree_WriteXml(const GWEN_PARSER_ELEMENT_TREE *et, GWEN_XMLNODE *node) {
  const GWEN_PARSER_ELEMENT *e;
  int rv;

  e=GWEN_ParserElement_Tree_GetFirst(et);
  while(e) {
    rv=GWEN_ParserElement_Tree__WriteXml(e, node);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    e=GWEN_ParserElement_Tree_GetNext(e);
  }

  return 0;
}



int GWEN_ParserElement_Tree_WriteXmlFile(GWEN_PARSER_ELEMENT_TREE *et, const char *fname) {
  int rv;
  GWEN_XMLNODE *rootNode;
  GWEN_XMLNODE *n;

  rootNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "gwen-parser-def");
  GWEN_XMLNode_AddChild(rootNode, n);

  rv=GWEN_ParserElement_Tree_WriteXml(et, n);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(rootNode);
    return rv;
  }

  rv=GWEN_XMLNode_WriteFile(rootNode, fname, GWEN_XML_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(rootNode);
    return rv;
  }


  GWEN_XMLNode_free(rootNode);
  return 0;
}






void GWEN_ParserElement_Dump(const GWEN_PARSER_ELEMENT *e, int indent) {
  int i;

  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "Element \"%s\"\n", GWEN_ParserElementType_toString(e->elementType));

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- Name     : \"%s\"\n", (e->name)?(e->name):"-?-");

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- DbName   : \"%s\"\n", (e->dbName)?(e->dbName):"-?-");

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- Type     : \"%s\"\n", (e->type)?(e->type):"-?-");

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- MinOccurs: %d\n", e->minOccurs);

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- MaxOccurs: %d\n", e->maxOccurs);

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- MinSize  : %d\n", e->minSize);

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- MaxSize  : %d\n", e->maxSize);

  for (i=0; i<indent+1; i++) fprintf(stderr, " ");
  fprintf(stderr, "- Data     : \"%s\"\n", (e->data)?(e->data):"-?-");

}



void GWEN_ParserElement_DumpWithChildren(const GWEN_PARSER_ELEMENT *e, int indent) {
  const GWEN_PARSER_ELEMENT *eChild;

  GWEN_ParserElement_Dump(e, indent);
  eChild=GWEN_ParserElement_Tree_GetFirstChild(e);
  while(eChild) {
    GWEN_ParserElement_DumpWithChildren(eChild, indent+2);
    eChild=GWEN_ParserElement_Tree_GetNext(eChild);
  }
}



void GWEN_ParserElement_Tree_Dump(const GWEN_PARSER_ELEMENT_TREE *et, int indent) {
  const GWEN_PARSER_ELEMENT *e;
  int i;

  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "ParserElement tree:\n");

  e=GWEN_ParserElement_Tree_GetFirst(et);
  while(e) {
    GWEN_ParserElement_DumpWithChildren(e, indent+2);
    e=GWEN_ParserElement_Tree_GetNext(e);
  }
}







