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


#include "gwenbuild/gwenbuild_p.h"
#include "gwenbuild/builders/cbuilder.h"
#include "gwenbuild/builders/staticlib.h"
#include "gwenbuild/builders/tm2builder.h"
#include "gwenbuild/builder_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>



GWB_BUILDER *_genBuilderForSourceFile(GWB_PROJECT *project, GWB_CONTEXT *context, GWB_FILE *file);
int _addOrBuildTargetSources(GWB_PROJECT *project, GWB_TARGET *target);
int _addSourcesOrMkBuildersAndGetTheirOutputs(GWB_PROJECT *project,
                                              GWB_TARGET *target,
                                              GWB_FILE_LIST2 *sourceFileList,
                                              GWB_FILE_LIST2 *newOutputList);
int _addSubTargets(GWB_PROJECT *project);
int _addSubTargetsForTarget(GWB_PROJECT *project, GWB_TARGET *target, GWEN_STRINGLIST *usedTargetList);
int _addOneSubTargetForTarget(GWB_TARGET *target, GWB_TARGET *subTarget);





GWENBUILD *GWBUILD_new(void)
{
  GWENBUILD *gwenbuild;

  GWEN_NEW_OBJECT(GWENBUILD, gwenbuild);
  GWBUILD_SetToolNameCC(gwenbuild, "gcc");
  GWBUILD_SetToolNameCXX(gwenbuild, "g++");
  GWBUILD_SetToolNameLD(gwenbuild, "ld");
  GWBUILD_SetToolNameAR(gwenbuild, "ar");
  GWBUILD_SetToolNameRANLIB(gwenbuild, "ranlib");

  return gwenbuild;
}



void GWBUILD_free(GWENBUILD *gwenbuild)
{
  if (gwenbuild) {
    free(gwenbuild->toolNameCC);
    free(gwenbuild->toolNameCXX);
    free(gwenbuild->toolNameLD);
    free(gwenbuild->toolNameAR);
    free(gwenbuild->toolNameRANLIB);

    GWEN_FREE_OBJECT(gwenbuild);
  }
}



const char *GWBUILD_GetToolNameCC(const GWENBUILD *gwenbuild)
{
  return gwenbuild->toolNameCC;
}



void GWBUILD_SetToolNameCC(GWENBUILD *gwenbuild, const char *s)
{
  free(gwenbuild->toolNameCC);
  if (s)
    gwenbuild->toolNameCC=strdup(s);
  else
    gwenbuild->toolNameCC=NULL;
}



const char *GWBUILD_GetToolNameCXX(const GWENBUILD *gwenbuild)
{
  return gwenbuild->toolNameCXX;
}



void GWBUILD_SetToolNameCXX(GWENBUILD *gwenbuild, const char *s)
{
  free(gwenbuild->toolNameCXX);
  if (s)
    gwenbuild->toolNameCXX=strdup(s);
  else
    gwenbuild->toolNameCXX=NULL;
}



const char *GWBUILD_GetToolNameLD(const GWENBUILD *gwenbuild)
{
  return gwenbuild->toolNameLD;
}



void GWBUILD_SetToolNameLD(GWENBUILD *gwenbuild, const char *s)
{
  free(gwenbuild->toolNameLD);
  if (s)
    gwenbuild->toolNameLD=strdup(s);
  else
    gwenbuild->toolNameLD=NULL;
}



const char *GWBUILD_GetToolNameAR(const GWENBUILD *gwenbuild)
{
  return gwenbuild->toolNameAR;
}



void GWBUILD_SetToolNameAR(GWENBUILD *gwenbuild, const char *s)
{
  free(gwenbuild->toolNameAR);
  if (s)
    gwenbuild->toolNameAR=strdup(s);
  else
    gwenbuild->toolNameAR=NULL;
}



const char *GWBUILD_GetToolNameRANLIB(const GWENBUILD *gwenbuild)
{
  return gwenbuild->toolNameRANLIB;
}



