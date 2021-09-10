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

#define DISABLE_DEBUGLOG


#include "gwenbuild/buildctx/buildctx.h"
#include "gwenbuild/buildctx/buildctx_run.h"
#include "gwenbuild/buildctx/buildctx_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>
#include <ctype.h>



GWB_BUILD_CONTEXT *GWB_BuildCtx_new()
{
  GWB_BUILD_CONTEXT *bctx;

  GWEN_NEW_OBJECT(GWB_BUILD_CONTEXT, bctx);
  bctx->commandList=GWB_BuildCmd_List2_new();
  bctx->fileList=GWB_File_List2_new();

  return bctx;
}



void GWB_BuildCtx_free(GWB_BUILD_CONTEXT *bctx)
{
  if (bctx) {
    GWB_BuildCmd_List2_free(bctx->waitingQueue);
    GWB_BuildCmd_List2_free(bctx->runningQueue);
    GWB_BuildCmd_List2_free(bctx->finishedQueue);

    GWB_BuildCmd_List2_FreeAll(bctx->commandList);
    GWB_File_List2_FreeAll(bctx->fileList);

    free(bctx->initialSourceDir);

    GWEN_FREE_OBJECT(bctx);
  }
}



const char *GWB_BuildCtx_GetInitialSourceDir(const GWB_BUILD_CONTEXT *bctx)
{
  return bctx->initialSourceDir;
}



void GWB_BuildCtx_SetInitialSourceDir(GWB_BUILD_CONTEXT *bctx, const char *s)
{
  free(bctx->initialSourceDir);
  bctx->initialSourceDir=s?strdup(s):NULL;
}



GWB_BUILD_CMD_LIST2 *GWB_BuildCtx_GetCommandList(const GWB_BUILD_CONTEXT *bctx)
{
  return bctx->commandList;
}



void GWB_BuildCtx_AddCommand(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *cmd)
{
  GWB_BuildCmd_List2_PushBack(bctx->commandList, cmd);
}



GWB_FILE_LIST2 *GWB_BuildCtx_GetFileList(const GWB_BUILD_CONTEXT *bctx)
{
  return bctx->fileList;
}



void GWB_BuildCtx_AddFile(GWB_BUILD_CONTEXT *bctx, GWB_FILE *file)
{
  GWB_File_SetId(file, ++(bctx->lastFileId));
  GWB_File_List2_PushBack(bctx->fileList, file);
}



GWB_FILE *GWB_BuildCtx_GetFileByPathAndName(const GWB_BUILD_CONTEXT *bctx, const char *folder, const char *fname)
{
  return GWB_File_List2_GetFileByPathAndName(bctx->fileList, folder, fname);
}



void GWB_BuildCtx_AddInFileToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE *file)
{
  GWB_FILE *storedFile;

  storedFile=GWB_BuildCtx_GetFileByPathAndName(bctx, GWB_File_GetFolder(file), GWB_File_GetName(file));
  if (storedFile)
    GWB_BuildCmd_AddInFile(bcmd, storedFile);
  else {
    GWB_FILE *fileCopy;

    fileCopy=GWB_File_dup(file);
    GWB_BuildCmd_AddInFile(bcmd, fileCopy);
    GWB_BuildCtx_AddFile(bctx, fileCopy);
  }
}



void GWB_BuildCtx_AddInFilesToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE_LIST2 *fileList)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      GWB_BuildCtx_AddInFileToCtxAndCmd(bctx, bcmd, file);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void GWB_BuildCtx_AddOutFileToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE *file)
{
  GWB_FILE *storedFile;

  storedFile=GWB_BuildCtx_GetFileByPathAndName(bctx, GWB_File_GetFolder(file), GWB_File_GetName(file));
  if (storedFile)
    GWB_BuildCmd_AddOutFile(bcmd, storedFile);
  else {
    GWB_FILE *fileCopy;

    fileCopy=GWB_File_dup(file);
    GWB_BuildCmd_AddOutFile(bcmd, fileCopy);
    GWB_BuildCtx_AddFile(bctx, fileCopy);
  }
}



void GWB_BuildCtx_AddOutFilesToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE_LIST2 *fileList)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      GWB_BuildCtx_AddOutFileToCtxAndCmd(bctx, bcmd, file);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void GWB_BuildCtx_Dump(const GWB_BUILD_CONTEXT *bctx, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "BuildCtx:\n");

  GWBUILD_Debug_PrintBuildCmdList2("commandList", bctx->commandList, indent+2);
  GWBUILD_Debug_PrintFileList2("fileList", bctx->fileList, indent+2);
}






