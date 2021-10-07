/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_BUILDER_H
#define TYPEMAKER2_BUILDER_H


#include <gwenhywfar/inherit.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/buffer.h>


#define TYPEMAKER2_BUILDER_WRITEFILE_FLAGS_WRITE_DEFS 0x0001
#define TYPEMAKER2_BUILDER_WRITEFILE_FLAGS_WRITE_TYPE 0x0002


typedef struct TYPEMAKER2_BUILDER TYPEMAKER2_BUILDER;
GWEN_INHERIT_FUNCTION_DEFS(TYPEMAKER2_BUILDER)


#include "tm_typemanager.h"
#include "tm_type.h"
#include "tm_member.h"


typedef int (*TYPEMAKER2_BUILDER_BUILD_FN)(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);


TYPEMAKER2_BUILDER *Typemaker2_Builder_new();
void Typemaker2_Builder_free(TYPEMAKER2_BUILDER *tb);

int Typemaker2_Builder_PrintFileNames(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);


GWEN_STRINGLIST *Typemaker2_Builder_GetPublicDeclarations(const TYPEMAKER2_BUILDER *tb);
GWEN_STRINGLIST *Typemaker2_Builder_GetLibraryDeclarations(const TYPEMAKER2_BUILDER *tb);
GWEN_STRINGLIST *Typemaker2_Builder_GetProtectedDeclarations(const TYPEMAKER2_BUILDER *tb);
GWEN_STRINGLIST *Typemaker2_Builder_GetPrivateDeclarations(const TYPEMAKER2_BUILDER *tb);
GWEN_STRINGLIST *Typemaker2_Builder_GetCode(const TYPEMAKER2_BUILDER *tb);


