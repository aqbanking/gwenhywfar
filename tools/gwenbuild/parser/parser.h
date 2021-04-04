/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_PARSER_H
#define GWBUILD_PARSER_H

#include "gwenbuild/context.h"
#include "gwenbuild/project.h"

#include <gwenhywfar/xml.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>


#define GWB_PARSER_FILENAME "0BUILD"



GWB_PROJECT *GWB_Parser_ReadBuildTree(GWENBUILD *gwbuild, const char *srcDir, GWB_KEYVALUEPAIR_LIST *givenOptionList);




GWB_CONTEXT *GWB_Parser_CopyContextForSubdir(const GWB_CONTEXT *sourceContext, const char *folder);
GWB_CONTEXT *GWB_Parser_CopyContextForTarget(const GWB_CONTEXT *sourceContext);

GWEN_XMLNODE *GWB_Parser_ReadBuildFile(const GWB_CONTEXT *currentContext, const char *fileName);
GWEN_STRINGLIST *GWB_Parser_ReadXmlDataIntoStringList(const GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
GWEN_BUFFER *GWB_Parser_ReadXmlDataIntoBufferAndExpand(const GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);

typedef int (*GWB_PARSER_PARSE_ELEMENT_FN)(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
int GWB_Parser_ParseWellKnownElements(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *n, GWB_PARSER_PARSE_ELEMENT_FN fn);

int GWB_Parser_ParseSubdirs(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn);
//int GWB_Parser_ParseSetVar(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
int GWB_Parser_ParseWriteFile(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);

void GWB_Parser_SetItemValue(GWEN_DB_NODE *db, const char *sId, const char *suffix, const char *value);

int GWB_Parser_ReplaceVarsBetweenAtSigns(const char *s, GWEN_BUFFER *dbuf, GWEN_DB_NODE *db);

#endif

