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
#include "binreloc.h"

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


GWEN_DIRECTORY *GWEN_Directory_new(){
  GWEN_DIRECTORY *d;

  GWEN_NEW_OBJECT(GWEN_DIRECTORY, d);
  return d;
}


void GWEN_Directory_free(GWEN_DIRECTORY *d){
  if (d) {
    if (d->handle)
      closedir(d->handle);
    d->handle=0;
    GWEN_FREE_OBJECT(d);
  }
}


int GWEN_Directory_Open(GWEN_DIRECTORY *d, const char *n){
  assert(d);

  d->handle=opendir(n);
  if (d->handle==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "opendir(%s): %s",
             n, strerror(errno));
    return GWEN_ERROR_NOT_FOUND;
  }
  return 0;
}


int GWEN_Directory_Close(GWEN_DIRECTORY *d){
  int rv;

  assert(d);
  rv=closedir(d->handle);
  d->handle=0;
  return rv;
}


int GWEN_Directory_Read(GWEN_DIRECTORY *d,
                        char *buffer,
                        unsigned int len){
  struct dirent *de;

  assert(d);
  assert(buffer);
  assert(len);
  assert(d->handle);

  de=readdir(d->handle);
  if (de) {
    if (len<strlen(de->d_name)+1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    strcpy(buffer,de->d_name);
    return 0;
  }
  return GWEN_ERROR_NOT_FOUND;
}


int GWEN_Directory_Rewind(GWEN_DIRECTORY *d){
  assert(d);
  if (d->handle==0)
    return GWEN_ERROR_INVALID;
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



int GWEN_Directory_CreatePublic(const char *path){

  if (mkdir(path,
            S_IRUSR | S_IWUSR | S_IXUSR
#ifdef S_IRGRP
            | S_IRGRP
#endif
#ifdef S_IXGRP
            | S_IXGRP
#endif
#ifdef S_IROTH
            | S_IROTH
#endif
#ifdef S_IXOTH
            | S_IXOTH
#endif
           )) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error on mkdir(%s): %s",
             path, strerror(errno));
    return -1;
  }
  return 0;
}



int GWEN_Directory_GetPrefixDirectory(char *buffer, unsigned int size){
  char *exeDir;

#if defined(OS_DARWIN) && defined(ENABLE_LOCAL_INSTALL)
  CFBundleRef mainBundle = CFBundleGetMainBundle() ;
  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  CFStringRef resourcesPath = CFURLCopyFileSystemPath(resourcesURL, kCFURLPOSIXPathStyle) ;
  CFIndex maxPathSize = CFStringGetMaximumSizeOfFileSystemRepresentation(resourcesPath);
  if ((exeDir = malloc(maxPathSize * sizeof(char)))) {
    CFStringGetFileSystemRepresentation(resourcesPath, exeDir , maxPathSize);
  }
  CFRelease(resourcesPath);
  CFRelease(resourcesURL);
#else
  exeDir=br_find_prefix(NULL);
#endif
  if (exeDir==(char*)NULL) {
    DBG_INFO(GWEN_LOGDOMAIN,
	     "Unable to determine exe folder");
    return GWEN_ERROR_GENERIC;
  }

  if ((strlen(exeDir)+1)>=size) {
    free(exeDir);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  strcpy(buffer, exeDir);
  free(exeDir);
  return 0;
}










