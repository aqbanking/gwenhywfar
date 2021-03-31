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


#include "gwenbuild/builders/cbuilder_p.h"
#include "gwenbuild/buildctx/buildctx.h"
#include "gwenbuild/builder_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>

#include <string.h>



GWEN_INHERIT(GWB_BUILDER, GWB_BUILDER_CBUILDER);



static int _init(GWB_BUILDER *builder);
static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static void _setCompilerName(GWB_BUILDER *builder, const char *s);

static int _generateOutputFileList(GWB_BUILDER *builder);
static int _isAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file);
static int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);
static void _addSourceFile(GWB_BUILDER *builder, GWB_FILE *f);

static GWB_BUILD_CMD *_genCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx, GWB_FILE *inFile, GWB_FILE *outFile);
static void _genBuildMessage(GWB_BUILD_CMD *bcmd, const GWB_FILE *inFile);
static void _addDefinesIncludesAndCflags(const GWB_CONTEXT *context, GWEN_BUFFER *argBuffer);





GWB_BUILDER *GWEN_CBuilder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context)
{
  GWB_BUILDER *builder;
  GWB_BUILDER_CBUILDER *xbuilder;
  int rv;

  builder=GWB_Builder_new(gwenbuild, context, "CBuilder");
  GWEN_NEW_OBJECT(GWB_BUILDER_CBUILDER, xbuilder);
  GWEN_INHERIT_SETDATA(GWB_BUILDER, GWB_BUILDER_CBUILDER, builder, xbuilder, _freeData);

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
  GWB_BUILDER_CBUILDER *xbuilder;

  xbuilder=(GWB_BUILDER_CBUILDER*) p;

  free(xbuilder->compilerName);
  GWEN_FREE_OBJECT(xbuilder);
}



int _init(GWB_BUILDER *builder)
{
  const char *s;

  s=GWBUILD_GetToolNameCC(GWB_Builder_GetGwenbuild(builder));
  if (!(s && *s)) {
    DBG_ERROR(NULL, "No compiler set.");
    return GWEN_ERROR_GENERIC;
  }
  _setCompilerName(builder, s);

  return 0;
}



void _setCompilerName(GWB_BUILDER *builder, const char *s)
{
  GWB_BUILDER_CBUILDER *xbuilder;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_CBUILDER, builder);
  if (xbuilder->compilerName)
    free(xbuilder->compilerName);
  if (s)
    xbuilder->compilerName=strdup(s);
  else
    xbuilder->compilerName=NULL;
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

        fileOut=GWB_File_CopyObjectAndChangeExtension(file, ".o");
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
      if (strcasecmp(ext, "c")==0) {
        DBG_DEBUG(NULL, "File \"%s\" is acceptable as input for CBuilder", s);
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

  bcmd=_genCmd(builder, bctx, inFile, outFile);
  if (bcmd==NULL) {
    DBG_INFO(NULL, "No build command created");
    return GWEN_ERROR_GENERIC;
  }

  GWB_BuildCtx_AddCommand(bctx, bcmd);
  return 0;
}



GWB_BUILD_CMD *_genCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx, GWB_FILE *inFile, GWB_FILE *outFile)
{
  GWB_BUILDER_CBUILDER *xbuilder;
  GWB_CONTEXT *context;
  GWEN_BUFFER *argBuffer;
  GWB_BUILD_CMD *bcmd;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_CBUILDER, builder);

  context=GWB_Builder_GetContext(builder);

  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  _addDefinesIncludesAndCflags(context, argBuffer);
  GWEN_Buffer_AppendString(argBuffer, " -c -fPIC "); /* compile arguments */
  //GWB_Builder_AddFileNameToBuffer(context, inFile, argBuffer);
  GWB_Builder_AddAbsFileNameToBuffer(context, inFile, argBuffer);
  GWEN_Buffer_AppendString(argBuffer, " -o ");
  GWB_Builder_AddFileNameToBuffer(context, outFile, argBuffer);

  /* we have everything, create cmd now */
  bcmd=GWB_BuildCmd_new();
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentBuildDir(context));
  GWB_BuildCmd_AddBuildCommand(bcmd, xbuilder->compilerName, GWEN_Buffer_GetStart(argBuffer));
  GWEN_Buffer_free(argBuffer);

  GWB_BuildCtx_AddInFileToCtxAndCmd(bctx, bcmd, inFile);
  GWB_BuildCtx_AddOutFileToCtxAndCmd(bctx, bcmd, outFile);

  _genBuildMessage(bcmd, inFile);

  return bcmd;
}



void _genBuildMessage(GWB_BUILD_CMD *bcmd, const GWB_FILE *inFile)
{
  GWEN_BUFFER *buf;
  const char *folder;
  const char *name;

  folder=GWB_File_GetFolder(inFile);
  name=GWB_File_GetName(inFile);
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(buf, "Compiling \"%s%s%s\"",
                         folder?folder:"",
                         folder?"/":"",
                         name?name:"<no name>");
  GWB_BuildCmd_SetBuildMessage(bcmd, GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);
}


void _addDefinesIncludesAndCflags(const GWB_CONTEXT *context, GWEN_BUFFER *argBuffer)
{
  GWB_KEYVALUEPAIR_LIST *kvpList;
  const char *s;

  /* add defines */
  kvpList=GWB_Context_GetDefineList(context);
  if (kvpList)
    GWB_KeyValuePair_List_WriteAllPairsToBuffer(kvpList, "-D", "=", " ", argBuffer);

  /* add includes */
  kvpList=GWB_Context_GetIncludeList(context);
  if (kvpList) {
    GWEN_Buffer_AppendString(argBuffer, " ");
    GWB_KeyValuePair_List_SampleValuesByKey(kvpList, "c", NULL, " ", argBuffer);
  }

  /* add cflags */
  s=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "cflags", 0, NULL);
  if (s && *s) {
    GWEN_Buffer_AppendString(argBuffer, " ");
    GWEN_Buffer_AppendString(argBuffer, s);
  }
}


