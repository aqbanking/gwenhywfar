/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GENERATOR_P_H
#define GWBUILD_GENERATOR_P_H

#include "gwenbuild/generator.h"


struct GWB_GENERATOR {
  uint32_t id;

  GWB_CONTEXT *context;

  GWB_FILE_LIST2 *inputFileList;
  GWB_FILE_LIST2 *outputFileList;

  int numBlockingInputFiles;

};


#endif
