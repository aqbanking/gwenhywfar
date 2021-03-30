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



void _writeKeyValuePairListToXml(const GWB_KEYVALUEPAIR_LIST *kvpList,
                                 GWEN_XMLNODE *xmlNode,
                                 const char *groupName,
                                 const char *keyVarName, const char *valueVarName);
void _writeKeyValuePairToXml(const GWB_KEYVALUEPAIR *kvp, GWEN_XMLNODE *xmlNode,
                             const char *keyVarName, const char *valueVarName);
void _writeIdsFromFile2ListToXml(const GWB_FILE_LIST2 *fileList2, GWEN_XMLNODE *xmlNode, const char *groupName);

void _readKeyValuePairListFromXml(GWB_KEYVALUEPAIR_LIST *kvpList,
                                  GWEN_XMLNODE *xmlNode,
                                  const char *groupName,
                                  const char *keyVarName,
                                  const char *valueVarName);
GWB_KEYVALUEPAIR *_readKeyValuePairFromXml(GWEN_XMLNODE *xmlNode, const char *keyVarName, const char *valueVarName);
void _readFileIdsFromXml(GWEN_XMLNODE *xmlNode,
                         const char *groupName,
                         GWB_FILE_LIST2 *ctxFileList, GWB_FILE_LIST2 *destFileList);




GWB_BUILD_CMD *GWB_BuildCmd_new(void)
{
  GWB_BUILD_CMD *bcmd;

  GWEN_NEW_OBJECT(GWB_BUILD_CMD, bcmd);
  GWEN_LIST_INIT(GWB_BUILD_CMD, bcmd);

  bcmd->inFileList2=GWB_File_List2_new();
  bcmd->outFileList2=GWB_File_List2_new();

  bcmd->prepareCommandList=GWB_KeyValuePair_List_new();
  bcmd->buildCommandList=GWB_KeyValuePair_List_new();

  return bcmd;
}



void GWB_BuildCmd_free(GWB_BUILD_CMD *bcmd)
{
  if (bcmd) {
    GWEN_LIST_FINI(GWB_BUILD_CMD, bcmd);

    GWB_BuildCmd_SetCurrentProcess(bcmd, NULL);
    GWB_BuildCmd_SetCurrentCommand(bcmd, NULL);
    GWB_KeyValuePair_List_free(bcmd->prepareCommandList);
    GWB_KeyValuePair_List_free(bcmd->buildCommandList);

    GWB_File_List2_free(bcmd->inFileList2);
    GWB_File_List2_free(bcmd->outFileList2);
    GWEN_FREE_OBJECT(bcmd);
  }
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



GWB_KEYVALUEPAIR_LIST *GWB_BuildCmd_GetPrepareCommandList(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->prepareCommandList;
}



void GWB_BuildCmd_AddPrepareCommand(GWB_BUILD_CMD *bcmd, const char *cmd, const char *args)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_new(cmd, args);
  GWB_KeyValuePair_List_Add(kvp, bcmd->prepareCommandList);
}



GWB_KEYVALUEPAIR_LIST *GWB_BuildCmd_GetBuildCommandList(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->buildCommandList;
}



void GWB_BuildCmd_AddBuildCommand(GWB_BUILD_CMD *bcmd, const char *cmd, const char *args)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_new(cmd, args);
  GWB_KeyValuePair_List_Add(kvp, bcmd->buildCommandList);
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



GWB_KEYVALUEPAIR *GWB_BuildCmd_GetCurrentCommand(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->currentCommand;
}



void GWB_BuildCmd_SetCurrentCommand(GWB_BUILD_CMD *bcmd, GWB_KEYVALUEPAIR *cmd)
{
  bcmd->currentCommand=cmd;
}






