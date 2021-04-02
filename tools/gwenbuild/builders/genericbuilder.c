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
void _addCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode);

GWEN_BUFFER *_readArgs(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode);
void _readArgsLoop(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *nArgs, GWEN_BUFFER *argsBuffer);
void _readArgsFixed(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsInputFiles(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsOutputFiles(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsLibraries(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsIncludes(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsDefines(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsUsedSubTargets(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);
void _readArgsIfHasUsedSubTargets(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer);

void _addMatchingFiles(GWB_BUILDER *builder, GWB_FILE_LIST2 *filesList, const char *pattern, int addAbs, GWEN_BUFFER *argsBuffer);
GWB_FILE *_getFileAtPosInList2(GWB_FILE_LIST2 *filesList, int index);
void _addStringListToBuffer(GWEN_STRINGLIST *sl, GWEN_BUFFER *argsBuffer);





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
      xbuilder->acceptedInputTypes=GWB_Parser_ReadXmlDataIntoStringList(context, n);

    n=GWEN_XMLNode_FindFirstTag(nInputFiles, "acceptedInputFiles", NULL, NULL);
    if (n)
      xbuilder->acceptedInputExt=GWB_Parser_ReadXmlDataIntoStringList(context, n);
  }
}



void _setupVariables(GWB_BUILDER *builder)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWENBUILD *gwenbuild;
  GWB_CONTEXT *context;
  GWB_TARGET *target;
  const char *s;
  GWEN_DB_NODE *db;
  int soCurrent;
  int soAge;
  int soRevision;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  gwenbuild=GWB_Builder_GetGwenbuild(builder);
  db=xbuilder->dbVars;
  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);

  soCurrent=GWB_Target_GetSoVersionCurrent(target);
  soAge=GWB_Target_GetSoVersionAge(target);
  soRevision=GWB_Target_GetSoVersionRevision(target);

  s=GWB_Context_GetCurrentRelativeDir(context);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "folder", s);

  s=GWB_Target_GetName(target);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "target_name", s);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT, "target_so_version_current", soCurrent);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT, "target_so_version_age", soAge);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT, "target_so_version_revision", soRevision);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT, "target_so_version_effective", soCurrent-soAge);

  s=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "cflags", 0, NULL);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "cflags", s);

  s=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "ldflags", 0, NULL);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "ldflags", s);

  s=GWBUILD_GetToolNameCC(gwenbuild);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "CC", s);

  s=GWBUILD_GetToolNameCXX(gwenbuild);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "CXX", s);

  s=GWBUILD_GetToolNameLD(gwenbuild);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "LD", s);

  s=GWBUILD_GetToolNameAR(gwenbuild);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "AR", s);

  s=GWBUILD_GetToolNameRANLIB(gwenbuild);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "RANLIB", s);

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
  if (!(fileList && GWB_File_List2_GetSize(fileList)>0))
    _setupAfterAddingFirstInputFile(builder);
}



void _setupAfterAddingFirstInputFile(GWB_BUILDER *builder)
{
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
      GWEN_BUFFER *nbuf;

      sFileType=GWEN_XMLNode_GetProperty(nFile, "type", NULL);
      nbuf=_readXmlDataIntoBufferAndExpand(xbuilder->dbVars, nFile);
      if (nbuf) {
	GWB_FILE *fileOut;
	GWB_FILE *storedFile;

	fileOut=GWB_File_new(folder, GWEN_Buffer_GetStart(nbuf), 0);
	GWEN_Buffer_free(nbuf);

	if (sFileType)
	  GWB_File_SetFileType(fileOut, sFileType);
	GWB_File_AddFlags(fileOut, GWB_FILE_FLAGS_GENERATED);

	storedFile=GWB_Project_GetFileByPathAndName(project,
						    GWB_File_GetFolder(fileOut),
						    GWB_File_GetName(fileOut));
	if (storedFile) {
	  GWB_File_free(fileOut);
	  fileOut=storedFile;
	  GWB_File_AddFlags(fileOut, GWB_FILE_FLAGS_GENERATED);
	  if (GWB_File_GetFileType(fileOut)==NULL)
	    GWB_File_SetFileType(fileOut, sFileType);
	}
	else {
	  GWB_Project_AddFile(project, fileOut);
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

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);
  return 0;
}



void _addBuildCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd)
{
  GWB_BUILDER_GENERIC *xbuilder;
  GWEN_XMLNODE *n;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  n=GWEN_XMLNode_FindFirstTag(xbuilder->xmlDescr, "buildCommands", NULL, NULL);
  if (n)
    _addCommands(builder, bcmd, n);
}



