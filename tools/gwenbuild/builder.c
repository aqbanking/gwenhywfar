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
#include <gwenhywfar/directory.h>



GWEN_LIST2_FUNCTIONS(GWB_BUILDER, GWB_Builder)
GWEN_INHERIT_FUNCTIONS(GWB_BUILDER)



GWB_BUILDER *GWB_Builder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context, const char *typeName)
{
  GWB_BUILDER *builder;

  GWEN_NEW_OBJECT(GWB_BUILDER, builder);
  GWEN_INHERIT_INIT(GWB_BUILDER, builder);
  builder->gwenbuild=gwenbuild;
  builder->context=context;
  if (typeName)
    builder->typeName=strdup(typeName);
  return builder;
}



void GWB_Builder_free(GWB_BUILDER *builder)
{
  if (builder) {
    GWEN_INHERIT_FINI(GWB_BUILDER, builder);

    free(builder->targetLinkSpec);
    free(builder->typeName);
    GWB_File_List2_free(builder->inputFileList2);
    GWB_File_List2_free(builder->outputFileList2);
  }
}



GWENBUILD *GWB_Builder_GetGwenbuild(const GWB_BUILDER *builder)
{
  return builder->gwenbuild;
}



uint32_t GWB_Builder_GetId(const GWB_BUILDER *builder)
{
  return builder->id;
}



const char *GWB_Builder_GetTypeName(const GWB_BUILDER *builder)
{
  return builder->typeName;
}



const char *GWB_Builder_GetTargetLinkSpec(const GWB_BUILDER *builder)
{
  return builder->targetLinkSpec;
}



void GWB_Builder_SetTargetLinkSpec(GWB_BUILDER *builder, const char *s)
{
  free(builder->targetLinkSpec);
  builder->targetLinkSpec=s?strdup(s):NULL;
}



GWB_CONTEXT *GWB_Builder_GetContext(const GWB_BUILDER *builder)
{
  return builder->context;
}



GWB_FILE_LIST2 *GWB_Builder_GetInputFileList2(const GWB_BUILDER *builder)
{
  return builder->inputFileList2;
}



void GWB_Builder_AddInputFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  if (builder->inputFileList2==NULL)
    builder->inputFileList2=GWB_File_List2_new();
  GWB_File_List2_PushBack(builder->inputFileList2, f);
}



GWB_FILE_LIST2 *GWB_Builder_GetOutputFileList2(const GWB_BUILDER *builder)
{
  return builder->outputFileList2;
}



void GWB_Builder_AddOutputFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  if (builder->outputFileList2==NULL)
    builder->outputFileList2=GWB_File_List2_new();
  GWB_File_List2_PushBack(builder->outputFileList2, f);
}



int GWB_Builder_GetNumBlockingInputFiles(const GWB_BUILDER *builder)
{
  return builder->numBlockingInputFiles;
}



int GWB_Builder_IncNumBlockingInputFiles(GWB_BUILDER *builder)
{
  builder->numBlockingInputFiles++;
  return builder->numBlockingInputFiles;
}



int GWB_Builder_DecNumBlockingInputFiles(GWB_BUILDER *builder)
{
  if (builder->numBlockingInputFiles>0)
    builder->numBlockingInputFiles--;
  return builder->numBlockingInputFiles;
}



int GWB_Builder_IsAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file)
{
  if (builder->isAcceptableInputFn)
    return builder->isAcceptableInputFn(builder, file);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWB_Builder_AddBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx)
{
  if (builder->addBuildCmdFn)
    return builder->addBuildCmdFn(builder, bctx);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



void GWB_Builder_AddSourceFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  if (builder->addSourceFileFn)
    return builder->addSourceFileFn(builder, f);
}



GWEN_BUILDER_ISACCEPTABLEINPUT_FN GWB_Builder_SetIsAcceptableInputFn(GWB_BUILDER *builder,
                                                                      GWEN_BUILDER_ISACCEPTABLEINPUT_FN fn)
{
  GWEN_BUILDER_ISACCEPTABLEINPUT_FN oldFn;

  oldFn=builder->isAcceptableInputFn;
  builder->isAcceptableInputFn=fn;
  return oldFn;
}



GWEN_BUILDER_ADDBUILDCMD_FN GWB_Builder_SetAddBuildCmdFn(GWB_BUILDER *builder, GWEN_BUILDER_ADDBUILDCMD_FN fn)
{
  GWEN_BUILDER_ADDBUILDCMD_FN oldFn;

  oldFn=builder->addBuildCmdFn;
  builder->addBuildCmdFn=fn;
  return oldFn;
}



GWB_BUILDER_ADDSOURCEFILE_FN GWB_Builder_SetAddSourceFileFn(GWB_BUILDER *builder, GWB_BUILDER_ADDSOURCEFILE_FN fn)
{
  GWB_BUILDER_ADDSOURCEFILE_FN oldFn;

  oldFn=builder->addSourceFileFn;
  builder->addSourceFileFn=fn;
  return oldFn;
}



