/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_FILE_P_H
#define GWBUILD_FILE_P_H


typedef struct GWB_FILE GWB_FILE;
GWEN_LIST2_FUNCTION_DEFS(GWB_FILE, GWB_File)


#include "gwenbuild/context.h"
#include "gwenbuild/generator.h"


uint32_t GWB_File_GetId(const GWB_FILE *f);
uint32_t GWB_File_GetFlags(const GWB_FILE *f);

const char *GWB_File_GetFolder(const GWB_FILE *f);
const char *GWB_File_GetName(const GWB_FILE *f);


GWB_GENERATOR_LIST2 *GWB_File_GetWaitingGeneratorList2(const GWB_FILE *f);




#endif
