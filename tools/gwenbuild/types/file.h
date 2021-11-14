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
#include <gwenhywfar/xml.h>
#include <gwenhywfar/stringlist.h>


typedef struct GWB_FILE GWB_FILE;
GWEN_LIST2_FUNCTION_DEFS(GWB_FILE, GWB_File)

#define GWB_FILE_FLAGS_DIST      0x0001
#define GWB_FILE_FLAGS_INSTALL   0x0002
#define GWB_FILE_FLAGS_GENERATED 0x0004


#include "gwenbuild/types/context.h"
#include "gwenbuild/buildctx/buildcmd.h"

#include <inttypes.h>



GWB_FILE *GWB_File_new(const char *folder, const char *fName, uint32_t id);
GWB_FILE *GWB_File_dup(const GWB_FILE *oldFile);
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

const char *GWB_File_GetInstallName(const GWB_FILE *f);
void GWB_File_SetInstallName(GWB_FILE *f, const char *s);

const char *GWB_File_GetExt(const GWB_FILE *f);


const char *GWB_File_GetBuilder(const GWB_FILE *f);
void GWB_File_SetBuilder(GWB_FILE *f, const char *s);


const char *GWB_File_GetInstallPath(const GWB_FILE *f);
void GWB_File_SetInstallPath(GWB_FILE *f, const char *s);


const char *GWB_File_GetFileType(const GWB_FILE *f);
void GWB_File_SetFileType(GWB_FILE *f, const char *s);


void GWB_File_toXml(const GWB_FILE *file, GWEN_XMLNODE *xmlNode);
GWB_FILE *GWB_File_fromXml(GWEN_XMLNODE *xmlNode);


GWB_BUILD_CMD_LIST2 *GWB_File_GetWaitingBuildCmdList2(const GWB_FILE *f);
void GWB_File_AddWaitingBuildCmd(GWB_FILE *f, GWB_BUILD_CMD *bcmd);
void GWB_File_ClearWaitingBuildCmds(GWB_FILE *file);


GWB_BUILD_CMD *GWB_File_GetBuildCmd(const GWB_FILE *f);
void GWB_File_SetBuildCmd(GWB_FILE *f, GWB_BUILD_CMD *bcmd);


void GWB_File_List2_FreeAll(GWB_FILE_LIST2 *fileList2);
GWB_FILE *GWB_File_List2_GetFileByPathAndName(const GWB_FILE_LIST2 *fileList, const char *folder, const char *fname);
GWB_FILE *GWB_File_List2_GetFileById(const GWB_FILE_LIST2 *fileList, uint32_t id);

GWB_FILE *GWB_File_List2_GetOrCreateFile(GWB_FILE_LIST2 *fileList, const char *folder, const char *fname);


GWB_FILE *GWB_File_List2_GetAt(const GWB_FILE_LIST2 *fileList, int index);


void GWB_File_List2_WriteXml(const GWB_FILE_LIST2 *fileList, GWEN_XMLNODE *xmlNode, const char *groupName);
void GWB_File_List2_ReadXml(GWEN_XMLNODE *xmlNode, const char *groupName, GWB_FILE_LIST2 *destFileList);


void GWB_File_ReplaceExtension(GWB_FILE *file, const char *newExt);
GWB_FILE *GWB_File_CopyObjectAndChangeExtension(const GWB_FILE *file, const char *newExt);

void GWB_File_AddFileList2ToFileList2(GWB_FILE_LIST2 *sourceList, GWB_FILE_LIST2 *destList, const char *ext);

void GWB_File_WriteFileNameToTopBuildDirString(const GWB_FILE *file, const char *initialSourceDir, GWEN_BUFFER *fbuf);
GWEN_STRINGLIST *GWB_File_FileListToTopBuildDirStringList(const GWB_FILE_LIST2 *fileList, const char *initialSourceDir);


#endif
