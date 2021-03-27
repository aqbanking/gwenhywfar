/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCMD_P_H
#define GWBUILD_BUILDCMD_P_H

#include "gwenbuild/buildctx/buildcmd.h"



struct GWB_BUILD_CMD {
  GWEN_LIST_ELEMENT(GWB_BUILD_CMD)

  char *folder;

  GWB_KEYVALUEPAIR_LIST *prepareCommandList;
  GWB_KEYVALUEPAIR_LIST *buildCommandList;

  int blockingFiles;
  GWB_FILE_LIST2 *inFileList2;
  GWB_FILE_LIST2 *outFileList2;
};


#endif
