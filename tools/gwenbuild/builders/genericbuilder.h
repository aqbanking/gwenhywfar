/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDERS_GENERIC_H
#define GWBUILD_BUILDERS_GENERIC_H


#include "gwenbuild/builder.h"

#include <gwenhywfar/xml.h>



GWB_BUILDER *GWB_GenericBuilder_Factory(GWENBUILD *gwenbuild, GWB_CONTEXT *context, const char *builderName);



#endif
