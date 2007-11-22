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


#ifndef GWEN_GUI_CGUI_H
#define GWEN_GUI_CGUI_H

#include <gwenhywfar/gui.h>


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API 
GWEN_GUI *GWEN_Gui_CGui_new();


GWENHYWFAR_API 
const char *GWEN_Gui_CGui_GetCharSet(const GWEN_GUI *gui);

GWENHYWFAR_API 
void GWEN_Gui_CGui_SetCharSet(GWEN_GUI *gui, const char *s);


GWENHYWFAR_API 
int GWEN_Gui_CGui_GetIsNonInteractive(const GWEN_GUI *gui);

GWENHYWFAR_API 
void GWEN_Gui_CGui_SetIsNonInteractive(GWEN_GUI *gui, int i);


#ifdef __cplusplus
}
#endif


#endif



