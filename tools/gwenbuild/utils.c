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




static GWB_KEYVALUEPAIR_LIST *_generateInstallList(const GWB_FILE_LIST2 *fileList, const char *initialSourceDir);
static int _filesChanged(const char *fileName, GWEN_STRINGLIST *slFileNameList);
static GWEN_STRINGLIST *_readBuildFileList(const char *fileName);
static void _writeProjectNameAndVersionToXml(const GWB_PROJECT *project, GWEN_XMLNODE *xmlNode);




int GWB_Utils_WriteProjectFileList(const GWB_PROJECT *project, const char *fileName)
{
  GWB_FILE_LIST2 *fileList;

  fileList=GWB_Project_GetFileList(project);
  if (fileList) {
    GWEN_XMLNODE *xmlNode;
    GWEN_XMLNODE *xmlFileList;
    int rv;
  
    xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    xmlFileList=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FileList");
    GWB_File_List2_WriteXml(fileList, xmlFileList, "file");
    GWEN_XMLNode_AddChild(xmlNode, xmlFileList);
  
    rv=GWEN_XMLNode_WriteFile(xmlNode, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
    GWEN_XMLNode_free(xmlNode);
    if (rv<0) {
      DBG_ERROR(NULL, "Error writing FileList to file \"%s\" (%d)", fileName, rv);
      return rv;
    }

  }

  return 0;
}



GWB_FILE_LIST2 *GWB_Utils_ReadFileList2(const char *fileName)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlFileList;
  int rv;

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL, "Error reading build file list from \"%s\"", fileName);
    GWEN_XMLNode_free(xmlRoot);
    return NULL;
  }

  xmlFileList=GWEN_XMLNode_FindFirstTag(xmlRoot, "FileList", NULL, NULL);
  if (xmlFileList) {
    GWB_FILE_LIST2 *fileList;

    fileList=GWB_File_List2_new();
    GWB_File_List2_ReadXml(xmlFileList, "file", fileList);
    if (GWB_File_List2_GetSize(fileList)==0) {
      GWB_File_List2_free(fileList);
      GWEN_XMLNode_free(xmlRoot);
      return NULL;
    }

    return fileList;
  }

  GWEN_XMLNode_free(xmlRoot);
  return NULL;
}



int GWB_Utils_WriteBuildFileList(const GWENBUILD *gwenbuild, const char *fileName)
{
  GWEN_STRINGLIST *sl;

  sl=GWBUILD_GetBuildFilenameList(gwenbuild);
  if (sl) {
    GWEN_XMLNODE *xmlRoot;
    GWEN_XMLNODE *xmlFileList;
    GWEN_STRINGLISTENTRY *se;
    int rv;

    xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    xmlFileList=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "BuildFiles");
    GWEN_XMLNode_AddChild(xmlRoot, xmlFileList);

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        GWEN_XMLNODE *xmlFile;
        GWEN_XMLNODE *xmlFileName;

        xmlFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "File");
        xmlFileName=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, s);
        GWEN_XMLNode_AddChild(xmlFile, xmlFileName);
        GWEN_XMLNode_AddChild(xmlFileList, xmlFile);
      }

      se=GWEN_StringListEntry_Next(se);
    }

    rv=GWEN_XMLNode_WriteFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
    GWEN_XMLNode_free(xmlRoot);
    if (rv<0) {
      DBG_ERROR(NULL, "Error writing build file list to file \"%s\" (%d)", fileName, rv);
      return rv;
    }
  }

  return 0;
}



int GWB_Utils_WriteInstallFileList(const GWB_PROJECT *project, const char *fileName)
{
  GWB_KEYVALUEPAIR_LIST *genFileList;
  const char *initialSourceDir;

  initialSourceDir=GWB_Context_GetInitialSourceDir(GWB_Project_GetRootContext(project));

  genFileList=_generateInstallList(GWB_Project_GetFileList(project), initialSourceDir);
  if (genFileList) {
    GWB_KEYVALUEPAIR *kvp;
    GWEN_XMLNODE *xmlRoot;
    GWEN_XMLNODE *xmlFileList;
    int rv;

    xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    xmlFileList=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InstallFiles");
    GWEN_XMLNode_AddChild(xmlRoot, xmlFileList);

    kvp=GWB_KeyValuePair_List_First(genFileList);
    while(kvp) {
      const char *sDestPath;
      const char *sSrcPath;

      sDestPath=GWB_KeyValuePair_GetKey(kvp);
      sSrcPath=GWB_KeyValuePair_GetValue(kvp);
      if (sDestPath && sSrcPath) {
        GWEN_XMLNODE *xmlFile;

        xmlFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InstallFile");
        GWEN_XMLNode_SetCharValue(xmlFile, "destination", sDestPath);
        GWEN_XMLNode_SetCharValue(xmlFile, "source", sSrcPath);
        GWEN_XMLNode_AddChild(xmlFileList, xmlFile);
      }

      kvp=GWB_KeyValuePair_List_Next(kvp);
    }

    rv=GWEN_XMLNode_WriteFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
    GWEN_XMLNode_free(xmlRoot);
    if (rv<0) {
      DBG_ERROR(NULL, "Error writing build file list to file \"%s\" (%d)", fileName, rv);
      return rv;
    }
  }

  return 0;
}



