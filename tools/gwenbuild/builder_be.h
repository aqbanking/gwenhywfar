/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDER_BE_H
#define GWBUILD_BUILDER_BE_H

#include "gwenbuild/builder.h"


typedef int (*GWEN_BUILDER_ISACCEPTABLEINPUT_FN)(GWB_BUILDER *builder, const GWB_FILE *file);
typedef int (*GWEN_BUILDER_ADDBUILDCMD_FN)(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);
typedef void (*GWB_BUILDER_ADDSOURCEFILE_FN)(GWB_BUILDER *builder, GWB_FILE *f);


void GWB_Builder_AddInputFile(GWB_BUILDER *builder, GWB_FILE *f);
/**
 * Files like *.so and *,a should be added first, because the first entry is used
 * when linking with sub-targets internally.
 */
void GWB_Builder_AddOutputFile(GWB_BUILDER *builder, GWB_FILE *f);

GWEN_BUILDER_ISACCEPTABLEINPUT_FN GWB_Builder_SetIsAcceptableInputFn(GWB_BUILDER *builder,
								      GWEN_BUILDER_ISACCEPTABLEINPUT_FN fn);
GWEN_BUILDER_ADDBUILDCMD_FN GWB_Builder_SetAddBuildCmdFn(GWB_BUILDER *builder, GWEN_BUILDER_ADDBUILDCMD_FN fn);

GWB_BUILDER_ADDSOURCEFILE_FN GWB_Builder_SetAddSourceFileFn(GWB_BUILDER *builder, GWB_BUILDER_ADDSOURCEFILE_FN fn);

void GWB_Builder_AddFileNamesToBuffer(const GWB_CONTEXT *context, const GWB_FILE_LIST2 *inFileList, GWEN_BUFFER *argBuffer);
void GWB_Builder_AddFileNameToBuffer(const GWB_CONTEXT *context, const GWB_FILE *file, GWEN_BUFFER *argBuffer);



#endif
