/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: cbanking_p.h 409 2005-08-05 18:13:06Z aquamaniac $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_CGUI_P_H
#define GWEN_GUI_CGUI_P_H



#define GWEN_GUI_CGUI_CHAR_ABORT  27
#define GWEN_GUI_CGUI_CHAR_DELETE 127
#define GWEN_GUI_CGUI_CHAR_ENTER  10

#include "cgui_l.h"
#include "cprogress_l.h"


typedef struct GWEN_GUI_CGUI GWEN_GUI_CGUI;
struct GWEN_GUI_CGUI {
  GWEN_GUI_CPROGRESS_LIST *progressList;

  uint32_t nextBoxId;
  uint32_t nextProgressId;
  char *charSet;
  int nonInteractive;

};




void GWENHYWFAR_CB GWEN_Gui_CGui_FreeData(void *bp, void *p);



int GWEN_Gui_CGui__ConvertFromUtf8(GWEN_GUI *gui,
				   const char *text,
				   int len,
				   GWEN_BUFFER *tbuf);


char GWEN_Gui_CGui__readCharFromStdin(int waitFor);
int GWEN_Gui_CGui__input(GWEN_GUI *gui,
			 uint32_t flags,
			 char *buffer,
			 int minLen,
			 int maxLen,
			 uint32_t guiid);

GWEN_GUI_CPROGRESS *GWEN_Gui_CGui__findProgress(GWEN_GUI *gui, uint32_t id);


int GWEN_Gui_CGui_MessageBox(GWEN_GUI *gui,
			     uint32_t flags,
			     const char *title,
			     const char *text,
			     const char *b1,
			     const char *b2,
			     const char *b3,
			     uint32_t guiid);

int GWEN_Gui_CGui_InputBox(GWEN_GUI *gui,
			   uint32_t flags,
			   const char *title,
			   const char *text,
			   char *buffer,
			   int minLen,
			   int maxLen,
			   uint32_t guiid);

uint32_t GWEN_Gui_CGui_ShowBox(GWEN_GUI *gui,
			       uint32_t flags,
			       const char *title,
			       const char *text,
			       uint32_t guiid);

void GWEN_Gui_CGui_HideBox(GWEN_GUI *gui, uint32_t id);

uint32_t GWEN_Gui_CGui_ProgressStart(GWEN_GUI *gui,
				     uint32_t progressFlags,
				     const char *title,
				     const char *text,
				     uint64_t total,
				     uint32_t guiid);

int GWEN_Gui_CGui_ProgressAdvance(GWEN_GUI *gui,
				  uint32_t id,
				  uint64_t progress);

int GWEN_Gui_CGui_ProgressLog(GWEN_GUI *gui,
			      uint32_t id,
			      GWEN_LOGGER_LEVEL level,
			      const char *text);

int GWEN_Gui_CGui_ProgressEnd(GWEN_GUI *gui, uint32_t id);

int GWEN_Gui_CGui_Print(GWEN_GUI *gui,
			const char *docTitle,
			const char *docType,
			const char *descr,
			const char *text,
			uint32_t guiid);

#endif


