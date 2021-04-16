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



static GWB_KEYVALUEPAIR_LIST *_generateInstallList(const GWB_FILE_LIST2 *fileList, const char *initialSourceDir);
static int _filesChanged(const char *fileName, GWEN_STRINGLIST *slFileNameList);
static GWEN_STRINGLIST *_readBuildFileList(const char *fileName);




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


