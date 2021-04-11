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


#include "gwenbuild/buildctx/buildcmd_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>



GWEN_LIST_FUNCTIONS(GWB_BUILD_CMD, GWB_BuildCmd);
GWEN_LIST2_FUNCTIONS(GWB_BUILD_CMD, GWB_BuildCmd);



static void _writeIdsFromFile2ListToXml(const GWB_FILE_LIST2 *fileList2, GWEN_XMLNODE *xmlNode, const char *groupName);

static void _readFileIdsFromXml(GWEN_XMLNODE *xmlNode,
                                const char *groupName,
                                GWB_FILE_LIST2 *ctxFileList, GWB_FILE_LIST2 *destFileList);
static void _writeSubCommandListToXml(const GWB_BUILD_SUBCMD_LIST *cmdList,
                                      GWEN_XMLNODE *xmlNode,
                                      const char *groupName);
static void _readSubCommandListFromXml(GWB_BUILD_SUBCMD_LIST *cmdList,
                                       GWEN_XMLNODE *xmlNode,
                                       const char *groupName);




GWB_BUILD_CMD *GWB_BuildCmd_new(void)
{
  GWB_BUILD_CMD *bcmd;

  GWEN_NEW_OBJECT(GWB_BUILD_CMD, bcmd);
  GWEN_LIST_INIT(GWB_BUILD_CMD, bcmd);

  bcmd->inFileList2=GWB_File_List2_new();
  bcmd->outFileList2=GWB_File_List2_new();

  bcmd->prepareCommandList=GWB_BuildSubCmd_List_new();
  bcmd->buildCommandList=GWB_BuildSubCmd_List_new();

  return bcmd;
}



GWB_BUILD_CMD *GWB_BuildCmd_dup(GWB_BUILD_CMD *origCmd)
{
  GWB_BUILD_CMD *bcmd;

  GWEN_NEW_OBJECT(GWB_BUILD_CMD, bcmd);
  GWEN_LIST_INIT(GWB_BUILD_CMD, bcmd);

  bcmd->builderName=(origCmd->builderName)?strdup(origCmd->builderName):NULL;
  bcmd->folder=(origCmd->folder)?strdup(origCmd->folder):NULL;
  bcmd->buildMessage=(origCmd->buildMessage)?strdup(origCmd->buildMessage):NULL;
  if (origCmd->prepareCommandList)
    bcmd->prepareCommandList=GWB_BuildSubCmd_List_dup(origCmd->prepareCommandList);
  else
    bcmd->prepareCommandList=GWB_BuildSubCmd_List_new();

  if (origCmd->buildCommandList)
    bcmd->buildCommandList=GWB_BuildSubCmd_List_dup(origCmd->buildCommandList);
  else
    bcmd->buildCommandList=GWB_BuildSubCmd_List_new();
  bcmd->blockingFiles=origCmd->blockingFiles;
  if (origCmd->inFileList2)
    bcmd->inFileList2=GWB_File_List2_dup(origCmd->inFileList2);
  else
    bcmd->inFileList2=GWB_File_List2_new();
  if (origCmd->outFileList2)
    bcmd->outFileList2=GWB_File_List2_dup(origCmd->outFileList2);
  else
    bcmd->outFileList2=GWB_File_List2_new();
  bcmd->currentProcess=NULL;
  bcmd->currentCommand=NULL;

  return bcmd;
}



void GWB_BuildCmd_free(GWB_BUILD_CMD *bcmd)
{
  if (bcmd) {
    GWEN_LIST_FINI(GWB_BUILD_CMD, bcmd);

    GWB_BuildCmd_SetCurrentProcess(bcmd, NULL);
    GWB_BuildCmd_SetCurrentCommand(bcmd, NULL);

    free(bcmd->buildMessage);
    free(bcmd->folder);
    free(bcmd->builderName);

    GWB_BuildSubCmd_List_free(bcmd->prepareCommandList);
    GWB_BuildSubCmd_List_free(bcmd->buildCommandList);

    GWB_File_List2_free(bcmd->inFileList2);
    GWB_File_List2_free(bcmd->outFileList2);
    GWEN_FREE_OBJECT(bcmd);
  }
}