void _addCommands(GWB_BUILDER *builder, GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "cmd", NULL, NULL);
  while(n) {
    const char *sToolName;
    GWEN_BUFFER *argsBuffer;

    sToolName=GWEN_XMLNode_GetProperty(xmlNode, "tool", NULL);
    argsBuffer=_readArgs(builder, bcmd, n);
    if (argsBuffer) {
      GWB_BuildCmd_AddBuildCommand(bcmd, sToolName, GWEN_Buffer_GetStart(argsBuffer));
      GWEN_Buffer_free(argsBuffer);
    }
    else
      GWB_BuildCmd_AddBuildCommand(bcmd, sToolName, NULL);

    n=GWEN_XMLNode_FindNextTag(n, "cmd", NULL, NULL);
  }

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
	_readArgsUsedSubTargets(builder, n, argsBuffer);
      else if (strcasecmp(sTagName, "includes")==0)
	_readArgsIncludes(builder, n, argsBuffer);
      else if (strcasecmp(sTagName, "defines")==0)
	_readArgsDefines(builder, n, argsBuffer);
      else if (strcasecmp(sTagName, "libraries")==0)
	_readArgsLibraries(builder, n, argsBuffer);
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
  GWB_BUILDER_GENERIC *xbuilder;
  GWB_CONTEXT *context;
  GWB_FILE_LIST2 *inFilesList;

  xbuilder=GWEN_INHERIT_GETDATA(GWB_BUILDER, GWB_BUILDER_GENERIC, builder);

  context=GWB_Builder_GetContext(builder);

  inFilesList=GWB_Builder_GetInputFileList2(builder);
  if (inFilesList) {
    int index;
    const char *pattern;

    pattern=GWEN_XMLNode_GetProperty(xmlNode, "match", NULL);
    index=GWEN_XMLNode_GetIntProperty(xmlNode, "index", -1);

    if (index>=0) {
      GWB_FILE *file;

      file=_getFileAtPosInList2(inFilesList, index);
      if (file)
	GWB_Builder_AddAbsFileNameToBuffer(context, file, argsBuffer);
    }
    else
      _addMatchingFiles(builder, inFilesList, pattern, 1, argsBuffer);
  }
}



void _readArgsOutputFiles(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_FILE_LIST2 *outFilesList;

  context=GWB_Builder_GetContext(builder);

  outFilesList=GWB_Builder_GetOutputFileList2(builder);
  if (outFilesList) {
    int index;
    const char *pattern;

    pattern=GWEN_XMLNode_GetProperty(xmlNode, "match", NULL);
    index=GWEN_XMLNode_GetIntProperty(xmlNode, "index", -1);

    if (index>=0) {
      GWB_FILE *file;

      file=_getFileAtPosInList2(outFilesList, index);
      if (file)
	GWB_Builder_AddAbsFileNameToBuffer(context, file, argsBuffer);
    }
    else
      _addMatchingFiles(builder, outFilesList, pattern, 0, argsBuffer); /* 0=add relative not absolute path */
  }
}



void _readArgsLibraries(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_TARGET *target;

  context=GWB_Builder_GetContext(builder);
  target=GWB_Context_GetCurrentTarget(context);
  _addStringListToBuffer(GWB_Target_GetUsedLibraryNameList(target), argsBuffer);
}



void _readArgsUsedSubTargets(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
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



void _readArgsDefines(GWB_BUILDER *builder, GWEN_XMLNODE *xmlNode, GWEN_BUFFER *argsBuffer)
{
  GWB_CONTEXT *context;
  GWB_KEYVALUEPAIR_LIST *kvpList;

  context=GWB_Builder_GetContext(builder);
  kvpList=GWB_Context_GetIncludeList(context);
  if (kvpList)
    GWB_KeyValuePair_List_WriteAllPairsToBuffer(kvpList, "-D", "=", " ", argsBuffer);
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


