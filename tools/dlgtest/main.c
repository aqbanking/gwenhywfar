/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Jun 02 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include <stdio.h>

#include <gwenhywfar/ui/loader.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>



int main(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nn;
  GWEN_DB_NODE *dbData;
  int res;

  GWEN_Logger_Open(0, "dlgtest", "dlgtest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelWarning);

  if (argc<3) {
    fprintf(stderr, "Usage: %s FILENAME DIALOGNAME.\n", argv[0]);
    return 1;
  }
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  if (GWEN_XML_ReadFile(n, argv[1], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 2;
  }

  nn=GWEN_XMLNode_FindFirstTag(n, "widget", "name", argv[2]);
  if (!nn) {
    DBG_ERROR(0, "Dialog \"%s\" not found", argv[2]);
    return 2;
  }

  dbData=GWEN_DB_Group_new("dialogData");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  res=GWEN_UILoader_ExecDialog(0, nn, dbData);

  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  if (res==1) {
    fprintf(stdout, "Dialog data:\n");
    GWEN_DB_Dump(dbData, stdout, 2);
  }

  fprintf(stdout, "Result of dialog was: ");
  switch(res) {
  case 1: fprintf(stdout, "Accepted.\n"); break;
  case 2: fprintf(stdout, "Aborted.\n"); break;
  case -1: fprintf(stdout, "Error.\n"); break;
  default: fprintf(stdout, "Unknown.\n"); break;
  }

  return 0;
}