const char *GWB_BuildCmd_GetBuilderName(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->builderName;
}



void GWB_BuildCmd_SetBuilderName(GWB_BUILD_CMD *bcmd, const char *s)
{
  free(bcmd->builderName);
  bcmd->builderName=s?strdup(s):NULL;
}






const char *GWB_BuildCmd_GetFolder(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->folder;
}



void GWB_BuildCmd_SetFolder(GWB_BUILD_CMD *bcmd, const char *s)
{
  free(bcmd->folder);
  bcmd->folder=s?strdup(s):NULL;
}



const char *GWB_BuildCmd_GetBuildMessage(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->buildMessage;
}



void GWB_BuildCmd_SetBuildMessage(GWB_BUILD_CMD *bcmd, const char *s)
{
  free(bcmd->buildMessage);
  bcmd->buildMessage=s?strdup(s):NULL;
}



GWB_BUILD_SUBCMD_LIST *GWB_BuildCmd_GetPrepareCommandList(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->prepareCommandList;
}



void GWB_BuildCmd_AddPrepareCommand(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *cmd)
{
  GWB_BuildSubCmd_List_Add(cmd, bcmd->prepareCommandList);
}



GWB_BUILD_SUBCMD_LIST *GWB_BuildCmd_GetBuildCommandList(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->buildCommandList;
}



void GWB_BuildCmd_AddBuildCommand(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *cmd)
{
  GWB_BuildSubCmd_List_Add(cmd, bcmd->buildCommandList);
}



int GWB_BuildCmd_GetBlockingFiles(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->blockingFiles;
}



void GWB_BuildCmd_SetBlockingFiles(GWB_BUILD_CMD *bcmd, int i)
{
  bcmd->blockingFiles=i;
}



int GWB_BuildCmd_IncBlockingFiles(GWB_BUILD_CMD *bcmd)
{
  return ++(bcmd->blockingFiles);
}



int GWB_BuildCmd_DecBlockingFiles(GWB_BUILD_CMD *bcmd)
{
  if (bcmd->blockingFiles>0)
    return --(bcmd->blockingFiles);
  else
    return bcmd->blockingFiles;
}



GWB_FILE_LIST2 *GWB_BuildCmd_GetInFileList2(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->inFileList2;
}



void GWB_BuildCmd_AddInFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file)
{
  if (file)
    GWB_File_List2_PushBack(bcmd->inFileList2, file);
}



GWB_FILE_LIST2 *GWB_BuildCmd_GetOutFileList2(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->outFileList2;
}



void GWB_BuildCmd_AddOutFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file)
{
  if (file)
    GWB_File_List2_PushBack(bcmd->outFileList2, file);
}



GWEN_PROCESS *GWB_BuildCmd_GetCurrentProcess(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->currentProcess;
}



void GWB_BuildCmd_SetCurrentProcess(GWB_BUILD_CMD *bcmd, GWEN_PROCESS *process)
{
  if (bcmd->currentProcess) {
    if (GWEN_Process_CheckState(bcmd->currentProcess)==GWEN_ProcessStateRunning) {
      DBG_ERROR(NULL, "Process is still running!");
      GWEN_Process_Terminate(bcmd->currentProcess);
    }
    GWEN_Process_free(bcmd->currentProcess);
  }
  bcmd->currentProcess=process;
}



GWB_BUILD_SUBCMD *GWB_BuildCmd_GetCurrentCommand(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->currentCommand;
}



void GWB_BuildCmd_SetCurrentCommand(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *cmd)
{
  bcmd->currentCommand=cmd;
}






