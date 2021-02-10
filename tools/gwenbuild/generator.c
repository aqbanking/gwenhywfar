/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gwenbuild/generator_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>



GWEN_LIST2_FUNCTIONS(GWB_GENERATOR, GWB_Generator)
GWEN_INHERIT_FUNCTIONS(GWB_GENERATOR)



GWB_GENERATOR *GWEN_Generator_new(uint32_t id)
{
  GWB_GENERATOR *gen;

  GWEN_NEW_OBJECT(GWB_GENERATOR, gen);
  GWEN_INHERIT_INIT(GWB_GENERATOR, gen);
  gen->id=id;
  return gen;
}



void GWEN_Generator_free(GWB_GENERATOR *gen)
{
  if (gen) {
    GWEN_INHERIT_FINI(GWB_GENERATOR, gen);
    GWB_File_List2_free(gen->inputFileList2);
    GWB_File_List2_free(gen->outputFileList2);
  }
}



uint32_t GWEN_Generator_GetId(const GWB_GENERATOR *gen)
{
  return gen->id;
}



GWB_CONTEXT *GWEN_Generator_GetContext(const GWB_GENERATOR *gen)
{
  return gen->context;
}



void GWEN_Generator_SetContext(GWB_GENERATOR *gen, GWB_CONTEXT *ctx)
{
  gen->context=ctx;
}



GWB_FILE_LIST2 *GWEN_Generator_GetInputFileList2(const GWB_GENERATOR *gen)
{
  return gen->inputFileList2;
}



void GWEN_Generator_AddInputFile(GWB_GENERATOR *gen, GWB_FILE *f)
{
  if (gen->inputFileList2==NULL)
    gen->inputFileList2=GWB_File_List2_new();
  GWB_File_List2_PushBack(gen->inputFileList2, f);
}



GWB_FILE_LIST2 *GWEN_Generator_GetOutputFileList2(const GWB_GENERATOR *gen)
{
  return gen->outputFileList2;
}



void GWEN_Generator_AddOutputFile(GWB_GENERATOR *gen, GWB_FILE *f)
{
  if (gen->outputFileList2==NULL)
    gen->outputFileList2=GWB_File_List2_new();
  GWB_File_List2_PushBack(gen->outputFileList2, f);
}



int GWEN_Generator_GetNumBlockingInputFiles(const GWB_GENERATOR *gen)
{
  return gen->numBlockingInputFiles;
}



int GWEN_Generator_IncNumBlockingInputFiles(GWB_GENERATOR *gen)
{
  gen->numBlockingInputFiles++;
  return gen->numBlockingInputFiles;
}



int GWEN_Generator_DecNumBlockingInputFiles(GWB_GENERATOR *gen)
{
  if (gen->numBlockingInputFiles>0)
    gen->numBlockingInputFiles--;
  return gen->numBlockingInputFiles;
}



