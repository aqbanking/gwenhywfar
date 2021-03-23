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


#include "gwenbuild/gwenbuild_p.h"

#include <gwenhywfar/debug.h>




GWENBUILD *GWBUILD_new(void)
{
  GWENBUILD *gwenbuild;

  GWEN_NEW_OBJECT(GWENBUILD, gwenbuild);
  return gwenbuild;
}



void GWBUILD_free(GWENBUILD *gwenbuild)
{
  if (gwenbuild) {
    GWEN_FREE_OBJECT(gwenbuild);
  }
}





GWBUILD_TARGETTYPE GWBUILD_TargetType_fromString(const char *s)
{
  if (s && *s) {
    if (strcasecmp(s, "InstallLibrary")==0)
      return GWBUILD_TargetType_InstallLibrary;
    else if (strcasecmp(s, "ConvenienceLibrary")==0)
      return GWBUILD_TargetType_ConvenienceLibrary;
    else if (strcasecmp(s, "Program")==0)
      return GWBUILD_TargetType_Program;
    else if (strcasecmp(s, "Objects")==0)
      return GWBUILD_TargetType_Objects;
    else {
      DBG_ERROR(NULL, "Invalid target type \"%s\"", s);
    }
  }
  else {
    DBG_ERROR(NULL, "Empty target type");
  }

  return GWBUILD_TargetType_Invalid;
}



void GWBUILD_Debug_PrintValue(const char *sName, const char *sValue, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s = %s\n", sName, sValue?sValue:"<empty>");
}



void GWBUILD_Debug_PrintIntValue(const char *sName, int value, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s = %d\n", sName, value);
}



void GWBUILD_Debug_PrintKvpList(const char *sName, const GWB_KEYVALUEPAIR_LIST *kvpList, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (kvpList) {
    const GWB_KEYVALUEPAIR *kvp;

    kvp=GWB_KeyValuePair_List_First(kvpList);
    while(kvp) {
      const char *sKey;
      const char *sValue;

      sKey=GWB_KeyValuePair_GetKey(kvp);
      sValue=GWB_KeyValuePair_GetValue(kvp);
      GWBUILD_Debug_PrintValue(sKey, sValue, indent+2);
      kvp=GWB_KeyValuePair_List_Next(kvp);
    }
  }
}



void GWBUILD_Debug_PrintDb(const char *sName, GWEN_DB_NODE *db, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (db)
    GWEN_DB_Dump(db, indent+2);
}



void GWBUILD_Debug_PrintFile(const char *sName, const GWB_FILE *file, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");

  if (sName)
    fprintf(stderr, "%s = ", sName);

  if (file) {
    const char *sFolder;
    const char *sName;
    const char *sInstallPath;
    const char *sFileType;
    uint32_t flags;
  
    sFolder=GWB_File_GetFolder(file);
    sName=GWB_File_GetName(file);
    flags=GWB_File_GetFlags(file);
    sFileType=GWB_File_GetFileType(file);
    sInstallPath=GWB_File_GetInstallPath(file);

    if (sFolder && *sFolder)
      fprintf(stderr, "%s/", sFolder);
    fprintf(stderr, "%s", sName?sName:"<no name>");
    fprintf(stderr, " (%s)", sFileType?sFileType:"no type");

    if (flags & GWB_FILE_FLAGS_DIST)
      fprintf(stderr, " DIST");
    if (flags & GWB_FILE_FLAGS_INSTALL)
      fprintf(stderr, " INSTALL");
    fprintf(stderr, " %s", sInstallPath?sInstallPath:"<no install path>");

    fprintf(stderr, "\n");
  }
  else
    fprintf(stderr, "<empty>\n");
}



void GWBUILD_Debug_PrintFileList2(const char *sName, const GWB_FILE_LIST2 *fileList2, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (fileList2) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList2);
    if (it) {
      GWB_FILE *file;

      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        GWBUILD_Debug_PrintFile(NULL, file, indent+2);
        file=GWB_File_List2Iterator_Next(it);
      }
      GWB_File_List2Iterator_free(it);
    }
  }
}



void GWBUILD_Debug_PrintTargetList2(const char *sName, const GWB_TARGET_LIST2 *targetList2, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s:\n", sName);

  if (targetList2) {
    GWB_TARGET_LIST2_ITERATOR *it;

    it=GWB_Target_List2_First(targetList2);
    if (it) {
      GWB_TARGET *target;

      target=GWB_Target_List2Iterator_Data(it);
      while(target) {
        GWB_Target_Dump(target, indent+2);
        target=GWB_Target_List2Iterator_Next(it);
      }
      GWB_Target_List2Iterator_free(it);
    }
  }
}



void GWBUILD_Debug_PrintStringList(const char *sName, const GWEN_STRINGLIST *sl, int indent)
{
  if (sl) {
    int i;
    const GWEN_STRINGLISTENTRY *se;

    for(i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "%s:\n", sName);

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      for(i=0; i<indent+2; i++)
        fprintf(stderr, " ");
      fprintf(stderr, "%s\n", (s && *s)?s:"<empty>");

      se=GWEN_StringListEntry_Next(se);
    }
  }
}





