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



#if 1




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



void GWB_BuildCtx_FillWaitingQueue(GWB_BUILD_CONTEXT *bctx, const char *builderName)
{
  GWB_BuildCmd_List2_free(bctx->waitingQueue);
  bctx->waitingQueue=GWB_BuildCtx_CreateBuildListForBuilders(bctx->commandList, builderName);

#if 0
  DBG_ERROR(NULL, "Got this queue:");
  GWBUILD_Debug_PrintBuildCmdList2("Waiting Queue", bctx->waitingQueue, 2);
#endif
}



GWB_BUILD_CMD_LIST2 *GWB_BuildCtx_CreateBuildListForFile(const GWB_FILE *file)
{
  GWB_BUILD_CMD_LIST2 *cmdList;
  GWB_BUILD_CMD *bcmd;

  cmdList=GWB_BuildCmd_List2_new();
  bcmd=GWB_File_GetBuildCmd(file);
  if (bcmd) {
    if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
      GWB_BuildCmd_List2_PushBack(cmdList, bcmd);
      _setupDepsForCmd(bcmd, cmdList);
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
      GWB_BuildCmd_List2_free(cmdList);
      return NULL;
    }
  }

  if (GWB_BuildCmd_List2_GetSize(cmdList)==0) {
    GWB_BuildCmd_List2_free(cmdList);
    return NULL;
  }
  return cmdList;
}



GWB_BUILD_CMD_LIST2 *GWB_BuildCtx_CreateBuildListForBuilders(const GWB_BUILD_CMD_LIST2 *sourceCmdList, const char *builderName)
{
  if (sourceCmdList) {
    GWB_BUILD_CMD_LIST2_ITERATOR *it;

    it=GWB_BuildCmd_List2_First(sourceCmdList);
    if (it) {
      GWB_BUILD_CMD_LIST2 *cmdList;

      cmdList=GWB_BuildCmd_List2_new();

      if (builderName && *builderName) {
        GWB_BUILD_CMD *bcmd;

        bcmd=GWB_BuildCmd_List2Iterator_Data(it);
        while(bcmd) {
          const char *s;

          s=GWB_BuildCmd_GetBuilderName(bcmd);
          if (s && strcasecmp(s, builderName)==0) {
            if (!_cmdIsInList(cmdList, bcmd)) {
              if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
                GWB_BuildCmd_List2_PushBack(cmdList, bcmd);
                _setupDepsForCmd(bcmd, cmdList);
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
          if (GWB_BuildCmd_GetFlags(bcmd) &GWB_BUILD_CMD_FLAGS_AUTO) {
            if (!_cmdIsInList(cmdList, bcmd)) {
              if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
                GWB_BuildCmd_List2_PushBack(cmdList, bcmd);
                _setupDepsForCmd(bcmd, cmdList);
              }
            }
          }
          bcmd=GWB_BuildCmd_List2Iterator_Next(it);
        }
        GWB_BuildCmd_List2Iterator_free(it);
      }

      if (GWB_BuildCmd_List2_GetSize(cmdList)==0) {
        GWB_BuildCmd_List2_free(cmdList);
        return NULL;
      }
      return cmdList;
    }
  }
  return NULL;
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





















#else






static void _initiallyFillQueues(GWB_BUILD_CONTEXT *bctx, const char *builderName);
static void _setupDepsForCmd(GWB_BUILD_CMD *bcmd);
static void _clearDeps(GWB_BUILD_CONTEXT *bctx);
static void _clearDepsInCommands(GWB_BUILD_CONTEXT *bctx);
static void _clearDepsInFiles(GWB_BUILD_CONTEXT *bctx);

static void _addNamedBuildersToList(GWB_BUILD_CMD_LIST2 *sourceCmdList,
                                    GWB_BUILD_CMD_LIST2 *targetCmdList,
                                    const char *builderName);
static void _addSourceCommandsToListWithDeps(GWB_BUILD_CMD_LIST2 *sourceCmdList, GWB_BUILD_CMD_LIST2 *targetCmdList);
static void _addWaitingCommandsToList(GWB_BUILD_CMD_LIST2 *targetCmdList, GWB_BUILD_CMD *cmd);
static int _cmdIsInList(const GWB_BUILD_CMD_LIST2 *sourceCmdList, const GWB_BUILD_CMD *cmd);





int GWB_BuildCtx_SetupDependencies(GWB_BUILD_CONTEXT *bctx)
{
  _clearDeps(bctx);

  if (bctx->commandList) {
    GWB_BUILD_CMD_LIST2_ITERATOR *it;

    it=GWB_BuildCmd_List2_First(bctx->commandList);
    if (it) {
      GWB_BUILD_CMD *bcmd;

      bcmd=GWB_BuildCmd_List2Iterator_Data(it);
      while(bcmd) {
        _setupDepsForCmd(bcmd);

        bcmd=GWB_BuildCmd_List2Iterator_Next(it);
      }
      GWB_BuildCmd_List2Iterator_free(it);
    }
  }

  return 0;
}




void _setupDepsForCmd(GWB_BUILD_CMD *bcmd)
{
  GWB_FILE_LIST2 *inFileList;
  
  inFileList=GWB_BuildCmd_GetInFileList2(bcmd);
  if (inFileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(inFileList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        if (GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED) {
          GWB_File_AddWaitingBuildCmd(file, bcmd);
          GWB_BuildCmd_IncBlockingFiles(bcmd);
        }

        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }
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



void _initiallyFillQueues(GWB_BUILD_CONTEXT *bctx, const char *builderName)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(bctx->commandList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    if (builderName && *builderName) {
      /* builder name given, don't check for auto flag */
      bcmd=GWB_BuildCmd_List2Iterator_Data(it);
      while(bcmd) {
        const char *s;

        s=GWB_BuildCmd_GetBuilderName(bcmd);
        if (s && strcasecmp(s, builderName)==0) {
          if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
            GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
          }
        }
        bcmd=GWB_BuildCmd_List2Iterator_Next(it);
      }
    }
    else {
      /* no builder name, only add commands with auto flag set */
      bcmd=GWB_BuildCmd_List2Iterator_Data(it);
      while(bcmd) {
        if (GWB_BuildCmd_GetFlags(bcmd) &GWB_BUILD_CMD_FLAGS_AUTO) {
          if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
            GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
          }
        }
        bcmd=GWB_BuildCmd_List2Iterator_Next(it);
      }
    }
    GWB_BuildCmd_List2Iterator_free(it);
  }
}



void GWB_BuildCtx_FillWaitingQueue(GWB_BUILD_CONTEXT *bctx, const char *builderName)
{
  GWB_BUILD_CMD_LIST2 *wantedCmdList;

  wantedCmdList=GWB_BuildCmd_List2_new();
  _addNamedBuildersToList(bctx->commandList, wantedCmdList, builderName);
  _addSourceCommandsToListWithDeps(wantedCmdList, bctx->waitingQueue);
  GWB_BuildCmd_List2_free(wantedCmdList);

  DBG_ERROR(NULL, "Got this queue:");
  GWBUILD_Debug_PrintBuildCmdList2("Waiting Queue", bctx->waitingQueue, 2);

}



void _addNamedBuildersToList(GWB_BUILD_CMD_LIST2 *sourceCmdList,
                             GWB_BUILD_CMD_LIST2 *targetCmdList,
                             const char *builderName)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(sourceCmdList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    /* builder name given, don't check for auto flag */
    bcmd=GWB_BuildCmd_List2Iterator_Data(it);
    while(bcmd) {
      const char *s;

      s=GWB_BuildCmd_GetBuilderName(bcmd);
      if (builderName==NULL || (s && strcasecmp(s, builderName)==0)) {
        if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
          DBG_ERROR(NULL, "Adding buildCmd \"%s\"", s?s:"<no build name>");
          GWB_BuildCmd_List2_PushBack(targetCmdList, bcmd);
        }
      }
      bcmd=GWB_BuildCmd_List2Iterator_Next(it);
    }
  }
}



