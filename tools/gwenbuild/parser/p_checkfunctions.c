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


#include "gwenbuild/parser/p_checkfunctions.h"
#include "gwenbuild/parser/parser.h"
#include "gwenbuild/tools/trylink.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static int _checkFunction(GWB_PROJECT *project, GWB_CONTEXT *currentContext, const char *fnName);
static void _setVarFunctionExists(GWEN_DB_NODE *db, const char *header, int exists);
static void _setDefineFunctionExists(GWB_PROJECT *project, const char *header);
static void _toUpperAndReplacePointWithUnderscore(char *ptr);
static void _replacePointWithUnderscore(char *ptr);






int GWB_ParseCheckFunctions(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  GWEN_STRINGLIST *fileNameList;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  fileNameList=GWB_Parser_ReadXmlDataIntoStringList(currentContext, xmlNode);
  if (fileNameList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(fileNameList);
    while(se) {
      const char *sFileName;

      sFileName=GWEN_StringListEntry_Data(se);
      if (sFileName && *sFileName) {
        rv=_checkFunction(project, currentContext, sFileName);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          GWEN_StringList_free(fileNameList);
          return rv;
        }
      }

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(fileNameList);
  }

  return 0;
}



int _checkFunction(GWB_PROJECT *project, GWB_CONTEXT *currentContext, const char *fnName)
{
  GWEN_BUFFER *codeBuf;
  int rv;

  codeBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(codeBuf, "int %s(void);\n", fnName);
  GWEN_Buffer_AppendArgs(codeBuf, "int main(int argc, char **argv) {return %s();}", fnName);

  rv=GWB_Tools_TryLink(GWB_Project_GetGwbuild(project), GWEN_Buffer_GetStart(codeBuf), NULL);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_Buffer_free(codeBuf);
    return rv;
  }

  _setVarFunctionExists(GWB_Context_GetVars(currentContext), fnName, (rv==0)?1:0);
  if (rv==0)
    _setDefineFunctionExists(project, fnName);
  return 0;
}



void _setVarFunctionExists(GWEN_DB_NODE *db, const char *header, int exists)
{
  GWEN_BUFFER *nameBuf;

  nameBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nameBuf, header);
  _replacePointWithUnderscore(GWEN_Buffer_GetStart(nameBuf));
  GWB_Parser_SetItemValue(db,
                          GWEN_Buffer_GetStart(nameBuf),
                          "_EXISTS", exists?"TRUE":"FALSE");
  GWEN_Buffer_free(nameBuf);
}



void _setDefineFunctionExists(GWB_PROJECT *project, const char *header)
{
  GWEN_BUFFER *nameBuf;

  nameBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nameBuf, "HAVE_");
  GWEN_Buffer_AppendString(nameBuf, header);
  _toUpperAndReplacePointWithUnderscore(GWEN_Buffer_GetStart(nameBuf));
  GWB_Project_SetDefine(project, GWEN_Buffer_GetStart(nameBuf), "1");

  GWEN_Buffer_free(nameBuf);
}



void _toUpperAndReplacePointWithUnderscore(char *ptr)
{
  while(ptr && *ptr) {
    if (*ptr<32)
      *ptr=32;
    else if (*ptr=='.' || *ptr=='/')
      *ptr='_';
    else
      *ptr=toupper(*ptr);
    ptr++;
  }
}



void _replacePointWithUnderscore(char *ptr)
{
  while(ptr && *ptr) {
    if (*ptr<32)
      *ptr=32;
    else if (*ptr=='.' || *ptr=='/')
      *ptr='_';
    ptr++;
  }
}



