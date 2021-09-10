/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GBUILDER_DESCR_H
#define GWBUILD_GBUILDER_DESCR_H



#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/list1.h>
#include <gwenhywfar/stringlist.h>



typedef struct GWB_GBUILDER_DESCR GWB_GBUILDER_DESCR;
GWEN_LIST_FUNCTION_DEFS(GWB_GBUILDER_DESCR, GWB_GBuilderDescr)


GWB_GBUILDER_DESCR *GWB_GBuilderDescr_new(GWEN_XMLNODE *xmlNode);
void GWB_GBuilderDescr_free(GWB_GBUILDER_DESCR *descr);

const char *GWB_GBuilderDescr_GetBuilderName(const GWB_GBUILDER_DESCR *descr);
GWEN_XMLNODE *GWB_GBuilderDescr_GetXmlDescr(const GWB_GBUILDER_DESCR *descr);

GWEN_STRINGLIST *GWB_GBuilderDescr_GetAcceptedInputTypes(const GWB_GBUILDER_DESCR *descr);
GWEN_STRINGLIST *GWB_GBuilderDescr_GetAcceptedInputExt(const GWB_GBUILDER_DESCR *descr);


GWB_GBUILDER_DESCR_LIST *GWB_GBuilderDescr_ReadAll(const char *folder);
GWB_GBUILDER_DESCR *GWB_GBuilderDescr_List_GetByName(const GWB_GBUILDER_DESCR_LIST *descrList, const char *name);


#endif
