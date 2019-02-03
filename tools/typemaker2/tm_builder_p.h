/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_BUILDER_P_H
#define TYPEMAKER2_BUILDER_P_H


#include "tm_builder.h"



struct TYPEMAKER2_BUILDER {
  GWEN_INHERIT_ELEMENT(TYPEMAKER2_BUILDER)

  GWEN_STRINGLIST *declarationsPublic;
  GWEN_STRINGLIST *declarationsLibrary;
  GWEN_STRINGLIST *declarationsProtected;
  GWEN_STRINGLIST *declarationsPrivate;

  GWEN_STRINGLIST *code;

  char *fileNamePublic;
  char *fileNameLibrary;
  char *fileNameProtected;
  char *fileNamePrivate;
  char *fileNameCode;

  char *sourceFileName;

  char *destFolder;

  TYPEMAKER2_TYPEMANAGER *typeManager;

  TYPEMAKER2_BUILDER_BUILD_FN buildFn;
};


GWEN_DB_NODE *Typemaker2_Builder_CreateDbForCall(TYPEMAKER2_BUILDER *tb,
                                                 TYPEMAKER2_TYPE *ty,
                                                 TYPEMAKER2_MEMBER *tm,
                                                 const char *src,
                                                 const char *dst);

int Typemaker2_Builder_ReplaceVars(const char *s,
                                   GWEN_DB_NODE *db,
                                   GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_Fn(TYPEMAKER2_BUILDER *tb,
                                 TYPEMAKER2_TYPE *ty,
                                 TYPEMAKER2_MEMBER *tm,
                                 const char *code,
                                 const char *src,
                                 const char *dst,
                                 GWEN_BUFFER *dbuf);


int Typemaker2_Builder_WriteFile(TYPEMAKER2_BUILDER *tb,
                                 TYPEMAKER2_TYPE *ty,
                                 const char *fileName,
                                 GWEN_STRINGLIST *sl,
                                 int acc);

int Typemaker2_Builder_WriteTypedefFile(TYPEMAKER2_BUILDER *tb,
                                        TYPEMAKER2_TYPE *ty,
                                        const char *fileName);

int Typemaker2_Builder_WriteTypedefFile_List1(TYPEMAKER2_BUILDER *tb,
                                              TYPEMAKER2_TYPE *ty,
                                              const char *fileName);

int Typemaker2_Builder_WriteTypedefFile_List2(TYPEMAKER2_BUILDER *tb,
                                              TYPEMAKER2_TYPE *ty,
                                              const char *fileName);

int Typemaker2_Builder_WriteTypedefFile_Tree(TYPEMAKER2_BUILDER *tb,
                                             TYPEMAKER2_TYPE *ty,
                                             const char *fileName);

int Typemaker2_Builder_WriteTypedefFile_IdMap(TYPEMAKER2_BUILDER *tb,
                                              TYPEMAKER2_TYPE *ty,
                                              const char *fileName);


int Typemaker2_Builder_DetermineOutFileNames(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);


#endif




