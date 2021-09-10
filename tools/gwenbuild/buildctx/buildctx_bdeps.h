/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCTX_BDEPS_H
#define GWBUILD_BUILDCTX_BDEPS_H

#include <gwenhywfar/xml.h>

#include "gwenbuild/buildctx/buildctx.h"


int GWB_BuildCtx_SetupDependencies(GWB_BUILD_CONTEXT *bctx);
int GWB_BuildCtx_FillWaitingQueue(GWB_BUILD_CONTEXT *bctx, const char *builderName);


int GWB_BuildCtx_AddBuildCmdsForFileToList(const GWB_FILE *file, GWB_BUILD_CMD_LIST2 *targetCmdList);
int GWB_BuildCtx_AddBuildCmdsByBuilderNameToList(const GWB_BUILD_CMD_LIST2 *sourceCmdList,
                                                 const char *builderName,
                                                 GWB_BUILD_CMD_LIST2 *targetCmdList);



#endif
