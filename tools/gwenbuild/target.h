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

#include <gwenhywfar/stringlist.h>



GWB_TARGET *GWB_Target_new(void);
void GWB_Target_free(GWB_TARGET *target);

const char *GWB_Target_GetName(const GWB_TARGET *target);
void GWB_Target_SetName(GWB_TARGET *target, const char *s);


GWBUILD_TARGETTYPE GWB_Target_GetTargetType(const GWB_TARGET *target);
void GWB_Target_SetTargetType(GWB_TARGET *target, GWBUILD_TARGETTYPE t);


GWB_CONTEXT *GWB_Target_GetContext(const GWB_TARGET *target);
void GWB_Target_SetContext(GWB_TARGET *target, GWB_CONTEXT *ctx);


GWEN_STRINGLIST *GWB_Target_GetSourceFileNameList(const GWB_TARGET *target);
void GWB_Target_AddSourceFileName(GWB_TARGET *target, const char *s);

GWEN_STRINGLIST *GWB_Target_GetUsedTargetNameList(const GWB_TARGET *target);
void GWB_Target_AddUsedTargetFileName(GWB_TARGET *target, const char *s);


GWB_BUILDER *GWB_Target_GetBuilder(const GWB_TARGET *target);
void GWB_Target_SetBuilder(GWB_TARGET *target, GWB_BUILDER *builder);

GWB_FILE *GWB_Target_GetOutputFile(const GWB_TARGET *target);
void GWB_Target_SetOutputFile(GWB_TARGET *target, GWB_FILE *f);



#endif
