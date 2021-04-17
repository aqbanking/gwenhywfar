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
#include "gwenbuild/buildctx/buildctx_xml.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>
#include <ctype.h>




static void _writeFileList2ToXml(const GWB_FILE_LIST2 *fileList, GWEN_XMLNODE *xmlNode, const char *groupName);
static void _readFilesFromXml(GWEN_XMLNODE *xmlNode, const char *groupName, GWB_FILE_LIST2 *destFileList);

static void _writeCommandList2ToXml(const GWB_BUILD_CMD_LIST2 *commandList, GWEN_XMLNODE *xmlNode, const char *groupName);
static void _readCommandsFromXml(GWB_BUILD_CONTEXT *bctx, GWEN_XMLNODE *xmlNode, const char *groupName);

static void _writeFileFlagsToXml(uint32_t flags, GWEN_XMLNODE *xmlNode, const char *varName);
static uint32_t _readFlagsFromChar(const char *flagsAsText);




void GWB_BuildCtx_toXml(const GWB_BUILD_CONTEXT *bctx, GWEN_XMLNODE *xmlNode)
{
  if (bctx->initialSourceDir)
    GWEN_XMLNode_SetCharValue(xmlNode, "initialSourceDir", bctx->initialSourceDir);

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
  const char *s;

  bctx=GWB_BuildCtx_new();

  s=GWEN_XMLNode_GetCharValue(xmlNode, "initialSourceDir", NULL);
  if (s)
    GWB_BuildCtx_SetInitialSourceDir(bctx, s);

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


