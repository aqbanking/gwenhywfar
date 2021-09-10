/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_KEYVALUEPAIR_P_H
#define GWBUILD_KEYVALUEPAIR_P_H

#include "gwenbuild/types/keyvaluepair.h"


struct GWB_KEYVALUEPAIR {
  GWEN_LIST_ELEMENT(GWB_KEYVALUEPAIR)
  char *key;
  char *value;
};


#endif
