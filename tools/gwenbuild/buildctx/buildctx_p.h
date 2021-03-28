/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCTX_P_H
#define GWBUILD_BUILDCTX_P_H


#include "gwenbuild/buildctx/buildctx.h"



struct GWB_BUILD_CONTEXT {
  int lastFileId;
  GWB_BUILD_CMD_LIST2 *commandList;
  GWB_FILE_LIST2 *fileList;
};


#endif
