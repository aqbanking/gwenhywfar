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


#include "gwenbuild/buildctx/buildctx_p.h"

#include <gwenhywfar/debug.h>

#include <unistd.h>



static void _setupDepsForCmd(GWB_BUILD_CMD *bcmd);

void _writeFileList2ToXml(const GWB_FILE_LIST2 *fileList, GWEN_XMLNODE *xmlNode, const char *groupName);
void _readFilesFromXml(GWEN_XMLNODE *xmlNode, const char *groupName, GWB_FILE_LIST2 *destFileList);

void _writeCommandList2ToXml(const GWB_BUILD_CMD_LIST2 *commandList, GWEN_XMLNODE *xmlNode, const char *groupName);
void _readCommandsFromXml(GWB_BUILD_CONTEXT *bctx, GWEN_XMLNODE *xmlNode, const char *groupName);

void _writeFileFlagsToXml(uint32_t flags, GWEN_XMLNODE *xmlNode, const char *varName);
uint32_t _readFlagsFromChar(const char *flagsAsText);

int _setupDependencies(GWB_BUILD_CONTEXT *bctx);

void _clearDeps(GWB_BUILD_CONTEXT *bctx);
void _clearDepsInCommands(GWB_BUILD_CONTEXT *bctx);
void _clearDepsInFiles(GWB_BUILD_CONTEXT *bctx);

void _setupCommands(GWB_BUILD_CONTEXT *bctx, int forPrepareCommands);
void _initiallyFillQueues(GWB_BUILD_CONTEXT *bctx);
void _createCommandQueues(GWB_BUILD_CONTEXT *bctx);
int _checkWaitingQueue(GWB_BUILD_CONTEXT *bctx, int maxStartAllowed);
int _startCommand(GWB_BUILD_CMD *bcmd);
int _checkRunningQueue(GWB_BUILD_CONTEXT *bctx);
void _signalJobFinished(GWB_BUILD_CMD *bcmd);
void _decBlockingFilesInWaitingBuildCommands(GWB_BUILD_CMD_LIST2 *waitingCommands);
void _abortAllCommands(GWB_BUILD_CONTEXT *bctx);
void _abortCommandsInQueue(GWB_BUILD_CMD_LIST2 *cmdList);




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

    GWEN_FREE_OBJECT(bctx);
  }
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



int _setupDependencies(GWB_BUILD_CONTEXT *bctx)
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






void GWB_BuildCtx_toXml(const GWB_BUILD_CONTEXT *bctx, GWEN_XMLNODE *xmlNode)
{
  if (bctx->fileList) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "fileList");
    _writeFileList2ToXml(bctx->fileList, xmlGroupNode, "file");
    GWEN_XMLNode_AddChild(xmlNode, xmlGroupNode);
  }

  if (bctx->commandList) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "commandList");
    _writeCommandList2ToXml(bctx->commandList, xmlGroupNode, "command");
    GWEN_XMLNode_AddChild(xmlNode, xmlGroupNode);
  }
}



GWB_BUILD_CONTEXT *GWB_BuildCtx_fromXml(GWEN_XMLNODE *xmlNode)
{
  GWB_BUILD_CONTEXT *bctx;
  GWEN_XMLNODE *xmlGroupNode;

  bctx=GWB_BuildCtx_new();

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "fileList", NULL, NULL);
  if (xmlGroupNode)
    _readFilesFromXml(xmlGroupNode, "file", bctx->fileList);

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "commandList", NULL, NULL);
  if (xmlGroupNode)
    _readCommandsFromXml(bctx, xmlGroupNode, "command");

  return bctx;
}



void _writeCommandList2ToXml(const GWB_BUILD_CMD_LIST2 *commandList, GWEN_XMLNODE *xmlNode, const char *groupName)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(commandList);
  if (it) {
    const GWB_BUILD_CMD *cmd;

    cmd=GWB_BuildCmd_List2Iterator_Data(it);
    while(cmd) {
      GWEN_XMLNODE *entryNode;

      entryNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, groupName);
      GWB_BuildCmd_toXml(cmd, entryNode);
      GWEN_XMLNode_AddChild(xmlNode, entryNode);
      cmd=GWB_BuildCmd_List2Iterator_Next(it);
    }
    GWB_BuildCmd_List2Iterator_free(it);
  }
}



