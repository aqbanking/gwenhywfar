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


#include "gwenbuild/builders/genericbuilder_p.h"
#include "gwenbuild/builder_be.h"
#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>
#include <gwenhywfar/text.h>

#include <string.h>



GWEN_INHERIT(GWB_BUILDER, GWB_BUILDER_GENERIC);


GWB_BUILDER *GWB_GenericBuilder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context, GWEN_XMLNODE *xmlDescr);

static void GWENHYWFAR_CB _freeData(void *bp, void *p);

void _readMainSettingsFromXml(GWB_BUILDER *builder);
void _setupVariables(GWB_BUILDER *builder);
void _setupAfterAddingFirstInputFile(GWB_BUILDER *builder);
void _setupOutFiles(GWB_BUILDER *builder);
void _setupTargetLinkSpec(GWB_BUILDER *builder);

GWEN_BUFFER *_readXmlDataIntoBufferAndExpand(GWEN_DB_NODE *db, GWEN_XMLNODE *xmlNode);

static int _isAcceptableInput(GWB_BUILDER *builder, const GWB_FILE *file);
static void _addSourceFile(GWB_BUILDER *builder, GWB_FILE *f);
static int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx);

void _addBuildCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd);
void _addPrepareCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd);
void _addCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, const char *groupName, GWB_BUILD_SUBCMD_LIST *cmdList);

void _readMainInputFile(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode);
void _readMainOutputFile(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode);
static void _readDepFile(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode);
GWEN_BUFFER *_readMainFilename(GWB_CONTEXT *context, GWEN_XMLNODE *xmlFile, GWB_FILE_LIST2 *filesList);
void _readBuildMessage(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode);

GWEN_BUFFER *_readArgs(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode);
void _readArgsLoop(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *nArgs, GWEN_BUFFER *argsBuffer);
void _readArgsFixed(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsInputFiles(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsOutputFiles(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsLibraries(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer);
void _readArgsIncludes(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsDefines(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer);
void _readArgsUsedSubTargets(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer);
void _readArgsIfHasUsedSubTargets(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsIfHasLibraries(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsHaveConfigH(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer);

void _addMatchingFiles(GWB_BUILDER *builder, GWB_FILE_LIST2 *filesList, const char *pattern, int addAbs, GWEN_BUFFER *argsBuffer);
GWB_FILE *_getFileAtPosInList2(GWB_FILE_LIST2 *filesList, int index);
void _addStringListToBuffer(GWEN_STRINGLIST *sl, GWEN_BUFFER *argsBuffer);




GWB_BUILDER *GWB_GenericBuilder_Factory(GWENBUILD *gwenbuild, GWB_CONTEXT *context, const char *builderName)
{
  GWB_BUILDER *builder;
  GWEN_BUFFER *nameBuf;
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlDescr;
  int rv;

  nameBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nameBuf, BUILDERDATADIR GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(nameBuf, builderName);
  GWEN_Buffer_AppendString(nameBuf, ".gwb");

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlRoot, GWEN_Buffer_GetStart(nameBuf), GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL,
              "No description for builder type \"%s\" found (%d, tried \"%s\")",
              builderName, rv, GWEN_Buffer_GetStart(nameBuf));
    GWEN_Buffer_free(nameBuf);
    return NULL;
  }

  xmlDescr=GWEN_XMLNode_FindFirstTag(xmlRoot, "GwenBuilder", NULL, NULL);
  if (xmlDescr==NULL) {
    DBG_ERROR(NULL, "No <GwenBuilder> element found in file \"%s\"", GWEN_Buffer_GetStart(nameBuf));
    GWEN_XMLNode_free(xmlRoot);
    GWEN_Buffer_free(nameBuf);
    return NULL;
  }
  GWEN_XMLNode_UnlinkChild(xmlRoot, xmlDescr);
  GWEN_XMLNode_free(xmlRoot);

  builder=GWB_GenericBuilder_new(gwenbuild, context, xmlDescr);
  if (builder==NULL) {
    DBG_ERROR(NULL, "Error in file \"%s\"", GWEN_Buffer_GetStart(nameBuf));
    GWEN_XMLNode_free(xmlDescr);
    GWEN_Buffer_free(nameBuf);
    return NULL;
  }
  GWEN_Buffer_free(nameBuf);

  return builder;
}





GWB_BUILDER *GWB_GenericBuilder_new(GWENBUILD *gwenbuild, GWB_CONTEXT *context, GWEN_XMLNODE *xmlDescr)
{
  GWB_BUILDER *builder;
  GWB_BUILDER_GENERIC *xbuilder;
  const char *s;

  s=GWEN_XMLNode_GetProperty(xmlDescr, "name", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "No builder name in XML builder description");
    return NULL;
  }
  builder=GWB_Builder_new(gwenbuild, context, s);
  GWEN_NEW_OBJECT(GWB_BUILDER_GENERIC, xbuilder);
  GWEN_INHERIT_SETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder, xbuilder, _freeData);

  xbuilder->builderName=strdup(s);

  xbuilder->dbVars=GWEN_DB_Group_new("vars");
  xbuilder->xmlDescr=xmlDescr;

  GWB_Builder_SetIsAcceptableInputFn(builder, _isAcceptableInput);
  GWB_Builder_SetAddSourceFileFn(builder, _addSourceFile);
  GWB_Builder_SetAddBuildCmdFn(builder, _addBuildCmd);

  _readMainSettingsFromXml(builder);
  _setupVariables(builder);

  return builder;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWB_BUILDER_GENERIC *xbuilder;

  xbuilder=(GWB_BUILDER_GENERIC*) p;

  GWEN_DB_Group_free(xbuilder->dbVars);
  GWEN_XMLNode_free(xbuilder->xmlDescr);
  free(xbuilder->toolName);
  free(xbuilder->builderName);
  GWEN_FREE_OBJECT(xbuilder);
}



void _readMainSettingsFromXml(GWB_BUILDER *builder)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWB_CONTEXT *context;
  GWEN_XMLNODE *nInputFiles;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  context=GWB_Builder_GetContext(builder);

  xbuilder->maxInputFiles=GWEN_XMLNode_GetIntProperty(xbuilder->xmlDescr, "maxinputfiles", -1);

  nInputFiles=GWEN_XMLNode_FindFirstTag(xbuilder->xmlDescr, "inputFiles", NULL, NULL);
  if (nInputFiles) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_FindFirstTag(nInputFiles, "acceptedInputTypes", NULL, NULL);
    if (n)
      xbuilder->acceptedInputTypes=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(context), n);

    n=GWEN_XMLNode_FindFirstTag(nInputFiles, "acceptedInputFiles", NULL, NULL);
    if (n)
      xbuilder->acceptedInputExt=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(context), n);
  }
}



