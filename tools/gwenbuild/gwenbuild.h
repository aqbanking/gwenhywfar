/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GWENBUILD_H
#define GWBUILD_GWENBUILD_H


/** main object of gwenbuild */
typedef struct GWENBUILD GWENBUILD;


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
#include "gwenbuild/target.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>



GWENBUILD *GWBUILD_new(void);
void GWBUILD_free(GWENBUILD *gwenbuild);


GWBUILD_TARGETTYPE GWBUILD_TargetType_fromString(const char *s);

void GWBUILD_Debug_PrintValue(const char *sName, const char *sValue, int indent);
void GWBUILD_Debug_PrintIntValue(const char *sName, int value, int indent);
void GWBUILD_Debug_PrintKvpList(const char *sName, const GWB_KEYVALUEPAIR_LIST *kvpList, int indent);
void GWBUILD_Debug_PrintDb(const char *sName, GWEN_DB_NODE *db, int indent);
void GWBUILD_Debug_PrintFileList2(const char *sName, const GWB_FILE_LIST2 *fileList2, int indent);
void GWBUILD_Debug_PrintTargetList2(const char *sName, const GWB_TARGET_LIST2 *targetList2, int indent);
void GWBUILD_Debug_PrintStringList(const char *sName, const GWEN_STRINGLIST *sl, int indent);
void GWBUILD_Debug_PrintFile(const char *sName, const GWB_FILE *file, int indent);

#endif
