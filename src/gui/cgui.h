/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2017 by Martin Preuss
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

/** @defgroup MOD_GUI_CGUI GUI Implementation for the Console
 * @ingroup MOD_GUI
 *
 * This is an implementation of GWEN_GUI for the console. It supports
 * interactive and non-interactive mode, provides a cache for passwords/pins
 * and TLS certificates. It converts UTF8 messages into other character sets
 * if requested (e.g. for consoles which do not support UTF8).
 */
/*@{*/

/** Constructor
 *
 */
GWENHYWFAR_API
GWEN_GUI *GWEN_Gui_CGui_new(void);

/*@}*/


/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif



