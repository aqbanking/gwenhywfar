/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTML_G_GENERIC_P_H
#define HTML_G_GENERIC_P_H


#include "g_generic_l.h"


static int HtmlGroup_Generic_EndTag(HTML_GROUP *g, const char *tagName);
static int HtmlGroup_Generic_AddData(HTML_GROUP *g, const char *data);
static int HtmlGroup_Generic_EndSubGroup(HTML_GROUP *g, HTML_GROUP *sg);

#endif

