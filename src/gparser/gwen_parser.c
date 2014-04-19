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

#include "gwen_parser_p.h"

#include <gwenhywfar/debug.h>


/* pull in check code */
#include "gwen_parser_check.c"
#include "gwen_parser_update.c"



/* Checks whether the given eData matches the given eDefinitions element */
int GWEN_Parser__CheckElement(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData) {
  GWEN_PARSER_ELEMENT_TYPE tDefinitions, tData;
  const char *nameDefinitions, *nameData;

  if (eDefinitions==NULL) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "No definition element");
    return GWEN_ERROR_BAD_DATA;
  }

  if (eData==NULL) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "No data element");
    return GWEN_ERROR_BAD_DATA;
  }

  /* compare data type */
  tDefinitions=GWEN_ParserElement_GetElementType(eDefinitions);
  tData=GWEN_ParserElement_GetElementType(eData);
  if (tDefinitions!=tData) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Unexpected element type (expected \%s\", found \%s\")",
              GWEN_ParserElementType_toString(tDefinitions),
              GWEN_ParserElementType_toString(tData));
    return GWEN_ERROR_BAD_DATA;
  }

  /* compare element name */
  nameDefinitions=GWEN_ParserElement_GetName(eDefinitions);
  if (nameDefinitions && !(*nameDefinitions))
    nameDefinitions=NULL;
  nameData=GWEN_ParserElement_GetName(eData);
  if (nameData && !(*nameData))
    nameData=NULL;

  if (!((nameDefinitions==NULL && nameData==NULL) ||
        (nameDefinitions && nameData && strcasecmp(nameDefinitions, nameData)==0))) {
    DBG_DEBUG(GWEN_LOGDOMAIN,
              "Unexpected element name (expected \"%s\", got \"%s\")",
              nameDefinitions?nameDefinitions:"<-->",
              nameData?nameData:"<-->");
    return GWEN_ERROR_BAD_DATA;
  }

  return 0;
}



const GWEN_PARSER_ELEMENT *GWEN_Parser__GetChoice(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData) {
  GWEN_PARSER_ELEMENT *e;

  e=GWEN_ParserElement_Tree_GetFirstChild(eDefinitions);
  while(e) {
    if (0==GWEN_Parser__CheckElement(e, eData))
      return e;
    e=GWEN_ParserElement_Tree_GetNext(e);
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "No matching choice found");
  return NULL;
}









