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
#include "gwenbuild/project.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>



GWENBUILD *GWBUILD_new(void);
void GWBUILD_free(GWENBUILD *gwenbuild);


int GWBUILD_MakeBuildersForTargets(GWB_PROJECT *project);


const char *GWBUILD_GetToolNameCC(const GWENBUILD *gwenbuild);
void GWBUILD_SetToolNameCC(GWENBUILD *gwenbuild, const char *s);

const char *GWBUILD_GetToolNameCXX(const GWENBUILD *gwenbuild);
void GWBUILD_SetToolNameCXX(GWENBUILD *gwenbuild, const char *s);

const char *GWBUILD_GetToolNameLD(const GWENBUILD *gwenbuild);
void GWBUILD_SetToolNameLD(GWENBUILD *gwenbuild, const char *s);

const char *GWBUILD_GetToolNameAR(const GWENBUILD *gwenbuild);
void GWBUILD_SetToolNameAR(GWENBUILD *gwenbuild, const char *s);

const char *GWBUILD_GetToolNameRANLIB(const GWENBUILD *gwenbuild);
void GWBUILD_SetToolNameRANLIB(GWENBUILD *gwenbuild, const char *s);




GWBUILD_TARGETTYPE GWBUILD_TargetType_fromString(const char *s);
const char *GWBUILD_TargetType_toString(GWBUILD_TARGETTYPE tt);

void GWBUILD_Debug_PrintValue(const char *sName, const char *sValue, int indent);
void GWBUILD_Debug_PrintIntValue(const char *sName, int value, int indent);
void GWBUILD_Debug_PrintKvpList(const char *sName, const GWB_KEYVALUEPAIR_LIST *kvpList, int indent);
void GWBUILD_Debug_PrintDb(const char *sName, GWEN_DB_NODE *db, int indent);
void GWBUILD_Debug_PrintFileList2(const char *sName, const GWB_FILE_LIST2 *fileList2, int indent);
void GWBUILD_Debug_PrintTargetList2(const char *sName, const GWB_TARGET_LIST2 *targetList2, int indent);
void GWBUILD_Debug_PrintStringList(const char *sName, const GWEN_STRINGLIST *sl, int indent);
void GWBUILD_Debug_PrintFile(const char *sName, const GWB_FILE *file, int indent);

#endif
