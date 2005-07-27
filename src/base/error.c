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
#include "error_p.h"
#include "gwenhywfar/debug.h"
#include "gwenhywfar/misc.h"

#include "gwenhywfar/inetsocket.h"
#include "gwenhywfar/inetaddr.h"
#include "gwenhywfar/bufferedio.h"
#include "gwenhywfar/crypt.h"
#include "gwenhywfar/libloader.h"


/* forward declaration */
const char *GWEN_Error_ErrorString(int c);


static int gwen_error_is_initialized=0;
static int gwen_error_nexttype=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_error_errorformlist=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_error_errorform=0;


GWEN_ERRORTYPEREGISTRATIONFORM *GWEN_ErrorType_new(){
  GWEN_ERRORTYPEREGISTRATIONFORM *f;

  GWEN_NEW_OBJECT(GWEN_ERRORTYPEREGISTRATIONFORM, f);
  GWEN_ErrorType_SetType(f, gwen_error_nexttype++);
  return f;
}



void GWEN_ErrorType_free(GWEN_ERRORTYPEREGISTRATIONFORM *f){
  if (f)
    free(f);
}



void GWEN_ErrorType_SetType(GWEN_ERRORTYPEREGISTRATIONFORM *f, int c){
  assert(f);
  f->typ=c;
}



const char *GWEN_ErrorType_GetName(GWEN_ERRORTYPEREGISTRATIONFORM *f) {
  assert(f);
  return f->name;
}



void GWEN_ErrorType_SetName(GWEN_ERRORTYPEREGISTRATIONFORM *f,
                            const char *name){
  assert(f);
  assert(name);
  assert((strlen(name)+1)<sizeof(f->name));
  strcpy(f->name, name);
}



void GWEN_ErrorType_SetMsgPtr(GWEN_ERRORTYPEREGISTRATIONFORM *f,
                              GWEN_ERRORMESSAGEPTR msgptr){
  assert(f);
  assert(msgptr);
  f->msgptr=msgptr;
}



GWEN_ERRORMESSAGEPTR
GWEN_ErrorType_GetMsgPtr(GWEN_ERRORTYPEREGISTRATIONFORM *f){
  assert(f);
  return f->msgptr;
}



GWEN_ERRORCODE GWEN_Error_ModuleInit() {

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Error_ModuleInit");
  if (!gwen_error_is_initialized) {
    GWEN_ERRORCODE err;

    gwen_error_errorform=GWEN_ErrorType_new();
    GWEN_ErrorType_SetName(gwen_error_errorform,
                           "ErrorManager");
    GWEN_ErrorType_SetMsgPtr(gwen_error_errorform,
                             GWEN_Error_ErrorString);
    err=GWEN_Error_RegisterType(gwen_error_errorform);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_ErrorType_free(gwen_error_errorform);
      return err;
    }
    gwen_error_is_initialized=1;
  } /* if not initialized */
  return 0;
}



