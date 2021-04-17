/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_PARSER_BUILDFILES_H
#define GWBUILD_PARSER_BUILDFILES_H


#include "gwenbuild/types/project.h"

#include <gwenhywfar/xml.h>



int GWB_ParseBuildFiles(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);



#endif

