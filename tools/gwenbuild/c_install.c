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

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>



static int _copyFile(const char *sSrcPath, const char *sDestPath);




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

    rv=_copyFile(sSrcPath, sDestPath);
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



int _copyFile(const char *sSrcPath, const char *sDestPath)
{
  int rv;
  struct stat st;

  if (lstat(sSrcPath, &st)==-1) {
    DBG_ERROR(NULL, "ERROR: stat(%s): %s", sSrcPath, strerror(errno));
    return GWEN_ERROR_GENERIC;
  }

  if ((st.st_mode & S_IFMT)==S_IFLNK) {
    char *symlinkbuf;
    int bufSizeNeeded;

    /* copy symlink */
    fprintf(stdout, "Installing symlink '%s'\n", sSrcPath);
    if (st.st_size==0)
      bufSizeNeeded=256;
    else
      bufSizeNeeded=st.st_size+1;
    symlinkbuf=(char*) malloc(bufSizeNeeded);
    assert(symlinkbuf);
    rv=readlink(sSrcPath, symlinkbuf, bufSizeNeeded);
    if (rv==-1) {
      DBG_ERROR(NULL, "ERROR: readlink(%s): %s", sSrcPath, strerror(errno));
      free(symlinkbuf);
      return GWEN_ERROR_GENERIC;
    }
    else if (rv==bufSizeNeeded) {
      DBG_ERROR(NULL, "Buffer too small (%d)", bufSizeNeeded);
      free(symlinkbuf);
      return GWEN_ERROR_GENERIC;
    }

    rv=GWEN_Directory_GetPath(sDestPath,
                              GWEN_DIR_FLAGS_PUBLIC_PATH | GWEN_DIR_FLAGS_PUBLIC_NAME |
                              GWEN_PATH_FLAGS_VARIABLE|
                              GWEN_PATH_FLAGS_CHECKROOT);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      free(symlinkbuf);
      return rv;
    }
    unlink(sDestPath);
    rv=symlink(symlinkbuf, sDestPath);
    if (rv==-1) {
      DBG_ERROR(NULL, "ERROR: symlink(%s): %s", sSrcPath, strerror(errno));
      free(symlinkbuf);
      return GWEN_ERROR_GENERIC;
    }
  }
  else if ((st.st_mode & S_IFMT)==S_IFREG) {
    mode_t newMode=0;

    fprintf(stdout, "Installing file '%s'\n", sSrcPath);
    rv=GWEN_Directory_GetPath(sDestPath,
                              GWEN_DIR_FLAGS_PUBLIC_PATH | GWEN_DIR_FLAGS_PUBLIC_NAME |
                              GWEN_PATH_FLAGS_VARIABLE|
                              GWEN_PATH_FLAGS_CHECKROOT);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }
  
    rv=GWEN_SyncIo_Helper_CopyFile(sSrcPath, sDestPath);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }

    newMode=S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    if (st.st_mode & S_IXUSR)
      newMode|=S_IXUSR|S_IXGRP|S_IXOTH;
    rv=chmod(sDestPath, newMode);
    if (rv<0) {
      DBG_ERROR(NULL, "ERROR: chmod(%s): %s", sSrcPath, strerror(errno));
      return rv;
    }
  }
  else {
    DBG_ERROR(NULL, "Unhandled file type \"%s\"", sSrcPath);
  }

  return 0;
}




