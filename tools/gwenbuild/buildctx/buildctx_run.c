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
#include "gwenbuild/buildctx/buildctx_run.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>
#include <ctype.h>




static void _setupCommands(GWB_BUILD_CONTEXT *bctx, int forPrepareCommands);
static void _createCommandQueues(GWB_BUILD_CONTEXT *bctx);
static int _checkWaitingQueue(GWB_BUILD_CONTEXT *bctx, int maxStartAllowed);
static int _startCommand(GWB_BUILD_CMD *bcmd, const GWEN_STRINGLIST *slOutFiles);
static int _checkRunningQueue(GWB_BUILD_CONTEXT *bctx);
static void _signalJobFinished(GWB_BUILD_CMD *bcmd);
static void _decBlockingFilesInWaitingBuildCommands(GWB_BUILD_CMD_LIST2 *waitingCommands);
static void _abortAllCommands(GWB_BUILD_CONTEXT *bctx);
static void _abortCommandsInQueue(GWB_BUILD_CMD_LIST2 *cmdList);

static int _needRunCurrentCommand(GWB_BUILD_CMD *bcmd, const GWEN_STRINGLIST *slInFiles, const GWEN_STRINGLIST *slOutFiles);
static void _finishCurrentCommand(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *currentCommand);

static int _checkDependencies(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *subCmd, const char *firstOutFileName);
static int _checkDatesOfFileAgainstList(const char *fileName, const GWEN_STRINGLIST *sl);
static GWEN_STRINGLIST *_getAbsoluteDeps(const char *folder, const char *fileName);
static GWEN_STRINGLIST *_makeAbsolutePaths(GWEN_STRINGLIST *slInput, const char *folder);
static GWEN_STRINGLIST *_readDepFile(const char *fileName);

static int _inFilesNewerThanOutFiles(const GWEN_STRINGLIST *slInFiles, const GWEN_STRINGLIST *slOutFiles);
static time_t _getHighestModificationTime(const GWEN_STRINGLIST *slFiles);
static time_t _getLowestModificationTime(const GWEN_STRINGLIST *slFiles);
static void _unlinkFilesInStringList(const GWEN_STRINGLIST *slFiles);





int GWB_BuildCtx_Run(GWB_BUILD_CONTEXT *bctx, int maxConcurrentJobs, int usePrepareCommands, const char *builderName)
{
  int waitingJobs;
  int runningJobs;

  GWB_BuildCtx_SetupDependencies(bctx);
  _setupCommands(bctx, usePrepareCommands);
  _createCommandQueues(bctx);
  GWB_BuildCtx_FillWaitingQueue(bctx, builderName);

  if (bctx->waitingQueue==NULL) {
    fprintf(stdout, "Nothing to do.\n");
    return 0;
  }

  waitingJobs=GWB_BuildCmd_List2_GetSize(bctx->waitingQueue);
  runningJobs=GWB_BuildCmd_List2_GetSize(bctx->runningQueue);
  while(waitingJobs+runningJobs) {
    int startedCommands;
    int changedCommands;

    startedCommands=_checkWaitingQueue(bctx, maxConcurrentJobs-runningJobs);
    if (startedCommands<0) {
      _abortAllCommands(bctx);
      return GWEN_ERROR_GENERIC;
    }

    changedCommands=_checkRunningQueue(bctx);
    if (changedCommands<0) { /* error */
      _abortAllCommands(bctx);
      return GWEN_ERROR_GENERIC;
    }

    if (startedCommands==0 && changedCommands==0) {
      if (runningJobs==0) {
        DBG_ERROR(NULL, "ERROR: No running jobs and none could be started, maybe circular dependencies?");
        _abortAllCommands(bctx);
        return GWEN_ERROR_GENERIC;
      }
      DBG_DEBUG(NULL, "Nothing changed, sleeping...");
      sleep(3);
    }

    waitingJobs=GWB_BuildCmd_List2_GetSize(bctx->waitingQueue);
    runningJobs=GWB_BuildCmd_List2_GetSize(bctx->runningQueue);
  } /* while */

  GWB_BuildCmd_List2_free(bctx->waitingQueue);
  GWB_BuildCmd_List2_free(bctx->runningQueue);
  GWB_BuildCmd_List2_free(bctx->finishedQueue);

  return 0;
}



