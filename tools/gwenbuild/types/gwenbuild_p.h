/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GWENBUILD_P_H
#define GWBUILD_GWENBUILD_P_H


#include "gwenbuild/types/gwenbuild.h"



struct GWENBUILD {
  GWEN_STRINGLIST *buildFilenameList;

  char *targetSystem;
  int targetIsWindows;

  GWB_GBUILDER_DESCR_LIST *builderDescrList;
  uint32_t flags;
};


#endif
