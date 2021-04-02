/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_PROJECT_H
#define GWBUILD_PROJECT_H


typedef struct GWB_PROJECT GWB_PROJECT;


#include "gwenbuild/gwenbuild.h"
#include "gwenbuild/context.h"
#include "gwenbuild/file.h"
#include "gwenbuild/builder.h"
#include "gwenbuild/target.h"
#include "gwenbuild/option.h"


#define GWB_PROJECT_FLAGS_SHARED 0x0001



GWB_PROJECT *GWB_Project_new(GWENBUILD *gwbuild, GWB_CONTEXT *ctx);
void GWB_Project_free(GWB_PROJECT *project);

GWENBUILD *GWB_Project_GetGwbuild(const GWB_PROJECT *project);

GWB_CONTEXT *GWB_Project_GetRootContext(const GWB_PROJECT *project);


const char *GWB_Project_GetProjectName(const GWB_PROJECT *project);
void GWB_Project_SetProjectName(GWB_PROJECT *project, const char *s);

void GWB_Project_SetVersion(GWB_PROJECT *project, int vMajor, int vMinor, int vPatchlevel, int vBuild);
int GWB_Project_GetVersionMajor(const GWB_PROJECT *project);
int GWB_Project_GetVersionMinor(const GWB_PROJECT *project);
int GWB_Project_GetVersionPatchlevel(const GWB_PROJECT *project);
int GWB_Project_GetVersionBuild(const GWB_PROJECT *project);

void GWB_Project_SetSoVersion(GWB_PROJECT *project, int vCurrent, int vAge, int vRevision);
int GWB_Project_GetSoVersionCurrent(const GWB_PROJECT *project);
int GWB_Project_GetSoVersionAge(const GWB_PROJECT *project);
int GWB_Project_GetSoVersionRevision(const GWB_PROJECT *project);

GWB_KEYVALUEPAIR_LIST *GWB_Project_GetDefineList(const GWB_PROJECT *project);
void GWB_Project_SetDefine(GWB_PROJECT *project, const char *name, const char *value);
void GWB_Project_ClearDefineList(GWB_PROJECT *project);


GWB_KEYVALUEPAIR_LIST *GWB_Project_GetGivenOptionList(const GWB_PROJECT *project);
void GWB_Project_SetGivenOptionList(GWB_PROJECT *project, GWB_KEYVALUEPAIR_LIST *kvpList);
void GWB_Project_SetGivenOption(GWB_PROJECT *project, const char *name, const char *value);
const char *GWB_Project_GetGivenOption(const GWB_PROJECT *project, const char *name);


GWB_FILE *GWB_Project_GetFileByPathAndName(const GWB_PROJECT *project, const char *folder, const char *fname);
void GWB_Project_AddFile(GWB_PROJECT *project, GWB_FILE *file);

GWB_TARGET_LIST2 *GWB_Project_GetTargetList(const GWB_PROJECT *project);
void GWB_Project_AddTarget(GWB_PROJECT *project, GWB_TARGET *target);
GWB_TARGET *GWB_Project_GetTargetByName(const GWB_PROJECT *project, const char *name);

GWB_BUILDER_LIST2 *GWB_Project_GetBuilderList(const GWB_PROJECT *project);
void GWB_Project_AddBuilder(GWB_PROJECT *project, GWB_BUILDER *builder);


GWB_OPTION_LIST *GWB_Project_GetOptionList(const GWB_PROJECT *project);
void GWB_Project_AddOption(GWB_PROJECT *project, GWB_OPTION *option);
GWB_OPTION *GWB_Project_GetOptionById(const GWB_PROJECT *project, const char *optionId);

uint32_t GWB_Project_GetFlags(const GWB_PROJECT *project);
void GWB_Project_SetFlags(GWB_PROJECT *project, uint32_t fl);
void GWB_Project_AddFlags(GWB_PROJECT *project, uint32_t fl);
void GWB_Project_DelFlags(GWB_PROJECT *project, uint32_t fl);


void GWB_Project_Dump(const GWB_PROJECT *project, int indent, int fullDump);


#endif
