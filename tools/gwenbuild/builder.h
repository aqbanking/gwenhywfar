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

typedef int (*GWEN_BUILDER_GENERATEOUTPUTFILELIST_FN)(GWB_BUILDER *builder);
typedef int (*GWEN_BUILDER_ISACCEPTABLEINPUT_FN)(GWB_BUILDER *builder, const GWB_FILE *file);
typedef int (*GWEN_BUILDER_ADDBUILDCMD_FN)(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);



GWB_BUILDER *GWEN_Builder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context, uint32_t id);
void GWEN_Builder_free(GWB_BUILDER *builder);

GWENBUILD *GWEN_Builder_GetGwenbuild(const GWB_BUILDER *builder);
uint32_t GWEN_Builder_GetId(const GWB_BUILDER *builder);

GWB_CONTEXT *GWEN_Builder_GetContext(const GWB_BUILDER *builder);

GWB_FILE_LIST2 *GWEN_Builder_GetInputFileList2(const GWB_BUILDER *builder);
void GWEN_Builder_AddInputFile(GWB_BUILDER *builder, GWB_FILE *f);

GWB_FILE_LIST2 *GWEN_Builder_GetOutputFileList2(const GWB_BUILDER *builder);
/**
 * Files like *.so and *,a should be added first, because the first entry is used
 * when linking with sub-targets internally.
 */
void GWEN_Builder_AddOutputFile(GWB_BUILDER *builder, GWB_FILE *f);

int GWEN_Builder_GetNumBlockingInputFiles(const GWB_BUILDER *builder);
int GWEN_Builder_IncNumBlockingInputFiles(GWB_BUILDER *builder);
int GWEN_Builder_DecNumBlockingInputFiles(GWB_BUILDER *builder);


int GWEN_Builder_GenerateOutputFileList(GWB_BUILDER *builder);
int GWEN_Builder_IsAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file);

int GWEN_Builder_AddBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);



GWEN_BUILDER_GENERATEOUTPUTFILELIST_FN GWEN_Builder_SetGenerateOutputFileListFn(GWB_BUILDER *builder,
										GWEN_BUILDER_GENERATEOUTPUTFILELIST_FN fn);
GWEN_BUILDER_ISACCEPTABLEINPUT_FN GWEN_Builder_SetIsAcceptableInputFn(GWB_BUILDER *builder,
								      GWEN_BUILDER_ISACCEPTABLEINPUT_FN fn);
GWEN_BUILDER_ADDBUILDCMD_FN GWEN_Builder_SetAddBuildCmdFn(GWB_BUILDER *builder, GWEN_BUILDER_ADDBUILDCMD_FN fn);


void GWEN_Builder_AddFileNamesToBuffer(const GWB_CONTEXT *context, const GWB_FILE_LIST2 *inFileList, GWEN_BUFFER *argBuffer);
void GWEN_Builder_AddFileNameToBuffer(const GWB_CONTEXT *context, const GWB_FILE *file, GWEN_BUFFER *argBuffer);



#endif
