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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "error_p.h"
#include "gwenhyfwar/debug.h"
#include "gwenhyfwar/misc.h"

// forward declaration
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

  DBG_VERBOUS(0, "Error_ModuleInit");
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
  } // if not initialized
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
                &gwen_error_errorformlist);

  DBG_INFO(0, "Registered type \"%s\" (%d)\n",
           tptr->name, tptr->typ);
  return 0;
}



GWEN_ERRORCODE GWEN_Error_UnregisterType(GWEN_ERRORTYPEREGISTRATIONFORM *tptr) {
  assert(tptr);

  GWEN_LIST_DEL(GWEN_ERRORTYPEREGISTRATIONFORM, tptr,
                &gwen_error_errorformlist);

  DBG_INFO(0, "Unregistered type \"%s\" (%d)\n",
           tptr->name, tptr->typ);
  return 0;
}



int GWEN_Error_FindType(const char *name){
  GWEN_ERRORTYPEREGISTRATIONFORM *tptr;

  // browse all types
  assert(name);
  tptr=gwen_error_errorformlist;
  while(tptr) {
    // compare typename to argument
    if (strcmp(tptr->name,
               name)==0)
      // match, return type number
      return tptr->typ;
    tptr=tptr->next;
  } // while
  // not found
  return -1;
}



const char *GWEN_Error_GetTypename(int t) {
  GWEN_ERRORTYPEREGISTRATIONFORM *tptr;

  // browse all types
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
  assert(c);
  (*c) &= 0x3fffffff;
  v = (v&3)<<30;
  (*c) |= v;
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
  assert(c);
  (*c) &= ~0x0fff0000;
  v = (v&0xfff)<<16;
  (*c) |= v;
}



int GWEN_Error_GetCode(GWEN_ERRORCODE c){
  return (short)(c & 0xffff);
}



void GWEN_Error_SetCode(GWEN_ERRORCODE *c, int v){
  assert(c);
  (*c) &= ~0xffff;
  (*c) |= v&0xffff;
}



// internal function
int GWEN_Error_ConcatStrings(char *dst, int dsize, const char *src) {
  if (!src)
    return 1;
  assert(dst);
  if ((strlen(dst)+strlen(src)+1)>=dsize)
    return 0;
  strcat(dst,src);
  return 1;
}


int GWEN_Error_ToString(GWEN_ERRORCODE c, char *buffer, int bsize) {
  char str[64]; // for number conversions
  const char *s;
  int i;
  GWEN_ERRORTYPEREGISTRATIONFORM *tptr;

  assert(buffer);
  if (bsize<64)
    return 0;
  buffer[0]=0;

  // severity
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
  } // switch
  if (!GWEN_Error_ConcatStrings(buffer,bsize,s))
    return 0;

  if (GWEN_Error_IsCustom(c)) {
    // this is an custom error, so the normal rules do not apply
    // error type
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Custom Type: "))
      return 0;
    sprintf(str,"%d (%04x)",
            GWEN_Error_GetType(c),
            GWEN_Error_GetType(c));
    if (!GWEN_Error_ConcatStrings(buffer,bsize,str))
      return 0;

    // error code
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Custom Code: "))
      return 0;
    sprintf(str,"%d (%04x)",
            GWEN_Error_GetCode(c),
            GWEN_Error_GetCode(c));
    if (!GWEN_Error_ConcatStrings(buffer,bsize,str))
      return 0;
  }
  else {
    // error type
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Type: "))
      return 0;
    s=GWEN_Error_GetTypename(GWEN_Error_GetType(c));
    if (s==(const char*)0) {
      sprintf(str,"Unknown (%4x)",GWEN_Error_GetType(c));
      s=str;
    }
    if (!GWEN_Error_ConcatStrings(buffer,bsize,s))
      return 0;

    // error code
    if (!GWEN_Error_ConcatStrings(buffer,bsize," Code: "))
      return 0;

    // get message function
    i=GWEN_Error_GetType(c);
    tptr=gwen_error_errorformlist;
    while(tptr) {
      if (tptr->typ==i)
        break;
      tptr=tptr->next;
    }
    if (tptr) {
      if (tptr->msgptr)
        s=tptr->msgptr(i);
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

  // finished
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
  } // switch
  return s;
}



