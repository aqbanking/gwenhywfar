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


#include "gwenbuild/builders/sharedlib_p.h"
#include "gwenbuild/builder_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>

#include <string.h>



GWEN_INHERIT(GWB_BUILDER, GWB_BUILDER_SHAREDLIB);



static int _init(GWB_BUILDER *builder);
static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static void _setLinkerName(GWB_BUILDER *builder, const char *s);
static void _setSoName(GWB_BUILDER *builder, const char *s);

static int _generateOutputFileList(GWB_BUILDER *builder);
static int _isAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file);
static void _addSourceFile(GWB_BUILDER *builder, GWB_FILE *f);
static int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);

static GWB_BUILD_CMD *_genCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx, GWB_FILE_LIST2 *inFileList, GWB_FILE *outFile);
static void _genBuildMessage(GWB_BUILD_CMD *bcmd, const GWB_FILE *outFile);
static void _appendFileNamesExceptStaticLibs(GWB_CONTEXT *context, GWEN_BUFFER *argBuffer, GWB_FILE_LIST2 *inFileList);
static void _appendFileNamesOfStaticLibs(GWB_CONTEXT *context, GWEN_BUFFER *argBuffer, GWB_FILE_LIST2 *inFileList);
static void _appendTargetsUsedLibraries(const GWB_TARGET *target, GWEN_BUFFER *argBuffer);





GWB_BUILDER *GWB_SharedLibBuilder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context)
{
  GWB_BUILDER *builder;
  GWB_BUILDER_SHAREDLIB *xbuilder;
  GWB_TARGET *target;
  int rv;

  target=GWB_Context_GetCurrentTarget(context);

  builder=GWB_Builder_new(gwenbuild, context, "SharedLibBuilder");
  GWEN_NEW_OBJECT(GWB_BUILDER_SHAREDLIB, xbuilder);
  GWEN_INHERIT_SETDATA(GWB_BUILDER, GWB_BUILDER_SHAREDLIB, builder, xbuilder, _freeData);

  GWB_Builder_SetIsAcceptableInputFn(builder, _isAcceptableInput);
  GWB_Builder_SetAddSourceFileFn(builder, _addSourceFile);
  GWB_Builder_SetAddBuildCmdFn(builder, _addBuildCmd);

  if (target) {
    xbuilder->soVersionCurrent=GWB_Target_GetSoVersionCurrent(target);
    xbuilder->soVersionAge=GWB_Target_GetSoVersionAge(target);
    xbuilder->soVersionRevision=GWB_Target_GetSoVersionRevision(target);
  }

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
  GWB_BUILDER_SHAREDLIB *xbuilder;

  xbuilder=(GWB_BUILDER_SHAREDLIB*) p;

  free(xbuilder->linkerName);
  free(xbuilder->soName);
  GWEN_FREE_OBJECT(xbuilder);
}



int _init(GWB_BUILDER *builder)
{
  const char *s;

  s=GWBUILD_GetToolNameCC(GWB_Builder_GetGwenbuild(builder));
  if (!(s && *s)) {
    DBG_ERROR(NULL, "No CC command set.");
    return GWEN_ERROR_GENERIC;
  }
  _setLinkerName(builder, s);

  return 0;
}



void _setLinkerName(GWB_BUILDER *builder, const char *s)
{
  GWB_BUILDER_SHAREDLIB *xbuilder;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_SHAREDLIB, builder);
  free(xbuilder->linkerName);
  xbuilder->linkerName=s?strdup(s):NULL;
}



void _setSoName(GWB_BUILDER *builder, const char *s)
{
  GWB_BUILDER_SHAREDLIB *xbuilder;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_SHAREDLIB, builder);
  free(xbuilder->soName);
  xbuilder->soName=s?strdup(s):NULL;
}



void _addSourceFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  GWB_FILE_LIST2 *fileList;

  GWB_Builder_AddInputFile(builder, f);
  fileList=GWB_Builder_GetOutputFileList2(builder);
  if (!(fileList && GWB_File_List2_GetSize(fileList)>0))
    _generateOutputFileList(builder);
}



