/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GWENBUILD_H
#define GWBUILD_GWENBUILD_H



typedef enum {
  GWBUILD_TargetType_Invalid=-1,
  GWBUILD_TargetType_None=0,
  GWBUILD_TargetType_InstallLibrary,
  GWBUILD_TargetType_ConvenienceLibrary,
  GWBUILD_TargetType_Program,
  GWBUILD_TargetType_Objects,
} GWBUILD_TARGETTYPE;



#include "gwenbuild/keyvaluepair.h"
#include "gwenbuild/file.h"

#include <gwenhywfar/db.h>



void GWBUILD_Debug_PrintValue(const char *sName, const char *sValue, int indent);
void GWBUILD_Debug_PrintKvpList(const char *sName, const GWB_KEYVALUEPAIR_LIST *kvpList, int indent);
void GWBUILD_Debug_PrintDb(const char *sName, GWEN_DB_NODE *db, int indent);
void GWBUILD_Debug_PrintFileList2(const char *sName, const GWB_FILE_LIST2 *fileList2, int indent);


#endif