void _abortAllCommands(GWB_BUILD_CONTEXT *bctx)
{
  _abortCommandsInQueue(bctx->waitingQueue);
  GWB_BuildCmd_List2_free(bctx->waitingQueue);

  _abortCommandsInQueue(bctx->runningQueue);
  GWB_BuildCmd_List2_free(bctx->runningQueue);

  _abortCommandsInQueue(bctx->finishedQueue);
  GWB_BuildCmd_List2_free(bctx->finishedQueue);
}



void _abortCommandsInQueue(GWB_BUILD_CMD_LIST2 *cmdList)
{
  GWB_BUILD_CMD *bcmd;

  while( (bcmd=GWB_BuildCmd_List2_GetFront(cmdList)) ) {
    GWB_BuildCmd_List2_PopFront(cmdList);
    GWB_BuildCmd_SetCurrentProcess(bcmd, NULL);
  } /* while */
}




void _setupCommands(GWB_BUILD_CONTEXT *bctx, int forPrepareCommands)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(bctx->commandList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    bcmd=GWB_BuildCmd_List2Iterator_Data(it);
    while(bcmd) {
      GWB_BUILD_SUBCMD_LIST *cmdList;
  
      if (forPrepareCommands)
        cmdList=GWB_BuildCmd_GetPrepareCommandList(bcmd);
      else
        cmdList=GWB_BuildCmd_GetBuildCommandList(bcmd);
      if (cmdList)
        GWB_BuildCmd_SetCurrentCommand(bcmd, GWB_BuildSubCmd_List_First(cmdList));
      bcmd=GWB_BuildCmd_List2Iterator_Next(it);
    }
    GWB_BuildCmd_List2Iterator_free(it);
  }
}



void _createCommandQueues(GWB_BUILD_CONTEXT *bctx)
{
  bctx->waitingQueue=GWB_BuildCmd_List2_new();
  bctx->finishedQueue=GWB_BuildCmd_List2_new();
  bctx->runningQueue=GWB_BuildCmd_List2_new();
}



int _checkWaitingQueue(GWB_BUILD_CONTEXT *bctx, int maxStartAllowed)
{
  GWB_BUILD_CMD_LIST2 *oldQueue;
  GWB_BUILD_CMD *bcmd;
  int started=0;
  int errors=0;

  oldQueue=bctx->waitingQueue;
  bctx->waitingQueue=GWB_BuildCmd_List2_new();

  while( (bcmd=GWB_BuildCmd_List2_GetFront(oldQueue)) ) {

    GWB_BuildCmd_List2_PopFront(oldQueue);
    if (started<maxStartAllowed) {
      if (GWB_BuildCmd_GetBlockingFiles(bcmd)==0) {
        int rv;
        GWEN_STRINGLIST *slInFiles;
        GWEN_STRINGLIST *slOutFiles;

        slInFiles=GWB_File_FileListToTopBuildDirStringList(GWB_BuildCmd_GetInFileList2(bcmd),
                                                           bctx->initialSourceDir);
        slOutFiles=GWB_File_FileListToTopBuildDirStringList(GWB_BuildCmd_GetOutFileList2(bcmd),
                                                            bctx->initialSourceDir);
        if (_needRunCurrentCommand(bcmd, slInFiles, slOutFiles)) {

	  rv=_startCommand(bcmd, slOutFiles);
	  if (rv<0) {
	    GWB_BuildCmd_List2_PushBack(bctx->finishedQueue, bcmd);
	    errors++;
	  }
	  else {
	    GWB_BuildCmd_List2_PushBack(bctx->runningQueue, bcmd);
	    started++;
	  }
	}
	else {
	  _finishCurrentCommand(bctx, bcmd, GWB_BuildCmd_GetCurrentCommand(bcmd));
	  started++;
        }
        GWEN_StringList_free(slOutFiles);
        GWEN_StringList_free(slInFiles);
      }
      else
        GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
    }
    else
      GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
  } /* while */
  GWB_BuildCmd_List2_free(oldQueue);

  if (errors)
    return GWEN_ERROR_GENERIC;
  return started;
}