void _setupVariables(GWB_BUILDER *builder)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWB_CONTEXT *context;
  const char *s;
  GWEN_DB_NODE *db;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  db=xbuilder->dbVars;
  context=GWB_Builder_GetContext(builder);

  s=GWB_Context_GetCurrentRelativeDir(context);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "folder", s);

  GWEN_DB_AddGroupChildren(db, GWB_Context_GetVars(context));
}



void _setToolName(GWB_BUILDER *builder, const char *s)
{
  GWB_BUILDER_GENERIC *xbuilder;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);
  free(xbuilder->toolName);
  xbuilder->toolName=s?strdup(s):NULL;
}



void _addSourceFile(GWB_BUILDER *builder, GWB_FILE *f)
{
  GWB_FILE_LIST2 *fileList;

  GWB_Builder_AddInputFile(builder, f);

  fileList=GWB_Builder_GetOutputFileList2(builder);
  if (!(fileList && GWB_File_List2_GetSize(fileList)>0)) {
    _setupAfterAddingFirstInputFile(builder);
  }
}



void _setupAfterAddingFirstInputFile(GWB_BUILDER *builder)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWB_FILE_LIST2 *fileList;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  fileList=GWB_Builder_GetInputFileList2(builder);
  if (fileList) {
    GWB_FILE *file;

    file=GWB_File_List2_GetFront(fileList);
    if (file) {
      const char *s;

      s=GWB_File_GetName(file);
      if (s) {
        const char *ptrToPoint;

        GWEN_DB_SetCharValue(xbuilder->dbVars, GWEN_DB_FLAGS_DEFAULT, "firstInputFile", s);
        ptrToPoint=strrchr(s, '.');
        if (ptrToPoint) {
          char *copyOfName;

          copyOfName=strndup(s, (ptrToPoint-s));
          GWEN_DB_SetCharValue(xbuilder->dbVars, GWEN_DB_FLAGS_DEFAULT, "firstInputFileNoExt", copyOfName);
          free(copyOfName);
        }
      }
    }
  }

  _setupOutFiles(builder);
  _setupTargetLinkSpec(builder);
}



