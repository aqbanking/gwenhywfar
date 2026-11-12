/***************************************************************************
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWEN_GUI_FOX16_API_H
#define GWEN_GUI_FOX16_API_H


#include <gwenhywfar/types.h>


#if defined __GNUC__ && (! defined (__sun)) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)) || defined _MSC_VER
# ifdef BUILDING_FOX16_GUI
/* building Fox16 */
#  if GWENHYWFAR_SYS_IS_WINDOWS
/* for windows */
#    define FOX16GUI_API __declspec (dllexport)
#  else
/* for non-win32 */
#    ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#      define FOX16GUI_API __attribute__((visibility("default")))
#    else
#      define FOX16GUI_API
#    endif
#  endif
# else
/* not building Fox16 */
#  if GWENHYWFAR_SYS_IS_WINDOWS
/* for windows */
#    define FOX16GUI_API __declspec (dllimport)
#  else
/* for non-win32 */
#    define FOX16GUI_API
#  endif
# endif
#endif



#endif

