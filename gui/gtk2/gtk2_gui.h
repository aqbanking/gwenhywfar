/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GTK2_GUI_H
#define GTK2_GUI_H


#include <gtk/gtk.h>


#if defined __GNUC__ && (! defined (__sun)) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
# ifdef BUILDING_GTK2_GUI
#   define GTK2GUI_API __attribute__ ((visibility("default")))
# else
#   define GTK2GUI_API
# endif
#else
# define GTK2GUI_API
#endif



#endif


