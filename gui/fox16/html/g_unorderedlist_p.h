/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTML_G_UNORDEREDLIST_P_H
#define HTML_G_UNORDEREDLIST_P_H


#include "g_unorderedlist_l.h"


typedef struct GROUP_UNORDEREDLIST GROUP_UNORDEREDLIST;
struct GROUP_UNORDEREDLIST {
  int row;
};

static void GWENHYWFAR_CB HtmlGroup_UnorderedList_FreeData(void *bp, void *p);


static int HtmlGroup_UnorderedList_StartTag(HTML_GROUP *g, const char *tagName);

#endif

