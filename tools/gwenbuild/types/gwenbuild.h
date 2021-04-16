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
  GWBUILD_TargetType_CxxProgram,
  GWBUILD_TargetType_Objects,
  GWBUILD_TargetType_Module,
} GWBUILD_TARGETTYPE;



#include "gwenbuild/types/keyvaluepair.h"
#include "gwenbuild/types/file.h"
#include "gwenbuild/types/target.h"
#include "gwenbuild/types/project.h"
#include "gwenbuild/buildctx/buildctx.h"
#include "gwenbuild/types/option.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/buffer.h>

#include <time.h>



GWENBUILD *GWBUILD_new(void);
void GWBUILD_free(GWENBUILD *gwenbuild);


int GWBUILD_MakeBuildersForTargets(GWB_PROJECT *project);
GWB_BUILD_CONTEXT *GWBUILD_MakeBuildCommands(GWB_PROJECT *project);


GWEN_STRINGLIST *GWBUILD_GetBuildFilenameList(const GWENBUILD *gwenbuild);
void GWBUILD_AddBuildFilename(GWENBUILD *gwenbuild, const char *s);


const char *GWBUILD_GetHostArch(void);
const char *GWBUILD_GetHostSystem(void);

const char *GWBUILD_GetArchFromTriplet(const char *sTriplet);
const char *GWBUILD_GetSystemFromTriplet(const char *sTriplet);


GWEN_STRINGLIST *GWBUILD_GetPathFromEnvironment(void);


void GWBUILD_AddFilesFromStringList(GWB_FILE_LIST2 *mainFileList,
                                    const char *sFolder,
                                    const GWEN_STRINGLIST *fileNameList,
                                    GWB_FILE_LIST2 *outFileList,
                                    uint32_t flagsToAdd,
                                    int copyFileForOutList);


GWBUILD_TARGETTYPE GWBUILD_TargetType_fromString(const char *s);
const char *GWBUILD_TargetType_toString(GWBUILD_TARGETTYPE tt);

time_t GWBUILD_GetModificationTimeOfFile(const char *filename);


void GWBUILD_Debug_PrintValue(const char *sName, const char *sValue, int indent);
void GWBUILD_Debug_PrintIntValue(const char *sName, int value, int indent);
void GWBUILD_Debug_PrintKvpList(const char *sName, const GWB_KEYVALUEPAIR_LIST *kvpList, int indent);
void GWBUILD_Debug_PrintDb(const char *sName, GWEN_DB_NODE *db, int indent);
void GWBUILD_Debug_PrintFileList2(const char *sName, const GWB_FILE_LIST2 *fileList2, int indent);
void GWBUILD_Debug_PrintTargetList2(const char *sName, const GWB_TARGET_LIST2 *targetList2, int indent, int fullDump);
void GWBUILD_Debug_PrintBuilderList2(const char *sName, const GWB_BUILDER_LIST2 *builderList2, int indent, int fullDump);
void GWBUILD_Debug_PrintBuildCmdList2(const char *sName, const GWB_BUILD_CMD_LIST2 *buildCmdList2, int indent);
void GWBUILD_Debug_PrintStringList(const char *sName, const GWEN_STRINGLIST *sl, int indent);
void GWBUILD_Debug_PrintFile(const char *sName, const GWB_FILE *file, int indent);
void GWBUILD_Debug_PrintOptionList(const char *sName, const GWB_OPTION_LIST *optionList, int indent);



#endif
