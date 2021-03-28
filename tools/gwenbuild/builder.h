/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDER_H
#define GWBUILD_BUILDER_H


#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>

typedef struct GWB_BUILDER GWB_BUILDER;
GWEN_LIST2_FUNCTION_DEFS(GWB_BUILDER, GWB_Builder)
GWEN_INHERIT_FUNCTION_DEFS(GWB_BUILDER)

#include "gwenbuild/gwenbuild.h"
#include "gwenbuild/context.h"
#include "gwenbuild/file.h"
#include "gwenbuild/buildctx/buildctx.h"

#include <inttypes.h>


void GWB_Builder_free(GWB_BUILDER *builder);

GWENBUILD *GWB_Builder_GetGwenbuild(const GWB_BUILDER *builder);

const char *GWB_Builder_GetTypeName(const GWB_BUILDER *builder);
uint32_t GWB_Builder_GetId(const GWB_BUILDER *builder);

GWB_CONTEXT *GWB_Builder_GetContext(const GWB_BUILDER *builder);

GWB_FILE_LIST2 *GWB_Builder_GetInputFileList2(const GWB_BUILDER *builder);

GWB_FILE_LIST2 *GWB_Builder_GetOutputFileList2(const GWB_BUILDER *builder);

int GWB_Builder_GetNumBlockingInputFiles(const GWB_BUILDER *builder);
int GWB_Builder_IncNumBlockingInputFiles(GWB_BUILDER *builder);
int GWB_Builder_DecNumBlockingInputFiles(GWB_BUILDER *builder);


int GWB_Builder_IsAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file);
void GWB_Builder_AddSourceFile(GWB_BUILDER *builder, GWB_FILE *f);
int GWB_Builder_AddBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);

void GWB_Builder_Dump(const GWB_BUILDER *builder, int indent, int fullDump);


#endif