void _setupOutFiles(GWB_BUILDER *builder)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWB_PROJECT *project;
  const char *folder;
  GWEN_XMLNODE *nOutputFiles;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  project=GWB_Target_GetProject(target);

  folder=GWB_Context_GetCurrentBuildDir(context);

  nOutputFiles=GWEN_XMLNode_FindFirstTag(xbuilder->xmlDescr, "outputFiles", NULL, NULL);
  if (nOutputFiles) {
    GWEN_XMLNODE *nFile;

    nFile=GWEN_XMLNode_FindFirstTag(nOutputFiles, "file", NULL, NULL);
    while (nFile) {
      const char *sFileType;
      const char *sInstall;
      GWEN_BUFFER *nbuf;

      sFileType=GWEN_XMLNode_GetProperty(nFile, "type", NULL);
      sInstall=GWEN_XMLNode_GetProperty(nFile, "install", NULL);
      nbuf=_readXmlDataIntoBufferAndExpand(xbuilder->dbVars, nFile);
      if (nbuf) {
	GWB_FILE *fileOut;
        const char *sTargetInstallPath=NULL;

        fileOut=GWB_File_List2_GetOrCreateFile(GWB_Project_GetFileList(project), folder, GWEN_Buffer_GetStart(nbuf));
        GWEN_Buffer_free(nbuf);

        if (sFileType)
          GWB_File_SetFileType(fileOut, sFileType);
        if (sInstall && strcasecmp(sInstall, "target")==0)
          sTargetInstallPath=GWB_Target_GetInstallPath(target);
        GWB_File_AddFlags(fileOut, GWB_FILE_FLAGS_GENERATED);

        if (sTargetInstallPath && *sTargetInstallPath) {
          GWB_File_SetInstallPath(fileOut, sTargetInstallPath);
          GWB_File_AddFlags(fileOut, GWB_FILE_FLAGS_INSTALL);
        }
	GWB_Builder_AddOutputFile(builder, fileOut);
      } /* if nbuf */

      nFile=GWEN_XMLNode_FindNextTag(nFile, "file", NULL, NULL);
    } /* while nFile */
  } /* if nOutputFiles */
}



void _setupTargetLinkSpec(GWB_BUILDER *builder)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWEN_XMLNODE *nTargetLinkSpec;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  nTargetLinkSpec=GWEN_XMLNode_FindFirstTag(xbuilder->xmlDescr, "targetLinkSpecs", NULL, NULL);
  if (nTargetLinkSpec) {
    GWEN_BUFFER *buf;

    buf=_readXmlDataIntoBufferAndExpand(xbuilder->dbVars, nTargetLinkSpec);
    if (buf) {
      GWB_Builder_SetTargetLinkSpec(builder, GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
    }
  }
}



GWEN_BUFFER *_readXmlDataIntoBufferAndExpand(GWEN_DB_NODE *db, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *xmlData;

  xmlData=GWEN_XMLNode_GetFirstData(xmlNode);
  if (xmlData) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlData);
    if (s && *s) {
      int rv;
      GWEN_BUFFER *buf;

      buf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_DB_ReplaceVars(db, s, buf);
      if(rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        GWEN_Buffer_free(buf);
        return NULL;
      }

      return buf;
    }
  }

  return NULL;
}



int _isAcceptableInput(GWEN_UNUSED GWB_BUILDER *builder, const GWB_FILE *f)
{
  GWB_BUILDER_GENERIC *xbuilder;
  const char *sType;
  const char *sExt;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  sType=GWB_File_GetFileType(f);
  sExt=GWB_File_GetExt(f);

  if (sType && xbuilder->acceptedInputTypes && GWEN_StringList_HasString(xbuilder->acceptedInputTypes, sType))
    return 1;

  if (sExt && xbuilder->acceptedInputExt && GWEN_StringList_HasString(xbuilder->acceptedInputExt, sExt))
    return 1;

  return 0;
}



