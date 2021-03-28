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

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif




#ifdef HAVE_SIGNAL_H

struct sigaction sigActionChild;


void _signalHandler(int s) {
  switch(s) {
  case SIGCHLD:
    //fprintf(stderr, "Child exited %d\n", s);
    break;
  default:
    fprintf(stderr, "Received unhandled signal %d\n", s);
    break;
  }
  signal(s, _signalHandler);
}



int _setSingleSignalHandler(struct sigaction *sa, int sig)
{
  sa->sa_handler=_signalHandler;
  sigemptyset(&sa->sa_mask);
  sa->sa_flags=0;
  if (sigaction(sig, sa, 0)) {
    DBG_ERROR(NULL, "Could not setup signal handler for signal %d", sig);
    return GWEN_ERROR_GENERIC;
  }
  return 0;
}



int _setSignalHandlers() {
  int rv;

  rv=_setSingleSignalHandler(&sigActionChild, SIGCHLD);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}

#endif




int test_ReadProject(int argc, char **argv)
{
  GWENBUILD *gwenbuild;
  GWB_PROJECT *project;
  const char *folder;
  int rv;

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

  //DBG_ERROR(NULL, "Tree successfully loaded.");
  //GWB_Project_Dump(project, 2);

  rv=GWBUILD_MakeBuildersForTargets(project);
  if (rv<0) {
    DBG_ERROR(NULL, "Error makeing builders for targets.");
    return 2;
  }

  DBG_ERROR(NULL, "Targets successfully created.");
  GWB_Project_Dump(project, 2, 0);
  return 0;
}






int main(int argc, char **argv)
{
  int rv;
  GWEN_GUI *gui;

#ifdef HAVE_SIGNAL_H
  signal(SIGCHLD, _signalHandler);
  //_setSignalHandlers();
#endif

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

