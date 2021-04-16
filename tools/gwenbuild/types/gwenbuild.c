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


#include "gwenbuild/types/gwenbuild_p.h"
#include "gwenbuild/builders/genericbuilder.h"
#include "gwenbuild/types/builder_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/stringlist.h>

/* for stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* for strerror */
#include <errno.h>
#include <string.h>




/* Changes these two functions for new target types or new source types */
static GWB_BUILDER *_genBuilderForSourceFile(GWENBUILD *gwenbuild, GWB_CONTEXT *context, GWB_FILE *file);
static GWB_BUILDER *_genBuilderForTarget(GWB_PROJECT *project, GWB_TARGET *target);

static int _addOrBuildTargetSources(GWB_PROJECT *project, GWB_TARGET *target);
static int _addSourcesOrMkBuildersAndGetTheirOutputs(GWB_PROJECT *project, 
                                                     GWB_TARGET *target,
                                                     GWB_FILE_LIST2 *sourceFileList,
                                                     GWB_FILE_LIST2 *newOutputList);
static int _addSubTargets(GWB_PROJECT *project);
static int _addSubTargetsForTarget(GWB_PROJECT *project, GWB_TARGET *target, GWEN_STRINGLIST *usedTargetList);
static int _addOneSubTargetForTarget(GWB_TARGET *target, GWB_TARGET *subTarget);

static int _addBuildCommandsFromBuilder(GWB_PROJECT *project, GWB_BUILD_CONTEXT *buildCtx);
static void _addExplicitBuildCommandsFromTargets(GWB_PROJECT *project, GWB_BUILD_CONTEXT *buildCtx);
static void _addFilesToBuildCtx(GWB_BUILD_CONTEXT *buildCtx, GWB_FILE_LIST2 *fileList);





GWENBUILD *GWBUILD_new(void)
{
  GWENBUILD *gwenbuild;

  GWEN_NEW_OBJECT(GWENBUILD, gwenbuild);
  gwenbuild->buildFilenameList=GWEN_StringList_new();

  return gwenbuild;
}



void GWBUILD_free(GWENBUILD *gwenbuild)
{
  if (gwenbuild) {
    GWEN_StringList_free(gwenbuild->buildFilenameList);

    GWEN_FREE_OBJECT(gwenbuild);
  }
}



GWEN_STRINGLIST *GWBUILD_GetBuildFilenameList(const GWENBUILD *gwenbuild)
{
  return gwenbuild->buildFilenameList;
}



void GWBUILD_AddBuildFilename(GWENBUILD *gwenbuild, const char *s)
{
  GWEN_StringList_AppendString(gwenbuild->buildFilenameList, s, 0, 1);
}



