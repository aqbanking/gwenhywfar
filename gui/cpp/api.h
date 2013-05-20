/***************************************************************************
    copyright   : (C) 2012 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWEN_GUI_CPP_API_H
#define GWEN_GUI_CPP_API_H


#include <gwenhywfar/types.h>

#if defined __GNUC__ && (! defined (__sun)) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
# ifdef BUILDING_CPP_GUI
   /* building Cpp */
#  if GWENHYWFAR_SYS_IS_WINDOWS
     /* for windows */
#    ifdef __declspec
#      define CPPGUI_API __declspec (dllexport)
#    else /* if __declspec */
#      define CPPGUI_API
#    endif /* if NOT __declspec */
#  else
     /* for non-win32 */
#    ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#      define CPPGUI_API __attribute__((visibility("default")))
#    else
#      define CPPGUI_API
#    endif
#  endif
# else
   /* not building Cpp */
#  if GWENHYWFAR_SYS_IS_WINDOWS
     /* for windows */
#    ifdef __declspec
#      define CPPGUI_API __declspec (dllimport)
#    else /* if __declspec */
#      define CPPGUI_API
#    endif /* if NOT __declspec */
#  else
     /* for non-win32 */
#    define CPPGUI_API
#  endif
# endif
#endif



#endif

