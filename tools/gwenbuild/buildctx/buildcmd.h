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
#include <gwenhywfar/process.h>


typedef struct GWB_BUILD_CMD GWB_BUILD_CMD;
GWEN_LIST_FUNCTION_DEFS(GWB_BUILD_CMD, GWB_BuildCmd);
GWEN_LIST2_FUNCTION_DEFS(GWB_BUILD_CMD, GWB_BuildCmd);

#include "gwenbuild/file.h"
#include "gwenbuild/keyvaluepair.h"
#include "gwenbuild/buildctx/buildsubcmd.h"



GWB_BUILD_CMD *GWB_BuildCmd_new(void);
GWB_BUILD_CMD *GWB_BuildCmd_dup(GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_free(GWB_BUILD_CMD *bcmd);


const char *GWB_BuildCmd_GetBuilderName(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_SetBuilderName(GWB_BUILD_CMD *bcmd, const char *s);


const char *GWB_BuildCmd_GetFolder(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_SetFolder(GWB_BUILD_CMD *bcmd, const char *s);

const char *GWB_BuildCmd_GetBuildMessage(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_SetBuildMessage(GWB_BUILD_CMD *bcmd, const char *s);


GWB_BUILD_SUBCMD_LIST *GWB_BuildCmd_GetPrepareCommandList(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddPrepareCommand(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *cmd);

GWB_BUILD_SUBCMD_LIST *GWB_BuildCmd_GetBuildCommandList(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddBuildCommand(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *cmd);


int GWB_BuildCmd_GetBlockingFiles(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_SetBlockingFiles(GWB_BUILD_CMD *bcmd, int i);
int GWB_BuildCmd_IncBlockingFiles(GWB_BUILD_CMD *bcmd);
int GWB_BuildCmd_DecBlockingFiles(GWB_BUILD_CMD *bcmd);

GWB_FILE_LIST2 *GWB_BuildCmd_GetInFileList2(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddInFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file);

GWB_FILE_LIST2 *GWB_BuildCmd_GetOutFileList2(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_AddOutFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file);



GWEN_PROCESS *GWB_BuildCmd_GetCurrentProcess(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_SetCurrentProcess(GWB_BUILD_CMD *bcmd, GWEN_PROCESS *process);

GWB_BUILD_SUBCMD *GWB_BuildCmd_GetCurrentCommand(const GWB_BUILD_CMD *bcmd);
void GWB_BuildCmd_SetCurrentCommand(GWB_BUILD_CMD *bcmd, GWB_BUILD_SUBCMD *cmd);


void GWB_BuildCmd_toXml(const GWB_BUILD_CMD *bcmd, GWEN_XMLNODE *xmlNode);
GWB_BUILD_CMD *GWB_BuildCmd_fromXml(GWEN_XMLNODE *xmlNode, GWB_FILE_LIST2 *fileList);


void GWB_BuildCmd_List2_FreeAll(GWB_BUILD_CMD_LIST2 *cmdList);


void GWB_BuildCmd_Dump(const GWB_BUILD_CMD *bcmd, int indent);
void GWB_BuildCmd_PrintDescriptionWithText(const GWB_BUILD_CMD *bcmd, int indent, const char *text);


#endif
