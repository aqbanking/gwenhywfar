/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_FILE_P_H
#define GWBUILD_FILE_P_H

#include "gwenbuild/file.h"


struct GWB_FILE {
  char *folder;
  char *name;
  uint32_t id;
  uint32_t flags;


  GWB_GENERATOR_LIST2 *waitingGeneratorList2;
};


#endif
