/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_TARGET_H
#define GWBUILD_TARGET_H


#include <gwenhywfar/list2.h>


typedef struct GWB_TARGET GWB_TARGET;
GWEN_LIST2_FUNCTION_DEFS(GWB_TARGET, GWB_Target)


#include "gwenbuild/context.h"
#include "gwenbuild/file.h"
#include "gwenbuild/builder.h"
#include "gwenbuild/gwenbuild.h"
#include "gwenbuild/project.h"

#include <gwenhywfar/stringlist.h>



GWB_TARGET *GWB_Target_new(GWB_PROJECT *project);
void GWB_Target_free(GWB_TARGET *target);

const char *GWB_Target_GetName(const GWB_TARGET *target);
void GWB_Target_SetName(GWB_TARGET *target, const char *s);


GWBUILD_TARGETTYPE GWB_Target_GetTargetType(const GWB_TARGET *target);
void GWB_Target_SetTargetType(GWB_TARGET *target, GWBUILD_TARGETTYPE t);


const char *GWB_Target_GetInstallPath(const GWB_TARGET *target);
void GWB_Target_SetInstallPath(GWB_TARGET *target, const char *s);


GWB_PROJECT *GWB_Target_GetProject(const GWB_TARGET *target);


void GWB_Target_SetSoVersion(GWB_TARGET *target, int vCurrent, int vAge, int vRevision);
int GWB_Target_GetSoVersionCurrent(const GWB_TARGET *target);
int GWB_Target_GetSoVersionAge(const GWB_TARGET *target);
int GWB_Target_GetSoVersionRevision(const GWB_TARGET *target);


GWB_CONTEXT *GWB_Target_GetContext(const GWB_TARGET *target);
void GWB_Target_SetContext(GWB_TARGET *target, GWB_CONTEXT *ctx);

GWB_FILE_LIST2 *GWB_Target_GetSourceFileList(const GWB_TARGET *target);
void GWB_Target_AddSourceFile(GWB_TARGET *target, GWB_FILE *file);

GWEN_STRINGLIST *GWB_Target_GetUsedTargetNameList(const GWB_TARGET *target);
void GWB_Target_AddUsedTargetName(GWB_TARGET *target, const char *s);


GWEN_STRINGLIST *GWB_Target_GetUsedLibraryNameList(const GWB_TARGET *target);
void GWB_Target_AddUsedLibraryName(GWB_TARGET *target, const char *s);


GWEN_STRINGLIST *GWB_Target_GetUsedTargetLinkSpecList(const GWB_TARGET *target);
void GWB_Target_AddUsedTargetLinkSpec(GWB_TARGET *target, const char *s);


GWB_BUILDER *GWB_Target_GetBuilder(const GWB_TARGET *target);
void GWB_Target_SetBuilder(GWB_TARGET *target, GWB_BUILDER *builder);

GWB_FILE *GWB_Target_GetOutputFile(const GWB_TARGET *target);
void GWB_Target_SetOutputFile(GWB_TARGET *target, GWB_FILE *f);

void GWB_Target_Dump(const GWB_TARGET *target, int indent, int fullDump);


#endif
