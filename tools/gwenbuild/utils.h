/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_UTILS_H
#define GWBUILD_UTILS_H


#include "gwenbuild/types/project.h"


int GWB_Utils_WriteProjectFileList(const GWB_PROJECT *project, const char *fileName);

GWB_FILE_LIST2 *GWB_Utils_ReadFileList2(const char *fileName);

int GWB_Utils_WriteBuildFileList(const GWENBUILD *gwenbuild, const char *fileName);

int GWB_Utils_WriteInstallFileList(const GWB_PROJECT *project, const char *fileName);
GWB_KEYVALUEPAIR_LIST *GWB_Utils_ReadInstallFileList(const char *fileName);

int GWB_Utils_WriteProjectToFile(const GWB_PROJECT *project, const char *fileName);

int GWB_Utils_WriteContextTreeToFile(const GWB_CONTEXT *ctx, const char *fileName);

int GWB_Utils_WriteProjectInfoToFile(const GWB_PROJECT *project, const char *fileName);
GWEN_XMLNODE *GWB_Utils_ReadProjectInfoFromFile(const char *fileName);


int GWB_Utils_BuildFilesChanged(const char *fileName);

int GWB_Utils_CopyFile(const char *sSrcPath, const char *sDestPath);


int GWB_Utils_VersionStringToDb(GWEN_DB_NODE *db, const char *prefix, const char *s);
int GWB_Utils_VersionStringToInt(const char *s);
void GWB_Utils_VersionToDbVar(GWEN_DB_NODE *db, const char *dbVarName,
                              int vmajor, int vminor, int vpatchlevel, int vbuild,
                              const char *vtag);


#endif

