/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_FILE_H
#define GWBUILD_FILE_H

#include <gwenhywfar/list2.h>


typedef struct GWB_FILE GWB_FILE;
GWEN_LIST2_FUNCTION_DEFS(GWB_FILE, GWB_File)


#include "gwenbuild/context.h"
#include "gwenbuild/generator.h"

#include <inttypes.h>



GWB_FILE *GWB_File_new(const char *folder, const char *fName, uint32_t id);
void GWB_File_free(GWB_FILE *f);


uint32_t GWB_File_GetId(const GWB_FILE *f);
void GWB_File_SetId(GWB_FILE *f, uint32_t i);

uint32_t GWB_File_GetFlags(const GWB_FILE *f);
void GWB_File_SetFlags(GWB_FILE *f, uint32_t i);
void GWB_File_AddFlags(GWB_FILE *f, uint32_t i);
void GWB_File_DelFlags(GWB_FILE *f, uint32_t i);

const char *GWB_File_GetFolder(const GWB_FILE *f);
void GWB_File_SetFolder(GWB_FILE *f, const char *s);

const char *GWB_File_GetName(const GWB_FILE *f);
void GWB_File_SetName(GWB_FILE *f, const char *s);


GWB_GENERATOR_LIST2 *GWB_File_GetWaitingGeneratorList2(const GWB_FILE *f);
void GWB_File_AddWaitingGenerator(GWB_FILE *f, GWB_GENERATOR *gen);


void GWB_File_List2_FreeAll(GWB_FILE_LIST2 *fileList2);


#endif
