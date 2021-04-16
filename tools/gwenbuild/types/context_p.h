/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_CONTEXT_P_H
#define GWBUILD_CONTEXT_P_H

#include "gwenbuild/types/context.h"


struct GWB_CONTEXT {
  GWEN_TREE2_ELEMENT(GWB_CONTEXT)

  char *initialSourceDir;
  char *currentRelativeDir;

  char *topBuildDir;
  char *topSourceDir;

  char *currentBuildDir;
  char *currentSourceDir;

  char *compilerFlags;
  char *linkerFlags;

  GWB_KEYVALUEPAIR_LIST *includeList; /* key=TYPE (e.g. tm2), value=includes */
  GWB_KEYVALUEPAIR_LIST *defineList;

  GWEN_DB_NODE *vars;

  GWB_FILE_LIST2 *sourceFileList2;

  GWB_TARGET *currentTarget;
};


#endif