GWEN_STRINGLIST *GWBUILD_GetPathFromEnvironment()
{
  const char *s;

  s=getenv("PATH");
  if (s && *s)
    return GWEN_StringList_fromString2(s, ":;", 1,
                                       GWEN_TEXT_FLAGS_DEL_QUOTES |
                                       GWEN_TEXT_FLAGS_CHECK_BACKSLASH |
                                       GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                                       GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS);
  return NULL;
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
    else if (strcasecmp(s, "CxxProgram")==0)
      return GWBUILD_TargetType_CxxProgram;
    else if (strcasecmp(s, "Objects")==0)
      return GWBUILD_TargetType_Objects;
    else if (strcasecmp(s, "Module")==0)
      return GWBUILD_TargetType_Module;
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
  case GWBUILD_TargetType_CxxProgram:         return "CxxProgram";
  case GWBUILD_TargetType_Objects:            return "objects";
  case GWBUILD_TargetType_Module:             return "module";
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
    uint32_t id;
    const char *sFolder;
    const char *sName;
    const char *sInstallPath;
    const char *sFileType;
    uint32_t flags;

    id=GWB_File_GetId(file);
    sFolder=GWB_File_GetFolder(file);
    sName=GWB_File_GetName(file);
    flags=GWB_File_GetFlags(file);
    sFileType=GWB_File_GetFileType(file);
    sInstallPath=GWB_File_GetInstallPath(file);

    fprintf(stderr, "[%5d] ", (int) id);
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



void GWBUILD_Debug_PrintOptionList(const char *sName, const GWB_OPTION_LIST *optionList, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (optionList) {
    const GWB_OPTION *option;

    option=GWB_Option_List_First(optionList);
    while(option) {
      GWB_Option_Dump(option, indent+2);
      option=GWB_Option_List_Next(option);
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
      fprintf(stderr, "[%s]\n", (s && *s)?s:"<empty>");

      se=GWEN_StringListEntry_Next(se);
    }
  }
}



int GWBUILD_MakeBuildersForTargets(GWB_PROJECT *project)
{
  GWB_TARGET_LIST2 *targetList;

  targetList=GWB_Project_GetTargetList(project);
  if (targetList) {
    GWB_TARGET_LIST2_ITERATOR *it;
    int rv;

    it=GWB_Target_List2_First(targetList);
    if (it) {
      GWB_TARGET *target;

      target=GWB_Target_List2Iterator_Data(it);
      while(target) {
        GWB_BUILDER *builder;

        builder=_genBuilderForTarget(project, target);
        if (builder==NULL) {
          DBG_INFO(NULL, "here)");
          GWB_Target_List2Iterator_free(it);
          return GWEN_ERROR_GENERIC;
        }
        GWB_Target_SetBuilder(target, builder);
        GWB_Project_AddBuilder(project, builder);

        rv=_addOrBuildTargetSources(project, target);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          return rv;
        }

        target=GWB_Target_List2Iterator_Next(it);
      }
      GWB_Target_List2Iterator_free(it);
    }

    rv=_addSubTargets(project);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
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
    DBG_ERROR(NULL, "Empty source file list in context of target \"%s\"", GWB_Target_GetId(target));
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
      DBG_INFO(NULL, "here (%d)", rv);
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
  GWENBUILD *gwenbuild;
  GWB_BUILDER *targetBuilder;
  GWB_FILE_LIST2_ITERATOR *it;
  GWB_CONTEXT *context;

  gwenbuild=GWB_Project_GetGwbuild(project);
  context=GWB_Target_GetContext(target);
  targetBuilder=GWB_Target_GetBuilder(target);

  it=GWB_File_List2_First(sourceFileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      DBG_DEBUG(NULL, "Checking target \"%s\": file \"%s\"",
                GWB_Target_GetId(target),
                GWB_File_GetName(file));
      if (GWB_Builder_IsAcceptableInput(targetBuilder, file)) {
        DBG_DEBUG(NULL, "- adding file \"%s\" as input for target \"%s\"",
                  GWB_File_GetName(file),
                  GWB_Target_GetId(target));
        GWB_Builder_AddSourceFile(targetBuilder, file);
      }
      else {
        GWB_BUILDER *sourceBuilder;

        sourceBuilder=_genBuilderForSourceFile(gwenbuild, context, file);
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
            DBG_INFO(NULL, "here (%d)", rv);
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

      subTarget=GWB_Project_GetTargetById(project, s);
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
  GWB_CONTEXT *context;
  GWB_BUILDER *targetBuilder;
  GWB_BUILDER *subTargetBuilder;
  GWB_FILE_LIST2 *subTargetOutputFileList;
  GWB_FILE *subTargetFile;
  const char *s;

  context=GWB_Target_GetContext(target);

  targetBuilder=GWB_Target_GetBuilder(target);
  if (targetBuilder==NULL) {
    DBG_ERROR(NULL, "No builder for target \"%s\"", GWB_Target_GetId(target));
    return GWEN_ERROR_GENERIC;
  }
  subTargetBuilder=GWB_Target_GetBuilder(subTarget);
  if (subTargetBuilder==NULL) {
    DBG_ERROR(NULL, "No builder for sub-target \"%s\"", GWB_Target_GetId(subTarget));
    return GWEN_ERROR_GENERIC;
  }

  subTargetOutputFileList=GWB_Builder_GetOutputFileList2(subTargetBuilder);
  if (subTargetOutputFileList==NULL) {
    DBG_ERROR(NULL, "No output file list in target \"%s\"", GWB_Target_GetId(subTarget));
    return GWEN_ERROR_GENERIC;
  }
  subTargetFile=GWB_File_List2_GetFront(subTargetOutputFileList);
  if (subTargetFile==NULL) {
    DBG_ERROR(NULL, "No output file in target \"%s\"", GWB_Target_GetId(subTarget));
    return GWEN_ERROR_GENERIC;
  }
  GWB_Builder_AddInputFile(targetBuilder, subTargetFile);

  s=GWB_Builder_GetTargetLinkSpec(subTargetBuilder);
  if (s && *s) {
    const char *folder;
    GWEN_BUFFER *linkSpecBuffer;

    /* determine path */
    folder=GWB_File_GetFolder(subTargetFile);

    linkSpecBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(linkSpecBuffer, "-L");
    GWB_Builder_AddRelativeFolderToBuffer(context, folder, 1, linkSpecBuffer); /* useBuildDir=1 */
    GWEN_Buffer_AppendString(linkSpecBuffer, " ");
    GWEN_Buffer_AppendString(linkSpecBuffer, s);
    GWB_Target_AddUsedTargetLinkSpec(target, GWEN_Buffer_GetStart(linkSpecBuffer));
    GWEN_Buffer_free(linkSpecBuffer);
  }
  return 0;
}



GWB_BUILD_CONTEXT *GWBUILD_MakeBuildCommands(GWB_PROJECT *project)
{
  int rv;
  GWB_BUILD_CONTEXT *buildCtx;
  GWB_CONTEXT *rootContext;

  rootContext=GWB_Project_GetRootContext(project);
  buildCtx=GWB_BuildCtx_new();
  GWB_BuildCtx_SetInitialSourceDir(buildCtx, GWB_Context_GetInitialSourceDir(rootContext));
  
  rv=_addBuildCommandsFromBuilder(project, buildCtx);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWB_BuildCtx_free(buildCtx);
    return NULL;
  }
  _addExplicitBuildCommandsFromTargets(project, buildCtx);

  return buildCtx;
}



int _addBuildCommandsFromBuilder(GWB_PROJECT *project, GWB_BUILD_CONTEXT *buildCtx)
{
  GWB_BUILDER_LIST2 *builderList;

  builderList=GWB_Project_GetBuilderList(project);
  if (builderList) {
    GWB_BUILDER_LIST2_ITERATOR *it;

    it=GWB_Builder_List2_First(builderList);
    if (it) {
      GWB_BUILDER *builder;

      builder=GWB_Builder_List2Iterator_Data(it);
      while(builder) {
        int rv;

        rv=GWB_Builder_AddBuildCmd(builder, buildCtx);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          GWB_Builder_List2Iterator_free(it);
          return rv;
        }
        builder=GWB_Builder_List2Iterator_Next(it);
      }

      GWB_Builder_List2Iterator_free(it);
      return 0;
    }
  }

  DBG_ERROR(NULL, "No targets in 0BUILD files");
  return GWEN_ERROR_NO_DATA;
}



void _addExplicitBuildCommandsFromTargets(GWB_PROJECT *project, GWB_BUILD_CONTEXT *buildCtx)
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
        GWB_BUILD_CMD_LIST *explicitBuildCmdList;

        explicitBuildCmdList=GWB_Target_GetExplicitBuildList(target);
        if (explicitBuildCmdList) {
          GWB_BUILD_CMD *cmd;

          cmd=GWB_BuildCmd_List_First(explicitBuildCmdList);
          while(cmd) {
            _addFilesToBuildCtx(buildCtx, GWB_BuildCmd_GetInFileList2(cmd));  /* assigns ids etc */
            _addFilesToBuildCtx(buildCtx, GWB_BuildCmd_GetOutFileList2(cmd));
            GWB_BuildCtx_AddCommand(buildCtx, GWB_BuildCmd_dup(cmd));
            cmd=GWB_BuildCmd_List_Next(cmd);
          }
        }
        target=GWB_Target_List2Iterator_Next(it);
      }

      GWB_Target_List2Iterator_free(it);
    }
  }
}



