/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GENERATOR_H
#define GWBUILD_GENERATOR_H


#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>

typedef struct GWB_GENERATOR GWB_GENERATOR;
GWEN_LIST2_FUNCTION_DEFS(GWB_GENERATOR, GWB_Generator)
GWEN_INHERIT_FUNCTION_DEFS(GWB_GENERATOR)


#include "gwenbuild/context.h"
#include "gwenbuild/file.h"

#include <inttypes.h>



GWB_GENERATOR *GWEN_Generator_new(uint32_t id);
void GWEN_Generator_free(GWB_GENERATOR *gen);

uint32_t GWEN_Generator_GetId(const GWB_GENERATOR *gen);

GWB_CONTEXT *GWEN_Generator_GetContext(const GWB_GENERATOR *gen);
void GWEN_Generator_SetContext(GWB_GENERATOR *gen, GWB_CONTEXT *ctx);

GWB_FILE_LIST2 *GWEN_Generator_GetInputFileList2(const GWB_GENERATOR *gen);
void GWEN_Generator_AddInputFile(GWB_GENERATOR *gen, GWB_FILE *f);

GWB_FILE_LIST2 *GWEN_Generator_GetOutputFileList2(const GWB_GENERATOR *gen);
void GWEN_Generator_AddOutputFile(GWB_GENERATOR *gen, GWB_FILE *f);

int GWEN_Generator_GetNumBlockingInputFiles(const GWB_GENERATOR *gen);
int GWEN_Generator_IncNumBlockingInputFiles(GWB_GENERATOR *gen);
int GWEN_Generator_DecNumBlockingInputFiles(GWB_GENERATOR *gen);



#endif
