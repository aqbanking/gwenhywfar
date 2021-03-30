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


#include "gwenbuild/builders/tm2builder_p.h"
#include "gwenbuild/buildctx/buildctx.h"
#include "gwenbuild/builder_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>

#include <string.h>



GWEN_INHERIT(GWB_BUILDER, GWB_BUILDER_TM2);



static int _init(GWB_BUILDER *builder);
static void GWENHYWFAR_CB _freeData(void *bp, void *p);

static int _generateOutputFileList(GWB_BUILDER *builder);
static int _isAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file);
static int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);
static void _addSourceFile(GWB_BUILDER *builder, GWB_FILE *f);

static GWB_BUILD_CMD *_genCmds(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx, GWB_FILE *inFile, GWB_FILE *outFile);
static void _genBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWB_FILE *inFile);
static void _genPrepareCmd(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWB_FILE *inFile);
static void _addIncludesAndTm2flags(const GWB_CONTEXT *context, GWEN_BUFFER *argBuffer);





GWB_BUILDER *GWEN_Tm2Builder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context)
{
  GWB_BUILDER *builder;
  GWB_BUILDER_TM2 *xbuilder;
  int rv;

  builder=GWB_Builder_new(gwenbuild, context, "Tm2Builder");
  GWEN_NEW_OBJECT(GWB_BUILDER_TM2, xbuilder);
  GWEN_INHERIT_SETDATA(GWB_BUILDER, GWB_BUILDER_TM2, builder, xbuilder, _freeData);

  GWB_Builder_SetIsAcceptableInputFn(builder, _isAcceptableInput);
  GWB_Builder_SetAddSourceFileFn(builder, _addSourceFile);
  GWB_Builder_SetAddBuildCmdFn(builder, _addBuildCmd);

  rv=_init(builder);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWB_Builder_free(builder);
    return NULL;
  }

  return builder;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWB_BUILDER_TM2 *xbuilder;

  xbuilder=(GWB_BUILDER_TM2*) p;

  GWEN_FREE_OBJECT(xbuilder);
}



int _init(GWEN_UNUSED GWB_BUILDER *builder)
{
  return 0;
}



void _addSourceFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  GWB_Builder_AddInputFile(builder, f);
  _generateOutputFileList(builder);
}



int _generateOutputFileList(GWB_BUILDER *builder)
{
  GWB_FILE_LIST2 *inputFileList2;
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWB_PROJECT *project;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  project=GWB_Target_GetProject(target);

  inputFileList2=GWB_Builder_GetInputFileList2(builder);
  if (inputFileList2) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(inputFileList2);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWB_FILE *fileOut;
        GWB_FILE *storedFile;

        fileOut=GWB_File_CopyObjectAndChangeExtension(file, ".c");
        if (fileOut==NULL) {
          DBG_INFO(NULL, "here");
          GWB_File_List2Iterator_free(it);
          return GWEN_ERROR_GENERIC;
        }
        GWB_File_AddFlags(fileOut, GWB_FILE_FLAGS_GENERATED);
        GWB_File_DelFlags(fileOut, GWB_FILE_FLAGS_INSTALL);

        storedFile=GWB_Project_GetFileByPathAndName(project,
                                                    GWB_File_GetFolder(fileOut),
                                                    GWB_File_GetName(fileOut));
        if (storedFile) {
          GWB_File_free(fileOut);
          fileOut=storedFile;
          GWB_File_AddFlags(fileOut, GWB_FILE_FLAGS_GENERATED);
        }
        else {
          GWB_Project_AddFile(project, fileOut);
        }
        GWB_Builder_AddOutputFile(builder, fileOut);
        file=GWB_File_List2Iterator_Next(it);
      }

      GWB_File_List2Iterator_free(it);
    }
  }
  else {
    DBG_ERROR(NULL, "No input files");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int _isAcceptableInput(GWEN_UNUSED GWB_BUILDER *builder, const GWB_FILE *file)
{
  const char *s;

  s=GWB_File_GetName(file);
  if (s && *s) {
    const char *ext;

    ext=strrchr(s, '.');
    if (ext) {
      ext++;
      if (strcasecmp(ext, "t2d")==0) {
        DBG_DEBUG(NULL, "File \"%s\" is acceptable as input for Tm2Builder", s);
        return 1;
      }
    }
  }

  return 0;
}



int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx)
{
  GWB_BUILD_CMD *bcmd;
  GWB_FILE *inFile;
  GWB_FILE *outFile;

  inFile=GWB_File_List2_GetFront(GWB_Builder_GetInputFileList2(builder));
  outFile=GWB_File_List2_GetFront(GWB_Builder_GetOutputFileList2(builder));

  if (inFile==NULL) {
    DBG_ERROR(NULL, "No input file");
    return GWEN_ERROR_GENERIC;
  }

  if (outFile==NULL) {
    DBG_ERROR(NULL, "No output file");
    return GWEN_ERROR_GENERIC;
  }

  bcmd=_genCmds(builder, bctx, inFile, outFile);
  if (bcmd==NULL) {
    DBG_INFO(NULL, "No build command created");
    return GWEN_ERROR_GENERIC;
  }

  GWB_BuildCtx_AddCommand(bctx, bcmd);
  return 0;
}