void _addFilesToBuildCtx(GWB_BUILD_CONTEXT *buildCtx, GWB_FILE_LIST2 *fileList)
{
  if (fileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWB_FILE *copyOfFile;

        copyOfFile=GWB_File_dup(file);
        GWB_BuildCtx_AddFile(buildCtx, copyOfFile);
        GWB_File_SetId(file, GWB_File_GetId(copyOfFile));
        file=GWB_File_List2Iterator_Next(it);
      }

      GWB_File_List2Iterator_free(it);
    }
  }
}



time_t GWBUILD_GetModificationTimeOfFile(const char *filename)
{
  struct stat st;

  if (lstat(filename, &st)==-1) {
    DBG_INFO(NULL, "Error on stat(%s): %s", filename, strerror(errno));
    return (time_t) 0;
  }

  return st.st_mtime;
}


/* code from https://stackoverflow.com/questions/152016/detecting-cpu-architecture-compile-time
 */
const char *GWBUILD_GetHostArch() { //Get current architecture, detectx nearly every architecture. Coded by Freak
#if defined(__x86_64__) || defined(_M_X64)
  return "x86_64";
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
  return "x86_32";
#elif defined(__ARM_ARCH_2__)
  return "ARM2";
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
  return "ARM3";
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
  return "ARM4T";
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
  return "ARM5"
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
    return "ARM6T2";
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
  return "ARM6";
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
  return "ARM7";
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
  return "ARM7A";
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
  return "ARM7R";
#elif defined(__ARM_ARCH_7M__)
  return "ARM7M";
#elif defined(__ARM_ARCH_7S__)
  return "ARM7S";
#elif defined(__aarch64__) || defined(_M_ARM64)
  return "ARM64";
#elif defined(mips) || defined(__mips__) || defined(__mips)
  return "MIPS";
#elif defined(__sh__)
  return "SUPERH";
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
  return "POWERPC";
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
  return "POWERPC64";
#elif defined(__sparc__) || defined(__sparc)
  return "SPARC";
#elif defined(__m68k__)
  return "M68K";
#else
  return "UNKNOWN";
#endif
}