void _writeFileList2ToXml(const GWB_FILE_LIST2 *fileList, GWEN_XMLNODE *xmlNode, const char *groupName)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      GWEN_XMLNODE *entryNode;
      const char *s;

      entryNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, groupName);
      GWEN_XMLNode_SetIntProperty(entryNode, "id", (int) GWB_File_GetId(file));
      s=GWB_File_GetFolder(file);
      if (s)
        GWEN_XMLNode_SetCharValue(entryNode, "folder", s);
      s=GWB_File_GetName(file);
      if (s)
        GWEN_XMLNode_SetCharValue(entryNode, "name", s);
      s=GWB_File_GetFileType(file);
      if (s)
        GWEN_XMLNode_SetCharValue(entryNode, "type", s);

      _writeFileFlagsToXml(GWB_File_GetFlags(file), entryNode, "flags");

      GWEN_XMLNode_AddChild(xmlNode, entryNode);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void _writeFileFlagsToXml(uint32_t flags, GWEN_XMLNODE *xmlNode, const char *varName)
{
  if (flags) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);

    if (flags & GWB_FILE_FLAGS_DIST) {
      if (GWEN_Buffer_GetUsedBytes(dbuf))
        GWEN_Buffer_AppendString(dbuf, " ");
      GWEN_Buffer_AppendString(dbuf, "DIST");
    }

    if (flags & GWB_FILE_FLAGS_INSTALL) {
      if (GWEN_Buffer_GetUsedBytes(dbuf))
        GWEN_Buffer_AppendString(dbuf, " ");
      GWEN_Buffer_AppendString(dbuf, "INSTALL");
    }

    if (flags & GWB_FILE_FLAGS_GENERATED) {
      if (GWEN_Buffer_GetUsedBytes(dbuf))
        GWEN_Buffer_AppendString(dbuf, " ");
      GWEN_Buffer_AppendString(dbuf, "GENERATED");
    }

    GWEN_XMLNode_SetCharValue(xmlNode, varName, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }
}



void _readCommandsFromXml(GWB_BUILD_CONTEXT *bctx, GWEN_XMLNODE *xmlNode, const char *groupName)
{
  GWEN_XMLNODE *xmlEntry;

  xmlEntry=GWEN_XMLNode_FindFirstTag(xmlNode, groupName, NULL, NULL);
  while(xmlEntry) {
    GWB_BUILD_CMD *bcmd;

    bcmd=GWB_BuildCmd_fromXml(xmlEntry, bctx->fileList);
    GWB_BuildCmd_List2_PushBack(bctx->commandList, bcmd);
    xmlEntry=GWEN_XMLNode_FindNextTag(xmlEntry, groupName, NULL, NULL);
  }
}



void _readFilesFromXml(GWEN_XMLNODE *xmlNode,
                       const char *groupName,
                       GWB_FILE_LIST2 *destFileList)
{
  GWEN_XMLNODE *xmlEntry;

  xmlEntry=GWEN_XMLNode_FindFirstTag(xmlNode, groupName, NULL, NULL);
  while(xmlEntry) {
    uint32_t id;

    id=(uint32_t) GWEN_XMLNode_GetIntProperty(xmlEntry, "id", 0);
    if (id==0) {
      DBG_ERROR(NULL, "FILE has no id");
    }
    else {
      GWB_FILE *file;
      const char *folder;
      const char *name;
      const char *fileType;
      const char *flagsAsText;
      uint32_t flags;

      folder=GWEN_XMLNode_GetCharValue(xmlEntry, "folder", NULL);
      name=GWEN_XMLNode_GetCharValue(xmlEntry, "name", NULL);
      fileType=GWEN_XMLNode_GetCharValue(xmlEntry, "type", NULL);
      flagsAsText=GWEN_XMLNode_GetCharValue(xmlEntry, "flags", NULL);
      flags=_readFlagsFromChar(flagsAsText);

      file=GWB_File_new(folder, name, id);
      GWB_File_SetFlags(file, flags);
      GWB_File_SetFileType(file, fileType);

      GWB_File_List2_PushBack(destFileList, file);
    }
    xmlEntry=GWEN_XMLNode_FindNextTag(xmlEntry, groupName, NULL, NULL);
  }
}



uint32_t _readFlagsFromChar(const char *flagsAsText)
{
  GWEN_STRINGLIST *sl;
  uint32_t flags=0;

  sl=GWEN_StringList_fromString(flagsAsText, " ", 1);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        if (strcasecmp(s, "DIST")==0)
          flags|=GWB_FILE_FLAGS_DIST;
        else if (strcasecmp(s, "INSTALL")==0)
          flags|=GWB_FILE_FLAGS_INSTALL;
        else if (strcasecmp(s, "GENERATED")==0)
          flags|=GWB_FILE_FLAGS_GENERATED;
        else {
          DBG_ERROR(NULL, "Unexpected FILE flag \"%s\"", s);
        }
      }
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(sl);
  }

  return flags;
}