int _addBuildCmd(GWB_BUILDER *builder, GWB_BUILD_CONTEXT *bctx)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWB_CONTEXT *context;
  GWB_BUILD_CMD *bcmd;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);
  context=GWB_Builder_GetContext(builder);

  bcmd=GWB_BuildCmd_new();
  GWB_BuildCmd_SetBuilderName(bcmd, xbuilder->builderName);
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentBuildDir(context));

  _addBuildCommands(builder, bcmd);
  _addPrepareCommands(builder, bcmd);

  GWB_BuildCtx_AddInFilesToCtxAndCmd(bctx, bcmd, GWB_Builder_GetInputFileList2(builder));
  GWB_BuildCtx_AddOutFilesToCtxAndCmd(bctx, bcmd, GWB_Builder_GetOutputFileList2(builder));

  GWB_BuildCtx_AddCommand(bctx, bcmd);

  return 0;
}



void _addBuildCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd)
{
  _addCommands(builder, bcmd, "buildCommands", GWB_BuildCmd_GetBuildCommandList(bcmd));
}



void _addPrepareCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd)
{
  _addCommands(builder, bcmd, "prepareCommands", GWB_BuildCmd_GetPrepareCommandList(bcmd));
}



void _addCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, const char *groupName, GWB_BUILD_SUBCMD_LIST *cmdList)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWEN_XMLNODE *n;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  n=GWEN_XMLNode_FindFirstTag(xbuilder->xmlDescr, groupName, NULL, NULL);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "cmd", NULL, NULL);
    while(n) {
      const char *sToolName;
      GWEN_BUFFER *argsBuffer;
      GWEN_BUFFER *toolNameBuffer;
      GWB_BUILD_SUBCMD *cmd;
      const char *s;

      toolNameBuffer=GWEN_Buffer_new(0, 256, 0, 1);
      sToolName=GWEN_XMLNode_GetProperty(n, "tool", NULL);
      if (sToolName) {
        GWEN_DB_ReplaceVars(xbuilder->dbVars, sToolName, toolNameBuffer);
        sToolName=GWEN_Buffer_GetStart(toolNameBuffer);
      }

      cmd=GWB_BuildSubCmd_new();
      GWB_BuildSubCmd_SetCommand(cmd, sToolName);

      s=GWEN_XMLNode_GetProperty(n, "ignoreResult", "FALSE");
      if (s && strcasecmp(s, "TRUE")==0)
        GWB_BuildSubCmd_AddFlags(cmd, GWB_BUILD_SUBCMD_FLAGS_IGNORE_RESULT);

      s=GWEN_XMLNode_GetProperty(n, "checkDates", "TRUE");
      if (s && strcasecmp(s, "TRUE")==0)
        GWB_BuildSubCmd_AddFlags(cmd, GWB_BUILD_SUBCMD_FLAGS_CHECK_DATES);

      s=GWEN_XMLNode_GetProperty(n, "checkDepends", "FALSE");
      if (s && strcasecmp(s, "TRUE")==0)
        GWB_BuildSubCmd_AddFlags(cmd, GWB_BUILD_SUBCMD_FLAGS_CHECK_DEPENDS);

      s=GWEN_XMLNode_GetProperty(n, "deleteOutFileFirst", "FALSE");
      if (s && strcasecmp(s, "TRUE")==0)
        GWB_BuildSubCmd_AddFlags(cmd, GWB_BUILD_SUBCMD_FLAGS_DEL_OUTFILES);

      argsBuffer=_readArgs(builder, bcmd, n);
      if (argsBuffer) {
        GWB_BuildSubCmd_SetArguments(cmd, GWEN_Buffer_GetStart(argsBuffer));
        GWEN_Buffer_free(argsBuffer);
      }

      _readMainInputFile(builder, cmd, n);
      _readMainOutputFile(builder, cmd, n);
      _readDepFile(builder, cmd, n);

      _readBuildMessage(builder, cmd, n);


      /* done */
      GWB_BuildSubCmd_List_Add(cmd, cmdList);

      GWEN_Buffer_free(toolNameBuffer);

      n=GWEN_XMLNode_FindNextTag(n, "cmd", NULL, NULL);
    }
  }
}



