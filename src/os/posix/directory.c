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
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gwenhywfar/debug.h>


static char gwen_directory_posix__homedir[256];
static int gwen_directory_posix__home_set=0;


GWEN_DIRECTORYDATA *GWEN_Directory_new(){
  GWEN_DIRECTORYDATA *d;

  d=(GWEN_DIRECTORYDATA *)malloc(sizeof(GWEN_DIRECTORYDATA));
  assert(d);
  memset(d,0,sizeof(GWEN_DIRECTORYDATA));
  return d;
}


void GWEN_Directory_free(GWEN_DIRECTORYDATA *d){
  if (d) {
    if (d->handle)
      closedir(d->handle);
    d->handle=0;
    free(d);
  }
}


int GWEN_Directory_Open(GWEN_DIRECTORYDATA *d, const char *n){
  assert(d);

  d->handle=opendir(n);
  if (d->handle==0)
    return 1;
  return 0;
}


int GWEN_Directory_Close(GWEN_DIRECTORYDATA *d){
  int rv;

  assert(d);
  rv=closedir(d->handle);
  d->handle=0;
  return rv;
}


int GWEN_Directory_Read(GWEN_DIRECTORYDATA *d,
                        char *buffer,
                        unsigned int len){
  struct dirent *de;

  assert(d);
  assert(buffer);
  assert(len);

  de=readdir(d->handle);
  if (de) {
    if (len<strlen(de->d_name)+1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
      return 1;
    }
    strcpy(buffer,de->d_name);
    return 0;
  }
  return 1;
}


int GWEN_Directory_Rewind(GWEN_DIRECTORYDATA *d){
  assert(d);
  if (d->handle==0)
    return 1;
  rewinddir(d->handle);
  return 0;
}



int GWEN_Directory_GetHomeDirectory(char *buffer, unsigned int size){
  if (!gwen_directory_posix__home_set) {
    struct passwd *p;

    p=getpwuid(geteuid());
    if (!p) {
      DBG_ERROR(GWEN_LOGDOMAIN, "%s at getpwuid", strerror(errno));
      endpwent();
      return -1;
    }
    if (sizeof(gwen_directory_posix__homedir)<strlen(p->pw_dir)+1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Internal: Buffer too small (need %d bytes)",
                (int)(strlen(p->pw_dir)+1));
      endpwent();
      return -1;
    }
    strcpy(gwen_directory_posix__homedir, p->pw_dir);
    gwen_directory_posix__home_set=1;
    endpwent();
  }

  if (size<strlen(gwen_directory_posix__homedir)+1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small (need %d bytes)",
	      (int)(strlen(gwen_directory_posix__homedir)+1));
    return -1;
  }
  strcpy(buffer, gwen_directory_posix__homedir);

  return 0;
}




int GWEN_Directory_Create(const char *path){

  if (mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error on mkdir(%s): %s",
             path, strerror(errno));
    return -1;
  }
  return 0;
}









