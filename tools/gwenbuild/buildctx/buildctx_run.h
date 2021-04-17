/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCTX_RUN_H
#define GWBUILD_BUILDCTX_RUN_H

#include <gwenhywfar/xml.h>

#include "gwenbuild/buildctx/buildctx.h"



int GWB_BuildCtx_Run(GWB_BUILD_CONTEXT *bctx, int maxConcurrentJobs, int usePrepareCommands, const char *builderName);



#endif
