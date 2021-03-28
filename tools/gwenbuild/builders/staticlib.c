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


#include "gwenbuild/builders/staticlib_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>

#include <string.h>



GWEN_INHERIT(GWB_BUILDER, GWB_BUILDER_STATICLIB);



static int _init(GWB_BUILDER *builder);
static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static void _setArchiverName(GWB_BUILDER *builder, const char *s);
static void _setRanlibName(GWB_BUILDER *builder, const char *s);

static int _generateOutputFileList(GWB_BUILDER *builder);
static int _isAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file);
static int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);

static GWB_BUILD_CMD *_genCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx, GWB_FILE_LIST2 *inFileList, GWB_FILE *outFile);





GWB_BUILDER *GWEN_StaticLibBuilder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context, uint32_t id)
{
  GWB_BUILDER *builder;
  GWB_BUILDER_STATICLIB *xbuilder;
  int rv;

  builder=GWB_Builder_new(gwenbuild, context, id);
  GWEN_NEW_OBJECT(GWB_BUILDER_STATICLIB, xbuilder);
  GWEN_INHERIT_SETDATA(GWB_BUILDER, GWB_BUILDER_STATICLIB, builder, xbuilder, _freeData);

  GWB_Builder_SetGenerateOutputFileListFn(builder, _generateOutputFileList);
  GWB_Builder_SetIsAcceptableInputFn(builder, _isAcceptableInput);
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
  GWB_BUILDER_STATICLIB *xbuilder;

  xbuilder=(GWB_BUILDER_STATICLIB*) p;

  free(xbuilder->archiverName);
  free(xbuilder->ranlibName);
  GWEN_FREE_OBJECT(xbuilder);
}



int _init(GWB_BUILDER *builder)
{
  const char *s;

  s=GWBUILD_GetToolNameAR(GWB_Builder_GetGwenbuild(builder));
  if (!(s && *s)) {
    DBG_ERROR(NULL, "No AR command set.");
    return GWEN_ERROR_GENERIC;
  }
  _setArchiverName(builder, s);

  s=GWBUILD_GetToolNameRANLIB(GWB_Builder_GetGwenbuild(builder));
  if (!(s && *s)) {
    DBG_ERROR(NULL, "No RANLIB command set.");
    return GWEN_ERROR_GENERIC;
  }
  _setRanlibName(builder, s);

  return 0;
}



void _setArchiverName(GWB_BUILDER *builder, const char *s)
{
  GWB_BUILDER_STATICLIB *xbuilder;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_STATICLIB, builder);
  free(xbuilder->archiverName);
  xbuilder->archiverName=s?strdup(s):NULL;
}



void _setRanlibName(GWB_BUILDER *builder, const char *s)
{
  GWB_BUILDER_STATICLIB *xbuilder;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_STATICLIB, builder);
  free(xbuilder->ranlibName);
  xbuilder->ranlibName=s?strdup(s):NULL;
}



int _generateOutputFileList(GWB_BUILDER *builder)
{
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWB_PROJECT *project;
  const char *name;
  const char *folder;
  GWEN_BUFFER *nbuf;
  GWB_FILE *fileOut;
  GWB_FILE *storedFile;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  project=GWB_Target_GetProject(target);

  folder=GWB_Context_GetCurrentBuildDir(context);

  name=GWB_Target_GetName(target);
  if (!(name && *name)) {
  }

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, "lib");
  GWEN_Buffer_AppendString(nbuf, name);
  GWEN_Buffer_AppendString(nbuf, ".a");

  fileOut=GWB_File_new(folder, GWEN_Buffer_GetStart(nbuf), 0);
  if (fileOut==NULL) {
    DBG_INFO(NULL, "here");
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(nbuf);

  GWB_File_AddFlags(fileOut, GWB_FILE_FLAGS_GENERATED);

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
      if (strcasecmp(ext, "o")==0) {
        DBG_INFO(NULL, "File \"%s\" is acceptable as input for StaticLibBuilder", s);
        return 1;
      }
    }
  }

  return 0;
}



int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx)
{
  GWB_BUILD_CMD *bcmd;
  GWB_FILE_LIST2 *inFileList;
  GWB_FILE *outFile;

  inFileList=GWB_Builder_GetInputFileList2(builder);
  outFile=GWB_File_List2_GetFront(GWB_Builder_GetOutputFileList2(builder));

  if (!(inFileList && GWB_File_List2_GetSize(inFileList))) {
    DBG_ERROR(NULL, "No input files");
    return GWEN_ERROR_GENERIC;
  }

  if (outFile==NULL) {
    DBG_ERROR(NULL, "No output file");
    return GWEN_ERROR_GENERIC;
  }

  bcmd=_genCmd(builder, bctx, inFileList, outFile);
  if (bcmd==NULL) {
    DBG_INFO(NULL, "No build command created");
    return GWEN_ERROR_GENERIC;
  }

  GWB_BuildCtx_AddCommand(bctx, bcmd);
  return 0;
}



GWB_BUILD_CMD *_genCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx, GWB_FILE_LIST2 *inFileList, GWB_FILE *outFile)
{
  GWB_BUILDER_STATICLIB *xbuilder;
  GWB_CONTEXT *context;
  GWEN_BUFFER *argBuffer;
  GWB_BUILD_CMD *bcmd;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_STATICLIB, builder);

  context=GWB_Builder_GetContext(builder);

  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(argBuffer, " rc ");
  GWB_Builder_AddFileNameToBuffer(context, outFile, argBuffer);
  GWEN_Buffer_AppendString(argBuffer, " ");
  GWB_Builder_AddFileNamesToBuffer(context, inFileList, argBuffer);

  /* commando to create libXX.a */
  bcmd=GWB_BuildCmd_new();
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentBuildDir(context));
  GWB_BuildCmd_AddBuildCommand(bcmd, xbuilder->archiverName, GWEN_Buffer_GetStart(argBuffer));
  GWEN_Buffer_Reset(argBuffer);

  /* ranlib command */
  GWB_Builder_AddFileNameToBuffer(context, outFile, argBuffer);
  GWB_BuildCmd_AddBuildCommand(bcmd, xbuilder->ranlibName, GWEN_Buffer_GetStart(argBuffer));
  GWEN_Buffer_free(argBuffer);

  GWB_BuildCtx_AddInFilesToCtxAndCmd(bctx, bcmd, inFileList);
  GWB_BuildCtx_AddOutFileToCtxAndCmd(bctx, bcmd, outFile);

  return bcmd;
}



