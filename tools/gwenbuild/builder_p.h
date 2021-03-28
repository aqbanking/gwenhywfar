/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDER_P_H
#define GWBUILD_BUILDER_P_H


#include "gwenbuild/builder_be.h"


struct GWB_BUILDER {
  GWEN_INHERIT_ELEMENT(GWB_BUILDER)

  uint32_t id;

  char *typeName;

  GWENBUILD *gwenbuild;

  GWB_CONTEXT *context;

  GWB_FILE_LIST2 *inputFileList2;
  GWB_FILE_LIST2 *outputFileList2;

  int numBlockingInputFiles;

  GWEN_BUILDER_ISACCEPTABLEINPUT_FN isAcceptableInputFn;
  GWEN_BUILDER_ADDBUILDCMD_FN addBuildCmdFn;
  GWB_BUILDER_ADDSOURCEFILE_FN addSourceFileFn;
};


#endif