GWEN_ERRORCODE GWEN_Error_ModuleFini() {
  if (gwen_error_is_initialized) {
    GWEN_ERRORCODE err;

    err=GWEN_Error_UnregisterType(gwen_error_errorform);
    GWEN_ErrorType_free(gwen_error_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_error_is_initialized=0;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_Error_RegisterType(GWEN_ERRORTYPEREGISTRATIONFORM *tptr){
  assert(tptr);

  GWEN_LIST_ADD(GWEN_ERRORTYPEREGISTRATIONFORM, tptr,
                &gwen_error_errorformlist)

  DBG_DEBUG(GWEN_LOGDOMAIN, "Registered type \"%s\" (%d)\n",
            tptr->name, tptr->typ);
  return 0;
}



GWEN_ERRORCODE GWEN_Error_UnregisterType(GWEN_ERRORTYPEREGISTRATIONFORM *tptr) {
  assert(tptr);

  GWEN_LIST_DEL(GWEN_ERRORTYPEREGISTRATIONFORM, tptr,
                &gwen_error_errorformlist);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Unregistered type \"%s\" (%d)\n",
            tptr->name, tptr->typ);
  return 0;
}



int GWEN_Error_FindType(const char *name){
  GWEN_ERRORTYPEREGISTRATIONFORM *tptr;

  /* browse all types */
  assert(name);
  tptr=gwen_error_errorformlist;
  while(tptr) {
    /* compare typename to argument */
    if (strcmp(tptr->name,
               name)==0)
      /* match, return type number */
      return tptr->typ;
    tptr=tptr->next;
  } /* while */
  /* not found */
  return -1;
}



const char *GWEN_Error_GetTypename(int t) {
  GWEN_ERRORTYPEREGISTRATIONFORM *tptr;

  /* browse all types */
  tptr=gwen_error_errorformlist;
  while(tptr) {
    if (tptr->typ==t)
      return tptr->name;
    tptr=tptr->next;
  } /* while */
  return 0;
}



GWEN_ERRORCODE GWEN_Error_new(int iscustom,
                              int severity,
                              int typ,
                              int code) {
  GWEN_ERRORCODE c;

  c=0;
  GWEN_Error_SetSeverity(&c, severity);
  GWEN_Error_SetType(&c, typ);
  GWEN_Error_SetIsCustom(&c, iscustom);
  GWEN_Error_SetCode(&c, code);

  return c;
}



int GWEN_Error_IsOk(GWEN_ERRORCODE c) {
  if (c==0)
    return 1;
  return (GWEN_Error_GetSeverity(c)<GWEN_ERROR_SEVERITY_ERR);
}



int GWEN_Error_GetSeverity(GWEN_ERRORCODE c){
  return (c>>30) & 3;
}



void GWEN_Error_SetSeverity(GWEN_ERRORCODE *c, int v){
  GWEN_ERRORCODE nc;

  assert(c);
  (*c) &= 0x3fffffff;
  nc=(v&3);
  nc=nc<<30;
  (*c) |= nc;
}



int GWEN_Error_IsCustom(GWEN_ERRORCODE c){
  return c & 0x20000000;
}



void GWEN_Error_SetIsCustom(GWEN_ERRORCODE *c, int iscustom){
  assert(c);
  if (iscustom)
    (*c) |= 0x20000000;
  else
    (*c) &= ~0x20000000;
}



int GWEN_Error_GetType(GWEN_ERRORCODE c){
  return (c>>16) & 0xfff;
}



void GWEN_Error_SetType(GWEN_ERRORCODE *c, int v){
  GWEN_ERRORCODE nc;

  assert(c);
  (*c) &= ~0x0fff0000;
  nc = (v&0xfff);
  nc=nc<<16;
  (*c) |= nc;
}



int GWEN_Error_GetCode(GWEN_ERRORCODE c){
  return (short)(c & 0xffff);
}



void GWEN_Error_SetCode(GWEN_ERRORCODE *c, int v){
  assert(c);
  (*c) &= ~0xffff;
  (*c) |= v&0xffff;
}



/* internal function */
static int 
GWEN_Error_ConcatStrings(char *dst, unsigned int dsize, const char *src) {
  if (!src)
    return 1;
  assert(dst);
  if ((strlen(dst)+strlen(src)+1)>=dsize)
    return 0;
  strcat(dst,src);
  return 1;
}


int GWEN_Error_ToString(GWEN_ERRORCODE c, char *buffer, int bsize) {
  char str[64]; /* for number conversions */
  const char *s;
  int i;
  GWEN_ERRORTYPEREGISTRATIONFORM *tptr;

  assert(buffer);
  if (bsize<64)
    return 0;
  buffer[0]=0;

  /* severity */
  if (!GWEN_Error_ConcatStrings(buffer,bsize," Severity: "))
    return 0;
  switch(GWEN_Error_GetSeverity(c)) {
  case GWEN_ERROR_SEVERITY_DEBUG:
    s="Debug";
    break;

  case GWEN_ERROR_SEVERITY_INFO:
    s="Info";
    break;

  case GWEN_ERROR_SEVERITY_WARN:
    s="Warning";
    break;

  case GWEN_ERROR_SEVERITY_ERR:
    s="Error";
    break;

  default:
    s="Unknown";
    break;
  } /* switch */
  if (!GWEN_Error_ConcatStrings(buffer,bsize,s))
    return 0;

  if (GWEN_Error_IsCustom(c)) {
    /* this is an custom error, so the normal rules do not apply
     * error type */
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Custom Type: "))
      return 0;
    sprintf(str,"%d (%04x)",
            GWEN_Error_GetType(c),
            GWEN_Error_GetType(c));
    if (!GWEN_Error_ConcatStrings(buffer,bsize,str))
      return 0;

    /* error code */
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Custom Code: "))
      return 0;
    sprintf(str,"%d (%04x)",
            GWEN_Error_GetCode(c),
            GWEN_Error_GetCode(c));
    if (!GWEN_Error_ConcatStrings(buffer,bsize,str))
      return 0;
  }
  else {
    /* error type */
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Type: "))
      return 0;
    s=GWEN_Error_GetTypename(GWEN_Error_GetType(c));
    if (s==(const char*)0) {
      sprintf(str,"Unknown (%4x)",GWEN_Error_GetType(c));
      s=str;
    }
    if (!GWEN_Error_ConcatStrings(buffer,bsize,s))
      return 0;

    /* error code */
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Code: "))
      return 0;

    /* get message function */
    i=GWEN_Error_GetType(c);
    tptr=gwen_error_errorformlist;
    while(tptr) {
      if (tptr->typ==i)
        break;
      tptr=tptr->next;
    }
    if (tptr) {
      if (tptr->msgptr)
        s=tptr->msgptr(GWEN_Error_GetCode(c));
      else
        s=(const char*)0;
    }
    else
      s=(const char*)0;

    if (s==(const char*)0) {
      if (!GWEN_Error_ConcatStrings(buffer,bsize,"Unknown"))
        return 0;
    }
    else {
      if (!GWEN_Error_ConcatStrings(buffer,bsize,s))
        return 0;
    }
    sprintf(str," (%d)", GWEN_Error_GetCode(c));
    s=str;
    if (!GWEN_Error_ConcatStrings(buffer,bsize,s))
      return 0;
  }

  /* finished */
  return 1;
}



const char *GWEN_Error_ErrorString(int c) {
  const char *s;

  switch(c) {
  case GWEN_ERROR_UNSPECIFIED:
    s="Undefined error";
    break;
  case GWEN_ERROR_COULD_NOT_REGISTER:
    s="Could not register";
    break;
  case GWEN_ERROR_COULD_NOT_UNREGISTER:
    s="Could not unregister";
    break;
  case GWEN_ERROR_INVALID_BUFFERSIZE:
    s="Invalid buffer size";
    break;
  default:
    s=(const char*)0;
  } /* switch */
  return s;
}



int GWEN_Error_GetSimpleCode(GWEN_ERRORCODE c){
  int code;
  const char *t;
  int r;

  if (c==0)
    return GWEN_SUCCESS;

  code=GWEN_Error_GetCode(c);
  t=GWEN_Error_GetTypename(GWEN_Error_GetType(c));
  r=GWEN_ERROR_GENERIC;

  if (t==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid error type %d",
              GWEN_Error_GetType(c));
    return GWEN_ERROR_GENERIC;
  }

  if (strcasecmp(t, GWEN_SOCKET_ERROR_TYPE)==0) {
    switch(code) {
    case GWEN_SOCKET_ERROR_BAD_SOCKETTYPE:
      r=GWEN_ERROR_BAD_SOCKETTYPE; break;
    case GWEN_SOCKET_ERROR_NOT_OPEN:
      r=GWEN_ERROR_NOT_OPEN; break;
    case GWEN_SOCKET_ERROR_TIMEOUT:
      r=GWEN_ERROR_TIMEOUT; break;
    case GWEN_SOCKET_ERROR_IN_PROGRESS:
      r=GWEN_ERROR_IN_PROGRESS; break;
    case GWEN_SOCKET_ERROR_STARTUP:
      r=GWEN_ERROR_STARTUP; break;
    case GWEN_SOCKET_ERROR_INTERRUPTED:
      r=GWEN_ERROR_INTERRUPTED; break;
    case GWEN_SOCKET_ERROR_UNSUPPORTED:
      r=GWEN_ERROR_UNSUPPORTED; break;
    case GWEN_SOCKET_ERROR_ABORTED:
      r=GWEN_ERROR_ABORTED; break;
    case GWEN_SOCKET_ERROR_BROKEN_PIPE:
      r=GWEN_ERROR_BROKEN_PIPE; break;
    default:
      r=GWEN_ERROR_GENERIC;
    }
  }
  else if (strcasecmp(t, GWEN_INETADDR_ERROR_TYPE)==0) {
    switch(code) {
    case GWEN_INETADDR_ERROR_MEMORY_FULL:
      r=GWEN_ERROR_MEMORY_FULL; break;
    case GWEN_INETADDR_ERROR_BAD_ADDRESS:
      r=GWEN_ERROR_BAD_ADDRESS; break;
    case GWEN_INETADDR_ERROR_BUFFER_OVERFLOW:
      r=GWEN_ERROR_BUFFER_OVERFLOW; break;
    case GWEN_INETADDR_ERROR_HOST_NOT_FOUND:
      r=GWEN_ERROR_HOST_NOT_FOUND; break;
    case GWEN_INETADDR_ERROR_NO_ADDRESS:
      r=GWEN_ERROR_NO_ADDRESS; break;
    case GWEN_INETADDR_ERROR_NO_RECOVERY:
      r=GWEN_ERROR_NO_RECOVERY; break;
    case GWEN_INETADDR_ERROR_TRY_AGAIN:
      r=GWEN_ERROR_TRY_AGAIN; break;
    case GWEN_INETADDR_ERROR_UNKNOWN_DNS_ERROR:
      r=GWEN_ERROR_UNKNOWN_DNS_ERROR; break;
    case GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY:
      r=GWEN_ERROR_BAD_ADDRESS_FAMILY; break;
    case GWEN_INETADDR_ERROR_UNSUPPORTED:
      r=GWEN_ERROR_UNSUPPORTED; break;
    default:
      r=GWEN_ERROR_GENERIC;
    }
  }
  else if (strcasecmp(t, GWEN_LIBLOADER_ERROR_TYPE)==0) {
    switch(code) {
    case GWEN_LIBLOADER_ERROR_COULD_NOT_LOAD:
      r=GWEN_ERROR_COULD_NOT_LOAD; break;
    case GWEN_LIBLOADER_ERROR_NOT_OPEN:
      r=GWEN_ERROR_NOT_OPEN; break;
    case GWEN_LIBLOADER_ERROR_COULD_NOT_CLOSE:
      r=GWEN_ERROR_CLOSE; break;
    case GWEN_LIBLOADER_ERROR_COULD_NOT_RESOLVE:
      r=GWEN_ERROR_COULD_NOT_RESOLVE; break;
    case GWEN_LIBLOADER_ERROR_NOT_FOUND:
      r=GWEN_ERROR_NOT_FOUND; break;
    default:
      r=GWEN_ERROR_GENERIC;
    }
  }
  else if (strcasecmp(t, GWEN_BUFFEREDIO_ERROR_TYPE)==0) {
    switch(code) {
    case GWEN_BUFFEREDIO_ERROR_READ:
      r=GWEN_ERROR_READ; break;
    case GWEN_BUFFEREDIO_ERROR_WRITE:
      r=GWEN_ERROR_WRITE; break;
    case GWEN_BUFFEREDIO_ERROR_CLOSE:
      r=GWEN_BUFFEREDIO_ERROR_CLOSE; break;
    case GWEN_BUFFEREDIO_ERROR_TIMEOUT:
      r=GWEN_ERROR_TIMEOUT; break;
    case GWEN_BUFFEREDIO_ERROR_PARTIAL:
      r=GWEN_ERROR_PARTIAL; break;
    case GWEN_BUFFEREDIO_ERROR_EOF:
      r=GWEN_ERROR_EOF; break;
    case GWEN_BUFFEREDIO_ERROR_NO_DATA:
      r=GWEN_ERROR_NO_DATA; break;
    default:
      r=GWEN_ERROR_GENERIC;
    }
  }
  else if (strcasecmp(t, GWEN_CRYPT_ERROR_TYPE)==0) {
    switch(code) {
    case GWEN_CRYPT_ERROR_ALREADY_REGISTERED:
      r=GWEN_ERROR_ALREADY_REGISTERED; break;
    case GWEN_CRYPT_ERROR_NOT_REGISTERED:
      r=GWEN_ERROR_NOT_REGISTERED; break;
    case GWEN_CRYPT_ERROR_BAD_SIZE:
      r=GWEN_ERROR_BAD_SIZE; break;
    case GWEN_CRYPT_ERROR_BUFFER_FULL:
      r=GWEN_ERROR_BUFFER_OVERFLOW; break;
    case GWEN_CRYPT_ERROR_ENCRYPT:
      r=GWEN_ERROR_ENCRYPT; break;
    case GWEN_CRYPT_ERROR_DECRYPT:
      r=GWEN_ERROR_DECRYPT; break;
    case GWEN_CRYPT_ERROR_SIGN:
      r=GWEN_ERROR_SIGN; break;
    case GWEN_CRYPT_ERROR_VERIFY:
      r=GWEN_ERROR_VERIFY; break;
    case GWEN_CRYPT_ERROR_UNSUPPORTED:
      r=GWEN_ERROR_UNSUPPORTED; break;
    case GWEN_CRYPT_ERROR_SSL:
      r=GWEN_ERROR_SSL; break;
    case GWEN_CRYPT_ERROR_GENERIC:
    default:
      r=GWEN_ERROR_GENERIC;
    }
  }

  return r;
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
  case GWEN_ERROR_UNSUPPORTED:
    s=I18N("Unsupported"); break;
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












