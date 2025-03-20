/***************************************************************************
    begin       : Wed Mar 22 2023
    copyright   : (C) 2023 by Martin Preuss
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


#ifndef GWEN_TIMESTAMP_H
#define GWEN_TIMESTAMP_H


#include <gwenhywfar/testframework.h>

#include <inttypes.h>
#include <time.h>


typedef struct GWEN_TIMESTAMP GWEN_TIMESTAMP;



GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_new(int year, int month, int day,
                                                  int hour, int minute, int second);
GWENHYWFAR_API void GWEN_Timestamp_free(GWEN_TIMESTAMP *tstamp);

GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_dup(const GWEN_TIMESTAMP *tstamp);

/**
 * Transform the given timestamp into a 64 bit value only to be used by
 * @ref GWEN_Timestamp_fromInt64().
 * No assumptions should be made about how this value is calculated.
 */
GWENHYWFAR_API int64_t GWEN_Timestamp_toInt64(const GWEN_TIMESTAMP *tstamp);
GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_fromInt64(int64_t i);

GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_fromStructTm(const struct tm *ltm);
GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_fromLocalTime(time_t ti);
GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_fromGmTime(time_t ti);

GWENHYWFAR_API time_t GWEN_Timestamp_toTimeT(const GWEN_TIMESTAMP *tstamp);

GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_NowInLocalTime(void);
GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_NowInGmTime(void);

GWENHYWFAR_API const char *GWEN_Timestamp_GetString(const GWEN_TIMESTAMP *tstamp);
GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_fromString(const char *s);

GWENHYWFAR_API int GWEN_Timestamp_toDb(const GWEN_TIMESTAMP *tstamp, GWEN_DB_NODE *db);
GWENHYWFAR_API GWEN_TIMESTAMP *GWEN_Timestamp_fromDb(GWEN_DB_NODE *db);


GWENHYWFAR_API void GWEN_Timestamp_SetDateAndTime(GWEN_TIMESTAMP *tstamp,
                                                  int year, int month, int day,
                                                  int hour, int minute, int second);

GWENHYWFAR_API void GWEN_Timestamp_SetDate(GWEN_TIMESTAMP *tstamp, int year, int month, int day);
GWENHYWFAR_API void GWEN_Timestamp_SetJulianDate(GWEN_TIMESTAMP *tstamp, int julian);
GWENHYWFAR_API void GWEN_Timestamp_SetTime(GWEN_TIMESTAMP *tstamp, int hour, int minute, int second);


GWENHYWFAR_API int GWEN_Timestamp_GetYear(const GWEN_TIMESTAMP *tstamp);

GWENHYWFAR_API int GWEN_Timestamp_GetMonth(const GWEN_TIMESTAMP *tstamp); /* 1=Jan */

GWENHYWFAR_API int GWEN_Timestamp_GetDay(const GWEN_TIMESTAMP *tstamp);

GWENHYWFAR_API int GWEN_Timestamp_GetHour(const GWEN_TIMESTAMP *tstamp);

GWENHYWFAR_API int GWEN_Timestamp_GetMinute(const GWEN_TIMESTAMP *tstamp);

GWENHYWFAR_API int GWEN_Timestamp_GetSecond(const GWEN_TIMESTAMP *tstamp);

GWENHYWFAR_API int GWEN_Timestamp_GetWeekDay(const GWEN_TIMESTAMP *tstamp);


GWENHYWFAR_API void GWEN_Timestamp_AddSeconds(GWEN_TIMESTAMP *tstamp, int seconds);



GWENHYWFAR_API int GWEN_Timestamp_Compare(const GWEN_TIMESTAMP *tstamp1, const GWEN_TIMESTAMP *tstamp0);


GWENHYWFAR_API int GWEN_Timestamp_AddTests(GWEN_TEST_MODULE *mod);


#endif
