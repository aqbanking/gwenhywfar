/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef TYPEMAKER2_TYPEMANAGER_H
#define TYPEMAKER2_TYPEMANAGER_H

#include "tm_type.h"

#include <stdio.h>


typedef struct TYPEMAKER2_TYPEMANAGER TYPEMAKER2_TYPEMANAGER;

TYPEMAKER2_TYPEMANAGER *Typemaker2_TypeManager_new();
void Typemaker2_TypeManager_free(TYPEMAKER2_TYPEMANAGER *tym);

const char *Typemaker2_TypeManager_GetLanguage(const TYPEMAKER2_TYPEMANAGER *tym);
void Typemaker2_TypeManager_SetLanguage(TYPEMAKER2_TYPEMANAGER *tym, const char *s);

const char *Typemaker2_TypeManager_GetApiDeclaration(const TYPEMAKER2_TYPEMANAGER *tym);
void Typemaker2_TypeManager_SetApiDeclaration(TYPEMAKER2_TYPEMANAGER *tym, const char *s);

void Typemaker2_TypeManager_AddFolder(TYPEMAKER2_TYPEMANAGER *tym, const char *s);

void Typemaker2_TypeManager_AddType(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty);
int Typemaker2_TypeManager_SetTypePtrs(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty);
int Typemaker2_TypeManager_SetMemberTypePtrs(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty);

TYPEMAKER2_TYPE *Typemaker2_TypeManager_FindType(TYPEMAKER2_TYPEMANAGER *tym, const char *s);
TYPEMAKER2_TYPE *Typemaker2_TypeManager_GetType(TYPEMAKER2_TYPEMANAGER *tym, const char *s);


TYPEMAKER2_TYPE *Typemaker2_TypeManager_LoadTypeFile(TYPEMAKER2_TYPEMANAGER *tym, const char *fileName);
int Typemaker2_TypeManager_LoadTypeFile2(TYPEMAKER2_TYPEMANAGER *tym, const char *fileName,
                                         TYPEMAKER2_TYPE_LIST2 *tlist2);

int Typemaker2_TypeManager_LoadTypeFileNoLookup(TYPEMAKER2_TYPEMANAGER *tym, const char *fileName,
                                                TYPEMAKER2_TYPE_LIST2 *tlist2);


void Typemaker2_TypeManager_Dump(TYPEMAKER2_TYPEMANAGER *tym, FILE *f, int indent);


#endif