GWB_BUILD_CMD *_genCmds(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx, GWB_FILE *inFile, GWB_FILE *outFile)
{
  GWB_CONTEXT *context;
  GWB_BUILD_CMD *bcmd;

  context=GWB_Builder_GetContext(builder);

  bcmd=GWB_BuildCmd_new();
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentBuildDir(context));

  _genBuildCmd(builder, bcmd, inFile);
  _genPrepareCmd(builder, bcmd, inFile);

  GWB_BuildCtx_AddInFileToCtxAndCmd(bctx, bcmd, inFile);
  GWB_BuildCtx_AddOutFileToCtxAndCmd(bctx, bcmd, outFile);

  return bcmd;
}



void _genBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWB_FILE *inFile)
{
  GWB_CONTEXT *context;
  GWEN_BUFFER *argBuffer;

  context=GWB_Builder_GetContext(builder);

  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  _addIncludesAndTm2flags(context, argBuffer);
  if (GWEN_Buffer_GetUsedBytes(argBuffer))
    GWEN_Buffer_AppendString(argBuffer, " ");
  GWB_Builder_AddFileNameToBuffer(context, inFile, argBuffer);

  /* we have everything, create cmd now */
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentBuildDir(context));
  GWB_BuildCmd_AddBuildCommand(bcmd, "typemaker2", GWEN_Buffer_GetStart(argBuffer));
  GWEN_Buffer_free(argBuffer);
}



void _genPrepareCmd(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWB_FILE *inFile)
{
  GWB_CONTEXT *context;
  GWEN_BUFFER *argBuffer;

  context=GWB_Builder_GetContext(builder);

  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  _addIncludesAndTm2flags(context, argBuffer);
  if (GWEN_Buffer_GetUsedBytes(argBuffer))
    GWEN_Buffer_AppendString(argBuffer, " ");
  GWEN_Buffer_AppendString(argBuffer, "--defs-only ");
  GWB_Builder_AddFileNameToBuffer(context, inFile, argBuffer);

  /* we have everything, create cmd now */
  GWB_BuildCmd_AddPrepareCommand(bcmd, "typemaker2", GWEN_Buffer_GetStart(argBuffer));
  GWEN_Buffer_free(argBuffer);
}



void _addIncludesAndTm2flags(const GWB_CONTEXT *context, GWEN_BUFFER *argBuffer)
{
  GWB_KEYVALUEPAIR_LIST *kvpList;
  const char *s;

  /* add includes */
  kvpList=GWB_Context_GetIncludeList(context);
  if (kvpList) {
    if (GWEN_Buffer_GetUsedBytes(argBuffer))
      GWEN_Buffer_AppendString(argBuffer, " ");
    GWB_KeyValuePair_List_SampleValuesByKey(kvpList, "tm2", NULL, " ", argBuffer);
  }

  /* add tm2flags */
  s=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "tm2flags", 0, NULL);
  if (s && *s) {
    if (GWEN_Buffer_GetUsedBytes(argBuffer))
      GWEN_Buffer_AppendString(argBuffer, " ");
    GWEN_Buffer_AppendString(argBuffer, s);
  }
}

