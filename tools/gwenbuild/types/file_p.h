/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_FILE_P_H
#define GWBUILD_FILE_P_H

#include "gwenbuild/types/file.h"


struct GWB_FILE {
  char *folder;
  char *name;
  char *fileType;
  char *installPath;
  char *builder;
  uint32_t id;
  uint32_t flags;


  GWB_BUILD_CMD_LIST2 *waitingBuildCmdList2;
  GWB_BUILD_CMD *buildCmd;
};


#endif