void _addSourceCommandsToListWithDeps(GWB_BUILD_CMD_LIST2 *sourceCmdList, GWB_BUILD_CMD_LIST2 *targetCmdList)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  DBG_ERROR(NULL, "Adding source commands to list with deps");
  it=GWB_BuildCmd_List2_First(sourceCmdList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    bcmd=GWB_BuildCmd_List2Iterator_Data(it);
    while(bcmd) {
      const char *sBuilderName;

      sBuilderName=GWB_BuildCmd_GetBuilderName(bcmd);
      DBG_ERROR(NULL, " Handling cmd \"%s\"", sBuilderName?sBuilderName:"<no name>");
      if (!(_cmdIsInList(targetCmdList, bcmd))) {
        DBG_ERROR(NULL, "  Adding buildCmd \"%s\"", sBuilderName?sBuilderName:"<no name>");
        GWB_BuildCmd_List2_PushBack(targetCmdList, bcmd);
        _addWaitingCommandsToList(targetCmdList, bcmd);
      }
      else {
        DBG_ERROR(NULL, "  BuildCmd \"%s\" already in list", sBuilderName?sBuilderName:"<no name>");
      }
      bcmd=GWB_BuildCmd_List2Iterator_Next(it);
    }
  }
  else {
    DBG_ERROR(NULL, "   Empty waiting command list");
  }
  DBG_ERROR(NULL, "Adding source commands to list with deps: Done");
}



void _addWaitingCommandsToList(GWB_BUILD_CMD_LIST2 *targetCmdList, GWB_BUILD_CMD *cmd)
{
  GWB_FILE_LIST2 *inFileList;
  const char *sBuilderName;

  sBuilderName=GWB_BuildCmd_GetBuilderName(cmd);
  DBG_ERROR(NULL, " Adding waiting commands for files in buildCmd \"%s\"", sBuilderName?sBuilderName:"<no build name>");

  inFileList=GWB_BuildCmd_GetInFileList2(cmd);
  if (inFileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(inFileList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWB_BUILD_CMD_LIST2 *waitingCmdList;
        const char *sFileName;

        sFileName=GWB_File_GetName(file);
        DBG_ERROR(NULL, "   Handling file \"%s\"", sFileName?sFileName:"<no file name>");

        waitingCmdList=GWB_File_GetWaitingBuildCmdList2(file);
        if (waitingCmdList)
          _addSourceCommandsToListWithDeps(waitingCmdList, targetCmdList);
        else {
          DBG_ERROR(NULL, "   No waiting command list");
        }
        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }

  DBG_ERROR(NULL, " Adding waiting commands for files in buildCmd \"%s\": Done", sBuilderName?sBuilderName:"<no build name>");
}



int _cmdIsInList(const GWB_BUILD_CMD_LIST2 *sourceCmdList, const GWB_BUILD_CMD *cmd)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(sourceCmdList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    /* builder name given, don't check for auto flag */
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



#endif