void GWBUILD_SetToolNameRANLIB(GWENBUILD *gwenbuild, const char *s)
{
  free(gwenbuild->toolNameRANLIB);
  if (s)
    gwenbuild->toolNameRANLIB=strdup(s);
  else
    gwenbuild->toolNameRANLIB=NULL;
}






GWBUILD_TARGETTYPE GWBUILD_TargetType_fromString(const char *s)
{
  if (s && *s) {
    if (strcasecmp(s, "InstallLibrary")==0)
      return GWBUILD_TargetType_InstallLibrary;
    else if (strcasecmp(s, "ConvenienceLibrary")==0 ||
             strcasecmp(s, "TempLibrary")==0)
      return GWBUILD_TargetType_ConvenienceLibrary;
    else if (strcasecmp(s, "Program")==0)
      return GWBUILD_TargetType_Program;
    else if (strcasecmp(s, "Objects")==0)
      return GWBUILD_TargetType_Objects;
    else {
      DBG_ERROR(NULL, "Invalid target type \"%s\"", s);
    }
  }
  else {
    DBG_ERROR(NULL, "Empty target type");
  }

  return GWBUILD_TargetType_Invalid;
}



const char *GWBUILD_TargetType_toString(GWBUILD_TARGETTYPE tt)
{
  switch(tt) {
  case GWBUILD_TargetType_Invalid:            return "invalid";
  case GWBUILD_TargetType_None:               return "none";
  case GWBUILD_TargetType_InstallLibrary:     return "InstallLibrary";
  case GWBUILD_TargetType_ConvenienceLibrary: return "ConvenienceLibrary";
  case GWBUILD_TargetType_Program:            return "program";
  case GWBUILD_TargetType_Objects:            return "objects";
  }

  return "invalid";
}


void GWBUILD_Debug_PrintValue(const char *sName, const char *sValue, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s = %s\n", sName, sValue?sValue:"<empty>");
}



void GWBUILD_Debug_PrintIntValue(const char *sName, int value, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s = %d\n", sName, value);
}



void GWBUILD_Debug_PrintKvpList(const char *sName, const GWB_KEYVALUEPAIR_LIST *kvpList, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (kvpList) {
    const GWB_KEYVALUEPAIR *kvp;

    kvp=GWB_KeyValuePair_List_First(kvpList);
    while(kvp) {
      const char *sKey;
      const char *sValue;

      sKey=GWB_KeyValuePair_GetKey(kvp);
      sValue=GWB_KeyValuePair_GetValue(kvp);
      GWBUILD_Debug_PrintValue(sKey, sValue, indent+2);
      kvp=GWB_KeyValuePair_List_Next(kvp);
    }
  }
}



void GWBUILD_Debug_PrintDb(const char *sName, GWEN_DB_NODE *db, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (db)
    GWEN_DB_Dump(db, indent+2);
}



void GWBUILD_Debug_PrintFile(const char *sName, const GWB_FILE *file, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");

  if (sName)
    fprintf(stderr, "%s = ", sName);

  if (file) {
    const char *sFolder;
    const char *sName;
    const char *sInstallPath;
    const char *sFileType;
    uint32_t flags;
  
    sFolder=GWB_File_GetFolder(file);
    sName=GWB_File_GetName(file);
    flags=GWB_File_GetFlags(file);
    sFileType=GWB_File_GetFileType(file);
    sInstallPath=GWB_File_GetInstallPath(file);

    if (sFolder && *sFolder)
      fprintf(stderr, "%s/", sFolder);
    fprintf(stderr, "%s", sName?sName:"<no name>");
    fprintf(stderr, " (%s)", sFileType?sFileType:"no type");

    if (flags & GWB_FILE_FLAGS_DIST)
      fprintf(stderr, " DIST");
    if (flags & GWB_FILE_FLAGS_INSTALL)
      fprintf(stderr, " INSTALL");
    if (flags & GWB_FILE_FLAGS_GENERATED)
      fprintf(stderr, " GENERATED");
    fprintf(stderr, " %s", sInstallPath?sInstallPath:"<no install path>");

    fprintf(stderr, "\n");
  }
  else
    fprintf(stderr, "<empty>\n");
}



