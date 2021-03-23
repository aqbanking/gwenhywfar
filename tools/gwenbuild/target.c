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

#include "gwenbuild/target_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>



GWEN_LIST2_FUNCTIONS(GWB_TARGET, GWB_Target)



GWB_TARGET *GWB_Target_new(void)
{
  GWB_TARGET *target;

  GWEN_NEW_OBJECT(GWB_TARGET, target);


  return target;
}



void GWB_Target_free(GWB_TARGET *target)
{
  if (target) {
    free(target->name);
    GWEN_StringList_free(target->sourceFileNameList);
    GWEN_StringList_free(target->usedTargetNameList);

    GWEN_FREE_OBJECT(target);
  }

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



GWBUILD_TARGETTYPE GWB_Target_GetTargetType(const GWB_TARGET *target)
{
  return target->targetType;
}



void GWB_Target_SetTargetType(GWB_TARGET *target, GWBUILD_TARGETTYPE t)
{
  target->targetType=t;
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



GWEN_STRINGLIST *GWB_Target_GetSourceFileNameList(const GWB_TARGET *target)
{
  return target->sourceFileNameList;
}



void GWB_Target_AddSourceFileName(GWB_TARGET *target, const char *s)
{
  if (target->sourceFileNameList==NULL)
    target->sourceFileNameList=GWEN_StringList_new();
  GWEN_StringList_AppendString(target->sourceFileNameList, s, 0, 1);
}



GWEN_STRINGLIST *GWB_Target_GetUsedTargetNameList(const GWB_TARGET *target)
{
  return target->usedTargetNameList;
}



void GWB_Target_AddUsedTargetFileName(GWB_TARGET *target, const char *s)
{
  if (target->usedTargetNameList==NULL)
    target->usedTargetNameList=GWEN_StringList_new();
  GWEN_StringList_AppendString(target->usedTargetNameList, s, 0, 1);
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



void GWB_Target_Dump(const GWB_TARGET *target, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "Target:\n");

  GWBUILD_Debug_PrintValue(     "name..............", target->name, indent+2);
  GWBUILD_Debug_PrintIntValue(  "type..............", target->targetType, indent+2);
  GWBUILD_Debug_PrintIntValue(  "soVersionCurrent..", target->soVersionCurrent, indent+2);
  GWBUILD_Debug_PrintIntValue(  "soVersionAge......", target->soVersionAge, indent+2);
  GWBUILD_Debug_PrintIntValue(  "soVersionRevision.", target->soVersionRevision, indent+2);
  GWBUILD_Debug_PrintStringList("sourceFileNameList", target->sourceFileNameList, indent+2);
  GWBUILD_Debug_PrintStringList("usedTargetNameList", target->usedTargetNameList, indent+2);
  GWBUILD_Debug_PrintStringList("usedLibraryList...", target->usedLibraryList, indent+2);
  GWBUILD_Debug_PrintFile(      "outputFile........", target->outputFile, indent+2);
  GWB_Context_Dump(target->context, indent+2);


}



