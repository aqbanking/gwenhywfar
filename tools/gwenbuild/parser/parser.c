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


#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>



static int _parseSubdir(GWB_PROJECT *project, GWB_CONTEXT *currentContext, const char *sFolder, GWB_PARSER_PARSE_ELEMENT_FN fn);





GWB_CONTEXT *GWB_Parser_CopyContextForSubdir(const GWB_CONTEXT *sourceContext, const char *folder)
{
  GWB_CONTEXT *newContext;

  newContext=GWB_Context_dup(sourceContext);

  GWB_Context_AddCurrentRelativeDir(newContext, folder);

  GWB_Context_InsertTopBuildDir(newContext, "..");
  GWB_Context_InsertTopSourceDir(newContext, "..");

  GWB_Context_AddCurrentBuildDir(newContext, folder);
  GWB_Context_AddCurrentSourceDir(newContext, folder);

  GWB_Context_ClearSourceFileList2(newContext);
  GWB_Context_ClearIncludeList(newContext);
  GWB_Context_ClearDefineList(newContext);

  return newContext;
}



GWEN_XMLNODE *GWB_Parser_ReadBuildFile(const GWB_CONTEXT *currentContext, const char *fileName)
{
  GWEN_BUFFER *fileNameBuf;
  const char *s;
  int rv;
  GWEN_XMLNODE *xmlDocNode;
  GWEN_XMLNODE *xmlGwbuildNode;

  fileNameBuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=GWB_Context_GetCurrentSourceDir(currentContext);
  if (s && *s) {
    GWEN_Buffer_AppendString(fileNameBuf, s);
    GWEN_Buffer_AppendString(fileNameBuf, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(fileNameBuf, fileName);

  xmlDocNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  rv=GWEN_XML_ReadFile(xmlDocNode, GWEN_Buffer_GetStart(fileNameBuf), GWEN_XML_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_INFO(NULL, "here %d)", rv);
    GWEN_Buffer_free(fileNameBuf);
    return NULL;
  }

  xmlGwbuildNode=GWEN_XMLNode_FindFirstTag(xmlDocNode, "gwbuild", NULL, NULL);
  if (xmlGwbuildNode==NULL) {
    DBG_ERROR(NULL, "File \"%s\" does not contain an \"gwbuild\" element", GWEN_Buffer_GetStart(fileNameBuf));
    GWEN_XMLNode_free(xmlDocNode);
    GWEN_Buffer_free(fileNameBuf);
    return NULL;
  }

  GWEN_XMLNode_UnlinkChild(xmlDocNode, xmlGwbuildNode);
  GWEN_XMLNode_free(xmlDocNode);
  GWEN_Buffer_free(fileNameBuf);

  return xmlGwbuildNode;
}



GWEN_STRINGLIST *GWB_Parser_ReadXmlDataIntoStringList(GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *xmlData;

  xmlData=GWEN_XMLNode_GetFirstData(xmlNode);
  if (xmlData) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlData);
    if (s && *s) {
      GWEN_STRINGLIST *sl;

      sl=GWEN_StringList_fromString(s, "", 1);
      if (sl==NULL) {
        DBG_ERROR(NULL, "Could not generate string list from data \"%s\"", s);
        return NULL;
      }

      return sl;
    }
  }

  return NULL;
}



int GWB_Parser_ParseSubdirs(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  GWEN_STRINGLIST *sl;

  sl=GWB_Parser_ReadXmlDataIntoStringList(xmlNode);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *sFolder;

      sFolder=GWEN_StringListEntry_Data(se);
      if (sFolder && *sFolder) {
        int rv;

        rv=_parseSubdir(project, currentContext, sFolder, fn);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          return rv;
        }
      }
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(sl);
  }

  return 0;
}



int _parseSubdir(GWB_PROJECT *project, GWB_CONTEXT *currentContext, const char *sFolder, GWB_PARSER_PARSE_ELEMENT_FN fn)
{

  GWB_CONTEXT *newContext;
  GWEN_XMLNODE *xmlNewFile;
  int rv;

  newContext=GWB_Parser_CopyContextForSubdir(currentContext, sFolder);

  xmlNewFile=GWB_Parser_ReadBuildFile(newContext, GWB_PARSER_FILENAME);
  if (xmlNewFile==NULL) {
    DBG_ERROR(NULL, "No valid node found in build file of subdir \"%s\"", sFolder);
    GWB_Context_free(newContext);
    return GWEN_ERROR_BAD_DATA;
  }

  rv=fn(project, newContext, xmlNewFile);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_XMLNode_free(xmlNewFile);
    GWB_Context_free(newContext);
    return rv;
  }
  GWEN_XMLNode_free(xmlNewFile);

  GWB_Context_Tree2_AddChild(currentContext, newContext);
  return 0;
}





