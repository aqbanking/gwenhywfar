/***************************************************************************
 begin       : August 03 2010
 copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef COCOA_GUI_H
#define COCOA_GUI_H



#if (defined __GNUC__ && (! defined (__sun)) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))) || defined _MSC_VER
# ifdef BUILDING_COCOA_GUI
#   define COCOAGUI_API __attribute__ ((visibility("default")))
# else
#   define COCOAGUI_API
# endif
#else
# define COCOAGUI_API
#endif


#include <gwenhywfar/gui.h>
#include <gwenhywfar/gui_be.h>


#ifdef __cplusplus
extern "C" {
#endif


COCOAGUI_API GWEN_GUI *Cocoa_Gui_new();


#ifdef __cplusplus
}
#endif



#endif

