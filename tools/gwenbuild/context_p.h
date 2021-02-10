/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_CONTEXT_P_H
#define GWBUILD_CONTEXT_P_H

#include "gwenbuild/context.h"


struct GWB_CONTEXT {
  char *topBuildDir;
  char *topSourceDir;

  char *currentBuildDir;
  char *currentSourceDir;

  char *compilerFlags;
  char *linkerFlags;

  GWB_KEYVALUEPAIR_LIST *includeList;
  GWB_KEYVALUEPAIR_LIST *defineList;

  GWEN_DB_NODE *vars;
};


#endif
