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
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef ENABLE_NLS
# include <locale.h>
#endif
#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
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
    GWEN_ErrorType_free(gwen_libloader_errorform);
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


GWEN_ERRORCODE GWEN_LibLoader_LoadLibrary(GWEN_LIBLOADER *h,
					  const char *name){
  const char *errorstring;

  assert(h);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Loading library \"%s\"", name);

  h->handle=dlopen(name, RTLD_LAZY);
  if (!h->handle) {

#ifdef HAVE_SETLOCALE
    const char *orig_locale = setlocale(LC_MESSAGES,NULL);
    char *currentLocale = strdup(orig_locale ? orig_locale : "C");
    setlocale(LC_MESSAGES,"C");
#endif

    /* The string is checked against the known "C" locale strings
       below. We *have* to switch to the C locale temporarily because
       otherwise the string might be any translated value. */
    errorstring = dlerror();

#ifdef HAVE_SETLOCALE
    setlocale(LC_MESSAGES,currentLocale);
    free(currentLocale);
#endif

    DBG_INFO(GWEN_LOGDOMAIN, "dlopen(%s): %s", name, errorstring);
    if (strstr(errorstring, "No such file")) {
      if (strstr(errorstring, name)) {
	return GWEN_Error_new(0,
			      GWEN_ERROR_SEVERITY_ERR,
			      GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
			      GWEN_LIBLOADER_ERROR_NOT_FOUND);
      }
    }
    else if (strstr(errorstring, "undefined symbol:")) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "GWEN: Error loading library: %s",
               errorstring);
      if (strstr(errorstring, name))
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                              GWEN_LIBLOADER_ERROR_COULD_NOT_RESOLVE);
      else
	return GWEN_Error_new(0,
			      GWEN_ERROR_SEVERITY_ERR,
			      GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
			      GWEN_LIBLOADER_ERROR_COULD_NOT_LOAD);
    }
    DBG_INFO(GWEN_LOGDOMAIN,
             "GWEN: Error loading library: %s",
             errorstring);
    return GWEN_Error_new(0,
			  GWEN_ERROR_SEVERITY_ERR,
			  GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
			  GWEN_LIBLOADER_ERROR_COULD_NOT_LOAD);
  }
  DBG_INFO(GWEN_LOGDOMAIN, "Loaded library \"%s\"", name);
  return 0;
}



GWEN_ERRORCODE GWEN_LibLoader_CloseLibrary(GWEN_LIBLOADER *h){

  assert(h);

  if (!h->handle)
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_NOT_OPEN);
  /* hmm, linux does not like this with libtowitoko.so and qlcsetup */
  if (dlclose(h->handle)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "GWEN: Error unloading library: %s",
              dlerror());
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_COULD_NOT_CLOSE);
  }
  h->handle=0;
  return 0;
}



GWEN_ERRORCODE GWEN_LibLoader_Resolve(GWEN_LIBLOADER *h,
                                      const char *name, void **p){
  assert(h);
  assert(name);
  assert(p);

  if (!h->handle)
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
                          GWEN_LIBLOADER_ERROR_NOT_OPEN);
  *p=dlsym(h->handle,name);
  if (!*p) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error resolving symbol \"%s\": %s\n",
              name, dlerror());
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE),
			  GWEN_LIBLOADER_ERROR_COULD_NOT_RESOLVE);
  }
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Resolved symbol \"%s\": %p\n",
              name, *p);
  return 0;
}