void GWBUILD_Debug_PrintFileList2(const char *sName, const GWB_FILE_LIST2 *fileList2, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (fileList2) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList2);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWBUILD_Debug_PrintFile(NULL, file, indent+2);
        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }
}



void GWBUILD_Debug_PrintTargetList2(const char *sName, const GWB_TARGET_LIST2 *targetList2, int indent, int fullDump)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (targetList2) {
    GWB_TARGET_LIST2_ITERATOR *it;

    it=GWB_Target_List2_First(targetList2);
    if (it) {
      GWB_TARGET *target;

      target=GWB_Target_List2Iterator_Data(it);
      while(target) {
        GWB_Target_Dump(target, indent+2, fullDump);
        target=GWB_Target_List2Iterator_Next(it);
      }
      GWB_Target_List2Iterator_free(it);
    }
  }
}



void GWBUILD_Debug_PrintBuilderList2(const char *sName, const GWB_BUILDER_LIST2 *builderList2, int indent, int fullDump)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (builderList2) {
    GWB_BUILDER_LIST2_ITERATOR *it;

    it=GWB_Builder_List2_First(builderList2);
    if (it) {
      GWB_BUILDER *builder;

      builder=GWB_Builder_List2Iterator_Data(it);
      while(builder) {
        GWB_Builder_Dump(builder, indent+2, fullDump);
        builder=GWB_Builder_List2Iterator_Next(it);
      }
      GWB_Builder_List2Iterator_free(it);
    }
  }
}



void GWBUILD_Debug_PrintBuildCmdList2(const char *sName, const GWB_BUILD_CMD_LIST2 *buildCmdList2, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (buildCmdList2) {
    GWB_BUILD_CMD_LIST2_ITERATOR *it;

    it=GWB_BuildCmd_List2_First(buildCmdList2);
    if (it) {
      GWB_BUILD_CMD *builder;

      builder=GWB_BuildCmd_List2Iterator_Data(it);
      while(builder) {
        GWB_BuildCmd_Dump(builder, indent+2);
        builder=GWB_BuildCmd_List2Iterator_Next(it);
      }
      GWB_BuildCmd_List2Iterator_free(it);
    }
  }
}



void GWBUILD_Debug_PrintStringList(const char *sName, const GWEN_STRINGLIST *sl, int indent)
{
  if (sl) {
    int i;
    const GWEN_STRINGLISTENTRY *se;

    for(i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "%s:\n", sName);

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      for(i=0; i<indent+2; i++)
        fprintf(stderr, " ");
      fprintf(stderr, "%s\n", (s && *s)?s:"<empty>");

      se=GWEN_StringListEntry_Next(se);
    }
  }
}



