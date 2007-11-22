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


#ifndef GWEN_GUI_CGUI_L_H
#define GWEN_GUI_CGUI_L_H


#include "cgui.h"
#include <gwenhywfar/buffer.h>


void GWEN_Gui_CGui_GetRawText(GWEN_GUI *gui,
			      const char *text,
			      GWEN_BUFFER *tbuf);




#endif



