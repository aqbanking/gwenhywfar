/***************************************************************************
  $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Nov 22 2002
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

#include "libloader_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef OS_WIN32
# ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
# endif
#else
# include <windows.h>
#endif


const char *GWEN_LibLoader_ErrorString(int c);


static int gwen_libloader_is_initialized=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_libloader_errorform=0;


GWEN_ERRORCODE GWEN_LibLoader_ModuleInit(){
  if (!gwen_libloader_is_initialized) {
    GWEN_ERRORCODE err;

    gwen_libloader_errorform=GWEN_ErrorType_new();
    GWEN_ErrorType_SetName(gwen_libloader_errorform,
                           GWEN_LIBLOADER_ERROR_TYPE);
    GWEN_ErrorType_SetMsgPtr(gwen_libloader_errorform,
                             GWEN_LibLoader_ErrorString);
    err=GWEN_Error_RegisterType(gwen_libloader_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_libloader_is_initialized=1;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_LibLoader_ModuleFini(){
  if (gwen_libloader_is_initialized) {
    GWEN_ERRORCODE err;

    err=GWEN_Error_UnregisterType(gwen_libloader_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_libloader_is_initialized=0;
  }
  return 0;
}



GWEN_LIBLOADER *GWEN_LibLoader_new(){
  GWEN_LIBLOADER *h;

  GWEN_NEW_OBJECT(GWEN_LIBLOADER, h);
  return h;
}



void GWEN_LibLoader_free(GWEN_LIBLOADER *h){
  if (h) {
    free(h);
  }
}



GWEN_ERRORCODE GWEN_LibLoader_OpenLibrary(GWEN_LIBLOADER *h,
                                          const char *name){
  assert(h);
#if DEBUGMODE>10
  fprintf(stderr,"LibLoader_OpenLibrary\n");
#endif

#if defined (OS_WIN32)
  h->handle=(void*)LoadLibrary(name);
  if (!h->handle) {
# if DEBUGMODE>0
    fprintf(stderr,
	    "LIBCHIPCARD: Error loading library \"%s\"\n",
	    name);
# endif
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_COULD_NOT_LOAD);
  }
#else // for linux and others
  h->handle=dlopen(name,RTLD_LAZY);
  if (!h->handle) {
    fprintf(stderr,
	    "LIBCHIPCARD: Error loading library \"%s\": %s\n",
	    name, dlerror());
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_COULD_NOT_LOAD);
  }
#endif
  return 0;
}



GWEN_ERRORCODE GWEN_LibLoader_CloseLibrary(GWEN_LIBLOADER *h){
  assert(h);
#if DEBUGMODE>10
  fprintf(stderr,"LibLoader_CloseLibrary\n");
#endif

  if (!h->handle)
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_NOT_OPEN);
#ifndef OS_WIN32
  /* hmm, linux does not like this with libtowitoko.so and qlcsetup */
  //result=(dlclose(_handle)==0);
  if (dlclose(h->handle)!=0) {
    fprintf(stderr,
	    "LIBCHIPCARD: Error unloading library: %s\n",
	    dlerror());
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_COULD_NOT_CLOSE);
  }
#else
  if (!FreeLibrary((HINSTANCE)h->handle))
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_COULD_NOT_CLOSE);
#endif
  h->handle=0;
  return 0;
}



GWEN_ERRORCODE GWEN_LibLoader_Resolve(GWEN_LIBLOADER *h,
                                      const char *name, void **p){
  assert(h);
  assert(name);
  assert(p);

#if DEBUGMODE>10
  fprintf(stderr,"LibLoader_Resolve\n");
#endif

  if (!h->handle)
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_NOT_OPEN);
#ifndef OS_WIN32
  *p=dlsym(h->handle,name);
  if (!*p) {
    DBG_ERROR(0, "Error resolving symbol \"%s\": %s\n",
              name, dlerror());
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_COULD_RESOLVE);
  }
#else
  *p=(void*)GetProcAddress((HINSTANCE)h->handle,name);
  if (!*p) {
    DBG_ERROR(0, "Error resolving symbol \"%s\"\n",
              name);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_COULD_RESOLVE);
  }
#endif
  DBG_VERBOUS(0, "Resolved symbol \"%s\": %08x\n",
              name, (int)*p);
  return 0;
}



const char *GWEN_LibLoader_ErrorString(int c){
  const char *s;

  switch(c) {
  case 0:
    s="Success";
    break;

  case GWEN_LIBLOADER_ERROR_COULD_NOT_LOAD:
    s="Could not load library";
    break;
  case GWEN_LIBLOADER_ERROR_NOT_OPEN:
    s="Library not open";
    break;
  case GWEN_LIBLOADER_ERROR_COULD_NOT_CLOSE:
    s="Could not close library";
    break;
  case GWEN_LIBLOADER_ERROR_COULD_RESOLVE:
    s="Could not resolve symbol";
    break;
  default:
    s=(const char*)0;
  } // switch
  return s;
}










