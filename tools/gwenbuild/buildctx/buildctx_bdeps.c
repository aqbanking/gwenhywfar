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


#include "gwenbuild/buildctx/buildctx_p.h"
#include "gwenbuild/buildctx/buildctx_bdeps.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>
#include <ctype.h>



static void _setupDepsForCmd(GWB_BUILD_CMD *bcmd, GWB_BUILD_CMD_LIST2 *targetCmdList);
static void _setBuildCmdInFiles(GWB_BUILD_CONTEXT *bctx);
static void _fileListSetBuildCmd(GWB_FILE_LIST2 *fileList, GWB_BUILD_CMD *bcmd);
static int _cmdIsInList(const GWB_BUILD_CMD_LIST2 *sourceCmdList, const GWB_BUILD_CMD *cmd);

static void _clearDeps(GWB_BUILD_CONTEXT *bctx);
static void _clearDepsInCommands(GWB_BUILD_CONTEXT *bctx);
static void _clearDepsInFiles(GWB_BUILD_CONTEXT *bctx);





int GWB_BuildCtx_SetupDependencies(GWB_BUILD_CONTEXT *bctx)
{
  _clearDeps(bctx);
  _setBuildCmdInFiles(bctx);
  return 0;
}



int GWB_BuildCtx_FillWaitingQueue(GWB_BUILD_CONTEXT *bctx, const char *builderName)
{
  int rv;

  rv=GWB_BuildCtx_AddBuildCmdsByBuilderNameToList(bctx->commandList, builderName, bctx->waitingQueue);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

#if 0
  DBG_ERROR(NULL, "Got this queue:");
  GWBUILD_Debug_PrintBuildCmdList2("Waiting Queue", bctx->waitingQueue, 2);
#endif

  return 0;
}



int GWB_BuildCtx_AddBuildCmdsForFileToList(const GWB_FILE *file, GWB_BUILD_CMD_LIST2 *targetCmdList)
{
  GWB_BUILD_CMD *bcmd;

  bcmd=GWB_File_GetBuildCmd(file);
  if (bcmd) {
    if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
      GWB_BuildCmd_List2_PushBack(targetCmdList, bcmd);
      _setupDepsForCmd(bcmd, targetCmdList);
    }
    else {
      const char *sBuilderName;
      const char *sFolder;
      const char *sFileName;

      sBuilderName=GWB_BuildCmd_GetBuilderName(bcmd);
      sFileName=GWB_File_GetName(file);
      sFolder=GWB_File_GetFolder(file);
      DBG_ERROR(NULL, "Builder \"%s\" for selected file \"%s\" in folder \"%s\" has no build commands",
                sBuilderName?sBuilderName:"<no name>",
                sFileName?sFileName:"<no name>",
                sFolder?sFolder:".");
    }
  }

  return 0;
}



int GWB_BuildCtx_AddBuildCmdsByBuilderNameToList(const GWB_BUILD_CMD_LIST2 *sourceCmdList,
                                                 const char *builderName,
                                                 GWB_BUILD_CMD_LIST2 *targetCmdList)
{
  if (sourceCmdList) {
    GWB_BUILD_CMD_LIST2_ITERATOR *it;

    it=GWB_BuildCmd_List2_First(sourceCmdList);
    if (it) {
      if (builderName && *builderName) {
        GWB_BUILD_CMD *bcmd;

        bcmd=GWB_BuildCmd_List2Iterator_Data(it);
        while(bcmd) {
          const char *s;

          s=GWB_BuildCmd_GetBuilderName(bcmd);
          if (s && strcasecmp(s, builderName)==0) {
            if (!_cmdIsInList(targetCmdList, bcmd)) {
              if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
                GWB_BuildCmd_List2_PushBack(targetCmdList, bcmd);
                _setupDepsForCmd(bcmd, targetCmdList);
              }
            }
          }
          bcmd=GWB_BuildCmd_List2Iterator_Next(it);
        }
        GWB_BuildCmd_List2Iterator_free(it);
      }
      else {
        GWB_BUILD_CMD *bcmd;

        bcmd=GWB_BuildCmd_List2Iterator_Data(it);
        while(bcmd) {
          if (GWB_BuildCmd_GetFlags(bcmd) & GWB_BUILD_CMD_FLAGS_AUTO) {
            if (!_cmdIsInList(targetCmdList, bcmd)) {
              if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
                GWB_BuildCmd_List2_PushBack(targetCmdList, bcmd);
                _setupDepsForCmd(bcmd, targetCmdList);
              }
            }
          }
          bcmd=GWB_BuildCmd_List2Iterator_Next(it);
        }
        GWB_BuildCmd_List2Iterator_free(it);
      }
    }
  }
  return 0;
}



