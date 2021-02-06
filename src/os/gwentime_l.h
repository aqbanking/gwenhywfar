/***************************************************************************
    begin       : Wed Mar 24 2004
    copyright   : (C) 2004 by Martin Preuss
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


#ifndef GWEN_TIME_L_H
#define GWEN_TIME_L_H


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/misc.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function is called by OS dependant implementations of
 * @ref GWEN_Time__GetCurrentTime.
 */
void GWEN_Time__SetSecsAndMSecs(GWEN_TIME *ti,
                                uint32_t secs,
                                uint32_t msecs);

/** @name Functions to be implemented by OS specific modules
 *
 */
/*@{*/
int GWEN_Time__GetCurrentTime(GWEN_TIME *ti);

/*@}*/


#ifdef __cplusplus
}
#endif



#endif

