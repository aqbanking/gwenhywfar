/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Dec 03 2002
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

#include "directory_p.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>



GWEN_DIRECTORYDATA *GWEN_Directory_new(){
  GWEN_DIRECTORYDATA *d;

  GWEN_NEW_OBJECT(GWEN_DIRECTORYDATA, d);
  return d;
}



void GWEN_Directory_free(GWEN_DIRECTORYDATA *d){
  if (d) {
    if (d->handle!=INVALID_HANDLE_VALUE)
      FindClose(d->handle);
    d->handle=INVALID_HANDLE_VALUE;
    GWEN_FREE_OBJECT(d);
  }
}



int GWEN_Directory_Open(GWEN_DIRECTORYDATA *d, const char *n){
  assert(d);
  assert(n);
  if ((strlen(n)+5)>=sizeof(d->pattern)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Directory name too long");
    return 1;
  }
  strcpy(d->pattern, n);
  strcat(d->pattern, "\\*.*");

  return GWEN_Directory_Rewind(d);;
}



int GWEN_Directory_Close(GWEN_DIRECTORYDATA *d){
  int rv;

  rv=0;
  if (d->handle!=INVALID_HANDLE_VALUE)
    rv=!FindClose(d->handle);
  d->handle=INVALID_HANDLE_VALUE;
  return rv;
}



int GWEN_Directory_Read(GWEN_DIRECTORYDATA *d,
			char *buffer,
			unsigned int len){
  WIN32_FIND_DATA wd;

  assert(d);
  assert(buffer);
  assert(len);

  if (d->lastName[0]==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No more entries");
    return 1;
  }

  /* copy existing entry */
  if ((strlen(d->lastName)>=len)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
    return 1;
  }
  strcpy(buffer, d->lastName);

  /* read next entry */
  d->lastName[0]=0;
  if (FindNextFile(d->handle,&wd)) {
    if ((strlen(wd.cFileName)+1)>=sizeof(d->lastName)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Entry too long");
    }

    strcpy(d->lastName,wd.cFileName);
  }
  return 0;
}



int GWEN_Directory_Rewind(GWEN_DIRECTORYDATA *d){
  WIN32_FIND_DATA wd;

  assert(d);
  d->handle=FindFirstFile(d->pattern,&wd);
  if (d->handle==INVALID_HANDLE_VALUE) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "No entry for \"%s\"", d->pattern);
    return 1;
  }
  if ((strlen(wd.cFileName)+1)>=sizeof(d->lastName)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry name too long");
    return 1;
  }
  strcpy(d->lastName,wd.cFileName);
  return 0;
}



int GWEN_Directory_Create(const char *path){

  if (_mkdir(path)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error on _mkdir(%s): %s",
             path, strerror(errno));
    return -1;
  }
  return 0;
}



int GWEN_Directory_CreatePublic(const char *path){
  /* same as above, since on WIN32 all folders are public (gulp) */
  return GWEN_Directory_Create(path);
}



int GWEN_Directory_GetHomeDirectory(char *buffer, unsigned int size){
  int rv;

  rv=GetWindowsDirectory(buffer, size);
  if (rv==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error on GetWindowsDirectory");
    return -1;
  }
  if (rv>=size) {
    DBG_INFO(GWEN_LOGDOMAIN, "Buffer too small");
    return -1;
  }
  return 0;
}