TYPEMAKER2_TYPEMANAGER *Typemaker2_Builder_GetTypeManager(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetTypeManager(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPEMANAGER *tm);

void Typemaker2_Builder_AddPublicDeclaration(TYPEMAKER2_BUILDER *tb, const char *s);
void Typemaker2_Builder_AddLibraryDeclaration(TYPEMAKER2_BUILDER *tb, const char *s);
void Typemaker2_Builder_AddProtectedDeclaration(TYPEMAKER2_BUILDER *tb, const char *s);
void Typemaker2_Builder_AddPrivateDeclaration(TYPEMAKER2_BUILDER *tb, const char *s);
void Typemaker2_Builder_AddDeclaration(TYPEMAKER2_BUILDER *tb, int access, const char *s);
void Typemaker2_Builder_AddCode(TYPEMAKER2_BUILDER *tb, const char *s);

const char *Typemaker2_Builder_GetFileNamePublic(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetFileNamePublic(TYPEMAKER2_BUILDER *tb, const char *s);

const char *Typemaker2_Builder_GetFileNameLibrary(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetFileNameLibrary(TYPEMAKER2_BUILDER *tb, const char *s);

const char *Typemaker2_Builder_GetFileNameProtected(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetFileNameProtected(TYPEMAKER2_BUILDER *tb, const char *s);

const char *Typemaker2_Builder_GetFileNamePrivate(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetFileNamePrivate(TYPEMAKER2_BUILDER *tb, const char *s);

const char *Typemaker2_Builder_GetFileNameCode(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetFileNameCode(TYPEMAKER2_BUILDER *tb, const char *s);

const char *Typemaker2_Builder_GetSourceFileName(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetSourceFileName(TYPEMAKER2_BUILDER *tb, const char *s);

const char *Typemaker2_Builder_GetTypedefFileName(const TYPEMAKER2_BUILDER *tb);
void Typemaker2_Builder_SetTypedefFileName(TYPEMAKER2_BUILDER *tb, const char *s);

void Typemaker2_Builder_SetDestFolderName(TYPEMAKER2_BUILDER *tb, const char *s);

void Typemaker2_Builder_SetBuildFn(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_BUILDER_BUILD_FN fn);


int Typemaker2_Builder_Build(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int Typemaker2_Builder_WriteFiles(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, uint32_t writeFlags);

int Typemaker2_Builder_MakeTypeList1(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int Typemaker2_Builder_MakeTypeList2(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int Typemaker2_Builder_MakeTypeTree(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);




GWEN_DB_NODE *Typemaker2_Builder_CreateDbForCall(TYPEMAKER2_BUILDER *tb,
                                                 TYPEMAKER2_TYPE *ty,
                                                 TYPEMAKER2_MEMBER *tm,
                                                 const char *src,
                                                 const char *dst);

int Typemaker2_Builder_ReplaceVars(const char *s,
                                   GWEN_DB_NODE *db,
                                   GWEN_BUFFER *dbuf);


/* see INVOKE_FN macro in tm_builder.c */
int Typemaker2_Builder_Invoke_ConstructFn(TYPEMAKER2_BUILDER *tb,
                                          TYPEMAKER2_TYPE *ty,
                                          TYPEMAKER2_MEMBER *tm,
                                          const char *src,
                                          const char *dst,
                                          GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_DestructFn(TYPEMAKER2_BUILDER *tb,
                                         TYPEMAKER2_TYPE *ty,
                                         TYPEMAKER2_MEMBER *tm,
                                         const char *src,
                                         const char *dst,
                                         GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_AssignFn(TYPEMAKER2_BUILDER *tb,
                                       TYPEMAKER2_TYPE *ty,
                                       TYPEMAKER2_MEMBER *tm,
                                       const char *src,
                                       const char *dst,
                                       GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_DupFn(TYPEMAKER2_BUILDER *tb,
                                    TYPEMAKER2_TYPE *ty,
                                    TYPEMAKER2_MEMBER *tm,
                                    const char *src,
                                    const char *dst,
                                    GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_CompareFn(TYPEMAKER2_BUILDER *tb,
                                        TYPEMAKER2_TYPE *ty,
                                        TYPEMAKER2_MEMBER *tm,
                                        const char *src,
                                        const char *dst,
                                        GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_ToDbFn(TYPEMAKER2_BUILDER *tb,
                                     TYPEMAKER2_TYPE *ty,
                                     TYPEMAKER2_MEMBER *tm,
                                     const char *src,
                                     const char *dst,
                                     GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_FromDbFn(TYPEMAKER2_BUILDER *tb,
                                       TYPEMAKER2_TYPE *ty,
                                       TYPEMAKER2_MEMBER *tm,
                                       const char *src,
                                       const char *dst,
                                       GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_ToXmlFn(TYPEMAKER2_BUILDER *tb,
                                      TYPEMAKER2_TYPE *ty,
                                      TYPEMAKER2_MEMBER *tm,
                                      const char *src,
                                      const char *dst,
                                      GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_FromXmlFn(TYPEMAKER2_BUILDER *tb,
                                        TYPEMAKER2_TYPE *ty,
                                        TYPEMAKER2_MEMBER *tm,
                                        const char *src,
                                        const char *dst,
                                        GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_ToObjectFn(TYPEMAKER2_BUILDER *tb,
                                         TYPEMAKER2_TYPE *ty,
                                         TYPEMAKER2_MEMBER *tm,
                                         const char *src,
                                         const char *dst,
                                         GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_FromObjectFn(TYPEMAKER2_BUILDER *tb,
                                           TYPEMAKER2_TYPE *ty,
                                           TYPEMAKER2_MEMBER *tm,
                                           const char *src,
                                           const char *dst,
                                           GWEN_BUFFER *dbuf);

int Typemaker2_Builder_Invoke_ToHashStringFn(TYPEMAKER2_BUILDER *tb,
                                             TYPEMAKER2_TYPE *ty,
                                             TYPEMAKER2_MEMBER *tm,
                                             const char *src,
                                             const char *dst,
                                             GWEN_BUFFER *dbuf);


#endif