int _generateOutputFileList(GWB_BUILDER *builder)
{
  GWB_BUILDER_SHAREDLIB *xbuilder;
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWB_PROJECT *project;
  const char *name;
  const char *folder;
  GWEN_BUFFER *nbuf;
  GWB_FILE *fileOut;
  GWB_FILE *storedFile;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_SHAREDLIB, builder);

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
  GWEN_Buffer_AppendString(nbuf, ".so");
  GWEN_Buffer_AppendArgs(nbuf, ".%d", xbuilder->soVersionCurrent);
  _setSoName(builder, GWEN_Buffer_GetStart(nbuf));
  GWEN_Buffer_AppendArgs(nbuf, ".%d.%d",
                         xbuilder->soVersionAge,
                         xbuilder->soVersionRevision);

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
        DBG_DEBUG(NULL, "File \"%s\" is acceptable as input for TmpLibBuilder", s);
        return 1;
      }
    }
    DBG_DEBUG(NULL, "File \"%s\" is NOT acceptable as input for TmpLibBuilder", s);
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
  GWB_BUILDER_SHAREDLIB *xbuilder;
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWEN_BUFFER *argBuffer;
  GWB_BUILD_CMD *bcmd;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_SHAREDLIB, builder);

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);

  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(argBuffer, "-shared -fPIC -nostdlib -Wl,-no-undefined");
  if (xbuilder->soName) {
    GWEN_Buffer_AppendString(argBuffer, " -Wl,-soname,");
    GWEN_Buffer_AppendString(argBuffer, xbuilder->soName);
    GWEN_Buffer_AppendString(argBuffer, " ");
  }

  _appendFileNamesExceptStaticLibs(context, argBuffer, inFileList);
  _appendTargetsUsedLibraries(target, argBuffer);
  _appendFileNamesOfStaticLibs(context, argBuffer, inFileList);

  GWEN_Buffer_AppendString(argBuffer, " -o ");
  GWB_Builder_AddFileNameToBuffer(context, outFile, argBuffer);
  GWEN_Buffer_AppendString(argBuffer, " ");

  /* command to create libXX.so */
  bcmd=GWB_BuildCmd_new();
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentBuildDir(context));
  GWB_BuildCmd_AddBuildCommand(bcmd, xbuilder->linkerName, GWEN_Buffer_GetStart(argBuffer));
  GWEN_Buffer_Reset(argBuffer);

  GWB_BuildCtx_AddInFilesToCtxAndCmd(bctx, bcmd, inFileList);
  GWB_BuildCtx_AddOutFileToCtxAndCmd(bctx, bcmd, outFile);

  _genBuildMessage(bcmd, outFile);

  return bcmd;
}



void _appendTargetsUsedLibraries(const GWB_TARGET *target, GWEN_BUFFER *argBuffer)
{
  GWEN_STRINGLIST *sl;

  sl=GWB_Target_GetUsedLibraryNameList(target);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        if (GWEN_Buffer_GetUsedBytes(argBuffer))
          GWEN_Buffer_AppendString(argBuffer, " ");
        GWEN_Buffer_AppendString(argBuffer, s);
      }
      se=GWEN_StringListEntry_Next(se);
    }
  }
}



void _appendFileNamesExceptStaticLibs(GWB_CONTEXT *context, GWEN_BUFFER *argBuffer, GWB_FILE_LIST2 *inFileList)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(inFileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      const char *s;

      s=GWB_File_GetExt(file);
      if (!(s && *s && strcasecmp(s, ".a")==0))
        GWB_Builder_AddFileNameToBuffer(context, file, argBuffer);
      file=GWB_File_List2Iterator_Next(it);
    }

    GWB_File_List2Iterator_free(it);
  }
}



void _appendFileNamesOfStaticLibs(GWB_CONTEXT *context, GWEN_BUFFER *argBuffer, GWB_FILE_LIST2 *inFileList)
{
  GWB_FILE_LIST2_ITERATOR *it;
  int staticLibCount=0;

  it=GWB_File_List2_First(inFileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      const char *s;

      s=GWB_File_GetExt(file);
      if (s && *s && strcasecmp(s, ".a")==0) {
        if (GWEN_Buffer_GetUsedBytes(argBuffer))
          GWEN_Buffer_AppendString(argBuffer, " ");
        if (staticLibCount==0)
          GWEN_Buffer_AppendString(argBuffer, "-Wl,--whole-archive ");
        GWB_Builder_AddFileNameToBuffer(context, file, argBuffer);
        staticLibCount++;
      }
      file=GWB_File_List2Iterator_Next(it);
    }

    GWB_File_List2Iterator_free(it);
  }
}



void _genBuildMessage(GWB_BUILD_CMD *bcmd, const GWB_FILE *outFile)
{
  GWEN_BUFFER *buf;
  const char *folder;
  const char *name;

  folder=GWB_File_GetFolder(outFile);
  name=GWB_File_GetName(outFile);
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(buf, "Linking \"%s%s%s\"",
                         folder?folder:"",
                         folder?"/":"",
                         name?name:"<no name>");
  GWB_BuildCmd_SetBuildMessage(bcmd, GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);
}


