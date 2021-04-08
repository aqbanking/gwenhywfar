/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDSUBCMD_P_H
#define GWBUILD_BUILDSUBCMD_P_H

#include "gwenbuild/buildctx/buildsubcmd.h"



struct GWB_BUILD_SUBCMD {
  GWEN_LIST_ELEMENT(GWB_BUILD_SUBCMD)

  uint32_t flags;
  char *command;
  char *arguments;

  char *buildMessage;

  char *mainInputFilePath;
  char *mainOutputFilePath;
  char *depFilePath;
};


#endif