void GWB_BuildCmd_toXml(const GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode)
{
  if (bcmd->builderName)
    GWEN_XMLNode_SetCharValue(xmlNode, "builderName", bcmd->builderName);

  if (bcmd->folder)
    GWEN_XMLNode_SetCharValue(xmlNode, "folder", bcmd->folder);

  if (bcmd->buildMessage)
    GWEN_XMLNode_SetCharValue(xmlNode, "buildMessage", bcmd->buildMessage);

  if (bcmd->prepareCommandList) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "prepareCommands");
    _writeSubCommandListToXml(bcmd->prepareCommandList, xmlGroupNode, "command");
    GWEN_XMLNode_AddChild(xmlNode, xmlGroupNode);
  }

  if (bcmd->buildCommandList) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "buildCommands");
    _writeSubCommandListToXml(bcmd->buildCommandList, xmlGroupNode, "command");
    GWEN_XMLNode_AddChild(xmlNode, xmlGroupNode);
  }

  if (bcmd->inFileList2) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "inputFiles");
    _writeIdsFromFile2ListToXml(bcmd->inFileList2, xmlGroupNode, "file");
    GWEN_XMLNode_AddChild(xmlNode, xmlGroupNode);
  }

  if (bcmd->outFileList2) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "outputFiles");
    _writeIdsFromFile2ListToXml(bcmd->outFileList2, xmlGroupNode, "file");
    GWEN_XMLNode_AddChild(xmlNode, xmlGroupNode);
  }
}



GWB_BUILD_CMD *GWB_BuildCmd_fromXml(GWEN_XMLNODE *xmlNode, GWB_FILE_LIST2 *fileList)
{
  GWB_BUILD_CMD *bcmd;
  GWEN_XMLNODE *xmlGroupNode;
  const char *s;

  bcmd=GWB_BuildCmd_new();

  s=GWEN_XMLNode_GetCharValue(xmlNode, "builderName", NULL);
  GWB_BuildCmd_SetBuilderName(bcmd, s);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "folder", NULL);
  GWB_BuildCmd_SetFolder(bcmd, s);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "buildMessage", NULL);
  GWB_BuildCmd_SetBuildMessage(bcmd, s);


  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "prepareCommands", NULL, NULL);
  if (xmlGroupNode)
    _readSubCommandListFromXml(bcmd->prepareCommandList, xmlGroupNode, "command");

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "buildCommands", NULL, NULL);
  if (xmlGroupNode)
    _readSubCommandListFromXml(bcmd->buildCommandList, xmlGroupNode, "command");

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "inputFiles", NULL, NULL);
  if (xmlGroupNode)
    _readFileIdsFromXml(xmlGroupNode, "file", fileList, bcmd->inFileList2);

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "outputFiles", NULL, NULL);
  if (xmlGroupNode)
    _readFileIdsFromXml(xmlGroupNode, "file", fileList, bcmd->outFileList2);

  return bcmd;
}



void _writeSubCommandListToXml(const GWB_BUILD_SUBCMD_LIST *cmdList,
                               GWEN_XMLNODE *xmlNode,
                               const char *groupName)
{
  const GWB_BUILD_SUBCMD *cmd;

  cmd=GWB_BuildSubCmd_List_First(cmdList);
  while(cmd) {
    GWEN_XMLNODE *entryNode;

    entryNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, groupName);
    GWB_BuildSubCmd_toXml(cmd, entryNode);
    GWEN_XMLNode_AddChild(xmlNode, entryNode);
    cmd=GWB_BuildSubCmd_List_Next(cmd);
  }
}



void _readSubCommandListFromXml(GWB_BUILD_SUBCMD_LIST *cmdList,
                                GWEN_XMLNODE *xmlNode,
                                const char *groupName)
{
  GWEN_XMLNODE *xmlEntry;

  xmlEntry=GWEN_XMLNode_FindFirstTag(xmlNode, groupName, NULL, NULL);
  while(xmlEntry) {
    GWB_BUILD_SUBCMD *cmd;

    cmd=GWB_BuildSubCmd_fromXml(xmlEntry);
    if (cmd)
      GWB_BuildSubCmd_List_Add(cmd, cmdList);
    xmlEntry=GWEN_XMLNode_FindNextTag(xmlEntry, groupName, NULL, NULL);
  }
}



