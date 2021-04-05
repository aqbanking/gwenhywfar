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

  char *builderName;

  char *folder;

  char *buildMessage;

  GWB_BUILD_SUBCMD_LIST *prepareCommandList;
  GWB_BUILD_SUBCMD_LIST *buildCommandList;

  int blockingFiles;
  GWB_FILE_LIST2 *inFileList2;
  GWB_FILE_LIST2 *outFileList2;

  GWEN_PROCESS *currentProcess;
  GWB_BUILD_SUBCMD *currentCommand;
};


#endif
