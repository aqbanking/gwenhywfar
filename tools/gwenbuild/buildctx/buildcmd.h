/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCMD_H
#define GWBUILD_BUILDCMD_H


#include <gwenhywfar/list.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/xml.h>


typedef struct GWB_BUILD_CMD GWB_BUILD_CMD;
GWEN_LIST_FUNCTION_DEFS(GWB_BUILD_CMD, GWB_BuildCmd);
GWEN_LIST2_FUNCTION_DEFS(GWB_BUILD_CMD, GWB_BuildCmd);

#include "gwenbuild/file.h"
#include "gwenbuild/keyvaluepair.h"



GWB_BUILD_CMD *GWB_BuildCmd_new(void);
void GWB_BuildCmd_free(GWB_BUILD_CMD *bcmd);


const char *GWB_BuildCmd_GetFolder(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_SetFolder(GWB_BUILD_CMD *bcmd, const char *s);

GWB_KEYVALUEPAIR_LIST *GWB_BuildCmd_GetPrepareCommandList(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddPrepareCommand(GWB_BUILD_CMD *bcmd, const char *cmd, const char *args);

GWB_KEYVALUEPAIR_LIST *GWB_BuildCmd_GetBuildCommandList(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddBuildCommand(GWB_BUILD_CMD *bcmd, const char *cmd, const char *args);


int GWB_BuildCmd_GetBlockingFiles(const GWB_BUILD_CMD *bcmd);
int GWB_BuildCmd_IncBlockingFiles(GWB_BUILD_CMD *bcmd);
int GWB_BuildCmd_DecBlockingFiles(GWB_BUILD_CMD *bcmd);

GWB_FILE_LIST2 *GWB_BuildCmd_GetInFileList2(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddInFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file);

GWB_FILE_LIST2 *GWB_BuildCmd_GetOutFileList2(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddOutFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file);


void GWB_BuildCmd_toXml(const GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode);


void GWB_BuildCmd_Dump(const GWB_BUILD_CMD *bcmd, int indent);


#endif
