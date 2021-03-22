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



GWB_FILE *GWB_Project_GetFileByPathAndName(const GWB_PROJECT *project, const char *folder, const char *fname);
void GWB_Project_AddFile(GWB_PROJECT *project, GWB_FILE *file);

GWB_TARGET_LIST2 *GWB_Project_GetTargetList(const GWB_PROJECT *project);
void GWB_Project_AddTarget(GWB_PROJECT *project, GWB_TARGET *target);

GWB_BUILDER_LIST2 *GWB_Project_GetBuilderList(const GWB_PROJECT *project);
void GWB_Project_AddBuilder(GWB_PROJECT *project, GWB_BUILDER *builder);



#endif
