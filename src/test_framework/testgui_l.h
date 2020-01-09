/***************************************************************************
 begin       : Thu Jan 09 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_TEST_GUI_L_H
#define GWEN_TEST_GUI_L_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/gui.h>



void TestGui_Extend(GWEN_GUI *gui);
void TestGui_Unextend(GWEN_GUI *gui);



void TestGui_SetLogFile(GWEN_GUI *gui, const char *fname);
void TestGui_SetLogLastX(GWEN_GUI *gui, int i);

void TestGui_AddLogLine(GWEN_GUI *gui, const char *mark);
void Test_Gui_FlushLogs(GWEN_GUI *gui);



#endif
