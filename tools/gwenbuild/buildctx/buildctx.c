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

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>
#include <ctype.h>


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
void _initiallyFillQueues(GWB_BUILD_CONTEXT *bctx, const char *builderName);
void _createCommandQueues(GWB_BUILD_CONTEXT *bctx);
int _checkWaitingQueue(GWB_BUILD_CONTEXT *bctx, int maxStartAllowed);
int _startCommand(GWB_BUILD_CMD *bcmd);
int _checkRunningQueue(GWB_BUILD_CONTEXT *bctx);
void _signalJobFinished(GWB_BUILD_CMD *bcmd);
void _decBlockingFilesInWaitingBuildCommands(GWB_BUILD_CMD_LIST2 *waitingCommands);
void _abortAllCommands(GWB_BUILD_CONTEXT *bctx);
void _abortCommandsInQueue(GWB_BUILD_CMD_LIST2 *cmdList);

int _needRunCurrentCommand(GWB_BUILD_CMD *bcmd);
int _file1IsNewerThanFile2(const char *filename1, const char *filename2);
void _finishCurrentCommand(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *currentCommand);

static int _checkDependencies(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *subCmd);
static int _checkDatesOfFileAgainstList(const char *fileName, const GWEN_STRINGLIST *sl);
static GWEN_STRINGLIST *_getAbsoluteDeps(const char *folder, const char *fileName);
static GWEN_STRINGLIST *_makeAbsolutePaths(GWEN_STRINGLIST *slInput, const char *folder);
static GWEN_STRINGLIST *_readDepFile(const char *fileName);



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






int GWB_BuildCtx_Run(GWB_BUILD_CONTEXT *bctx, int maxConcurrentJobs, int usePrepareCommands, const char *builderName)
{
  int waitingJobs;
  int runningJobs;

  _setupDependencies(bctx);
  _setupCommands(bctx, usePrepareCommands);
  _createCommandQueues(bctx);
  _initiallyFillQueues(bctx, builderName);

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



void _initiallyFillQueues(GWB_BUILD_CONTEXT *bctx, const char *builderName)
{
  GWB_BUILD_CMD_LIST2_ITERATOR *it;

  it=GWB_BuildCmd_List2_First(bctx->commandList);
  if (it) {
    GWB_BUILD_CMD *bcmd;

    if (builderName && *builderName) {
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
      bcmd=GWB_BuildCmd_List2Iterator_Data(it);
      while(bcmd) {
        if (GWB_BuildCmd_GetCurrentCommand(bcmd)) {
          GWB_BuildCmd_List2_PushBack(bctx->waitingQueue, bcmd);
        }
        bcmd=GWB_BuildCmd_List2Iterator_Next(it);
      }
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

	if (_needRunCurrentCommand(bcmd)) {
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
	else {
	  _finishCurrentCommand(bctx, bcmd, GWB_BuildCmd_GetCurrentCommand(bcmd));
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



int _needRunCurrentCommand(GWB_BUILD_CMD *bcmd)
{
  GWB_BUILD_SUBCMD *currentCommand;

  currentCommand=GWB_BuildCmd_GetCurrentCommand(bcmd);
  if (currentCommand) {
    uint32_t flags;

    flags=GWB_BuildSubCmd_GetFlags(currentCommand);

    if (flags & GWB_BUILD_SUBCMD_FLAGS_CHECK_DATES) {
      DBG_DEBUG(NULL, "Checking dates of \"%s\" vs \"%s\"",
		GWB_BuildSubCmd_GetMainInputFilePath(currentCommand),
		GWB_BuildSubCmd_GetMainOutputFilePath(currentCommand));
      if (_file1IsNewerThanFile2(GWB_BuildSubCmd_GetMainInputFilePath(currentCommand),
				 GWB_BuildSubCmd_GetMainOutputFilePath(currentCommand))) {
	DBG_DEBUG(NULL, "First file is newer, rebuild needed.");
	return 1;
      }
    }
    else
      return 1;

    if (flags & GWB_BUILD_SUBCMD_FLAGS_CHECK_DEPENDS) {
      int rv;

      rv=_checkDependencies(bcmd, currentCommand);
      if (rv==-1)
	return 0;
      return 1;
    }
  }

  return 0;
}



int _checkDependencies(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *subCmd)
{
  const char *depFileName;
  
  depFileName=GWB_BuildSubCmd_GetDepFilePath(subCmd);
  if (depFileName) {
    const char *outFileName;
  
    outFileName=GWB_BuildSubCmd_GetMainOutputFilePath(subCmd);
    if (outFileName) {
      GWEN_STRINGLIST *sl;

      DBG_DEBUG(NULL, "Checking depend file \"%s\"", depFileName);
      sl=_getAbsoluteDeps(GWB_BuildCmd_GetFolder(bcmd), depFileName);
      if (sl) {
	int rv;

	//GWBUILD_Debug_PrintStringList(depFileName, sl, 2);
	rv=_checkDatesOfFileAgainstList(outFileName, sl);
	GWEN_StringList_free(sl);
	return rv;
      }
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
    return 1; /* need rebuild */
  }
  se=GWEN_StringList_FirstEntry(sl);
  if (se) {
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
	time_t tCurrent;

	tCurrent=GWBUILD_GetModificationTimeOfFile(s);
	if (tCurrent==0)
	  return 1; /* need rebuild */
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

  return 0;
}



int _file1IsNewerThanFile2(const char *filename1, const char *filename2)
{
  time_t t1=0;
  time_t t2=0;

  if (filename1) {
    t1=GWBUILD_GetModificationTimeOfFile(filename1);
    if (t1==0) {
      DBG_ERROR(NULL, "No time for \"%s\"", filename1);
    }
  }
  if (filename2) {
    t2=GWBUILD_GetModificationTimeOfFile(filename2);
    if (t2==0) {
      DBG_INFO(NULL, "No time for \"%s\"", filename2);
    }
  }
  if (t1==0 || t2==0) {
    return 1;
  }
  if (difftime(t2, t1)<0.0) {
    DBG_DEBUG(NULL, "File \"%s\" is newer than \"%s\"", filename1, filename2);
    return 1;
  }
  return 0;
}



int _startCommand(GWB_BUILD_CMD *bcmd)
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




