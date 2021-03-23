/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_FILE_H
#define GWBUILD_FILE_H

#include <gwenhywfar/list2.h>


typedef struct GWB_FILE GWB_FILE;
GWEN_LIST2_FUNCTION_DEFS(GWB_FILE, GWB_File)

#define GWB_FILE_FLAGS_DIST    0x0001
#define GWB_FILE_FLAGS_INSTALL 0x0002


#include "gwenbuild/context.h"
#include "gwenbuild/builder.h"

#include <inttypes.h>



GWB_FILE *GWB_File_new(const char *folder, const char *fName, uint32_t id);
void GWB_File_free(GWB_FILE *f);


uint32_t GWB_File_GetId(const GWB_FILE *f);
void GWB_File_SetId(GWB_FILE *f, uint32_t i);

uint32_t GWB_File_GetFlags(const GWB_FILE *f);
void GWB_File_SetFlags(GWB_FILE *f, uint32_t i);
void GWB_File_AddFlags(GWB_FILE *f, uint32_t i);
void GWB_File_DelFlags(GWB_FILE *f, uint32_t i);

const char *GWB_File_GetFolder(const GWB_FILE *f);
void GWB_File_SetFolder(GWB_FILE *f, const char *s);

const char *GWB_File_GetName(const GWB_FILE *f);
void GWB_File_SetName(GWB_FILE *f, const char *s);


const char *GWB_File_GetInstallPath(const GWB_FILE *f);
void GWB_File_SetInstallPath(GWB_FILE *f, const char *s);


const char *GWB_File_GetFileType(const GWB_FILE *f);
void GWB_File_SetFileType(GWB_FILE *f, const char *s);


GWB_BUILDER_LIST2 *GWB_File_GetWaitingBuilderList2(const GWB_FILE *f);
void GWB_File_AddWaitingBuilder(GWB_FILE *f, GWB_BUILDER *builder);


void GWB_File_List2_FreeAll(GWB_FILE_LIST2 *fileList2);


#endif