int GWBUILD_GetPathBetweenFolders(const char *folder1, const char *folder2, GWEN_BUFFER *resultBuffer)
{
  GWEN_XMLNODE *xmlNodeRoot;
  GWEN_XMLNODE *xmlNodeFolder1;
  GWEN_XMLNODE *xmlNodeFolder2;
  GWEN_BUFFER *absFolder1Buffer;
  GWEN_BUFFER *absFolder2Buffer;
  int rv;

  xmlNodeRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  absFolder1Buffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Directory_GetAbsoluteFolderPath(folder1, absFolder1Buffer);
  DBG_ERROR(NULL, "Folder1: \"%s\" -> \"%s\"", folder1, GWEN_Buffer_GetStart(absFolder1Buffer));

  absFolder2Buffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Directory_GetAbsoluteFolderPath(folder2, absFolder2Buffer);
  DBG_ERROR(NULL, "Folder2: \"%s\" -> \"%s\"", folder2, GWEN_Buffer_GetStart(absFolder2Buffer));

  xmlNodeFolder1=GWEN_XMLNode_GetNodeByXPath(xmlNodeRoot, GWEN_Buffer_GetStart(absFolder1Buffer), 0);
  GWEN_Buffer_free(absFolder1Buffer);
  xmlNodeFolder2=GWEN_XMLNode_GetNodeByXPath(xmlNodeRoot, GWEN_Buffer_GetStart(absFolder2Buffer), 0);
  GWEN_Buffer_free(absFolder2Buffer);

  rv=GWEN_XMLNode_GetPathBetween(xmlNodeFolder1, xmlNodeFolder2, resultBuffer);
  GWEN_XMLNode_free(xmlNodeRoot);
  if (rv<0) {
    DBG_ERROR(NULL, "Not path found between folders \"%s\" and \"%s\"", folder1, folder2);
    return rv;
  }
  DBG_ERROR(NULL, "Path: \"%s\"", GWEN_Buffer_GetStart(resultBuffer));

  return 0;
}




int GWBUILD_MakeBuildersForTargets(GWB_PROJECT *project)
{
  GWENBUILD *gwenbuild;
  GWB_TARGET_LIST2 *targetList;

  gwenbuild=GWB_Project_GetGwbuild(project);

  targetList=GWB_Project_GetTargetList(project);
  if (targetList) {
    GWB_TARGET_LIST2_ITERATOR *it;
    int rv;

    it=GWB_Target_List2_First(targetList);
    if (it) {
      GWB_TARGET *target;

      target=GWB_Target_List2Iterator_Data(it);
      while(target) {
        GWB_BUILDER *builder=NULL;

        switch(GWB_Target_GetTargetType(target)) {
        case GWBUILD_TargetType_Invalid:
        case GWBUILD_TargetType_None:
          break;
        case GWBUILD_TargetType_InstallLibrary:
          break;
        case GWBUILD_TargetType_ConvenienceLibrary:
          builder=GWEN_StaticLibBuilder_new(gwenbuild, GWB_Target_GetContext(target), 0);
          break;
        case GWBUILD_TargetType_Program:
          break;
        case GWBUILD_TargetType_Objects:
          break;
        }
        if (builder==NULL) {
          DBG_ERROR(NULL,
                    "Could not create builder for type \"%s\"",
                    GWBUILD_TargetType_toString(GWB_Target_GetTargetType(target)));
          GWB_Target_List2Iterator_free(it);
          return GWEN_ERROR_GENERIC;
        }
        GWB_Target_SetBuilder(target, builder);
        GWB_Project_AddBuilder(project, builder);

        rv=_addOrBuildTargetSources(project, target);
        if (rv<0) {
          DBG_ERROR(NULL, "here (%d)", rv);
          return rv;
        }

        target=GWB_Target_List2Iterator_Next(it);
      }
      GWB_Target_List2Iterator_free(it);
    }

    rv=_addSubTargets(project);
    if (rv<0) {
      DBG_ERROR(NULL, "here (%d)", rv);
      return rv;
    }
  }
  return 0;
}



int _addOrBuildTargetSources(GWB_PROJECT *project, GWB_TARGET *target)
{
  GWB_FILE_LIST2 *fileList1;
  GWB_CONTEXT *context;

  context=GWB_Target_GetContext(target);
  fileList1=GWB_Context_GetSourceFileList2(context);
  if (!(fileList1 && GWB_File_List2_GetSize(fileList1)>0)) {
    DBG_ERROR(NULL, "Empty source file list in context of target \"%s\"", GWB_Target_GetName(target));
    GWB_Target_Dump(target, 2, 1);
    return GWEN_ERROR_GENERIC;
  }

  fileList1=GWB_File_List2_dup(fileList1);
  while(GWB_File_List2_GetSize(fileList1)>0) {
    GWB_FILE_LIST2 *fileList2;
    int rv;

    fileList2=GWB_File_List2_new();
    rv=_addSourcesOrMkBuildersAndGetTheirOutputs(project, target, fileList1, fileList2);
    if (rv<0) {
      DBG_ERROR(NULL, "here (%d)", rv);
      GWB_File_List2_free(fileList1);
      GWB_File_List2_free(fileList1);
      return rv;
    }
    GWB_File_List2_free(fileList1);
    fileList1=fileList2;
  }
  GWB_File_List2_free(fileList1);
  return 0;
}