const char *GWBUILD_GetHostSystem() {
#if defined(__linux__)
  return "linux";
#elif defined(__sun)
  return "solaris";
#elif defined(__FreeBSD__)
  return "freebsd";
#elif defined(__NetBSD__)
  return "netbsd";
#elif defined(__OpenBSD__)
  return "openbsd";
#elif defined(__APPLE__)
  return "osx";
#elif defined(__hpux)
  return "hpux";

#elif defined(__osf__)
  return "tru64";
#elif defined(__sgi)
  return "irix";
#elif defined(_AIX)
  return "aix";
#elif defined(_WIN32)
  return "windows";
#else
  return "unknown";
#endif
}



const char *GWBUILD_GetArchFromTriplet(const char *sTriplet)
{
  if (-1!=GWEN_Text_ComparePattern(sTriplet, "*x86_64*", 0))
    return "x86_64";
  else if (-1!=GWEN_Text_ComparePattern(sTriplet, "*i?86*", 0))
    return "x86_32";
  else
    return "unknown";
}



const char *GWBUILD_GetSystemFromTriplet(const char *sTriplet)
{
  if (-1!=GWEN_Text_ComparePattern(sTriplet, "*mingw*", 0))
    return "windows";
  else if (-1!=GWEN_Text_ComparePattern(sTriplet, "*linux*", 0))
    return "linux";
  else
    return "unknown";
}



void GWBUILD_AddFilesFromStringList(GWB_FILE_LIST2 *mainFileList,
                                    const char *sFolder,
                                    const GWEN_STRINGLIST *fileNameList,
                                    GWB_FILE_LIST2 *outFileList,
                                    uint32_t flagsToAdd,
                                    int copyFileForOutList)
{
  if (fileNameList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(fileNameList);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        GWB_FILE *file;

        file=GWB_File_List2_GetOrCreateFile(mainFileList, sFolder, s);
        GWB_File_AddFlags(file, flagsToAdd);
        if (outFileList) {
          if (copyFileForOutList)
            GWB_File_List2_PushBack(outFileList, GWB_File_dup(file));
          else
            GWB_File_List2_PushBack(outFileList, file);
        }
      }

      se=GWEN_StringListEntry_Next(se);
    }
  }
}



