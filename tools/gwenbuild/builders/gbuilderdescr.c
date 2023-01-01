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

#include "gwenbuild/builders/gbuilderdescr_p.h"
#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>



GWEN_LIST_FUNCTIONS(GWB_GBUILDER_DESCR, GWB_GBuilderDescr)



static void _readSettingsFromXml(GWB_GBUILDER_DESCR *descr);
GWB_GBUILDER_DESCR *_readFromFile(const char *fileName);





GWB_GBUILDER_DESCR *GWB_GBuilderDescr_new(GWEN_XMLNODE *xmlNode)
{
  GWB_GBUILDER_DESCR *descr;
  const char *s;

  s=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "No builder name in XML builder description");
    return NULL;
  }

  GWEN_NEW_OBJECT(GWB_GBUILDER_DESCR, descr);
  GWEN_LIST_INIT(GWB_GBUILDER_DESCR, descr);
  descr->builderName=strdup(s);
  descr->xmlDescr=xmlNode;

  _readSettingsFromXml(descr);

  return descr;
}



void _readSettingsFromXml(GWB_GBUILDER_DESCR *descr)
{
  GWEN_XMLNODE *nInputFiles;

  nInputFiles=GWEN_XMLNode_FindFirstTag(descr->xmlDescr, "inputFiles", NULL, NULL);
  if (nInputFiles) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_FindFirstTag(nInputFiles, "acceptedInputTypes", NULL, NULL);
    if (n)
      descr->acceptedInputTypes=GWB_Parser_ReadXmlDataIntoStringList(NULL, n, 1);

    n=GWEN_XMLNode_FindFirstTag(nInputFiles, "acceptedInputFiles", NULL, NULL);
    if (n)
      descr->acceptedInputExt=GWB_Parser_ReadXmlDataIntoStringList(NULL, n, 1);
  }
}



void GWB_GBuilderDescr_free(GWB_GBUILDER_DESCR *descr)
{
  if (descr) {
    GWEN_LIST_FINI(GWB_GBUILDER_DESCR, descr);

    free(descr->builderName);
    GWEN_XMLNode_free(descr->xmlDescr);
    GWEN_StringList_free(descr->acceptedInputTypes);
    GWEN_StringList_free(descr->acceptedInputExt);

    GWEN_FREE_OBJECT(descr);
  }
}



const char *GWB_GBuilderDescr_GetBuilderName(const GWB_GBUILDER_DESCR *descr)
{
  return descr->builderName;
}



GWEN_XMLNODE *GWB_GBuilderDescr_GetXmlDescr(const GWB_GBUILDER_DESCR *descr)
{
  return descr->xmlDescr;
}



GWEN_STRINGLIST *GWB_GBuilderDescr_GetAcceptedInputTypes(const GWB_GBUILDER_DESCR *descr)
{
  return descr->acceptedInputTypes;
}



GWEN_STRINGLIST *GWB_GBuilderDescr_GetAcceptedInputExt(const GWB_GBUILDER_DESCR *descr)
{
  return descr->acceptedInputExt;
}





GWB_GBUILDER_DESCR *_readFromFile(const char *fileName)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlDescr;
  GWB_GBUILDER_DESCR *descr;
  int rv;

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL, "No builder description found in file \"%s\" (%d)", fileName, rv);
    return NULL;
  }

  xmlDescr=GWEN_XMLNode_FindFirstTag(xmlRoot, "GwenBuilder", NULL, NULL);
  if (xmlDescr==NULL) {
    DBG_ERROR(NULL, "No <GwenBuilder> element found in file \"%s\"", fileName);
    GWEN_XMLNode_free(xmlRoot);
    return NULL;
  }
  GWEN_XMLNode_UnlinkChild(xmlRoot, xmlDescr);
  GWEN_XMLNode_free(xmlRoot);

  descr=GWB_GBuilderDescr_new(xmlDescr);
  if (descr==NULL) {
    DBG_ERROR(NULL, "Error in file \"%s\"", fileName);
    GWEN_XMLNode_free(xmlDescr);
    return NULL;
  }

  return descr;
}



GWB_GBUILDER_DESCR_LIST *GWB_GBuilderDescr_ReadAll(const char *folder)
{
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *se;
  GWB_GBUILDER_DESCR_LIST *descrList;
  int rv;

  sl=GWEN_StringList_new();
  rv=GWEN_Directory_GetMatchingFilesRecursively(folder, sl, "*.gwb");
  if (rv<0) {
    DBG_ERROR(NULL, "here (%d)", rv);
    GWEN_StringList_free(sl);
    return NULL;
  }

  descrList=GWB_GBuilderDescr_List_new();
  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      GWB_GBUILDER_DESCR *descr;

      DBG_INFO(NULL, "Reading builder descr file \"%s\"", s);
      descr=_readFromFile(s);
      if (descr==NULL) {
        DBG_ERROR(NULL, "Error reading description from file \"%s\", ignoring", s);
      }
      else
        GWB_GBuilderDescr_List_Add(descr, descrList);
    }

    se=GWEN_StringListEntry_Next(se);
  }
  GWEN_StringList_free(sl);

  return descrList;
}



GWB_GBUILDER_DESCR *GWB_GBuilderDescr_List_GetByName(const GWB_GBUILDER_DESCR_LIST *descrList, const char *name)
{
  GWB_GBUILDER_DESCR *descr;

  descr=GWB_GBuilderDescr_List_First(descrList);
  while(descr) {
    if (descr->builderName && strcasecmp(descr->builderName, name)==0)
      return descr;
    descr=GWB_GBuilderDescr_List_Next(descr);
  }

  return NULL;
}










