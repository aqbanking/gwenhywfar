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





int GWEN_Parser__CheckElementAndChildren(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData, int depth) {
  int rv;
  const GWEN_PARSER_ELEMENT *dReal=eDefinitions;
  const GWEN_PARSER_ELEMENT *eDefChild=NULL;
  const GWEN_PARSER_ELEMENT *eDataChild=NULL;

  /* check choice or direct element */
  if (GWEN_ParserElement_GetElementType(eDefinitions)==GWEN_ParserElementType_Choice) {
    /* check choice */
    dReal=GWEN_Parser__GetChoice(eDefinitions, eData);
    if (dReal==NULL) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "No matching choice found");
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    /* compare directly */
    rv=GWEN_Parser__CheckElement(eDefinitions, eData);
    if (rv<0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* check children */
  eDefChild=GWEN_ParserElement_Tree_GetFirstChild(dReal);
  if (eData)
    eDataChild=GWEN_ParserElement_Tree_GetFirstChild(eData);

  rv=GWEN_Parser__CheckSequence(eDefChild, eDataChild, depth+1);
  if (rv<0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_Parser__CheckSequence(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData, int depth) {
  const GWEN_PARSER_ELEMENT *d;
  const GWEN_PARSER_ELEMENT *e;
  int count=0;

  d=eDefinitions;
  e=eData;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Entering sequence [%d]", depth);

  while(d) {
    int rv;

    DBG_VERBOUS(GWEN_LOGDOMAIN, "Checking definition \"%s\" (%s) against \"%s\" (%s) [%d]",
                d?GWEN_ParserElement_GetName(d):"-?-",
                GWEN_ParserElementType_toString(GWEN_ParserElement_GetElementType(d)),
                e?(GWEN_ParserElement_GetName(e)):"-NULL-",
                e?(GWEN_ParserElementType_toString(GWEN_ParserElement_GetElementType(e))):"-NULL-", depth);

    rv=GWEN_Parser__CheckElementAndChildren(d, e, depth+1);
    if (rv==0) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Matches [%d]", depth);
      /* does match */
      if ((GWEN_ParserElement_GetMaxOccurs(d)==-1)|| (count<GWEN_ParserElement_GetMaxOccurs(d))) {
        /* number is ok, advance to next */

        DBG_VERBOUS(GWEN_LOGDOMAIN, "Element \"%s\" (%s) is ok (%d) [%d]",
                    e?(GWEN_ParserElement_GetName(e)):"-NULL-",
                    e?(GWEN_ParserElementType_toString(GWEN_ParserElement_GetElementType(e))):"-NULL-",
                    count+1, depth);
        count++;
        if (e)
          e=GWEN_ParserElement_Tree_GetNext(e);
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "Too many counts of this element (%d, maxOccurs=%d)",
                 count, GWEN_ParserElement_GetMaxOccurs(d));
        return GWEN_ERROR_BAD_DATA;
      }
    }
    else {
      /* does not match */

      DBG_VERBOUS(GWEN_LOGDOMAIN, "Does not match [%d]", depth);
      if (count<GWEN_ParserElement_GetMinOccurs(d)) {
        /* too few counts */
        DBG_INFO(GWEN_LOGDOMAIN, "Too few counts of element \"%s\" ([%s], got %d, minOccurs=%d) [%d]",
                 d?GWEN_ParserElement_GetName(d):"-?-",
                 GWEN_ParserElementType_toString(GWEN_ParserElement_GetElementType(d)),
                 count, GWEN_ParserElement_GetMinOccurs(d), depth);
        return GWEN_ERROR_BAD_DATA;
      }
      else {
        /* ok, advance to next definition */
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Element \"%s\" (%s) does not match, but that's ok [%d]",
                    e?(GWEN_ParserElement_GetName(e)):"-NULL-",
                    e?(GWEN_ParserElementType_toString(GWEN_ParserElement_GetElementType(e))):"-NULL-", depth);
        count=0;
        d=GWEN_ParserElement_Tree_GetNext(d);
      }
    }
  }

  if (e) {
    DBG_INFO(GWEN_LOGDOMAIN, "Still data elements but no definition elements");
    return GWEN_ERROR_BAD_DATA;
  }

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Leaving sequence [%d]", depth);

  return 0;
}



int GWEN_Parser_CheckTree(const GWEN_PARSER_ELEMENT_TREE *tDefinitions, const GWEN_PARSER_ELEMENT_TREE *tData) {
  const GWEN_PARSER_ELEMENT *d;
  const GWEN_PARSER_ELEMENT *e;
  int rv;

  d=GWEN_ParserElement_Tree_GetFirst(tDefinitions);
  e=GWEN_ParserElement_Tree_GetFirst(tData);
  rv=GWEN_Parser__CheckSequence(d, e, 0);
  if (rv<0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}






