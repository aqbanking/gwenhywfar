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
#include <gwenhywfar/gwentime.h>

#include <unistd.h>
#include <ctype.h>


static int _prepareBaseLogPath(GWB_BUILD_CONTEXT *bctx);



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



const char *GWB_BuildCtx_GetLogPath(const GWB_BUILD_CONTEXT *bctx)
{
  return bctx->logPath;
}



void GWB_BuildCtx_SetLogPath(GWB_BUILD_CONTEXT *bctx, const char *s)
{
  free(bctx->logPath);
  bctx->logPath=s?strdup(s):NULL;
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
  DBG_INFO(NULL, "Adding file%s/%s", GWB_File_GetFolder(file), GWB_File_GetName(file));
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
    GWB_BuildCtx_AddFile(bctx, fileCopy);
    GWB_BuildCmd_AddInFile(bcmd, fileCopy);
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
    GWB_BuildCtx_AddFile(bctx, fileCopy);
    GWB_BuildCmd_AddOutFile(bcmd, fileCopy);
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



int GWB_BuildCtx_CreateAndSetLogFilenameForSubCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_SUBCMD *cmd)
{
  GWEN_BUFFER *dbuf;
  int rv;

  if (bctx->logPath==NULL) {
    rv=_prepareBaseLogPath(bctx);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(dbuf, bctx->logPath);

#if 0
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(dbuf), 0);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error opening/creating folder \"%s\": %d", GWEN_Buffer_GetStart(dbuf), rv);
    GWEN_Buffer_free(dbuf);
    return rv;
  }
#endif
  /*GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S);*/
  GWEN_Buffer_AppendString(dbuf, "-");
  GWEN_Buffer_AppendArgs(dbuf, "%05d", ++(bctx->lastLogfileId));
  GWEN_Buffer_AppendString(dbuf, ".log");

  GWB_BuildSubCmd_SetLogFilename(cmd, GWEN_Buffer_GetStart(dbuf));
  GWEN_Buffer_free(dbuf);

  return 0;
}



int _prepareBaseLogPath(GWB_BUILD_CONTEXT *bctx)
{
  GWEN_BUFFER *dbuf;
  GWEN_TIME *ti;
  int rv;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  ti=GWEN_CurrentTime();

  GWEN_Buffer_AppendString(dbuf, ".logs");
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(dbuf), 0);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error getting path \"%s\": %d", GWEN_Buffer_GetStart(dbuf), rv);
    GWEN_Time_free(ti);
    GWEN_Buffer_free(dbuf);
    return rv;
  }
  GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S);

  rv=GWEN_Time_toString(ti, "YYYYMMDD-hhmmss", dbuf);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error adding time string to buffer: %d", rv);
    GWEN_Time_free(ti);
    GWEN_Buffer_free(dbuf);
    return rv;
  }

  GWB_BuildCtx_SetLogPath(bctx, GWEN_Buffer_GetStart(dbuf));
  GWEN_Time_free(ti);
  GWEN_Buffer_free(dbuf);

  return 0;
}