GWEN_STRINGLIST *_fileListToTopBuildDirStringList(const char *initialSourceDir, GWB_FILE_LIST2 *fileList)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWEN_STRINGLIST *sl;
    GWB_FILE *file;
    GWEN_BUFFER *fbuf;

    sl=GWEN_StringList_new();
    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      GWB_File_WriteFileNameToTopBuildDirString(file, initialSourceDir, fbuf);
      GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(fbuf), 0, 1);
      GWEN_Buffer_Reset(fbuf);
      file=GWB_File_List2Iterator_Next(it);
    } /* while */
    GWEN_Buffer_Reset(fbuf);
    GWB_File_List2Iterator_free(it);

    if (GWEN_StringList_Count(sl)==0) {
      GWEN_StringList_free(sl);
      return NULL;
    }
    return sl;
  }

  return NULL;
}



int _needRunCurrentCommand(GWB_BUILD_CMD *bcmd, const GWEN_STRINGLIST *slInFiles, const GWEN_STRINGLIST *slOutFiles)
{
  GWB_BUILD_SUBCMD *currentCommand;

  currentCommand=GWB_BuildCmd_GetCurrentCommand(bcmd);
  if (currentCommand) {
    uint32_t cmdFlags;
    uint32_t subCmdFlags;

    cmdFlags=GWB_BuildCmd_GetFlags(bcmd);
    subCmdFlags=GWB_BuildSubCmd_GetFlags(currentCommand);

    if (cmdFlags & GWB_BUILD_CMD_FLAGS_CHECK_DATES) {
      if (_inFilesNewerThanOutFiles(slInFiles, slOutFiles)) {
        /* need rebuild */
        DBG_INFO(NULL, "Input files newer than output files, rebuild needed");
        return 1;
      }
    }
    else
      /* dont check dates, always rebuild */
      return 1;

    if (subCmdFlags & GWB_BUILD_SUBCMD_FLAGS_CHECK_DEPENDS) {
      int rv;

      rv=_checkDependencies(bcmd, currentCommand, GWEN_StringList_FirstString(slOutFiles));
      if (rv==-1) {
        DBG_INFO(NULL, "Dependencies flag NO rebuild needed (%d)", rv);
        return 0;
      }
      DBG_INFO(NULL, "Dependencies flag rebuild needed (%d)", rv);
      return 1;
    }
  }

  DBG_INFO(NULL, "Rebuild not needed");
  return 0;
}



/* return 0: no rebuild needed; 1: rebuild needed */
int _inFilesNewerThanOutFiles(const GWEN_STRINGLIST *slInFiles, const GWEN_STRINGLIST *slOutFiles)
{
  time_t tiHighestInFileTime;
  time_t tiLowestOutFileTime;

  tiHighestInFileTime=_getHighestModificationTime(slInFiles);
  tiLowestOutFileTime=_getLowestModificationTime(slOutFiles);
  if (tiHighestInFileTime==0 || tiLowestOutFileTime==0) {
    DBG_INFO(NULL, "Either input or output time not available");
    return 1;
  }
  if (tiHighestInFileTime>tiLowestOutFileTime)
    return 1;
  return 0;
}



time_t _getLowestModificationTime(const GWEN_STRINGLIST *slFiles)
{
  time_t tiLowest=0;

  if (slFiles) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(slFiles);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        time_t tiFile;

        tiFile=GWBUILD_GetModificationTimeOfFile(s);
        if (tiFile>0) {
          if (tiLowest==0)
            tiLowest=tiFile;
          else if (tiFile<tiLowest)
            tiLowest=tiFile;
        }
        else {
          DBG_INFO(NULL, "No modification time for \"%s\"", s);
          return 0;
        }
      }

      se=GWEN_StringListEntry_Next(se);
    }
  }

  return tiLowest;
}



time_t _getHighestModificationTime(const GWEN_STRINGLIST *slFiles)
{
  time_t tiHighest=0;

  if (slFiles) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(slFiles);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        time_t tiFile;

        tiFile=GWBUILD_GetModificationTimeOfFile(s);
        if (tiFile>0) {
          if (tiHighest==0)
            tiHighest=tiFile;
          else if (tiFile>tiHighest)
            tiHighest=tiFile;
        }
        else {
          DBG_INFO(NULL, "No modification time for \"%s\"", s);
          return 0;
        }
      }

      se=GWEN_StringListEntry_Next(se);
    }
  }

  return tiHighest;
}