void _writeIdsFromFile2ListToXml(const GWB_FILE_LIST2 *fileList2,
                                 GWEN_XMLNODE *xmlNode,
                                 const char *groupName)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList2);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      GWEN_XMLNODE *entryNode;
      uint32_t id;

      entryNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, groupName);
      id=GWB_File_GetId(file);
      GWEN_XMLNode_SetIntProperty(entryNode, "id", (int) id);
      GWEN_XMLNode_AddChild(xmlNode, entryNode);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void _readFileIdsFromXml(GWEN_XMLNODE *xmlNode,
                         const char *groupName,
                         GWB_FILE_LIST2 *ctxFileList, GWB_FILE_LIST2 *destFileList)
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

      file=GWB_File_List2_GetFileById(ctxFileList, id);
      if (file==NULL) {
        DBG_ERROR(NULL, "FILE %d not found", (int) id);
      }
      else
        GWB_File_List2_PushBack(destFileList, file);
    }
    xmlEntry=GWEN_XMLNode_FindNextTag(xmlEntry, groupName, NULL, NULL);
  }
}



void GWB_BuildCmd_List2_FreeAll(GWB_BUILD_CMD_LIST2 *cmdList)
{
  if (cmdList) {
    GWB_BUILD_CMD_LIST2_ITERATOR *it;

    it=GWB_BuildCmd_List2_First(cmdList);
    if (it) {
      GWB_BUILD_CMD *cmd;

      cmd=GWB_BuildCmd_List2Iterator_Data(it);
      while(cmd) {
	GWB_BuildCmd_free(cmd);
        cmd=GWB_BuildCmd_List2Iterator_Next(it);
      }
    }
    GWB_BuildCmd_List2_free(cmdList);
  }
}





void GWB_BuildCmd_Dump(const GWB_BUILD_CMD *bcmd, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "BuildCmd:\n");

  GWBUILD_Debug_PrintValue(   "folder.......", bcmd->folder, indent+2);
  GWBUILD_Debug_PrintIntValue("blockingFiles", bcmd->blockingFiles, indent+2);
  GWBUILD_Debug_PrintKvpList("prepareCommandList", bcmd->prepareCommandList, indent+2);
  GWBUILD_Debug_PrintKvpList("buildCommandList", bcmd->buildCommandList, indent+2);
  GWBUILD_Debug_PrintFileList2("inFileList2", bcmd->inFileList2, indent+2);
  GWBUILD_Debug_PrintFileList2("outFileList2", bcmd->outFileList2, indent+2);

}



void GWB_BuildCmd_PrintDescriptionWithText(const GWB_BUILD_CMD *bcmd, int indent, const char *text)
{
  if (bcmd) {
    const GWB_FILE *fileIn=NULL;
    const GWB_FILE *fileOut=NULL;

    if (bcmd->inFileList2)
      fileIn=GWB_File_List2_GetFront(bcmd->inFileList2);
    if (bcmd->outFileList2)
      fileOut=GWB_File_List2_GetFront(bcmd->outFileList2);
    if (fileIn || fileOut) {
      const char *folder;
      const char *fileNameIn=NULL;
      const char *fileNameOut=NULL;

      folder=GWB_BuildCmd_GetFolder(bcmd);

      if (fileIn)
        fileNameIn=GWB_File_GetName(fileIn);
      if (fileOut)
        fileNameOut=GWB_File_GetName(fileOut);

      if (fileNameIn && fileNameOut) {
        int i;

        for(i=0; i<indent; i++)
          fprintf(stderr, " ");
        fprintf(stderr, "%s: %s (%s -> %s)\n",
                folder?folder:"<no folder>",
                text?text:"BuildCmd",
                fileNameIn?fileNameIn:"<no filename>",
                fileNameOut?fileNameOut:"<no filename>");
      }
      else if (fileIn) {
        int i;

        for(i=0; i<indent; i++)
          fprintf(stderr, " ");
        fprintf(stderr, "%s: %s (%s) \n",
                folder?folder:"<no folder>",
                text?text:"BuildCmd",
                fileNameIn?fileNameIn:"<no filename>");
      }
    }
  }
}






