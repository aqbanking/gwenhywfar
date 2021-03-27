/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_CONTEXT_H
#define GWBUILD_CONTEXT_H


#include <gwenhywfar/tree2.h>


typedef struct GWB_CONTEXT GWB_CONTEXT;
GWEN_TREE2_FUNCTION_DEFS(GWB_CONTEXT, GWB_Context)


#include "gwenbuild/keyvaluepair.h"
#include "gwenbuild/file.h"
#include "gwenbuild/target.h"

#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/db.h>



GWB_CONTEXT *GWB_Context_new();
GWB_CONTEXT *GWB_Context_dup(const GWB_CONTEXT *originalCtx);
void GWB_Context_free(GWB_CONTEXT *ctx);

const char *GWB_Context_GetCurrentRelativeDir(const GWB_CONTEXT *ctx);
void GWB_Context_SetCurrentRelativeDir(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_AddCurrentRelativeDir(GWB_CONTEXT *ctx, const char *s);


const char *GWB_Context_GetTopBuildDir(const GWB_CONTEXT *ctx);
void GWB_Context_SetTopBuildDir(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_InsertTopBuildDir(GWB_CONTEXT *ctx, const char *s);

const char *GWB_Context_GetTopSourceDir(const GWB_CONTEXT *ctx);
void GWB_Context_SetTopSourceDir(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_InsertTopSourceDir(GWB_CONTEXT *ctx, const char *s);

const char *GWB_Context_GetCurrentBuildDir(const GWB_CONTEXT *ctx);
void GWB_Context_SetCurrentBuildDir(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_AddCurrentBuildDir(GWB_CONTEXT *ctx, const char *s);

const char *GWB_Context_GetCurrentSourceDir(const GWB_CONTEXT *ctx);
void GWB_Context_SetCurrentSourceDir(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_AddCurrentSourceDir(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_InsertCurrentSourceDir(GWB_CONTEXT *ctx, const char *s);

const char *GWB_Context_GetCompilerFlags(const GWB_CONTEXT *ctx);
void GWB_Context_SetCompilerFlags(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_AddCompilerFlags(GWB_CONTEXT *ctx, const char *s);

const char *GWB_Context_GetLinkerFlags(const GWB_CONTEXT *ctx);
void GWB_Context_SetLinkerFlags(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_AddLinkerFlags(GWB_CONTEXT *ctx, const char *s);

GWB_KEYVALUEPAIR_LIST *GWB_Context_GetIncludeList(const GWB_CONTEXT *ctx);
void GWB_Context_AddInclude(GWB_CONTEXT *ctx, const char *genType, const char *incl);
void GWB_Context_ClearIncludeList(GWB_CONTEXT *ctx);

GWB_KEYVALUEPAIR_LIST *GWB_Context_GetDefineList(const GWB_CONTEXT *ctx);
void GWB_Context_SetDefine(GWB_CONTEXT *ctx, const char *name, const char *value);
void GWB_Context_ClearDefineList(GWB_CONTEXT *ctx);

GWEN_DB_NODE *GWB_Context_GetVars(const GWB_CONTEXT *ctx);

GWB_TARGET *GWB_Context_GetCurrentTarget(const GWB_CONTEXT *ctx);

/** doesn't take over target */
void GWB_Context_SetCurrentTarget(GWB_CONTEXT *ctx, GWB_TARGET *target);


GWB_FILE_LIST2 *GWEN_Context_GetSourceFileList2(const GWB_CONTEXT *ctx);
void GWB_Context_AddSourceFile(GWB_CONTEXT *ctx, GWB_FILE *f);
void GWB_Context_ClearSourceFileList2(GWB_CONTEXT *ctx);


void GWB_Context_Dump(const GWB_CONTEXT *ctx, int indent);
void GWB_Context_Tree2_Dump(const GWB_CONTEXT *ctx, int indent);


#endif