GWEN_ERRORCODE GWEN_LibLoader_OpenLibraryWithPath(GWEN_LIBLOADER *h,
                                                  const char *path,
						  const char *name){
  GWEN_BUFFER *buffer;
  unsigned int pos;
  unsigned int pos2;
  unsigned int i;
  int missingSoExt;
  int missingLibPrefix;
  GWEN_ERRORCODE err;

  assert(h);
  assert(name);
  missingSoExt=0;
  missingLibPrefix=0;
  buffer=GWEN_Buffer_new(0, 256, 0, 1);

  if (path) {
    GWEN_Buffer_AppendString(buffer, path);
    GWEN_Buffer_AppendByte(buffer, '/');
  }
  /* remember current position */
  pos=GWEN_Buffer_GetPos(buffer);
  /* append name of the library to load */
  GWEN_Buffer_AppendString(buffer, name);
  i=strlen(name);

  /* check whether we have the ".so" extension */
  if ((i<=3) || (strcmp(name+i-3, ".so")!=0)) {
    /* no SO-extension, add it myself */
    missingSoExt=1;
  }

  /* check whether we have the "lib" prefix */
  if ((i<=3) || (strncmp(name, "lib", 3)!=0)) {
    /* no SO-extension, add it myself */
    missingLibPrefix=1;
  }

  /* try to load the library */
  err=GWEN_LibLoader_LoadLibrary(h, GWEN_Buffer_GetStart(buffer));
  if (GWEN_Error_IsOk(err)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Library \"%s\" loaded",
             GWEN_Buffer_GetStart(buffer));
    GWEN_Buffer_free(buffer);
    return 0;
  }

  /* could not load, check why */
  /*if (GWEN_Error_GetType(err)!=
      GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE) ||
      GWEN_Error_GetCode(err)!=GWEN_LIBLOADER_ERROR_NOT_FOUND) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    DBG_INFO(GWEN_LOGDOMAIN, "Could not load library \"%s\"",
             GWEN_Buffer_GetStart(buffer));
    GWEN_Buffer_free(buffer);
    return err;
  }
  */

  /* hmm, not found, try some variants */
  if (missingSoExt) {
    /* try again, this time with ".so" appended */
    pos2=GWEN_Buffer_GetPos(buffer);
    GWEN_Buffer_AppendString(buffer, ".so");
    err=GWEN_LibLoader_LoadLibrary(h, GWEN_Buffer_GetStart(buffer));
    if (GWEN_Error_IsOk(err)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Library \"%s\" loaded",
               GWEN_Buffer_GetStart(buffer));
      GWEN_Buffer_free(buffer);
      return 0;
    }
    GWEN_Buffer_Crop(buffer, 0, pos2);
    GWEN_Buffer_SetPos(buffer, pos2);

    /* could not load, check why */
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE) ||
        GWEN_Error_GetCode(err)!=GWEN_LIBLOADER_ERROR_NOT_FOUND) {
      DBG_INFO(GWEN_LOGDOMAIN, "Could not load library \"%s\"",
               GWEN_Buffer_GetStart(buffer));
      GWEN_Buffer_free(buffer);
      return err;
    }
  }

  if (missingLibPrefix) {
    GWEN_Buffer_SetPos(buffer, pos);
    /* insert "lib" */
    GWEN_Buffer_InsertString(buffer, "lib");
    /* try again */
    err=GWEN_LibLoader_LoadLibrary(h, GWEN_Buffer_GetStart(buffer));
    if (GWEN_Error_IsOk(err)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Library \"%s\" loaded",
               GWEN_Buffer_GetStart(buffer));
      GWEN_Buffer_free(buffer);
      return 0;
    }

    /* could not load, check why */
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_LIBLOADER_ERROR_TYPE) ||
        GWEN_Error_GetCode(err)!=GWEN_LIBLOADER_ERROR_NOT_FOUND) {
      DBG_INFO(GWEN_LOGDOMAIN, "Could not load library \"%s\"",
               GWEN_Buffer_GetStart(buffer));
      GWEN_Buffer_free(buffer);
      return err;
    }

    /* try again, this time with ".so" AND "lib" */
    if (missingSoExt) {
      GWEN_Buffer_AppendString(buffer, ".so");
      err=GWEN_LibLoader_LoadLibrary(h, GWEN_Buffer_GetStart(buffer));
      if (GWEN_Error_IsOk(err)) {
        DBG_INFO(GWEN_LOGDOMAIN, "Library \"%s\" loaded",
                 GWEN_Buffer_GetStart(buffer));
        GWEN_Buffer_free(buffer);
        return 0;
      }
    }
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Library \"%s\" name (or variants) not found, giving up",
           name);
  GWEN_Buffer_free(buffer);
  return err;
}



GWEN_ERRORCODE GWEN_LibLoader_OpenLibrary(GWEN_LIBLOADER *h,
					  const char *name){
  return GWEN_LibLoader_OpenLibraryWithPath(h, 0, name);
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
  case GWEN_LIBLOADER_ERROR_COULD_NOT_RESOLVE:
    s="Could not resolve symbol";
    break;
  case GWEN_LIBLOADER_ERROR_NOT_FOUND:
    s="Library not found";
    break;
  default:
    s=(const char*)0;
  } /* switch */
  return s;
}










