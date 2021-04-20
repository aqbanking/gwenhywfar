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

#include "gwenbuild/types/target_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>



GWEN_LIST2_FUNCTIONS(GWB_TARGET, GWB_Target)



GWB_TARGET *GWB_Target_new(GWB_PROJECT *project)
{
  GWB_TARGET *target;

  GWEN_NEW_OBJECT(GWB_TARGET, target);

  target->sourceFileList=GWB_File_List2_new();
  target->usedTargetNameList=GWEN_StringList_new();
  target->usedLibraryList=GWEN_StringList_new();
  target->usedTargetLinkSpecList=GWEN_StringList_new();
  target->project=project;
  target->explicitBuildList=GWB_BuildCmd_List_new();


  return target;
}



void GWB_Target_free(GWB_TARGET *target)
{
  if (target) {
    free(target->installPath);
    free(target->name);
    free(target->id);
    GWB_File_List2_free(target->sourceFileList);
    GWEN_StringList_free(target->usedTargetNameList);
    GWEN_StringList_free(target->usedTargetLinkSpecList);
    GWEN_StringList_free(target->usedLibraryList);
    GWB_BuildCmd_List_free(target->explicitBuildList);

    GWEN_FREE_OBJECT(target);
  }
}



GWB_PROJECT *GWB_Target_GetProject(const GWB_TARGET *target)
{
  return target->project;
}



const char *GWB_Target_GetName(const GWB_TARGET *target)
{
  return target->name;
}



void GWB_Target_SetName(GWB_TARGET *target, const char *s)
{
  if (target->name)
    free(target->name);
  if (s)
    target->name=strdup(s);
  else
    target->name=NULL;
}



const char *GWB_Target_GetId(const GWB_TARGET *target)
{
  return target->id;
}



void GWB_Target_SetId(GWB_TARGET *target, const char *s)
{
  if (target->id)
    free(target->id);
  if (s)
    target->id=strdup(s);
  else
    target->id=NULL;
}



GWBUILD_TARGETTYPE GWB_Target_GetTargetType(const GWB_TARGET *target)
{
  return target->targetType;
}



void GWB_Target_SetTargetType(GWB_TARGET *target, GWBUILD_TARGETTYPE t)
{
  target->targetType=t;
}



const char *GWB_Target_GetInstallPath(const GWB_TARGET *target)
{
  return target->installPath;
}



void GWB_Target_SetInstallPath(GWB_TARGET *target, const char *s)
{
  free(target->installPath);
  target->installPath=s?strdup(s):NULL;
}



void GWB_Target_SetSoVersion(GWB_TARGET *target, int vCurrent, int vAge, int vRevision)
{
  target->soVersionCurrent=vCurrent;
  target->soVersionAge=vAge;
  target->soVersionRevision=vRevision;
}



int GWB_Target_GetSoVersionCurrent(const GWB_TARGET *target)
{
  return target->soVersionCurrent;
}



int GWB_Target_GetSoVersionAge(const GWB_TARGET *target)
{
  return target->soVersionAge;
}



int GWB_Target_GetSoVersionRevision(const GWB_TARGET *target)
{
  return target->soVersionRevision;
}



GWB_CONTEXT *GWB_Target_GetContext(const GWB_TARGET *target)
{
  return target->context;
}



void GWB_Target_SetContext(GWB_TARGET *target, GWB_CONTEXT *ctx)
{
  target->context=ctx;
}



GWB_FILE_LIST2 *GWB_Target_GetSourceFileList(const GWB_TARGET *target)
{
  return target->sourceFileList;
}



void GWB_Target_AddSourceFile(GWB_TARGET *target, GWB_FILE *file)
{
  GWB_File_List2_PushBack(target->sourceFileList, file);
}




GWEN_STRINGLIST *GWB_Target_GetUsedTargetNameList(const GWB_TARGET *target)
{
  return target->usedTargetNameList;
}



void GWB_Target_AddUsedTargetName(GWB_TARGET *target, const char *s)
{
  if (target->usedTargetNameList==NULL)
    target->usedTargetNameList=GWEN_StringList_new();
  GWEN_StringList_AppendString(target->usedTargetNameList, s, 0, 1);
}



