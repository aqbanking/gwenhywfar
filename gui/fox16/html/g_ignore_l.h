/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTML_G_IGNORE_L_H
#define HTML_G_IGNORE_L_H


#include "htmlgroup_l.h"



HTML_GROUP *HtmlGroup_Ignore_new(const char *groupName,
				 HTML_GROUP *parent,
				 GWEN_XML_CONTEXT *ctx);


#endif
