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

#include "directory.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <chameleon/debug.h>



DIRECTORYDATA *Directory_new(){
  DIRECTORYDATA *d;

  d=(DIRECTORYDATA *)malloc(sizeof(DIRECTORYDATA));
  assert(d);
  memset(d,0,sizeof(DIRECTORYDATA));
  return d;
}


void Directory_free(DIRECTORYDATA *d){
  if (d) {
    if (d->handle!=INVALID_HANDLE_VALUE)
      FindClose(d->handle);
    d->handle=INVALID_HANDLE_VALUE;
    free(d);
  }
}


int Directory_Open(DIRECTORYDATA *d, const char *n){
  assert(d);
  assert(n);
  if ((strlen(n)+5)>=sizeof(d->pattern)) {
    DBG_ERROR("Directory name too long");
    return 1;
  }
  strcpy(d->pattern, n);
  strcat(d->pattern, "\\*.*");

  return Directory_Rewind(d);;
}


int Directory_Close(DIRECTORYDATA *d){
	int rv;

	rv=0;
    if (d->handle!=INVALID_HANDLE_VALUE)
      rv=!FindClose(d->handle);
    d->handle=INVALID_HANDLE_VALUE;
	return rv;
}


int Directory_Read(DIRECTORYDATA *d,
		   char *buffer,
		   unsigned int len){
  WIN32_FIND_DATA wd;

  assert(d);
  assert(buffer);
  assert(len);

  if (d->lastName[0]==0) {
    DBG_INFO("No more entries");
    return 1;
  }

  /* copy existing entry */
  if ((strlen(d->lastName)>=len)) {
    DBG_ERROR("Buffer too small");
    return 1;
  }
  strcpy(buffer, d->lastName);

  /* read next entry */
  d->lastName[0]=0;
  if (FindNextFile(d->handle,&wd)) {
	if ((strlen(wd.cFileName)+1)>=sizeof(d->lastName)) {
		DBG_ERROR("Entry too long");
    }
	
    strcpy(d->lastName,wd.cFileName);
  }
  return 0;
}


int Directory_Rewind(DIRECTORYDATA *d){
  WIN32_FIND_DATA wd;

  assert(d);
  d->handle=FindFirstFile(d->pattern,&wd);
  if (d->handle==INVALID_HANDLE_VALUE) {
    DBG_ERROR("No entry for \"%s\"", d->pattern);
    return 1;
  }
  if ((strlen(wd.cFileName)+1)>=sizeof(d->lastName)) {
    DBG_ERROR("Entry name too long");
    return 1;
  }
  strcpy(d->lastName,wd.cFileName);
  return 0;
}






