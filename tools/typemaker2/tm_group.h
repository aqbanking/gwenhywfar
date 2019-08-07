/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_GROUP_H
#define TYPEMAKER2_GROUP_H

#include <gwenhywfar/tree.h>
#include <gwenhywfar/xml.h>


typedef struct TYPEMAKER2_GROUP TYPEMAKER2_GROUP;
GWEN_TREE_FUNCTION_DEFS(TYPEMAKER2_GROUP, Typemaker2_Group)


TYPEMAKER2_GROUP *Typemaker2_Group_new();
void Typemaker2_Group_free(TYPEMAKER2_GROUP *gr);
void Typemaker2_Group_Attach(TYPEMAKER2_GROUP *gr);

const char *Typemaker2_Group_GetTitle(TYPEMAKER2_GROUP *gr);
void Typemaker2_Group_SetTitle(TYPEMAKER2_GROUP *gr, const char *s);

const char *Typemaker2_Group_GetDescription(TYPEMAKER2_GROUP *gr);
void Typemaker2_Group_SetDescription(TYPEMAKER2_GROUP *gr, const char *s);

int Typemaker2_Group_readXml(TYPEMAKER2_GROUP *gr, GWEN_XMLNODE *node, const char *wantedLang);



#endif



