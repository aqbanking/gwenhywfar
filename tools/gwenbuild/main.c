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

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/debug.h>





int test_ReadProject(int argc, char **argv)
{
  GWENBUILD *gwenbuild;
  GWB_PROJECT *project;
  const char *folder;

  gwenbuild=GWBUILD_new();

  if (argc<2) {
    DBG_ERROR(NULL, "Folder needed");
    return 1;
  }

  folder=argv[1];
  project=GWB_Parser_ReadBuildTree(gwenbuild, folder);
  if (project==NULL) {
    DBG_ERROR(NULL, "Error reading build files.");
    return 2;
  }

  DBG_ERROR(NULL, "Tree successfully loaded.");
  GWB_Project_Dump(project, 2);
  return 0;
}






int main(int argc, char **argv)
{
  int rv;
  GWEN_GUI *gui;

  rv=GWEN_Init();
  if (rv<0) {
    DBG_ERROR(NULL, "Error on GWEN_Init (%d)", rv);
    return 2;
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  GWEN_Logger_Open(0, "gwenbuild", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);

  GWEN_Logger_SetLevel(NULL, GWEN_LoggerLevel_Info);

  rv=test_ReadProject(argc, argv);
  if (rv) {
    DBG_ERROR(NULL, "here (%d)", rv);
    return 3;
  }


  return 0;
}

