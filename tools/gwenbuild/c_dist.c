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


#include "c_dist.h"
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



static int _writeDistFiles(const char *initialSourceDir, const GWB_FILE_LIST2 *fileList, const char *folder);




int GWB_MkDist()
{
  GWB_FILE_LIST2 *projectFileList;
  GWEN_XMLNODE *xmlProjectInfo;
  GWEN_BUFFER *nameBuf;
  const char *initialSourceDir;
  const char *projectName;
  int versionMajor;
  int versionMinor;
  int versionPatchlevel;
  int versionBuild;
  const char *versionTag;
  int rv;

  projectFileList=GWB_Utils_ReadFileList2(".gwbuild.files");
  if (projectFileList==NULL) {
    DBG_ERROR(NULL, "No file list read.");
    return GWEN_ERROR_GENERIC;
  }

  xmlProjectInfo=GWB_Utils_ReadProjectInfoFromFile(".gwbuild.projectinfo");
  if (xmlProjectInfo==NULL) {
    DBG_ERROR(NULL, "Error reading project info.");
    GWB_File_List2_free(projectFileList);
    return GWEN_ERROR_GENERIC;
  }

  initialSourceDir=GWEN_XMLNode_GetCharValue(xmlProjectInfo, "initialSourceDir", NULL);
  projectName=GWEN_XMLNode_GetCharValue(xmlProjectInfo, "projectName", NULL);
  versionMajor=GWEN_XMLNode_GetIntValue(xmlProjectInfo, "versionMajor", 0);
  versionMinor=GWEN_XMLNode_GetIntValue(xmlProjectInfo, "versionMinor", 0);
  versionPatchlevel=GWEN_XMLNode_GetIntValue(xmlProjectInfo, "versionPatchlevel", 0);
  versionBuild=GWEN_XMLNode_GetIntValue(xmlProjectInfo, "versionBuild", 0);
  versionTag=GWEN_XMLNode_GetCharValue(xmlProjectInfo, "versionTag", NULL);
  if (!(projectName && *projectName)) {
    DBG_ERROR(NULL, "Missing project name in project info file");
    GWEN_XMLNode_free(xmlProjectInfo);
    GWB_File_List2_free(projectFileList);
    return GWEN_ERROR_GENERIC;
  }

  nameBuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (versionBuild>0)
    GWEN_Buffer_AppendArgs(nameBuf,
			   "%s-%d.%d.%d.%d%s",
			   projectName,
			   versionMajor, versionMinor, versionPatchlevel, versionBuild,
			   versionTag?versionTag:"");
  else
    GWEN_Buffer_AppendArgs(nameBuf,
			   "%s-%d.%d.%d%s",
			   projectName,
			   versionMajor, versionMinor, versionPatchlevel,
			   versionTag?versionTag:"");

  rv=_writeDistFiles(initialSourceDir, projectFileList, GWEN_Buffer_GetStart(nameBuf));
  if (rv<0) {
    DBG_ERROR(NULL, "Error copying files into dist folder \"%s\"",
	      GWEN_Buffer_GetStart(nameBuf));
    GWEN_Buffer_free(nameBuf);
    GWEN_XMLNode_free(xmlProjectInfo);
    GWB_File_List2_free(projectFileList);
    return rv;
  }
  GWEN_Buffer_free(nameBuf);
  GWEN_XMLNode_free(xmlProjectInfo);
  GWB_File_List2_free(projectFileList);


  return 0;
}



int _writeDistFiles(const char *initialSourceDir,
		    const GWB_FILE_LIST2 *fileList,
		    const char *folder)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWB_FILE *file;
    GWEN_BUFFER *sourceNameBuf;
    GWEN_BUFFER *destNameBuf;

    sourceNameBuf=GWEN_Buffer_new(0, 256, 0, 1);
    destNameBuf=GWEN_Buffer_new(0, 256, 0, 1);
    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      uint32_t flags;
      const char *s;

      flags=GWB_File_GetFlags(file);
      if (flags & GWB_FILE_FLAGS_DIST) {
	int rv;

	/* generate source path */
	if (!(flags & GWB_FILE_FLAGS_GENERATED)) {
	  GWEN_Buffer_AppendString(sourceNameBuf, initialSourceDir);
	  GWEN_Buffer_AppendString(sourceNameBuf, GWEN_DIR_SEPARATOR_S);
	}
	s=GWB_File_GetFolder(file);
	if (s && *s) {
	  GWEN_Buffer_AppendString(sourceNameBuf, s);
	  GWEN_Buffer_AppendString(sourceNameBuf, GWEN_DIR_SEPARATOR_S);
	}
	GWEN_Buffer_AppendString(sourceNameBuf, GWB_File_GetName(file));

	/* generate target path */
	GWEN_Buffer_AppendString(destNameBuf, folder);
	GWEN_Buffer_AppendString(destNameBuf, GWEN_DIR_SEPARATOR_S);
	if (s && *s) {
	  GWEN_Buffer_AppendString(destNameBuf, s);
	  GWEN_Buffer_AppendString(destNameBuf, GWEN_DIR_SEPARATOR_S);
	}
	GWEN_Buffer_AppendString(destNameBuf, GWB_File_GetName(file));

	fprintf(stdout, "Moving file '%s' into dist folder\n", GWEN_Buffer_GetStart(sourceNameBuf));
	rv=GWB_Utils_CopyFile(GWEN_Buffer_GetStart(sourceNameBuf), GWEN_Buffer_GetStart(destNameBuf));
	if (rv<0) {
	  DBG_ERROR(NULL, "Error copying \"%s\" into dist folder (%d)",
		    GWEN_Buffer_GetStart(sourceNameBuf), rv);
	  GWEN_Buffer_free(sourceNameBuf);
	  GWEN_Buffer_free(destNameBuf);
	  return rv;
	}

	GWEN_Buffer_Reset(sourceNameBuf);
	GWEN_Buffer_Reset(destNameBuf);
      }

      file=GWB_File_List2Iterator_Next(it);
    }
    GWEN_Buffer_free(sourceNameBuf);
    GWEN_Buffer_free(destNameBuf);
    GWB_File_List2Iterator_free(it);
  }

  return 0;

}




