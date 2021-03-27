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


#include "gwenbuild/builder_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>



GWEN_LIST2_FUNCTIONS(GWB_BUILDER, GWB_Builder)
GWEN_INHERIT_FUNCTIONS(GWB_BUILDER)



GWB_BUILDER *GWEN_Builder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context, uint32_t id)
{
  GWB_BUILDER *builder;

  GWEN_NEW_OBJECT(GWB_BUILDER, builder);
  GWEN_INHERIT_INIT(GWB_BUILDER, builder);
  builder->gwenbuild=gwenbuild;
  builder->context=context;
  builder->id=id;
  return builder;
}



void GWEN_Builder_free(GWB_BUILDER *builder)
{
  if (builder) {
    GWEN_INHERIT_FINI(GWB_BUILDER, builder);
    GWB_File_List2_free(builder->inputFileList2);
    GWB_File_List2_free(builder->outputFileList2);
  }
}


GWENBUILD *GWEN_Builder_GetGwenbuild(const GWB_BUILDER *builder)
{
  return builder->gwenbuild;
}



uint32_t GWEN_Builder_GetId(const GWB_BUILDER *builder)
{
  return builder->id;
}



GWB_CONTEXT *GWEN_Builder_GetContext(const GWB_BUILDER *builder)
{
  return builder->context;
}



GWB_FILE_LIST2 *GWEN_Builder_GetInputFileList2(const GWB_BUILDER *builder)
{
  return builder->inputFileList2;
}



void GWEN_Builder_AddInputFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  if (builder->inputFileList2==NULL)
    builder->inputFileList2=GWB_File_List2_new();
  GWB_File_List2_PushBack(builder->inputFileList2, f);
}



GWB_FILE_LIST2 *GWEN_Builder_GetOutputFileList2(const GWB_BUILDER *builder)
{
  return builder->outputFileList2;
}



void GWEN_Builder_AddOutputFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  if (builder->outputFileList2==NULL)
    builder->outputFileList2=GWB_File_List2_new();
  GWB_File_List2_PushBack(builder->outputFileList2, f);
}



int GWEN_Builder_GetNumBlockingInputFiles(const GWB_BUILDER *builder)
{
  return builder->numBlockingInputFiles;
}



int GWEN_Builder_IncNumBlockingInputFiles(GWB_BUILDER *builder)
{
  builder->numBlockingInputFiles++;
  return builder->numBlockingInputFiles;
}



int GWEN_Builder_DecNumBlockingInputFiles(GWB_BUILDER *builder)
{
  if (builder->numBlockingInputFiles>0)
    builder->numBlockingInputFiles--;
  return builder->numBlockingInputFiles;
}



int GWEN_Builder_GenerateOutputFileList(GWB_BUILDER *builder)
{
  if (builder->generateOutputFileListFn)
    return builder->generateOutputFileListFn(builder);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Builder_IsAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file)
{
  if (builder->isAcceptableInputFn)
    return builder->isAcceptableInputFn(builder, file);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Builder_AddBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx)
{
  if (builder->addBuildCmdFn)
    return builder->addBuildCmdFn(builder, bctx);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_BUILDER_GENERATEOUTPUTFILELIST_FN GWEN_Builder_SetGenerateOutputFileListFn(GWB_BUILDER *builder,
                                                                                GWEN_BUILDER_GENERATEOUTPUTFILELIST_FN fn)
{
  GWEN_BUILDER_GENERATEOUTPUTFILELIST_FN oldFn;

  oldFn=builder->generateOutputFileListFn;
  builder->generateOutputFileListFn=fn;
  return oldFn;
}



GWEN_BUILDER_ISACCEPTABLEINPUT_FN GWEN_Builder_SetIsAcceptableInputFn(GWB_BUILDER *builder,
                                                                      GWEN_BUILDER_ISACCEPTABLEINPUT_FN fn)
{
  GWEN_BUILDER_ISACCEPTABLEINPUT_FN oldFn;

  oldFn=builder->isAcceptableInputFn;
  builder->isAcceptableInputFn=fn;
  return oldFn;
}



GWEN_BUILDER_ADDBUILDCMD_FN GWEN_Builder_SeAddBuildCmdFn(GWB_BUILDER *builder, GWEN_BUILDER_ADDBUILDCMD_FN fn)
{
  GWEN_BUILDER_ADDBUILDCMD_FN oldFn;

  oldFn=builder->addBuildCmdFn;
  builder->addBuildCmdFn=fn;
  return oldFn;
}



void GWEN_Builder_AddFileNamesToBuffer(const GWB_CONTEXT *context, const GWB_FILE_LIST2 *inFileList, GWEN_BUFFER *argBuffer)
{
  GWB_FILE_LIST2_ITERATOR *it;
  int entriesAdded=0;

  it=GWB_File_List2_First(inFileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      if (entriesAdded)
        GWEN_Buffer_AppendString(argBuffer, " ");
      GWEN_Builder_AddFileNameToBuffer(context, file, argBuffer);
      entriesAdded++;
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void GWEN_Builder_AddFileNameToBuffer(const GWB_CONTEXT *context, const GWB_FILE *file, GWEN_BUFFER *argBuffer)
{
  const char *s;

  if (!(GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED)) {
    s=GWB_Context_GetCurrentSourceDir(context);
    if (s && *s) {
      GWEN_Buffer_AppendString(argBuffer, s);
      GWEN_Buffer_AppendString(argBuffer, GWEN_DIR_SEPARATOR_S);
    }
  }
  s=GWB_File_GetName(file);
  if (s && *s)
    GWEN_Buffer_AppendString(argBuffer, s);
}




