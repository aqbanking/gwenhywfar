/***************************************************************************
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GWENHYWFAR_GUI_NOGUI_H
#define GWENHYWFAR_GUI_NOGUI_H


#include <gwenhywfar/gui.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Create a GUI which does nothing at all, no interaction whatsoever.
 * Every functions just returns "0" (meaning all fine) without doing anything.
 *
 * You can use this GUI in side-threads which are not allowed to call real GUI functions but want to
 * use functions which might call GWEN_GUI functions.
 *
 * Due to its simplicity it can also serve as a template for your own GUI implementation.
 */
GWENHYWFAR_API GWEN_GUI *GWEN_NoGui_new(void);



#ifdef __cplusplus
}
#endif


#endif