int _addSourcesOrMkBuildersAndGetTheirOutputs(GWB_PROJECT *project,
                                              GWB_TARGET *target,
                                              GWB_FILE_LIST2 *sourceFileList,
                                              GWB_FILE_LIST2 *newOutputList)
{
  GWB_BUILDER *targetBuilder;
  GWB_FILE_LIST2_ITERATOR *it;
  GWB_CONTEXT *context;

  context=GWB_Target_GetContext(target);
  targetBuilder=GWB_Target_GetBuilder(target);
  it=GWB_File_List2_First(sourceFileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      DBG_ERROR(NULL, "Checking target \"%s\": file \"%s\"",
                GWB_Target_GetName(target),
                GWB_File_GetName(file));
      if (GWB_Builder_IsAcceptableInput(targetBuilder, file)) {
        DBG_ERROR(NULL, "- adding file \"%s\" as input for target \"%s\"",
                  GWB_File_GetName(file),
                  GWB_Target_GetName(target));
        GWB_Builder_AddSourceFile(targetBuilder, file);
      }
      else {
        GWB_BUILDER *sourceBuilder;

        sourceBuilder=_genBuilderForSourceFile(project, context, file);
        if (sourceBuilder) {
          GWB_FILE_LIST2 *buildersOutputFileList;

          buildersOutputFileList=GWB_Builder_GetOutputFileList2(sourceBuilder);
          GWB_Project_AddBuilder(project, sourceBuilder);
          GWB_File_AddFileList2ToFileList2(buildersOutputFileList, newOutputList, ".c");
          GWB_File_AddFileList2ToFileList2(buildersOutputFileList, newOutputList, ".cpp");
          GWB_File_AddFileList2ToFileList2(buildersOutputFileList, newOutputList, ".o");
        }
      }
      file=GWB_File_List2Iterator_Next(it);
    }

    GWB_File_List2Iterator_free(it);
  }

  return 0;
}



GWB_BUILDER *_genBuilderForSourceFile(GWB_PROJECT *project, GWB_CONTEXT *context, GWB_FILE *file)
{
  const char *name;
  const char *ext;
  GWENBUILD *gwenbuild;
  GWB_BUILDER *builder;

  gwenbuild=GWB_Project_GetGwbuild(project);

  name=GWB_File_GetName(file);
  if (!(name && *name)) {
    DBG_ERROR(NULL, "No file name.");
    return NULL;
  }
  ext=GWB_File_GetExt(file);
  if (ext==NULL) {
    DBG_ERROR(NULL, "Unable to determine builder for source file \"%s\"", name);
    return NULL;
  }
  ext++;

  if (strcasecmp(ext, "c")==0)
    builder=GWEN_CBuilder_new(gwenbuild, context, 0);
  else if (strcasecmp(ext, "t2d")==0)
    builder=GWEN_Tm2Builder_new(gwenbuild, context, 0);
  /* add more here */
  else {
    DBG_ERROR(NULL, "Unable to determine builder for source file \"%s\" (unhandled ext)", name);
    return NULL;
  }

  GWB_Builder_AddSourceFile(builder, file);

  return builder;
}



