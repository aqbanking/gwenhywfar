/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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

#include "error_p.h"

/* Internationalization */
#ifdef ENABLE_NLS
# include <libintl.h>
# include <locale.h>
# define I18N(m) dgettext("gwenhywfar", m)
# define I18S(m) m
#else
# define I18N(m) m
# define I18S(m) m
#endif


#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include "gwenhywfar/debug.h"
#include "gwenhywfar/misc.h"

#include "gwenhywfar/inetsocket.h"
#include "gwenhywfar/inetaddr.h"
#include "gwenhywfar/libloader.h"



int GWEN_Error_ModuleInit() {
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Error_ModuleInit");
  return 0;
}



int GWEN_Error_ModuleFini() {
  return 0;
}



int GWEN_Error_ToString(int c, char *buffer, int bsize) {
  const char *s;

  assert(buffer);
  assert(bsize);
  s=GWEN_Error_SimpleToString(c);
  snprintf(buffer, bsize-1, "Error %d [%s]", c, s?s:"none");
  /* make sure the string is NULL terminated */
  buffer[bsize-1]=0;

  /* finished */
  return 1;
}



const char *GWEN_Error_SimpleToString(int i){
  const char *s;

  switch(i) {
  case GWEN_SUCCESS:
    s=I18N("Ok"); break;
  case GWEN_ERROR_GENERIC:
    s=I18N("Generic error"); break;
  case GWEN_ERROR_ABORTED:
    s=I18N("Aborted"); break;
  case GWEN_ERROR_NOT_AVAILABLE:
    s=I18N("Not available"); break;
  case GWEN_ERROR_BAD_SOCKETTYPE:
    s=I18N("Bad socket type"); break;
  case GWEN_ERROR_NOT_OPEN:
    s=I18N("Not open"); break;
  case GWEN_ERROR_TIMEOUT:
    s=I18N("Timeout"); break;
  case GWEN_ERROR_IN_PROGRESS:
    s=I18N("In Progress"); break;
  case GWEN_ERROR_STARTUP:
    s=I18N("Startup error"); break;
  case GWEN_ERROR_INTERRUPTED:
    s=I18N("Interrupted system call"); break;
  case GWEN_ERROR_NOT_SUPPORTED:
    s=I18N("Not supported"); break;
  case GWEN_ERROR_BROKEN_PIPE:
    s=I18N("Broken pipe"); break;
  case GWEN_ERROR_MEMORY_FULL:
    s=I18N("Memory full"); break;
  case GWEN_ERROR_BAD_ADDRESS:
    s=I18N("Bad address"); break;
  case GWEN_ERROR_BUFFER_OVERFLOW:
    s=I18N("Buffer overflow"); break;
  case GWEN_ERROR_HOST_NOT_FOUND:
    s=I18N("Host not found"); break;
  case GWEN_ERROR_NO_ADDRESS:
    s=I18N("No address"); break;
  case GWEN_ERROR_NO_RECOVERY:
    s=I18N("No recovery"); break;
  case GWEN_ERROR_TRY_AGAIN:
    s=I18N("Try again"); break;
  case GWEN_ERROR_UNKNOWN_DNS_ERROR:
    s=I18N("Unknown DNS error"); break;
  case GWEN_ERROR_BAD_ADDRESS_FAMILY:
    s=I18N("Bad address family"); break;
  case GWEN_ERROR_COULD_NOT_LOAD:
    s=I18N("Could not load"); break;
  case GWEN_ERROR_COULD_NOT_RESOLVE:
    s=I18N("Could not resolve a symbol"); break;
  case GWEN_ERROR_NOT_FOUND:
    s=I18N("Not found"); break;
  case GWEN_ERROR_READ:
    s=I18N("Could not read"); break;
  case GWEN_ERROR_WRITE:
    s=I18N("Could not write"); break;
  case GWEN_ERROR_CLOSE:
    s=I18N("Could not close"); break;
  case GWEN_ERROR_NO_DATA:
    s=I18N("No data"); break;
  case GWEN_ERROR_PARTIAL:
    s=I18N("Partial data"); break;
  case GWEN_ERROR_EOF:
    s=I18N("EOF met"); break;
  case GWEN_ERROR_ALREADY_REGISTERED:
    s=I18N("Already registered"); break;
  case GWEN_ERROR_NOT_REGISTERED:
    s=I18N("Not registered"); break;
  case GWEN_ERROR_BAD_SIZE:
    s=I18N("Bad size"); break;
  case GWEN_ERROR_ENCRYPT:
    s=I18N("Could not encrypt"); break;
  case GWEN_ERROR_DECRYPT:
    s=I18N("Could not decrypt"); break;
  case GWEN_ERROR_SIGN:
    s=I18N("Could not sign"); break;
  case GWEN_ERROR_VERIFY:
    s=I18N("Could not verify"); break;
  case GWEN_ERROR_SSL:
    s=I18N("Generic SSL error"); break;
  default:
    s="Unknown error";
  }

  return s;
}












