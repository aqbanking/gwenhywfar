/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTML_G_TABLEROW_P_H
#define HTML_G_TABLEROW_P_H


#include "g_tablerow_l.h"


typedef struct GROUP_TABLEROW GROUP_TABLEROW;
struct GROUP_TABLEROW {
  int columns;
};


static void GWENHYWFAR_CB HtmlGroup_TableRow_FreeData(void *bp, void *p);

static int HtmlGroup_TableRow_StartTag(HTML_GROUP *g, const char *tagName);

#endif