/*
 * --------------------------------------------------------------------------------------------
 * Add new targets or known source types below.
 * --------------------------------------------------------------------------------------------
 */


GWB_BUILDER *_genBuilderForSourceFile(GWENBUILD *gwenbuild, GWB_CONTEXT *context, GWB_FILE *file)
{
  const char *builderName;
  const char *name;
  const char *ext;
  GWB_BUILDER *builder;

  name=GWB_File_GetName(file);
  if (!(name && *name)) {
    DBG_ERROR(NULL, "No file name.");
    return NULL;
  }
  ext=GWB_File_GetExt(file);
  if (ext==NULL) {
    DBG_DEBUG(NULL, "Unable to determine builder for source file \"%s\"", name);
    return NULL;
  }

  builderName=GWB_File_GetBuilder(file);
  if (!(builderName && *builderName)) {
    DBG_INFO(NULL, "Determining builder type for file \%s\"", name);
    if (strcasecmp(ext, ".c")==0)
      builderName="cbuilder";
    else if (strcasecmp(ext, ".cpp")==0)
      builderName="cxxbuilder";
    else if (strcasecmp(ext, ".t2d")==0 || strcasecmp(ext, ".xml")==0)
      builderName="tm2builder";
    /* add more here */
    else {
      DBG_DEBUG(NULL, "Unable to determine builder for source file \"%s\" (unhandled ext)", name);
      return NULL;
    }
    GWB_File_SetBuilder(file, builderName);
  }

  DBG_INFO(NULL, "Selected builder type is for file \%s\" is \"%s\"", name, builderName);
  builder=GWB_GenericBuilder_Factory(gwenbuild, context, builderName);
  if (builder==NULL) {
    DBG_ERROR(NULL, "Could not create builder for type \"%s\"", ext);
    return NULL;
  }

  GWB_Builder_AddSourceFile(builder, file);

  return builder;
}



GWB_BUILDER *_genBuilderForTarget(GWB_PROJECT *project, GWB_TARGET *target)
{
  GWB_BUILDER *builder=NULL;
  GWENBUILD *gwenbuild;

  gwenbuild=GWB_Project_GetGwbuild(project);
  
  switch(GWB_Target_GetTargetType(target)) {
  case GWBUILD_TargetType_Invalid:
  case GWBUILD_TargetType_None:
    break;
  case GWBUILD_TargetType_InstallLibrary:
    /* TODO: take project's "shared" attribute into account */
    //builder=GWB_SharedLibBuilder_new(gwenbuild, GWB_Target_GetContext(target));
    builder=GWB_GenericBuilder_Factory(gwenbuild, GWB_Target_GetContext(target), "sharedlib");
    break;
  case GWBUILD_TargetType_ConvenienceLibrary:
    //builder=GWEN_TmpLibBuilder_new(gwenbuild, GWB_Target_GetContext(target));
    builder=GWB_GenericBuilder_Factory(gwenbuild, GWB_Target_GetContext(target), "tmplib");
    break;
  case GWBUILD_TargetType_Program:
    builder=GWB_GenericBuilder_Factory(gwenbuild, GWB_Target_GetContext(target), "app");
    break;
  case GWBUILD_TargetType_CxxProgram:
    builder=GWB_GenericBuilder_Factory(gwenbuild, GWB_Target_GetContext(target), "cxxapp");
    break;
  case GWBUILD_TargetType_Objects:
    break;
  case GWBUILD_TargetType_Module:
    builder=GWB_GenericBuilder_Factory(gwenbuild, GWB_Target_GetContext(target), "module");
    break;
  }
  if (builder==NULL) {
    DBG_ERROR(NULL,
              "Could not create builder for type \"%s\"",
              GWBUILD_TargetType_toString(GWB_Target_GetTargetType(target)));
    return NULL;
  }

  return builder;
}