void _unlinkFilesInStringList(const GWEN_STRINGLIST *slFiles)
{
  if (slFiles) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(slFiles);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        DBG_DEBUG(NULL, "Deleting \"%s\"", s);
        unlink(s);
      }

      se=GWEN_StringListEntry_Next(se);
    }
  }
}



/* return 1: need rebuild, -1: Need no rebuild, 0: undecided */
int _checkDependencies(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *subCmd, const char *firstOutFileName)
{
  const char *depFileName;
  
  depFileName=GWB_BuildSubCmd_GetDepFilePath(subCmd);
  if (depFileName && firstOutFileName) {
    GWEN_STRINGLIST *sl;

    DBG_DEBUG(NULL, "Checking depend file \"%s\"", depFileName);
    sl=_getAbsoluteDeps(GWB_BuildCmd_GetFolder(bcmd), depFileName);
    if (sl) {
      int rv;

      //GWBUILD_Debug_PrintStringList(depFileName, sl, 2);
      rv=_checkDatesOfFileAgainstList(firstOutFileName, sl);
      GWEN_StringList_free(sl);
      return rv;
    }
    else {
      DBG_DEBUG(NULL, "Could not load depend file \"%s\"", depFileName);
    }
  }
  else {
    if (depFileName==NULL) {
      DBG_DEBUG(NULL, "No depFileName for %s", firstOutFileName?firstOutFileName:"<no outfile name>");
    }
    if (firstOutFileName==NULL) {
      DBG_DEBUG(NULL, "No outFileName");
    }
  }

  return 0; /* indeterminate */
}



int _checkDatesOfFileAgainstList(const char *fileName, const GWEN_STRINGLIST *sl)
{
  time_t tFile;
  GWEN_STRINGLISTENTRY *se;

  tFile=GWBUILD_GetModificationTimeOfFile(fileName);
  if (tFile==0) {
    DBG_DEBUG(NULL, "%s: No modification time, need rebuild", fileName);
    return 1; /* need rebuild */
  }
  se=GWEN_StringList_FirstEntry(sl);
  if (se) {
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
	time_t tCurrent;

        DBG_DEBUG(NULL, "  Checking dep: %s", s);
	tCurrent=GWBUILD_GetModificationTimeOfFile(s);
        if (tCurrent==0) {
          DBG_DEBUG(NULL, "No modification time for dependency \"%s\", need rebuild", s);
          return 1; /* need rebuild */
        }
	if (difftime(tFile, tCurrent)<0.0) {
          DBG_DEBUG(NULL, "File \"%s\" is newer than \"%s\", rebuild needed", s, fileName);
	  return 1; /* definately need rebuild */
	}
      }

      se=GWEN_StringListEntry_Next(se);
    }

    DBG_DEBUG(NULL, "No dependency is newer than file \"%s\", NO rebuild needed", fileName);
    return -1; /* definately no need for rebuild */
  }
  else {
    DBG_DEBUG(NULL, "Empty dependency list, rebuild needed");
  }

  return 0;
}



int _startCommand(GWB_BUILD_CMD *bcmd, const GWEN_STRINGLIST *slOutFiles)
{
  GWB_BUILD_SUBCMD *currentCommand;

  currentCommand=GWB_BuildCmd_GetCurrentCommand(bcmd);
  if (currentCommand) {
    const char *folder;
    const char *cmd;
    const char *args;

    folder=GWB_BuildCmd_GetFolder(bcmd);
    cmd=GWB_BuildSubCmd_GetCommand(currentCommand);
    args=GWB_BuildSubCmd_GetArguments(currentCommand);

    if (cmd && *cmd) {
      GWEN_PROCESS *process;
      GWEN_PROCESS_STATE pstate;
      const char *buildMessage;

      if (GWB_BuildSubCmd_List_Previous(currentCommand)==NULL) {
        /* first command */
        if (slOutFiles && (GWB_BuildCmd_GetFlags(bcmd) & GWB_BUILD_CMD_FLAGS_DEL_OUTFILES)) {
          _unlinkFilesInStringList(slOutFiles);
        }
      }

      buildMessage=GWB_BuildSubCmd_GetBuildMessage(currentCommand);
      if (buildMessage)
        fprintf(stdout, "%s [%s]\n", buildMessage, cmd);
      else
        fprintf(stdout, "%s %s\n", cmd, args);

      process=GWEN_Process_new();
      if (folder && *folder)
        GWEN_Process_SetFolder(process, folder);
      GWB_BuildCmd_SetCurrentProcess(bcmd, process);
      pstate=GWEN_Process_Start(process, cmd, args);
      if (pstate!=GWEN_ProcessStateRunning) {
        DBG_ERROR(NULL, "Error starting command process (%d)", pstate);
        GWB_BuildCmd_SetCurrentProcess(bcmd, NULL);
        return GWEN_ERROR_GENERIC;
      }
      DBG_DEBUG(NULL, "Process started");
      return 0;
    }
    else {
      DBG_ERROR(NULL, "No command in build command");
      return GWEN_ERROR_GENERIC;
    }
  }
  else {
    DBG_ERROR(NULL, "No current command in build command");
    return GWEN_ERROR_GENERIC;
  }
}



