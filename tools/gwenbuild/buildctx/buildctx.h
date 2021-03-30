/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_BUILDCTX_H
#define GWBUILD_BUILDCTX_H

#include <gwenhywfar/xml.h>


typedef struct GWB_BUILD_CONTEXT GWB_BUILD_CONTEXT;

#include "gwenbuild/buildctx/buildcmd.h"
#include "gwenbuild/file.h"


GWB_BUILD_CONTEXT *GWB_BuildCtx_new();
void GWB_BuildCtx_free(GWB_BUILD_CONTEXT *bctx);

GWB_BUILD_CMD_LIST2 *GWB_BuildCtx_GetCommandList(const GWB_BUILD_CONTEXT *bctx);
void GWB_BuildCtx_AddCommand(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd);

GWB_FILE_LIST2 *GWB_BuildCtx_GetFileList(const GWB_BUILD_CONTEXT *bctx);
void GWB_BuildCtx_AddFile(GWB_BUILD_CONTEXT *bctx, GWB_FILE *file);
GWB_FILE *GWB_BuildCtx_GetFileByPathAndName(const GWB_BUILD_CONTEXT *bctx, const char *folder, const char *fname);

void GWB_BuildCtx_AddInFilesToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE_LIST2 *fileList);
void GWB_BuildCtx_AddOutFilesToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE_LIST2 *fileList);


void GWB_BuildCtx_AddInFileToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE *file);
void GWB_BuildCtx_AddOutFileToCtxAndCmd(GWB_BUILD_CONTEXT *bctx, GWB_BUILD_CMD *bcmd, GWB_FILE *file);

int GWB_BuildCtx_SetupDependencies(GWB_BUILD_CONTEXT *bctx);

void GWB_BuildCtx_toXml(const GWB_BUILD_CONTEXT *bctx, GWEN_XMLNODE *xmlNode);
GWB_BUILD_CONTEXT *GWB_BuildCtx_fromXml(GWEN_XMLNODE *xmlNode);


void GWB_BuildCtx_Dump(const GWB_BUILD_CONTEXT *bctx, int indent);


#endif
