/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCTX_XML_H
#define GWBUILD_BUILDCTX_XML_H

#include <gwenhywfar/xml.h>

#include "gwenbuild/buildctx/buildctx.h"


GWB_BUILD_CONTEXT *GWB_BuildCtx_ReadFromXmlFile(const char *fileName);
int GWB_BuildCtx_WriteToXmlFile(const GWB_BUILD_CONTEXT *buildCtx, const char *fileName);



#endif