void GWB_BuildCmd_toXml(const GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode)
{
  if (bcmd->folder)
    GWEN_XMLNode_SetCharValue(xmlNode, "folder", bcmd->folder);

  if (bcmd->prepareCommandList) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "prepareCommands");
    _writeKeyValuePairListToXml(bcmd->prepareCommandList, xmlGroupNode, "command", "cmd", "args");
    GWEN_XMLNode_AddChild(xmlNode, xmlGroupNode);
  }

  if (bcmd->buildCommandList) {
    GWEN_XMLNODE *xmlGroupNode;

    xmlGroupNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "buildCommands");
    _writeKeyValuePairListToXml(bcmd->buildCommandList, xmlGroupNode, "command", "cmd", "args");
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

  s=GWEN_XMLNode_GetCharValue(xmlNode, "folder", NULL);
  GWB_BuildCmd_SetFolder(bcmd, s);

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "prepareCommands", NULL, NULL);
  if (xmlGroupNode)
    _readKeyValuePairListFromXml(bcmd->prepareCommandList, xmlGroupNode, "command", "cmd", "args");

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "buildCommands", NULL, NULL);
  if (xmlGroupNode)
    _readKeyValuePairListFromXml(bcmd->buildCommandList, xmlGroupNode, "command", "cmd", "args");

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "inputFiles", NULL, NULL);
  if (xmlGroupNode)
    _readFileIdsFromXml(xmlGroupNode, "file", fileList, bcmd->inFileList2);

  xmlGroupNode=GWEN_XMLNode_FindFirstTag(xmlNode, "outputFiles", NULL, NULL);
  if (xmlGroupNode)
    _readFileIdsFromXml(xmlGroupNode, "file", fileList, bcmd->outFileList2);

  return bcmd;
}



void _writeKeyValuePairListToXml(const GWB_KEYVALUEPAIR_LIST *kvpList,
                                 GWEN_XMLNODE *xmlNode,
                                 const char *groupName,
                                 const char *keyVarName, const char *valueVarName)
{
  const GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_List_First(kvpList);
  while(kvp) {
    GWEN_XMLNODE *entryNode;

    entryNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, groupName);
    _writeKeyValuePairToXml(kvp, entryNode, keyVarName, valueVarName);
    GWEN_XMLNode_AddChild(xmlNode, entryNode);
    kvp=GWB_KeyValuePair_List_Next(kvp);
  }
}



void _writeKeyValuePairToXml(const GWB_KEYVALUEPAIR *kvp, GWEN_XMLNODE *xmlNode,
                             const char *keyVarName, const char *valueVarName)
{
  const char *key;
  const char *value;

  key=GWB_KeyValuePair_GetKey(kvp);
  value=GWB_KeyValuePair_GetValue(kvp);
  if (key)
    GWEN_XMLNode_SetCharValue(xmlNode, keyVarName, key);
  if (value)
    GWEN_XMLNode_SetCharValue(xmlNode, valueVarName, value);
}



void _readKeyValuePairListFromXml(GWB_KEYVALUEPAIR_LIST *kvpList,
                                  GWEN_XMLNODE *xmlNode,
                                  const char *groupName,
                                  const char *keyVarName,
                                  const char *valueVarName)
{
  GWEN_XMLNODE *xmlEntry;

  xmlEntry=GWEN_XMLNode_FindFirstTag(xmlNode, groupName, NULL, NULL);
  while(xmlEntry) {
    GWB_KEYVALUEPAIR *kvp;

    kvp=_readKeyValuePairFromXml(xmlEntry, keyVarName, valueVarName);
    if (kvp)
      GWB_KeyValuePair_List_Add(kvp, kvpList);
    xmlEntry=GWEN_XMLNode_FindNextTag(xmlEntry, groupName, NULL, NULL);
  }
}



GWB_KEYVALUEPAIR *_readKeyValuePairFromXml(GWEN_XMLNODE *xmlNode, const char *keyVarName, const char *valueVarName)
{
  const char *key;
  const char *value;

  key=GWEN_XMLNode_GetCharValue(xmlNode, keyVarName, "NULL");
  value=GWEN_XMLNode_GetCharValue(xmlNode, valueVarName, "NULL");
  return GWB_KeyValuePair_new(key, value);
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






