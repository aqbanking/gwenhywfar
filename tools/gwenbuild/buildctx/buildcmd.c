/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gwenbuild/buildctx/buildcmd_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>



GWEN_LIST_FUNCTIONS(GWB_BUILD_CMD, GWB_BuildCmd);
GWEN_LIST2_FUNCTIONS(GWB_BUILD_CMD, GWB_BuildCmd);





GWB_BUILD_CMD *GWB_BuildCmd_new(void)
{
  GWB_BUILD_CMD *bcmd;

  GWEN_NEW_OBJECT(GWB_BUILD_CMD, bcmd);
  GWEN_LIST_INIT(GWB_BUILD_CMD, bcmd);

  bcmd->inFileList2=GWB_File_List2_new();
  bcmd->outFileList2=GWB_File_List2_new();

  bcmd->prepareCommandList=GWB_KeyValuePair_List_new();
  bcmd->buildCommandList=GWB_KeyValuePair_List_new();

  return bcmd;
}



void GWB_BuildCmd_free(GWB_BUILD_CMD *bcmd)
{
  if (bcmd) {
    GWEN_LIST_FINI(GWB_BUILD_CMD, bcmd);

    GWB_KeyValuePair_List_free(bcmd->prepareCommandList);
    GWB_KeyValuePair_List_free(bcmd->buildCommandList);

    GWB_File_List2_free(bcmd->inFileList2);
    GWB_File_List2_free(bcmd->outFileList2);
    GWEN_FREE_OBJECT(bcmd);
  }
}



const char *GWB_BuildCmd_GetFolder(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->folder;
}



void GWB_BuildCmd_SetFolder(GWB_BUILD_CMD *bcmd, const char *s)
{
  free(bcmd->folder);
  bcmd->folder=s?strdup(s):NULL;
}



GWB_KEYVALUEPAIR_LIST *GWB_BuildCmd_GetPrepareCommandList(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->prepareCommandList;
}



void GWB_BuildCmd_AddPrepareCommand(GWB_BUILD_CMD *bcmd, const char *cmd, const char *args)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_new(cmd, args);
  GWB_KeyValuePair_List_Add(kvp, bcmd->prepareCommandList);
}



GWB_KEYVALUEPAIR_LIST *GWB_BuildCmd_GetBuildCommandList(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->buildCommandList;
}



void GWB_BuildCmd_AddBuildCommand(GWB_BUILD_CMD *bcmd, const char *cmd, const char *args)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_new(cmd, args);
  GWB_KeyValuePair_List_Add(kvp, bcmd->buildCommandList);
}



int GWB_BuildCmd_GetBlockingFiles(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->blockingFiles;
}



int GWB_BuildCmd_IncBlockingFiles(GWB_BUILD_CMD *bcmd)
{
  return ++(bcmd->blockingFiles);
}



int GWB_BuildCmd_DecBlockingFiles(GWB_BUILD_CMD *bcmd)
{
  if (bcmd->blockingFiles>0)
    return --(bcmd->blockingFiles);
  else
    return bcmd->blockingFiles;
}



GWB_FILE_LIST2 *GWB_BuildCmd_GetInFileList2(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->inFileList2;
}



void GWB_BuildCmd_AddInFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file)
{
  if (file)
    GWB_File_List2_PushBack(bcmd->inFileList2, file);
}



GWB_FILE_LIST2 *GWB_BuildCmd_GetOutFileList2(const GWB_BUILD_CMD *bcmd)
{
  return bcmd->outFileList2;
}



void GWB_BuildCmd_AddOutFile(GWB_BUILD_CMD *bcmd, GWB_FILE *file)
{
  if (file)
    GWB_File_List2_PushBack(bcmd->outFileList2, file);
}




