/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_TRYCOMPILE_H
#define GWBUILD_TRYCOMPILE_H


#include "gwenbuild/types/gwenbuild.h"


int GWB_Tools_TryCompile(GWB_CONTEXT *context, const char *testCode);

int GWB_Tools_CheckCompilerArgs(GWB_CONTEXT *context, const char *compilerArgs);



#endif

