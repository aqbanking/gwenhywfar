/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GBUILDER_DESCR_P_H
#define GWBUILD_GBUILDER_DESCR_P_H


#include "gwenbuild/builders/gbuilderdescr.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>



typedef struct GWB_GBUILDER_DESCR GWB_GBUILDER_DESCR;
struct GWB_GBUILDER_DESCR {
  GWEN_LIST_ELEMENT(GWB_GBUILDER_DESCR)

  char *builderName;
  GWEN_XMLNODE *xmlDescr;

  GWEN_STRINGLIST *acceptedInputTypes;
  GWEN_STRINGLIST *acceptedInputExt;
};




#endif