void _readBuildMessage(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWEN_XMLNODE *xmlMsg;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  xmlMsg=GWEN_XMLNode_FindFirstTag(xmlNode, "buildMessage", NULL, NULL);
  if (xmlMsg) {
    GWEN_BUFFER *buf;

    buf=_readXmlDataIntoBufferAndExpand(xbuilder->dbVars, xmlMsg);
    if (buf) {
      GWB_BuildSubCmd_SetBuildMessage(cmd, GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
    }
  }
}



void _readMainInputFile(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *xmlFile;

  xmlFile=GWEN_XMLNode_FindFirstTag(xmlNode, "mainInputFile", NULL, NULL);
  if (xmlFile) {
    GWB_CONTEXT *context;
    GWB_FILE_LIST2 *filesList;
    GWEN_BUFFER *filenameBuffer;

    context=GWB_Builder_GetContext(builder);
    filesList=GWB_Builder_GetInputFileList2(builder);

    filenameBuffer=_readMainFilename(context, xmlFile, filesList);
    if (filenameBuffer) {
      GWB_BuildSubCmd_SetMainInputFilePath(cmd, GWEN_Buffer_GetStart(filenameBuffer));
      GWEN_Buffer_free(filenameBuffer);
    }
  }
}



void _readMainOutputFile(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *xmlFile;

  xmlFile=GWEN_XMLNode_FindFirstTag(xmlNode, "mainOutputFile", NULL, NULL);
  if (xmlFile) {
    GWB_CONTEXT *context;
    GWB_FILE_LIST2 *filesList;
    GWEN_BUFFER *filenameBuffer;

    context=GWB_Builder_GetContext(builder);
    filesList=GWB_Builder_GetOutputFileList2(builder);

    filenameBuffer=_readMainFilename(context, xmlFile, filesList);
    if (filenameBuffer) {
      GWB_BuildSubCmd_SetMainOutputFilePath(cmd, GWEN_Buffer_GetStart(filenameBuffer));
      GWEN_Buffer_free(filenameBuffer);
    }
  }
}



void _readDepFile(GWB_BUILDER *builder, GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *xmlFile;

  xmlFile=GWEN_XMLNode_FindFirstTag(xmlNode, "depFile", NULL, NULL);
  if (xmlFile) {
    GWB_CONTEXT *context;
    GWB_FILE_LIST2 *filesList;
    GWEN_BUFFER *filenameBuffer;

    context=GWB_Builder_GetContext(builder);
    filesList=GWB_Builder_GetOutputFileList2(builder);

    filenameBuffer=_readMainFilename(context, xmlFile, filesList);
    if (filenameBuffer) {
      GWB_BuildSubCmd_SetDepFilePath(cmd, GWEN_Buffer_GetStart(filenameBuffer));
      GWEN_Buffer_free(filenameBuffer);
    }
  }
}



GWEN_BUFFER *_readMainFilename(GWB_CONTEXT *context, GWEN_XMLNODE *xmlFile, GWB_FILE_LIST2 *filesList)
{
  if (filesList) {
    int index;

    index=GWEN_XMLNode_GetIntProperty(xmlFile, "index", -1);
    if (index>=0) {
      GWB_FILE *file;

      file=_getFileAtPosInList2(filesList, index);
      if (file) {
        GWEN_BUFFER *filenameBuffer;
        const char *s;

        filenameBuffer=GWEN_Buffer_new(0, 256, 0, 1);

        if (!(GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED)) {
          s=GWB_Context_GetInitialSourceDir(context);
          if (s && *s) {
            GWEN_Buffer_AppendString(filenameBuffer, s);
            GWEN_Buffer_AppendString(filenameBuffer, GWEN_DIR_SEPARATOR_S);
          }
        }
        s=GWB_File_GetFolder(file);
        if (s && *s) {
          GWEN_Buffer_AppendString(filenameBuffer, s);
          GWEN_Buffer_AppendString(filenameBuffer, GWEN_DIR_SEPARATOR_S);
        }
        GWEN_Buffer_AppendString(filenameBuffer, GWB_File_GetName(file));
        return filenameBuffer;
      }
    }
  }
  return NULL;
}



GWEN_BUFFER *_readArgs(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *nArgs;

  nArgs=GWEN_XMLNode_FindFirstTag(xmlNode, "args", NULL, NULL);
  if (nArgs) {
    GWEN_BUFFER *argsBuffer;

    argsBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    _readArgsLoop(builder, bcmd, nArgs, argsBuffer);
    if (GWEN_Buffer_GetUsedBytes(argsBuffer))
      return argsBuffer;
    GWEN_Buffer_free(argsBuffer);
  }

  return NULL;
}



void _readArgsLoop(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *nArgs, GWEN_BUFFER *argsBuffer)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(nArgs);
  while(n) {
    const char *sTagName;

    sTagName=GWEN_XMLNode_GetData(n);
    if (sTagName) {
      if (strcasecmp(sTagName, "fixed")==0)
	_readArgsFixed(builder, n, argsBuffer);
      else if (strcasecmp(sTagName, "inputFiles")==0)
	_readArgsInputFiles(builder, n, argsBuffer);
      else if (strcasecmp(sTagName, "outputFiles")==0)
	_readArgsOutputFiles(builder, n, argsBuffer);
      else if (strcasecmp(sTagName, "ifHasSubTargetLibs")==0)
	_readArgsIfHasUsedSubTargets(builder, bcmd, n, argsBuffer);
      else if (strcasecmp(sTagName, "subTargetLibs")==0)
	_readArgsUsedSubTargets(builder, argsBuffer);
      else if (strcasecmp(sTagName, "includes")==0)
	_readArgsIncludes(builder, n, argsBuffer);
      else if (strcasecmp(sTagName, "defines")==0)
	_readArgsDefines(builder, argsBuffer);
      else if (strcasecmp(sTagName, "ifHasLibraries")==0)
	_readArgsIfHasLibraries(builder, bcmd, n, argsBuffer);
      else if (strcasecmp(sTagName, "libraries")==0)
        _readArgsLibraries(builder, argsBuffer);
      else if (strcasecmp(sTagName, "haveConfigH")==0)
        _readArgsHaveConfigH(builder, argsBuffer);
      else if (strcasecmp(sTagName, "blank")==0) {
	GWEN_Buffer_AppendString(argsBuffer, " ");
      }
      else {
	DBG_ERROR(NULL, "Unhandled tag \"%s\", ignoring", sTagName);
      }
    }

    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */
}



void _readArgsFixed(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWEN_BUFFER *buf;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  buf=_readXmlDataIntoBufferAndExpand(xbuilder->dbVars, xmlNode);
  if (buf) {
    GWEN_Buffer_AppendString(argsBuffer, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }
}



void _readArgsInputFiles(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_FILE_LIST2 *inFilesList;

  context=GWB_Builder_GetContext(builder);

  inFilesList=GWB_Builder_GetInputFileList2(builder);
  if (inFilesList) {
    int index;
    const char *pattern;
    int useAbsPath;

    useAbsPath=(strcasecmp(GWEN_XMLNode_GetProperty(xmlNode, "mode", "relpath"), "abspath")==0)?1:0;
    pattern=GWEN_XMLNode_GetProperty(xmlNode, "match", NULL);
    index=GWEN_XMLNode_GetIntProperty(xmlNode, "index", -1);

    if (index>=0) {
      GWB_FILE *file;

      file=_getFileAtPosInList2(inFilesList, index);
      if (file) {
        if (useAbsPath)
          GWB_Builder_AddAbsFileNameToBuffer(context, file, argsBuffer);
        else
          GWB_Builder_AddFileNameToBuffer(context, file, argsBuffer);
      }
    }
    else
      _addMatchingFiles(builder, inFilesList, pattern, useAbsPath, argsBuffer);
  }
}



void _readArgsOutputFiles(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_FILE_LIST2 *filesList;

  context=GWB_Builder_GetContext(builder);

  filesList=GWB_Builder_GetOutputFileList2(builder);
  if (filesList) {
    int index;
    const char *pattern;
    const char *useField;
    int useAbsPath;

    useAbsPath=(strcasecmp(GWEN_XMLNode_GetProperty(xmlNode, "mode", "relpath"), "abspath")==0)?1:0;

    pattern=GWEN_XMLNode_GetProperty(xmlNode, "match", NULL);
    index=GWEN_XMLNode_GetIntProperty(xmlNode, "index", -1);
    useField=GWEN_XMLNode_GetProperty(xmlNode, "useFields", NULL);
    if (index>=0) {
      GWB_FILE *file;

      file=_getFileAtPosInList2(filesList, index);
      if (file) {
        if (useField && strcasecmp(useField, "installPath")==0) {
          const char *s;

          s=GWB_File_GetInstallPath(file);
          if (s)
            GWEN_Buffer_AppendString(argsBuffer, s);
        }
        else {
          if (useAbsPath)
            GWB_Builder_AddAbsFileNameToBuffer(context, file, argsBuffer);
          else
            GWB_Builder_AddFileNameToBuffer(context, file, argsBuffer);
        }
      }
    }
    else
      _addMatchingFiles(builder, filesList, pattern, useAbsPath, argsBuffer);
  }
}



void _readArgsLibraries(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_TARGET *target;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  _addStringListToBuffer(GWB_Target_GetUsedLibraryNameList(target), argsBuffer);
}



void _readArgsUsedSubTargets(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_TARGET *target;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  _addStringListToBuffer(GWB_Target_GetUsedTargetLinkSpecList(target), argsBuffer);
}



void _readArgsIfHasUsedSubTargets(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWEN_STRINGLIST *sl;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  sl=GWB_Target_GetUsedTargetLinkSpecList(target);
  if (sl && GWEN_StringList_Count(sl)>0)
    _readArgsLoop(builder, bcmd, xmlNode, argsBuffer);
}



void _readArgsIfHasLibraries(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWEN_STRINGLIST *sl;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  sl=GWB_Target_GetUsedLibraryNameList(target);
  if (sl && GWEN_StringList_Count(sl)>0)
    _readArgsLoop(builder, bcmd, xmlNode, argsBuffer);
}



void _readArgsIncludes(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_KEYVALUEPAIR_LIST *kvpList;
  const char *fileType;

  fileType=GWEN_XMLNode_GetProperty(xmlNode, "type", "c");

  context=GWB_Builder_GetContext(builder);
  kvpList=GWB_Context_GetIncludeList(context);
  if (kvpList) {
    GWEN_Buffer_AppendString(argsBuffer, " ");
    GWB_KeyValuePair_List_SampleValuesByKey(kvpList, fileType, NULL, " ", argsBuffer);
  }
}



void _readArgsDefines(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_KEYVALUEPAIR_LIST *kvpList;

  context=GWB_Builder_GetContext(builder);
  kvpList=GWB_Context_GetDefineList(context);
  if (kvpList)
    GWB_KeyValuePair_List_WriteAllPairsToBuffer(kvpList, "-D", "=", " ", argsBuffer);
}



void _readArgsHaveConfigH(GWB_BUILDER *builder, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  GWB_PROJECT *project;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  project=GWB_Target_GetProject(target);

  if (GWB_Project_GetFlags(project) & GWB_PROJECT_FLAGS_CONFIG_H) {
    GWEN_Buffer_AppendString(argsBuffer, " -DHAVE_CONFIG_H ");
  }
}



void _addMatchingFiles(GWB_BUILDER *builder, GWB_FILE_LIST2 *filesList, const char *pattern, int addAbs, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;

  context=GWB_Builder_GetContext(builder);

  if (filesList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(filesList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
	const char *fname;

	fname=GWB_File_GetName(file);
	if (fname) {
	  if (pattern==NULL || -1!=GWEN_Text_ComparePattern(fname, pattern, 0)) {
            if (GWEN_Buffer_GetUsedBytes(argsBuffer))
              GWEN_Buffer_AppendString(argsBuffer, " ");
            if (addAbs)
	      GWB_Builder_AddAbsFileNameToBuffer(context, file, argsBuffer);
	    else
	      GWB_Builder_AddFileNameToBuffer(context, file, argsBuffer);
	  }
	}
	file=GWB_File_List2Iterator_Next(it);
      }

      GWB_File_List2Iterator_free(it);
    }
  }
}



GWB_FILE *_getFileAtPosInList2(GWB_FILE_LIST2 *filesList, int index)
{
  GWB_FILE_LIST2_ITERATOR *it;
  int i=0;
  
  it=GWB_File_List2_First(filesList);
  if (it) {
    GWB_FILE *file;
  
    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      if (i==index) {
	GWB_File_List2Iterator_free(it);
	return file;
      }
      i++;
      file=GWB_File_List2Iterator_Next(it);
    }

    GWB_File_List2Iterator_free(it);
  }

  return NULL;
}



void _addStringListToBuffer(GWEN_STRINGLIST *sl, GWEN_BUFFER *argsBuffer)
{
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
	if (GWEN_Buffer_GetUsedBytes(argsBuffer))
          GWEN_Buffer_AppendString(argsBuffer, " ");
        GWEN_Buffer_AppendString(argsBuffer, s);
      }
      se=GWEN_StringListEntry_Next(se);
    }
  }
}


