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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gwentime_p.h"
#include "../gwentime_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <time.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>



/* The idea of this function is taken from a posting by Anders Carlsson on the mailing list
 * bug-gnu-chess (http://mail.gnu.org/archive/html/bug-gnu-chess/2004-01/msg00020.html)
*/
int GWEN_Time__GetCurrentTime(GWEN_TIME *ti)
{
  long sec, msec;
  union {
    uint64_t ns100; /* time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  } current_date;

  GetSystemTimeAsFileTime(&(current_date.ft));

  msec=(long)((current_date.ns100 / 10000LL) % 1000LL);
  sec=(long)((current_date.ns100-(116444736000000000LL))/10000000LL);
  GWEN_Time__SetSecsAndMSecs(ti, sec, msec);
  return 0;
}
