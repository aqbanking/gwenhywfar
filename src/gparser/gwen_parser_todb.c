/***************************************************************************
 begin       : Fri Jul 25 2014
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





int GWEN_Parser__toDbElementAndChildren(GWEN_PARSER_ELEMENT *eData, GWEN_DB_NODE *dbParent, int depth) {
  int rv;
  GWEN_PARSER_ELEMENT *eChild;
  const char *s;
  const char *groupName;

  groupName=GWEN_ParserElement_GetDbName(eData);
  if (groupName) {
    s=GWEN_ParserElement_GetData(eData);
    if (s && *s)
      GWEN_DB_SetCharValue(dbParent, GWEN_DB_FLAGS_DEFAULT, groupName, s);
  }

  /* check children */
  eChild=GWEN_ParserElement_Tree_GetFirstChild(eData);
  if (eChild) {
    GWEN_DB_NODE *dbThis=NULL;

    if (groupName && *groupName)
      dbThis=GWEN_DB_GetGroup(dbParent, GWEN_DB_FLAGS_DEFAULT, groupName);
    while(eChild) {
      rv=GWEN_Parser__toDbElementAndChildren(eChild, dbThis?dbThis:dbParent, depth+1);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d) [%d]", rv, depth);
        return rv;
      }

      GWEN_ParserElement_Tree_GetNext(eChild);
    }
  }

  return 0;
}



int GWEN_Parser_ToDbTree(GWEN_PARSER_ELEMENT_TREE *tData, GWEN_DB_NODE *db) {
  GWEN_PARSER_ELEMENT *e;

  e=GWEN_ParserElement_Tree_GetFirst(tData);
  while(e) {
    int rv;

    rv=GWEN_Parser__toDbElementAndChildren(e, db, 0);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    e=GWEN_ParserElement_Tree_GetNext(e);
  }

  return 0;
}






