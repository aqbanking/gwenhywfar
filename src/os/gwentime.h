/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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


#ifndef GWEN_TIME_H
#define GWEN_TIME_H


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>

#ifdef __cplusplus
extern "C" {
#endif

GWENHYWFAR_API typedef struct GWEN_TIME GWEN_TIME;



GWENHYWFAR_API GWEN_TIME *GWEN_CurrentTime();

/**
 * Parses the data and time from the given string according to the template
 * string (quite similar to sscanf).
 * The following characters are accepted in the template string:
 * <table border="1">
 *  <tr><td>Character</td><td>Meaning</td></tr>
 *  <tr><td>Y</td><td>digit of the year</td></tr>
 *  <tr><td>M</td><td>digit of the month</td></tr>
 *  <tr><td>D</td><td>digit of the day of month</td></tr>
 *  <tr><td>h</td><td>digit of the hour</td></tr>
 *  <tr><td>m</td><td>digit of the minute</td></tr>
 *  <tr><td>s</td><td>digit of the second</td></tr>
 * </table>
 * All other characters are ignored. <br>
 * Some examples of valid patterns follow:
 * <ul>
 *  <li>"YYYYMMDD"</li>
 *  <li>"YYMMDD"</li>
 *  <li>"YY/MM/DD"</li>
 *  <li>"YYYYMMDD hh:mm:ss"</li>
 *  <li>"YYYYMMDD hh:mm"</li>
 *  <li>"YYYYMMDD hhmmss"</li>
 *  <li>et cetera</li>
 * </ul>
 * @return 0 on error, a GWEN_TIME pointer otherwise
 * @param s string containing the date/time
 * @param tmpl template string
 */
GWENHYWFAR_API GWEN_TIME *GWEN_Time_fromString(const char *s,
                                               const char *tmpl);
/**
 * Creates a GWEN_TIME object from the return value of @ref GWEN_Time_Seconds.
 */
GWENHYWFAR_API GWEN_TIME *GWEN_Time_fromSeconds(GWEN_TYPE_UINT32 s);
GWENHYWFAR_API void GWEN_Time_free(GWEN_TIME *t);
GWENHYWFAR_API GWEN_TIME *GWEN_Time_dup(const GWEN_TIME *t);

/**
 * Returns the time in seconds since the epoch (00:00:00 UTC Jan 1, 1970).
 */
GWENHYWFAR_API GWEN_TYPE_UINT32 GWEN_Time_Seconds(GWEN_TIME *t);

/** returns the time in milliseconds */
GWENHYWFAR_API double GWEN_Time_Milliseconds(GWEN_TIME *t);

/**
 * Returns the difference between t1 and t2 in milliseconds
 */
GWENHYWFAR_API double GWEN_Time_Diff(GWEN_TIME *t1, GWEN_TIME *t0);

GWENHYWFAR_API int GWEN_Time_GetBrokenDownTime(const GWEN_TIME *t,
                                               int *hours,
                                               int *mins,
                                               int *secs);
GWENHYWFAR_API int GWEN_Time_GetBrokenDownDate(const GWEN_TIME *t,
                                               int *days,
                                               int *month,
                                               int *year);


#ifdef __cplusplus
}
#endif



#endif