GWB_BUILD_CONTEXT *GWB_BuildCtx_ReadFromXmlFile(const char *fileName)
{
  GWB_BUILD_CONTEXT *buildCtx;
  GWEN_XMLNODE *xmlNode;
  GWEN_XMLNODE *xmlBuildCtx;
  int rv;

  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlNode, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL, "Error reading BuildContext file \"%s\": Bad XML (%d)", fileName, rv);
    GWEN_XMLNode_free(xmlNode);
    return NULL;
  }

  xmlBuildCtx=GWEN_XMLNode_FindFirstTag(xmlNode, "BuildContext", NULL, NULL);
  if (xmlBuildCtx==NULL) {
    DBG_ERROR(NULL, "XML file \"%s\" doesn not contain a BuildContext element,", fileName);
    GWEN_XMLNode_free(xmlNode);
    return NULL;
  }

  buildCtx=GWB_BuildCtx_fromXml(xmlBuildCtx);
  GWEN_XMLNode_free(xmlNode);

  return buildCtx;
}



int GWB_BuildCtx_WriteToXmlFile(const GWB_BUILD_CONTEXT *buildCtx, const char *fileName)
{
  GWEN_XMLNODE *xmlNode;
  GWEN_XMLNODE *xmlBuildCtx;
  int rv;

  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  xmlBuildCtx=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "BuildContext");
  GWB_BuildCtx_toXml(buildCtx, xmlBuildCtx);
  GWEN_XMLNode_AddChild(xmlNode, xmlBuildCtx);

  rv=GWEN_XMLNode_WriteFile(xmlNode, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  GWEN_XMLNode_free(xmlNode);
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing BuildContext to file \"%s\" (%d)", fileName, rv);
    return rv;
  }

  return 0;
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






int GWB_BuildCtx_Run(GWB_BUILD_CONTEXT *bctx, int maxConcurrentJobs, int usePrepareCommands)
{
  int waitingJobs;
  int runningJobs;

  _setupDependencies(bctx);
  _setupCommands(bctx, usePrepareCommands);
  _createCommandQueues(bctx);
  _initiallyFillQueues(bctx);

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
      GWB_KEYVALUEPAIR_LIST *cmdList;
  
      if (forPrepareCommands)
        cmdList=GWB_BuildCmd_GetPrepareCommandList(bcmd);
      else
        cmdList=GWB_BuildCmd_GetBuildCommandList(bcmd);
      if (cmdList)
        GWB_BuildCmd_SetCurrentCommand(bcmd, GWB_KeyValuePair_List_First(cmdList));
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



void _initiallyFillQueues(GWB_BUILD_CONTEXT *bctx)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(bctx->commandList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    bcmd=GWB_BuildCmd_List2Iterator_Data(it);
    while(bcmd) {
      if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
        GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
      }
      bcmd=GWB_BuildCmd_List2Iterator_Next(it);
    }
    GWB_BuildCmd_List2Iterator_free(it);
  }
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

        rv=_startCommand(bcmd);
        if (rv<0) {
          GWB_BuildCmd_List2_PushBack(bctx->finishedQueue, bcmd);
          errors++;
        }
        else {
          GWB_BuildCmd_List2_PushBack(bctx->runningQueue, bcmd);
          started++;
        }
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



int _startCommand(GWB_BUILD_CMD *bcmd)
{
  GWB_KEYVALUEPAIR *currentCommand;

  currentCommand=GWB_BuildCmd_GetCurrentCommand(bcmd);
  if (currentCommand) {
    const char *folder;
    const char *cmd;
    const char *args;

    folder=GWB_BuildCmd_GetFolder(bcmd);
    cmd=GWB_KeyValuePair_GetKey(currentCommand);
    args=GWB_KeyValuePair_GetValue(currentCommand);

    if (cmd && *cmd) {
      GWEN_PROCESS *process;
      GWEN_PROCESS_STATE pstate;

      if (0) {
	const char *buildMessage;

	buildMessage=GWB_BuildCmd_GetBuildMessage(bcmd);
	if (buildMessage)
	  fprintf(stdout, "%s [%s]\r", buildMessage, cmd);
      }
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

    GWB_BuildCmd_List2_PopFront(oldRunningQueue);

    process=GWB_BuildCmd_GetCurrentProcess(bcmd);
    pstate=GWEN_Process_CheckState(process);
    if (pstate!=GWEN_ProcessStateRunning) {
      changes++;
      if (pstate==GWEN_ProcessStateExited) {
        int result;

        result=GWEN_Process_GetResult(process);
        if (result) {
          DBG_ERROR(NULL, "Command exited with result %d", result);
          GWB_BuildCmd_List2_PushBack(bctx->finishedQueue, bcmd);
          errors++;
        }
        else {
          GWB_KEYVALUEPAIR *nextCommand;

          /* process successfully finished */
          _signalJobFinished(bcmd);
          nextCommand=GWB_KeyValuePair_List_Next(GWB_BuildCmd_GetCurrentCommand(bcmd));
          GWB_BuildCmd_SetCurrentCommand(bcmd, nextCommand);
          if (nextCommand)
            GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
          else
            GWB_BuildCmd_List2_PushBack(bctx->finishedQueue, bcmd);
        }
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





