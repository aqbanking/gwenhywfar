/***************************************************************************
    begin       : Fri Apr 02 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_OPTION_P_H
#define GWBUILD_OPTION_P_H

#include "gwenbuild/types/option.h"


struct GWB_OPTION {
  GWEN_LIST_ELEMENT(GWB_OPTION)

  char *id;
  int optionType;
  char *defaultValue;
  char *definePrefix;

  GWB_KEYVALUEPAIR_LIST *aliasList;
  GWEN_STRINGLIST *choiceList;
};


#endif