void _clearDeps(GWB_BUILD_CONTEXT *bctx)
{
  _clearDepsInCommands(bctx);
  _clearDepsInFiles(bctx);
}



void _clearDepsInCommands(GWB_BUILD_CONTEXT *bctx)
{
  if (bctx->commandList) {
    GWB_BUILD_CMD_LIST2_ITERATOR *it;

    it=GWB_BuildCmd_List2_First(bctx->commandList);
    if (it) {
      GWB_BUILD_CMD *bcmd;

      bcmd=GWB_BuildCmd_List2Iterator_Data(it);
      while(bcmd) {
        GWB_BuildCmd_SetBlockingFiles(bcmd, 0);
        bcmd=GWB_BuildCmd_List2Iterator_Next(it);
      }
      GWB_BuildCmd_List2Iterator_free(it);
    }
  }
}



void _clearDepsInFiles(GWB_BUILD_CONTEXT *bctx)
{
  if (bctx->fileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(bctx->fileList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWB_File_ClearWaitingBuildCmds(file);
        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }
}




void _setupDepsForCmd(GWB_BUILD_CMD *bcmd, GWB_BUILD_CMD_LIST2 *targetCmdList)
{
  GWB_FILE_LIST2 *fileList;

  fileList=GWB_BuildCmd_GetInFileList2(bcmd);
  if (fileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWB_BUILD_CMD *fileBuildCmd;

        fileBuildCmd=GWB_File_GetBuildCmd(file);
        if (fileBuildCmd) {
          GWB_File_AddWaitingBuildCmd(file, bcmd);
          GWB_BuildCmd_IncBlockingFiles(bcmd);
          if (!_cmdIsInList(targetCmdList, fileBuildCmd)) {
            if (GWB_BuildCmd_GetCurrentCommand(fileBuildCmd)) {
              GWB_BuildCmd_List2_PushBack(targetCmdList, fileBuildCmd);
              _setupDepsForCmd(fileBuildCmd, targetCmdList); /* recursion! */
            }
          }
        }

        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }
}



void _setBuildCmdInFiles(GWB_BUILD_CONTEXT *bctx)
{
  if (bctx->commandList) {
    GWB_BUILD_CMD_LIST2_ITERATOR *it;

    it=GWB_BuildCmd_List2_First(bctx->commandList);
    if (it) {
      GWB_BUILD_CMD *bcmd;

      bcmd=GWB_BuildCmd_List2Iterator_Data(it);
      while(bcmd) {
        _fileListSetBuildCmd(GWB_BuildCmd_GetOutFileList2(bcmd), bcmd);
        bcmd=GWB_BuildCmd_List2Iterator_Next(it);
      }
      GWB_BuildCmd_List2Iterator_free(it);
    }
  }
}



void _fileListSetBuildCmd(GWB_FILE_LIST2 *fileList, GWB_BUILD_CMD *bcmd)
{
  if (fileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWB_File_SetBuildCmd(file, bcmd);
        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }
}



int _cmdIsInList(const GWB_BUILD_CMD_LIST2 *sourceCmdList, const GWB_BUILD_CMD *cmd)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(sourceCmdList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    bcmd=GWB_BuildCmd_List2Iterator_Data(it);
    while(bcmd) {
      if (bcmd==cmd) {
        GWB_BuildCmd_List2Iterator_free(it);
        return 1;
      }
      bcmd=GWB_BuildCmd_List2Iterator_Next(it);
    }
    GWB_BuildCmd_List2Iterator_free(it);
  }

  return 0;
}