int _checkRunningQueue(GWB_BUILD_CONTEXT *bctx)
{
  GWB_BUILD_CMD_LIST2 *oldRunningQueue;
  GWB_BUILD_CMD *bcmd;
  int changes=0;
  int errors=0;

  oldRunningQueue=bctx->runningQueue;
  bctx->runningQueue=GWB_BuildCmd_List2_new();

  while( (bcmd=GWB_BuildCmd_List2_GetFront(oldRunningQueue)) ) {
    GWEN_PROCESS *process;
    GWEN_PROCESS_STATE pstate;
    GWB_BUILD_SUBCMD *currentCommand;

    GWB_BuildCmd_List2_PopFront(oldRunningQueue);
    currentCommand=GWB_BuildCmd_GetCurrentCommand(bcmd);
    process=GWB_BuildCmd_GetCurrentProcess(bcmd);
    pstate=GWEN_Process_CheckState(process);
    if (pstate!=GWEN_ProcessStateRunning) {
      changes++;
      if (pstate==GWEN_ProcessStateExited) {
        int result;

        result=GWEN_Process_GetResult(process);
        if (result && !(GWB_BuildSubCmd_GetFlags(currentCommand) & GWB_BUILD_SUBCMD_FLAGS_IGNORE_RESULT)) {
          DBG_ERROR(NULL, "Command exited with result %d", result);
          GWB_BuildCmd_List2_PushBack(bctx->finishedQueue, bcmd);
          errors++;
        }
	else
	  _finishCurrentCommand(bctx, bcmd, currentCommand);
      }
      else {
        DBG_ERROR(NULL, "Command aborted (status: %d)", pstate);
        GWB_BuildCmd_List2_PushBack(bctx->finishedQueue, bcmd);
        errors++;
      }
      GWB_BuildCmd_SetCurrentProcess(bcmd, NULL); /* no longer running */
    }
    else
      GWB_BuildCmd_List2_PushBack(bctx->runningQueue, bcmd); /* still running, put back */
  } /* while still commands in running queue */

  GWB_BuildCmd_List2_free(oldRunningQueue);

  if (errors)
    return GWEN_ERROR_GENERIC;
  return changes;
}



void _finishCurrentCommand(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *currentCommand)
{
  GWB_BUILD_SUBCMD *nextCommand;
  
  nextCommand=GWB_BuildSubCmd_List_Next(currentCommand);
  GWB_BuildCmd_SetCurrentCommand(bcmd, nextCommand);
  if (nextCommand)
    GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
  else {
    _signalJobFinished(bcmd);
    GWB_BuildCmd_List2_PushBack(bctx->finishedQueue, bcmd);
  }
}



void _signalJobFinished(GWB_BUILD_CMD *bcmd)
{
  GWB_FILE_LIST2 *outFileList;

  outFileList=GWB_BuildCmd_GetOutFileList2(bcmd);
  if (outFileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(outFileList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWB_BUILD_CMD_LIST2 *waitingCommands;

        waitingCommands=GWB_File_GetWaitingBuildCmdList2(file);
        if (waitingCommands)
          _decBlockingFilesInWaitingBuildCommands(waitingCommands);
        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }

}



void _decBlockingFilesInWaitingBuildCommands(GWB_BUILD_CMD_LIST2 *waitingCommands)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(waitingCommands);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    bcmd=GWB_BuildCmd_List2Iterator_Data(it);
    while(bcmd) {
      GWB_BuildCmd_DecBlockingFiles(bcmd);
      bcmd=GWB_BuildCmd_List2Iterator_Next(it);
    }

    GWB_BuildCmd_List2Iterator_free(it);
  }
}



