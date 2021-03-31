/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDERS_SHAREDLIB_P_H
#define GWBUILD_BUILDERS_SHAREDLIB_P_H


#include "gwenbuild/builders/sharedlib.h"



typedef struct GWB_BUILDER_SHAREDLIB GWB_BUILDER_SHAREDLIB;
struct GWB_BUILDER_SHAREDLIB {
  char *linkerName;

  char *soName;

  int soVersionCurrent;
  int soVersionAge;
  int soVersionRevision;
};




#endif
