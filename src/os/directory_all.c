/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Nov 23 2003
    copyright   : (C) 2003 by Martin Preuss
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


#include <gwenhywfar/directory.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/buffer.h>

#include <unistd.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <sys/types.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>



void *GWEN_Directory_HandlePathElement(const char *entry,
                                       void *data,
                                       unsigned int flags){
  char *p;
  struct stat st;
  int exists;
  int withDrive;
  GWEN_BUFFER *buf;
  GWEN_BUFFER *ebuf;

  withDrive=0;
#ifdef OS_WIN32
  ebuf=0;
  if (isalpha(*entry)) {
    if (entry[1]==':') {
      ebuf=GWEN_Buffer_new(0, strlen(entry)+2, 0, 1);
      GWEN_Buffer_AppendString(ebuf, entry);
      GWEN_Buffer_AppendByte(ebuf, '\\');
      withDrive=1;
      entry=GWEN_Buffer_GetStart(ebuf);
    }
  }
#else
  ebuf=0;
#endif

  if (strcasecmp(entry, "..")==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "\"..\" detected");
    GWEN_Buffer_free(ebuf);
    return 0;
  }

  buf=(GWEN_BUFFER*)data;
  if (GWEN_Buffer_GetUsedBytes(buf) && !withDrive) {
#ifdef OS_WIN32
    GWEN_Buffer_AppendByte(buf, '\\');
#else
    GWEN_Buffer_AppendByte(buf, '/');
#endif
  }
  GWEN_Buffer_AppendString(buf, entry);

  /* check for existence of the file/folder */
  p=GWEN_Buffer_GetStart(buf);
  DBG_DEBUG(GWEN_LOGDOMAIN, "Checking entry \"%s\"", p);
  if (stat(p, &st)) {
    exists=0;
    DBG_DEBUG(GWEN_LOGDOMAIN, "stat: %s (%s)", strerror(errno), p);
    if ((flags & GWEN_PATH_FLAGS_PATHMUSTEXIST) ||
        ((flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST))) {
      DBG_INFO(GWEN_LOGDOMAIN, "Path \"%s\" does not exist (it should)", p);
      GWEN_Buffer_free(ebuf);
      return 0;
    }
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Checking for type");
    exists=1;
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      if (!S_ISREG(st.st_mode)) {
        DBG_INFO(GWEN_LOGDOMAIN, "%s not a regular file", p);
	GWEN_Buffer_free(ebuf);
        return 0;
      }
    }
    else {
      if (!S_ISDIR(st.st_mode)) {
        DBG_INFO(GWEN_LOGDOMAIN, "%s not a direcory", p);
	GWEN_Buffer_free(ebuf);
        return 0;
      }
    }
    if ((flags & GWEN_PATH_FLAGS_PATHMUSTNOTEXIST) ||
        ((flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST))) {
      DBG_INFO(GWEN_LOGDOMAIN, "Path \"%s\" does not exist (it should)", p);
      GWEN_Buffer_free(ebuf);
      return 0;
    }
  } /* if stat is ok */

  if (!exists) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Entry \"%s\" does not exist", p);
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      /* create file */
      int fd;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Creating file \"%s\"", p);
      fd=open(p, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
      if (fd==-1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "open: %s (%s)", strerror(errno), p);
	GWEN_Buffer_free(ebuf);
        return 0;
      }
      close(fd);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Sucessfully created");
    }
    else {
      /* create dir */
      DBG_DEBUG(GWEN_LOGDOMAIN, "Creating folder \"%s\"", p);

      if (GWEN_Directory_Create(p)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not create directory \"%s\"", p);
	GWEN_Buffer_free(ebuf);
        return 0;
      }
    }
  } /* if exists */
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Entry \"%s\" exists", p);
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Returning this: %s", p);
  GWEN_Buffer_free(ebuf);
  return buf;
}



int GWEN_Directory_GetPath(const char *path,
                           unsigned int flags) {
  GWEN_BUFFER *buf;
  void *p;

  assert(path);
  buf=GWEN_Buffer_new(0, strlen(path)+10, 0, 1);
  p=GWEN_Path_Handle(path, buf,
                     flags | GWEN_PATH_FLAGS_CHECKROOT,
                     GWEN_Directory_HandlePathElement);
  if (!p) {
    DBG_INFO(GWEN_LOGDOMAIN, "Path so far: \"%s\"", GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return -1;
  }
  GWEN_Buffer_free(buf);
  return 0;
}

