int _addSubTargets(GWB_PROJECT *project)
{
  GWB_TARGET_LIST2 *targetList;

  targetList=GWB_Project_GetTargetList(project);
  if (targetList) {
    GWB_TARGET_LIST2_ITERATOR *it;

    it=GWB_Target_List2_First(targetList);
    if (it) {
      GWB_TARGET *target;

      target=GWB_Target_List2Iterator_Data(it);
      while(target) {
        GWEN_STRINGLIST *usedTargetList;

        usedTargetList=GWB_Target_GetUsedTargetNameList(target);
        if (usedTargetList && GWEN_StringList_Count(usedTargetList)>0) {
          int rv;

          rv=_addSubTargetsForTarget(project, target, usedTargetList);
          if (rv<0) {
            DBG_ERROR(NULL, "here (%d)", rv);
            GWB_Target_List2Iterator_free(it);
            return rv;
          }
        }

        target=GWB_Target_List2Iterator_Next(it);
      }
      GWB_Target_List2Iterator_free(it);
    }
  }
  return 0;
}



int _addSubTargetsForTarget(GWB_PROJECT *project, GWB_TARGET *target, GWEN_STRINGLIST *usedTargetList)
{
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(usedTargetList);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      GWB_TARGET *subTarget;

      subTarget=GWB_Project_GetTargetByName(project, s);
      if (subTarget) {
        int rv;

        rv=_addOneSubTargetForTarget(target, subTarget);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          return rv;
        }
      }
    }
    se=GWEN_StringListEntry_Next(se);
  }

  return 0;
}



int _addOneSubTargetForTarget(GWB_TARGET *target, GWB_TARGET *subTarget)
{
  GWB_BUILDER *targetBuilder;
  GWB_BUILDER *subTargetBuilder;
  GWB_FILE_LIST2 *subTargetOutputFileList;
  GWB_FILE *subTargetFile;

  targetBuilder=GWB_Target_GetBuilder(target);
  if (targetBuilder==NULL) {
    DBG_ERROR(NULL, "No builder for target \"%s\"", GWB_Target_GetName(target));
    return GWEN_ERROR_GENERIC;
  }
  subTargetBuilder=GWB_Target_GetBuilder(subTarget);
  if (subTargetBuilder==NULL) {
    DBG_ERROR(NULL, "No builder for sub-target \"%s\"", GWB_Target_GetName(subTarget));
    return GWEN_ERROR_GENERIC;
  }

  subTargetOutputFileList=GWB_Builder_GetOutputFileList2(subTargetBuilder);
  if (subTargetOutputFileList==NULL) {
    DBG_ERROR(NULL, "No output file list in target \"%s\"", GWB_Target_GetName(subTarget));
    return GWEN_ERROR_GENERIC;
  }
  subTargetFile=GWB_File_List2_GetFront(subTargetOutputFileList);
  if (subTargetFile==NULL) {
    DBG_ERROR(NULL, "No output file in target \"%s\"", GWB_Target_GetName(subTarget));
    return GWEN_ERROR_GENERIC;
  }
  GWB_Builder_AddInputFile(targetBuilder, subTargetFile);
  return 0;
}



GWB_BUILD_CONTEXT *GWBUILD_MakeBuildCommands(GWB_PROJECT *project)
{
  GWB_BUILDER_LIST2 *builderList;

  builderList=GWB_Project_GetBuilderList(project);
  if (builderList) {
    GWB_BUILDER_LIST2_ITERATOR *it;

    it=GWB_Builder_List2_First(builderList);
    if (it) {
      GWB_BUILD_CONTEXT *buildCtx;
      GWB_BUILDER *builder;

      buildCtx=GWB_BuildCtx_new();
      builder=GWB_Builder_List2Iterator_Data(it);
      while(builder) {
        int rv;

        rv=GWB_Builder_AddBuildCmd(builder, buildCtx);
        if (rv<0) {
          DBG_ERROR(NULL, "here (%d)", rv);
          GWB_Builder_List2Iterator_free(it);
          GWB_BuildCtx_free(buildCtx);
          return NULL;
        }
        builder=GWB_Builder_List2Iterator_Next(it);
      }

      GWB_Builder_List2Iterator_free(it);
      return buildCtx;
    }
  }
  return NULL;
}





