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


#include "c_install.h"
#include "utils.h"
#include "gwenbuild/filenames.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>




int GWB_InstallFiles(const char *fileName, const char *destDir)
{
  GWB_KEYVALUEPAIR_LIST *kvpList;
  GWB_KEYVALUEPAIR *kvp;
  GWEN_BUFFER *destPathBuf;

  kvpList=GWB_Utils_ReadInstallFileList(fileName);
  if (kvpList==NULL) {
    DBG_ERROR(NULL, "Error reading install file list (file \"%s\")", fileName);
    return GWEN_ERROR_GENERIC;
  }

  destPathBuf=GWEN_Buffer_new(0, 256, 0, 1);
  kvp=GWB_KeyValuePair_List_First(kvpList);
  while(kvp) {
    const char *sDestPath;
    const char *sSrcPath;
    int rv;

    sDestPath=GWB_KeyValuePair_GetKey(kvp);
    sSrcPath=GWB_KeyValuePair_GetValue(kvp);
    if (destDir) {
      GWEN_Buffer_AppendString(destPathBuf, destDir);
      GWEN_Buffer_AppendString(destPathBuf, GWEN_DIR_SEPARATOR_S);
      GWEN_Buffer_AppendString(destPathBuf, sDestPath);
      sDestPath=GWEN_Buffer_GetStart(destPathBuf);
    }

    fprintf(stdout, "Installing file '%s'\n", sSrcPath);
    rv=GWB_Utils_CopyFile(sSrcPath, sDestPath);
    if (rv<0) {
      fprintf(stderr, "ERROR: Error installing file \"%s\"\n", sSrcPath);
      GWEN_Buffer_free(destPathBuf);
      GWB_KeyValuePair_List_free(kvpList);
      return 2;
    }
    GWEN_Buffer_Reset(destPathBuf);
    kvp=GWB_KeyValuePair_List_Next(kvp);
  }
  GWEN_Buffer_free(destPathBuf);
  GWB_KeyValuePair_List_free(kvpList);
  return 0;
}





