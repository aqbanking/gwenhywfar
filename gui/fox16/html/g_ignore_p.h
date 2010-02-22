/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTML_G_IGNORE_P_H
#define HTML_G_IGNORE_P_H


#include "g_ignore_l.h"

#include <gwenhywfar/stringlist.h>



typedef struct HTML_GROUP_IGNORE HTML_GROUP_IGNORE;
struct HTML_GROUP_IGNORE {
  GWEN_STRINGLIST *openTags;
};


static void GWENHYWFAR_CB HtmlGroup_Ignore_FreeData(void *bp, void *p);


static int HtmlGroup_Ignore_StartTag(HTML_GROUP *g, const char *tagName);
static int HtmlGroup_Ignore_EndTag(HTML_GROUP *g, const char *tagName);
static int HtmlGroup_Ignore_AddData(HTML_GROUP *g, const char *data);

#endif

