/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_PROJECT_P_H
#define GWBUILD_PROJECT_P_H


#include "gwenbuild/types/project.h"



struct GWB_PROJECT {
  GWENBUILD *gwbuild;
  GWB_CONTEXT *contextTree;

  GWB_FILE_LIST2 *fileList;
  GWB_TARGET_LIST2 *targetList;
  GWB_BUILDER_LIST2 *builderList;

  GWB_KEYVALUEPAIR_LIST *defineList;

  GWB_KEYVALUEPAIR_LIST *givenOptionList;

  GWB_OPTION_LIST *optionList;

  char *projectName;
  int versionMajor;
  int versionMinor;
  int versionPatchlevel;
  int versionBuild;
  char *versionTag;

  int soVersionCurrent;
  int soVersionAge;
  int soVersionRevision;

  uint32_t flags;

  GWB_BUILD_CMD_LIST *explicitBuildList;
};




#endif
