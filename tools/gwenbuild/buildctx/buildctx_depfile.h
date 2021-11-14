/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCTX_DEPFILE_H
#define GWBUILD_BUILDCTX_DEPFILE_H

#include <gwenhywfar/stringlist.h>

#include "gwenbuild/buildctx/buildctx.h"


GWEN_STRINGLIST *GWB_BuildCtx_ReadAndTranslateDepfile(const char *folder, const char *fileName);



#endif