GWEN_STRINGLIST *_getAbsoluteDeps(const char *folder, const char *fileName)
{
  GWEN_STRINGLIST *slInput;

  slInput=_readDepFile(fileName);
  if (slInput) {
    GWEN_STRINGLIST *slOutput;

    slOutput=_makeAbsolutePaths(slInput, folder);
    if (slOutput) {
      GWEN_StringList_free(slInput);
      return slOutput;
    }
    GWEN_StringList_free(slInput);
  }

  return NULL;
}



GWEN_STRINGLIST *_readDepFile(const char *fileName)
{
  GWEN_BUFFER *fileBuffer;
  int rv;
  char *s;

  fileBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Helper_ReadFile(fileName, fileBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "here (%d)", rv);
    GWEN_Buffer_free(fileBuffer);
    return NULL;
  }

  s=strchr(GWEN_Buffer_GetStart(fileBuffer), ':');
  if (s) {
    GWEN_STRINGLIST *slDependencies;

    slDependencies=GWEN_StringList_fromString2(s+1, " ",
					       1,
					       GWEN_TEXT_FLAGS_DEL_QUOTES |
					       GWEN_TEXT_FLAGS_CHECK_BACKSLASH |
					       GWEN_TEXT_FLAGS_DEL_MULTIPLE_BLANKS|
					       GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS|
					       GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS);
    if (slDependencies) {
      GWEN_Buffer_free(fileBuffer);
      return slDependencies;
    }
  }
  GWEN_Buffer_free(fileBuffer);
  return NULL;
}



GWEN_STRINGLIST *_makeAbsolutePaths(GWEN_STRINGLIST *slInput, const char *folder)
{
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(slInput);
  if (se) {
    GWEN_STRINGLIST *slOutput;

    slOutput=GWEN_StringList_new();
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s) {
	while(*s && *s<33)
	  s++;
	if (*s) {
	  if (*s=='/') {
	    GWEN_BUFFER *buf;

	    buf=GWEN_Buffer_new(0, 256, 0, 1);
	    while(*s && *s>31)
	      GWEN_Buffer_AppendByte(buf, *(s++));
	    GWEN_StringList_AppendString(slOutput, GWEN_Buffer_GetStart(buf), 0, 1);
	    GWEN_Buffer_free(buf);
	  }
	  else {
	    const char *extPtr;

	    extPtr=strrchr(s, '/');
	    if (extPtr) {
	      GWEN_BUFFER *buf;
	      GWEN_BUFFER *absBuf;
	      const char *sTmp;

	      buf=GWEN_Buffer_new(0, 256, 0, 1);
	      if (folder) {
		GWEN_Buffer_AppendString(buf, folder);
		GWEN_Buffer_AppendString(buf, GWEN_DIR_SEPARATOR_S);
	      }
	      GWEN_Buffer_AppendBytes(buf, s, extPtr-s); /* exclude '/' */
	      absBuf=GWEN_Buffer_new(0, 256, 0, 1);
	      GWEN_Directory_GetAbsoluteFolderPath(GWEN_Buffer_GetStart(buf), absBuf);
	      if (GWEN_Buffer_GetUsedBytes(absBuf))
		GWEN_Buffer_AppendString(absBuf, GWEN_DIR_SEPARATOR_S);

	      extPtr++;
	      sTmp=extPtr;
	      while(*sTmp && !isspace(*sTmp))
		sTmp++;
	      GWEN_Buffer_AppendBytes(absBuf, extPtr, sTmp-extPtr);
	      GWEN_StringList_AppendString(slOutput, GWEN_Buffer_GetStart(absBuf), 0, 1);
	      GWEN_Buffer_free(absBuf);
	      GWEN_Buffer_free(buf);
	    }
	  }
	}
      }

      se=GWEN_StringListEntry_Next(se);
    }
    if (GWEN_StringList_Count(slOutput)==0) {
      GWEN_StringList_free(slOutput);
      return NULL;
    }

    return slOutput;
  }

  return NULL;
}

