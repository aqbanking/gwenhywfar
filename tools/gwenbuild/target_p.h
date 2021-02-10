/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_TARGET_P_H
#define GWBUILD_TARGET_P_H


#include "gwenbuild/target.h"


struct GWB_TARGET {
  GWEN_TREE2_ELEMENT(GWB_TARGET)

  char *name;
  GWBUILD_TARGETTYPE targetType;

  GWB_CONTEXT *context;

  GWEN_STRINGLIST *sourceFileNameList;
  GWEN_STRINGLIST *usedTargetNameList;

  GWB_GENERATOR *generator;
  GWB_FILE *outputFile;
};


#endif