GWB_KEYVALUEPAIR_LIST *GWB_Utils_ReadInstallFileList(const char *fileName)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlFileList;
  int rv;

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL, "Error reading build file list from \"%s\"", fileName);
    GWEN_XMLNode_free(xmlRoot);
    return NULL;
  }

  xmlFileList=GWEN_XMLNode_FindFirstTag(xmlRoot, "InstallFiles", NULL, NULL);
  if (xmlFileList) {
    GWB_KEYVALUEPAIR_LIST *kvpList;
    GWEN_XMLNODE *xmlFile;

    kvpList=GWB_KeyValuePair_List_new();
    xmlFile=GWEN_XMLNode_FindFirstTag(xmlFileList, "InstallFile", NULL, NULL);
    while(xmlFile) {
      GWB_KEYVALUEPAIR *kvp;
      const char *sDestPath;
      const char *sSrcPath;

      sDestPath=GWEN_XMLNode_GetCharValue(xmlFile, "destination", NULL);
      sSrcPath=GWEN_XMLNode_GetCharValue(xmlFile, "source", NULL);
      kvp=GWB_KeyValuePair_new(sDestPath, sSrcPath);
      GWB_KeyValuePair_List_Add(kvp, kvpList);

      xmlFile=GWEN_XMLNode_FindNextTag(xmlFile, "InstallFile", NULL, NULL);
    }
    GWEN_XMLNode_free(xmlRoot);

    if (GWB_KeyValuePair_List_GetCount(kvpList)==0) {
      GWB_KeyValuePair_List_free(kvpList);
      return NULL;
    }

    return kvpList;
  }

  GWEN_XMLNode_free(xmlRoot);
  return NULL;
}



GWB_KEYVALUEPAIR_LIST *_generateInstallList(const GWB_FILE_LIST2 *fileList, const char *initialSourceDir)
{
  if (fileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList);
    if (it) {
      GWB_FILE *file;
      GWB_KEYVALUEPAIR_LIST *kvpList;
      GWEN_BUFFER *keyBuf;
      GWEN_BUFFER *valueBuf;

      keyBuf=GWEN_Buffer_new(0, 256, 0, 1);
      valueBuf=GWEN_Buffer_new(0, 256, 0, 1);
      kvpList=GWB_KeyValuePair_List_new();
      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        if (GWB_File_GetFlags(file) & GWB_FILE_FLAGS_INSTALL) {
          const char *s;
          GWB_KEYVALUEPAIR *kvp;

          s=GWB_File_GetInstallPath(file);
          if (s && *s) {
            GWEN_Buffer_AppendString(keyBuf, s);
            GWEN_Buffer_AppendString(keyBuf, GWEN_DIR_SEPARATOR_S);
            GWEN_Buffer_AppendString(keyBuf, GWB_File_GetName(file));

            GWB_File_WriteFileNameToTopBuildDirString(file, initialSourceDir, valueBuf);
            kvp=GWB_KeyValuePair_new(GWEN_Buffer_GetStart(keyBuf),  GWEN_Buffer_GetStart(valueBuf));
            GWB_KeyValuePair_List_Add(kvp, kvpList);
            GWEN_Buffer_Reset(valueBuf);
            GWEN_Buffer_Reset(keyBuf);
          }
        }
        file=GWB_File_List2Iterator_Next(it);
      }
      GWEN_Buffer_free(valueBuf);
      GWEN_Buffer_free(keyBuf);
      GWB_File_List2Iterator_free(it);
      if (GWB_KeyValuePair_List_GetCount(kvpList)==0) {
        GWB_KeyValuePair_List_free(kvpList);
        return NULL;
      }
      return kvpList;
    }
  }

  return NULL;
}



int GWB_Utils_BuildFilesChanged(const char *fileName)
{
  GWEN_STRINGLIST *slFileNameList;

  slFileNameList=_readBuildFileList(fileName);
  if (slFileNameList) {
    if (_filesChanged(fileName, slFileNameList)) {
      GWEN_StringList_free(slFileNameList);
      return 1;
    }
    GWEN_StringList_free(slFileNameList);
  }

  return 0;
}



int _filesChanged(const char *fileName, GWEN_STRINGLIST *slFileNameList)
{
  time_t mtSourceFile;
  GWEN_STRINGLISTENTRY *se;

  mtSourceFile=GWBUILD_GetModificationTimeOfFile(fileName);
  if (mtSourceFile==(time_t) 0) {
    DBG_INFO(NULL, "here");
    return 1; /* assume changed */
  }

  se=GWEN_StringList_FirstEntry(slFileNameList);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      time_t mt;

      mt=GWBUILD_GetModificationTimeOfFile(s);
      if (mt!=(time_t) 0) {
        if (difftime(mt, mtSourceFile)>0) {
          DBG_ERROR(NULL, "File \"%s\" changed.", s);
          return 1;
        }
      }
    }

    se=GWEN_StringListEntry_Next(se);
  }

  DBG_DEBUG(NULL, "Files unchanged since last setup.");
  return 0;
}



