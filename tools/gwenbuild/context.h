/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_CONTEXT_H
#define GWBUILD_CONTEXT_H


#include "gwenbuild/keyvaluepair.h"

#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/db.h>



typedef struct GWB_CONTEXT GWB_CONTEXT;

GWB_CONTEXT *GWB_Context_new();
GWB_CONTEXT *GWB_Context_dup(const GWB_CONTEXT *originalCtx);
void GWB_Context_free(GWB_CONTEXT *ctx);

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

const char *GWB_Context_GetCompilerFlags(const GWB_CONTEXT *ctx);
void GWB_Context_SetCompilerFlags(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_AddCompilerFlags(GWB_CONTEXT *ctx, const char *s);

const char *GWB_Context_GetLinkerFlags(const GWB_CONTEXT *ctx);
void GWB_Context_SetLinkerFlags(GWB_CONTEXT *ctx, const char *s);
void GWB_Context_AddLinkerFlags(GWB_CONTEXT *ctx, const char *s);

GWEN_STRINGLIST *GWB_Context_GetIncludeList(const GWB_CONTEXT *ctx);
void GWB_Context_AddInclude(GWB_CONTEXT *ctx, const char *folder);

GWB_KEYVALUEPAIR_LIST *GWB_Context_GetDefineList(const GWB_CONTEXT *ctx);
void GWB_Context_SetDefine(GWB_CONTEXT *ctx, const char *name, const char *value);

GWEN_DB_NODE *GWB_Context_GetVars(const GWB_CONTEXT *ctx);




#endif
