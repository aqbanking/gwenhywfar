/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDSUBCMD_H
#define GWBUILD_BUILDSUBCMD_H

#include <gwenhywfar/list.h>


typedef struct GWB_BUILD_SUBCMD GWB_BUILD_SUBCMD;
GWEN_LIST_FUNCTION_DEFS(GWB_BUILD_SUBCMD, GWB_BuildSubCmd);


#include "gwenbuild/buildctx/buildcmd.h"


#define GWB_BUILD_SUBCMD_FLAGS_IGNORE_RESULT 0x0001

#define GWB_BUILD_SUBCMD_FLAGS_CHECK_DATES   0x0002
#define GWB_BUILD_SUBCMD_FLAGS_CHECK_DEPENDS 0x0004




GWB_BUILD_SUBCMD *GWB_BuildSubCmd_new(void);
GWB_BUILD_SUBCMD *GWB_BuildSubCmd_dup(const GWB_BUILD_SUBCMD *origCmd);
void GWB_BuildSubCmd_free(GWB_BUILD_SUBCMD *cmd);


uint32_t GWB_BuildSubCmd_GetFlags(const GWB_BUILD_SUBCMD *cmd);
void GWB_BuildSubCmd_SetFlags(GWB_BUILD_SUBCMD *cmd, uint32_t f);
void GWB_BuildSubCmd_AddFlags(GWB_BUILD_SUBCMD *cmd, uint32_t f);
void GWB_BuildSubCmd_DelFlags(GWB_BUILD_SUBCMD *cmd, uint32_t f);

const char *GWB_BuildSubCmd_GetCommand(const GWB_BUILD_SUBCMD *cmd);
void GWB_BuildSubCmd_SetCommand(GWB_BUILD_SUBCMD *cmd, const char *s);

const char *GWB_BuildSubCmd_GetArguments(const GWB_BUILD_SUBCMD *cmd);
void GWB_BuildSubCmd_SetArguments(GWB_BUILD_SUBCMD *cmd, const char *s);

const char *GWB_BuildSubCmd_GetBuildMessage(const GWB_BUILD_SUBCMD *cmd);
void GWB_BuildSubCmd_SetBuildMessage(GWB_BUILD_SUBCMD *cmd, const char *s);


const char *GWB_BuildSubCmd_GetMainInputFilePath(const GWB_BUILD_SUBCMD *cmd);
void GWB_BuildSubCmd_SetMainInputFilePath(GWB_BUILD_SUBCMD *cmd, const char *s);

const char *GWB_BuildSubCmd_GetMainOutputFilePath(const GWB_BUILD_SUBCMD *cmd);
void GWB_BuildSubCmd_SetMainOutputFilePath(GWB_BUILD_SUBCMD *cmd, const char *s);

const char *GWB_BuildSubCmd_GetDepFilePath(const GWB_BUILD_SUBCMD *cmd);
void GWB_BuildSubCmd_SetDepFilePath(GWB_BUILD_SUBCMD *cmd, const char *s);



void GWB_BuildSubCmd_toXml(const GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode);
GWB_BUILD_SUBCMD *GWB_BuildSubCmd_fromXml(GWEN_XMLNODE *xmlNode);

GWB_BUILD_SUBCMD_LIST *GWB_BuildSubCmd_List_dup(const GWB_BUILD_SUBCMD_LIST *cmdList);


#endif