GWEN_STRINGLIST *_readBuildFileList(const char *fileName)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlFileList;
  int rv;

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL, "Error reading build file list from \"%s\"", fileName);
    GWEN_XMLNode_free(xmlRoot);
    return NULL;
  }

  xmlFileList=GWEN_XMLNode_FindFirstTag(xmlRoot, "BuildFiles", NULL, NULL);
  if (xmlFileList) {
    GWEN_STRINGLIST *sl;
    GWEN_XMLNODE *xmlFile;

    sl=GWEN_StringList_new();
    xmlFile=GWEN_XMLNode_FindFirstTag(xmlFileList, "File", NULL, NULL);
    while(xmlFile) {
      GWEN_XMLNODE *xmlFileName;

      xmlFileName=GWEN_XMLNode_GetFirstData(xmlFile);
      if (xmlFileName) {
        const char *s;

        s=GWEN_XMLNode_GetData(xmlFileName);
        if (s && *s)
          GWEN_StringList_AppendString(sl, s, 0, 1);
      }

      xmlFile=GWEN_XMLNode_FindNextTag(xmlFile, "File", NULL, NULL);
    }

    if (GWEN_StringList_Count(sl)==0) {
      GWEN_StringList_free(sl);
      GWEN_XMLNode_free(xmlRoot);
      return NULL;
    }
    GWEN_XMLNode_free(xmlRoot);
    return sl;
  }

  GWEN_XMLNode_free(xmlRoot);
  return NULL;
}



int GWB_Utils_WriteProjectToFile(const GWB_PROJECT *project, const char *fileName)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlProject;
  int rv;

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  xmlProject=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Project");
  GWB_Project_toXml(project, xmlProject);
  GWEN_XMLNode_AddChild(xmlRoot, xmlProject);

  rv=GWEN_XMLNode_WriteFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  GWEN_XMLNode_free(xmlRoot);
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing project to file \"%s\" (%d)", fileName, rv);
    return rv;
  }

  return 0;
}



int GWB_Utils_CopyFile(const char *sSrcPath, const char *sDestPath)
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



int GWB_Utils_WriteProjectInfoToFile(const GWB_PROJECT *project, const char *fileName)
{
  GWB_CONTEXT *rootContext;
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlProject;
  const char *initialSourceDir;
  int rv;

  rootContext=GWB_Project_GetRootContext(project);
  initialSourceDir=GWB_Context_GetInitialSourceDir(rootContext);

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  xmlProject=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "ProjectInfo");
  if (initialSourceDir && *initialSourceDir)
    GWEN_XMLNode_SetCharValue(xmlProject, "initialSourceDir", initialSourceDir);
  _writeProjectNameAndVersionToXml(project, xmlProject);
  GWEN_XMLNode_AddChild(xmlRoot, xmlProject);

  rv=GWEN_XMLNode_WriteFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  GWEN_XMLNode_free(xmlRoot);
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing project to file \"%s\" (%d)", fileName, rv);
    return rv;
  }

  return 0;
}



void _writeProjectNameAndVersionToXml(const GWB_PROJECT *project, GWEN_XMLNODE *xmlNode)
{
  const char *s;
  s=GWB_Project_GetProjectName(project);
  if (s)
    GWEN_XMLNode_SetCharValue(xmlNode, "projectName", s);
  GWEN_XMLNode_SetIntValue(xmlNode, "versionMajor", GWB_Project_GetVersionMajor(project));
  GWEN_XMLNode_SetIntValue(xmlNode, "versionMinor", GWB_Project_GetVersionMinor(project));
  GWEN_XMLNode_SetIntValue(xmlNode, "versionPatchlevel", GWB_Project_GetVersionPatchlevel(project));
  GWEN_XMLNode_SetIntValue(xmlNode, "versionBuild", GWB_Project_GetVersionBuild(project));
  s=GWB_Project_GetVersionTag(project);
  if (s)
    GWEN_XMLNode_SetCharValue(xmlNode, "versionTag", s);

}



GWEN_XMLNODE *GWB_Utils_ReadProjectInfoFromFile(const char *fileName)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlProjectInfo;
  int rv;

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL, "Error reading project info from \"%s\"", fileName);
    GWEN_XMLNode_free(xmlRoot);
    return NULL;
  }

  xmlProjectInfo=GWEN_XMLNode_FindFirstTag(xmlRoot, "ProjectInfo", NULL, NULL);
  if (xmlProjectInfo) {
    GWEN_XMLNode_UnlinkChild(xmlRoot, xmlProjectInfo);
    GWEN_XMLNode_free(xmlRoot);
    return xmlProjectInfo;
  }

  GWEN_XMLNode_free(xmlRoot);
  return NULL;
}