void GWB_Builder_AddFileNamesToBuffer(const GWB_CONTEXT *context, const GWB_FILE_LIST2 *inFileList, GWEN_BUFFER *argBuffer)
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
      GWB_Builder_AddFileNameToBuffer(context, file, argBuffer);
      entriesAdded++;
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void GWB_Builder_AddFileNameToBuffer(const GWB_CONTEXT *context, const GWB_FILE *file, GWEN_BUFFER *argBuffer)
{
  const char *folder;
  const char *buildDir;
  const char *initialSourceDir;
  GWEN_BUFFER *realFileFolderBuffer;
  GWEN_BUFFER *relBuffer;

  buildDir=GWB_Context_GetCurrentBuildDir(context);
  initialSourceDir=GWB_Context_GetInitialSourceDir(context);

  folder=GWB_File_GetFolder(file);

  realFileFolderBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  if (!(GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED)) {
    GWEN_Buffer_AppendString(realFileFolderBuffer, initialSourceDir);
    GWEN_Buffer_AppendString(realFileFolderBuffer, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(realFileFolderBuffer, folder);

  relBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Path_GetPathBetween(buildDir, GWEN_Buffer_GetStart(realFileFolderBuffer), relBuffer);

  if (GWEN_Buffer_GetUsedBytes(relBuffer))
    GWEN_Buffer_AppendString(relBuffer, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(relBuffer, GWB_File_GetName(file));

  GWEN_Buffer_AppendString(argBuffer, GWEN_Buffer_GetStart(relBuffer));

  GWEN_Buffer_free(relBuffer);
  GWEN_Buffer_free(realFileFolderBuffer);
}



void GWB_Builder_AddRelativeFolderToBuffer(const GWB_CONTEXT *context, const char *folder, int useBuildDir, GWEN_BUFFER *argBuffer)
{
  const char *buildDir;
  const char *initialSourceDir;
  GWEN_BUFFER *realFileFolderBuffer;
  GWEN_BUFFER *relBuffer;

  buildDir=GWB_Context_GetCurrentBuildDir(context);
  initialSourceDir=GWB_Context_GetInitialSourceDir(context);

  realFileFolderBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  if (!useBuildDir) {
    GWEN_Buffer_AppendString(realFileFolderBuffer, initialSourceDir);
    GWEN_Buffer_AppendString(realFileFolderBuffer, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(realFileFolderBuffer, folder);

  relBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Path_GetPathBetween(buildDir, GWEN_Buffer_GetStart(realFileFolderBuffer), relBuffer);
  if (GWEN_Buffer_GetUsedBytes(relBuffer)>0)
    GWEN_Buffer_AppendString(argBuffer, GWEN_Buffer_GetStart(relBuffer));
  else
    GWEN_Buffer_AppendString(argBuffer, ".");
  GWEN_Buffer_free(relBuffer);
  GWEN_Buffer_free(realFileFolderBuffer);
}



void GWB_Builder_AddAbsFileNameToBuffer(const GWB_CONTEXT *context, const GWB_FILE *file, GWEN_BUFFER *argBuffer)
{
  const char *folder;
  const char *initialSourceDir;
  GWEN_BUFFER *realFileFolderBuffer;
  GWEN_BUFFER *absBuffer;

  initialSourceDir=GWB_Context_GetInitialSourceDir(context);

  folder=GWB_File_GetFolder(file);

  realFileFolderBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  if (!(GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED)) {
    GWEN_Buffer_AppendString(realFileFolderBuffer, initialSourceDir);
    GWEN_Buffer_AppendString(realFileFolderBuffer, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(realFileFolderBuffer, folder);

  absBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Directory_GetAbsoluteFolderPath(GWEN_Buffer_GetStart(realFileFolderBuffer), absBuffer);

  if (GWEN_Buffer_GetUsedBytes(absBuffer))
    GWEN_Buffer_AppendString(absBuffer, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(absBuffer, GWB_File_GetName(file));

  GWEN_Buffer_AppendString(argBuffer, GWEN_Buffer_GetStart(absBuffer));

  GWEN_Buffer_free(absBuffer);
  GWEN_Buffer_free(realFileFolderBuffer);
}



void GWB_Builder_Dump(const GWB_BUILDER *builder, int indent, int fullDump)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "Builder:\n");

  GWBUILD_Debug_PrintValue(    "typeName.......", builder->typeName, indent+2);
  GWBUILD_Debug_PrintFileList2("inputFileList2.", builder->inputFileList2, indent+2);
  GWBUILD_Debug_PrintFileList2("outputFileList2", builder->outputFileList2, indent+2);
  if (fullDump)
    GWB_Context_Dump(builder->context, indent+2);

}



