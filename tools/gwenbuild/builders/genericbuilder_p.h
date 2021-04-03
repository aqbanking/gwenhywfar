/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDERS_GENERIC_P_H
#define GWBUILD_BUILDERS_GENERIC_P_H


#include "gwenbuild/builders/genericbuilder.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>



typedef struct GWB_BUILDER_GENERIC GWB_BUILDER_GENERIC;
struct GWB_BUILDER_GENERIC {
  char *builderName;
  char *toolName;
  GWEN_DB_NODE *dbVars;
  GWEN_XMLNODE *xmlDescr;

  int maxInputFiles;
  GWEN_STRINGLIST *acceptedInputTypes;
  GWEN_STRINGLIST *acceptedInputExt;
  char *outFileType;
};




#endif
