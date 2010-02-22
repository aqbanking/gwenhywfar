/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLGROUP_P_H
#define HTMLGROUP_P_H


#include "htmlgroup_l.h"


struct HTML_GROUP {
  GWEN_INHERIT_ELEMENT(HTML_GROUP)

  HTML_GROUP *parent;
  GWEN_XML_CONTEXT *xmlContext;

  HTML_PROPS *properties;

  char *groupName;

  HTML_GROUP_STARTTAG_FN startTagFn;
  HTML_GROUP_ENDTAG_FN endTagFn;
  HTML_GROUP_ADDDATA_FN addDataFn;
  HTML_GROUP_ENDSUBGROUP_FN endSubGroupFn;

  HTML_OBJECT *object;
};



#endif