GWEN_STRINGLIST *GWB_Target_GetUsedLibraryNameList(const GWB_TARGET *target)
{
  return target->usedLibraryList;
}



void GWB_Target_AddUsedLibraryName(GWB_TARGET *target, const char *s)
{
  if (target->usedLibraryList==NULL)
    target->usedLibraryList=GWEN_StringList_new();
  GWEN_StringList_AppendString(target->usedLibraryList, s, 0, 1);
}



GWEN_STRINGLIST *GWB_Target_GetUsedTargetLinkSpecList(const GWB_TARGET *target)
{
  return target->usedTargetLinkSpecList;
}



void GWB_Target_AddUsedTargetLinkSpec(GWB_TARGET *target, const char *s)
{
  if (target->usedTargetLinkSpecList==NULL)
    target->usedTargetLinkSpecList=GWEN_StringList_new();
  GWEN_StringList_AppendString(target->usedTargetLinkSpecList, s, 0, 1);
}




GWB_BUILDER *GWB_Target_GetBuilder(const GWB_TARGET *target)
{
  return target->builder;
}



void GWB_Target_SetBuilder(GWB_TARGET *target, GWB_BUILDER *builder)
{
  target->builder=builder;
}



GWB_FILE *GWB_Target_GetOutputFile(const GWB_TARGET *target)
{
  return target->outputFile;
}



void GWB_Target_SetOutputFile(GWB_TARGET *target, GWB_FILE *f)
{
  target->outputFile=f;
}



GWB_BUILD_CMD_LIST *GWB_Target_GetExplicitBuildList(const GWB_TARGET *target)
{
  return target->explicitBuildList;
}



void GWB_Target_AddExplicitBuild(GWB_TARGET *target, GWB_BUILD_CMD *bcmd)
{
  GWB_BuildCmd_List_Add(bcmd, target->explicitBuildList);
}



void GWB_Target_toXml(const GWB_TARGET *target, GWEN_XMLNODE *xmlNode)
{
  if (target->id)
    GWEN_XMLNode_SetCharValue(xmlNode, "id", target->id);
  if (target->name)
    GWEN_XMLNode_SetCharValue(xmlNode, "name", target->name);
  GWEN_XMLNode_SetIntValue(xmlNode, "soVersionCurrent", target->soVersionCurrent);
  GWEN_XMLNode_SetIntValue(xmlNode, "soVersionAge", target->soVersionAge);
  GWEN_XMLNode_SetIntValue(xmlNode, "soVersionRevision", target->soVersionRevision);

  if (target->sourceFileList) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SourceFileList");
    GWB_File_List2_WriteXml(target->sourceFileList, n, "File");
    GWEN_XMLNode_AddChild(xmlNode, n);
  }
}





void GWB_Target_Dump(const GWB_TARGET *target, int indent, int fullDump)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "Target:\n");

  GWBUILD_Debug_PrintValue(     "id....................", target->id, indent+2);
  GWBUILD_Debug_PrintValue(     "name..................", target->name, indent+2);
  GWBUILD_Debug_PrintValue(     "type..................", GWBUILD_TargetType_toString(target->targetType), indent+2);
  GWBUILD_Debug_PrintIntValue(  "soVersionCurrent......", target->soVersionCurrent, indent+2);
  GWBUILD_Debug_PrintIntValue(  "soVersionAge..........", target->soVersionAge, indent+2);
  GWBUILD_Debug_PrintIntValue(  "soVersionRevision.....", target->soVersionRevision, indent+2);
  GWBUILD_Debug_PrintFileList2( "sourceFileList........", target->sourceFileList, indent+2);
  GWBUILD_Debug_PrintStringList("usedTargetNameList....", target->usedTargetNameList, indent+2);
  GWBUILD_Debug_PrintStringList("usedTargetLinkSpecList", target->usedTargetLinkSpecList, indent+2);
  GWBUILD_Debug_PrintStringList("usedLibraryList.......", target->usedLibraryList, indent+2);
  GWBUILD_Debug_PrintFile(      "outputFile............", target->outputFile, indent+2);
  if (fullDump)
    GWB_Context_Dump(target->context, indent+2);


}



