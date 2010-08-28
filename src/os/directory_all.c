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
#include <gwenhywfar/text.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
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
#include <ctype.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



void *GWEN_Directory_HandlePathElement(const char *entry,
                                       void *data,
                                       unsigned int flags){
  char *p;
  struct stat st;
  int exists;
  int withDrive;
  GWEN_BUFFER *buf;
  GWEN_BUFFER *ebuf = 0;
  const char *origEntry;

  withDrive=0;
  origEntry=entry;

#ifdef OS_WIN32
  if (entry && isalpha(*entry)) {
    int len;

    /* append backslash if entry only consists of a drive specification */
    len=strlen(entry);
    if ( (len==2) && (entry[1] == ':') ) {
      ebuf=GWEN_Buffer_new(0, len+2, 0, 1);
      GWEN_Buffer_AppendString(ebuf, entry);
      GWEN_Buffer_AppendByte(ebuf, '\\');
      withDrive=1;
      entry=GWEN_Buffer_GetStart(ebuf);
    }
  }
#endif /* OS_WIN32 */

  if (strcasecmp(entry, "..")==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "\"..\" detected");
    GWEN_Buffer_free(ebuf);
    return 0;
  }

  buf=(GWEN_BUFFER*)data;
  if (GWEN_Buffer_GetUsedBytes(buf) && !withDrive) {
    char c;

    c=GWEN_Buffer_GetStart(buf)[GWEN_Buffer_GetUsedBytes(buf)-1];
#ifdef OS_WIN32
    if (c!='\\')
      GWEN_Buffer_AppendByte(buf, '\\');
#else
    if (c!='/')
      GWEN_Buffer_AppendByte(buf, '/');
#endif /* OS_WIN32 */
  }
  GWEN_Buffer_AppendString(buf, entry);

  /* check for existence of the file/folder */
  p=GWEN_Buffer_GetStart(buf);
  DBG_DEBUG(GWEN_LOGDOMAIN, "Checking path \"%s\"", p);
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
      DBG_INFO(GWEN_LOGDOMAIN, "Path \"%s\" exists (it should not)", p);
      GWEN_Buffer_free(ebuf);
      return 0;
    }
  } /* if stat is ok */

  if (!exists) {
    int isPublic;

    DBG_DEBUG(GWEN_LOGDOMAIN, "Entry \"%s\" does not exist", p);

    isPublic=(
              ((flags & GWEN_PATH_FLAGS_LAST) &&
               (flags & GWEN_DIR_FLAGS_PUBLIC_NAME)) ||
              (!(flags & GWEN_PATH_FLAGS_LAST) &&
               (flags & GWEN_DIR_FLAGS_PUBLIC_PATH))
             );

    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      /* create file */
      int fd;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Creating file \"%s\"", p);
      if (isPublic)
	fd=open(p, O_RDWR | O_CREAT | O_TRUNC,
		S_IRUSR | S_IWUSR
#ifdef S_IRGRP
		| S_IRGRP
#endif
#ifdef S_IROTH
		| S_IROTH
#endif
	       );
      else
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

      if (isPublic) {
	if (GWEN_Directory_CreatePublic(p)) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Could not create directory \"%s\"", p);
	  GWEN_Buffer_free(ebuf);
	  return 0;
	}
      }
      else {
	if (GWEN_Directory_Create(p)) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Could not create directory \"%s\"", p);
	  GWEN_Buffer_free(ebuf);
	  return 0;
	}
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



int GWEN_Directory_OsifyPath(const char *path, GWEN_BUFFER *pbuf,
                             int transformDriveElement){
  int len;
  const char *p;

  len=strlen(path);
  p=path;

  /* handle drive letters (only check for normal slashes here) */
#ifdef OS_WIN32
  if (transformDriveElement) {
    if (*p=='/')
      if (isalpha(p[1]))
	if (p[2]=='/' || p[2]==0) {
	  GWEN_Buffer_AppendByte(pbuf, p[0]);
          GWEN_Buffer_AppendByte(pbuf, ':');
          p+=2;
        }
  }
#endif

  while(*p) {
    if (*p=='/' || *p=='\\') {
      while (*p=='/' || *p=='\\')
        p++;
#ifdef OS_WIN32
      GWEN_Buffer_AppendByte(pbuf, '\\');
#else
      GWEN_Buffer_AppendByte(pbuf, '/');
#endif
    }
    else {
      GWEN_Buffer_AppendByte(pbuf, *p);
      p++;
    }
  }

  return 0;
}



int GWEN_Directory_FindFileInPaths(const GWEN_STRINGLIST *paths,
                                   const char *filePath,
                                   GWEN_BUFFER *fbuf) {
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(paths);
  while(se) {
    GWEN_BUFFER *tbuf;
    FILE *f;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, GWEN_StringListEntry_Data(se));
    GWEN_Buffer_AppendString(tbuf, DIRSEP);
    GWEN_Buffer_AppendString(tbuf, filePath);
    DBG_DEBUG(GWEN_LOGDOMAIN, "Trying \"%s\"",
	      GWEN_Buffer_GetStart(tbuf));
    f=fopen(GWEN_Buffer_GetStart(tbuf), "r");
    if (f) {
      fclose(f);
      DBG_DEBUG(GWEN_LOGDOMAIN,
		"File \"%s\" found in folder \"%s\"",
		filePath,
		GWEN_StringListEntry_Data(se));
      GWEN_Buffer_AppendBuffer(fbuf, tbuf);
      GWEN_Buffer_free(tbuf);
      return 0;
    }
    GWEN_Buffer_free(tbuf);

    se=GWEN_StringListEntry_Next(se);
  }

  DBG_INFO(GWEN_LOGDOMAIN, "File \"%s\" not found", filePath);
  return GWEN_ERROR_NOT_FOUND;
}



int GWEN_Directory_FindPathForFile(const GWEN_STRINGLIST *paths,
                                   const char *filePath,
                                   GWEN_BUFFER *fbuf) {
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(paths);
  while(se) {
    GWEN_BUFFER *tbuf;
    FILE *f;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, GWEN_StringListEntry_Data(se));
    GWEN_Buffer_AppendString(tbuf, DIRSEP);
    GWEN_Buffer_AppendString(tbuf, filePath);
    DBG_DEBUG(GWEN_LOGDOMAIN, "Trying \"%s\"",
	      GWEN_Buffer_GetStart(tbuf));
    f=fopen(GWEN_Buffer_GetStart(tbuf), "r");
    if (f) {
      fclose(f);
      DBG_INFO(GWEN_LOGDOMAIN,
	       "File \"%s\" found in folder \"%s\"",
	       filePath,
	       GWEN_StringListEntry_Data(se));
      GWEN_Buffer_AppendString(fbuf, GWEN_StringListEntry_Data(se));
      GWEN_Buffer_free(tbuf);
      return 0;
    }
    GWEN_Buffer_free(tbuf);

    se=GWEN_StringListEntry_Next(se);
  }

  DBG_INFO(GWEN_LOGDOMAIN, "File \"%s\" not found", filePath);
  return GWEN_ERROR_NOT_FOUND;
}



int GWEN_Directory_GetTmpDirectory(char *buffer, unsigned int size)
{
  const char *tmp_dir;
  assert(buffer);

  /* Copied from http://svn.gnome.org/viewcvs/glib/trunk/glib/gutils.c */
  tmp_dir = getenv ("TMPDIR");
  if (!tmp_dir)
    tmp_dir = getenv ("TMP");
  if (!tmp_dir)
    tmp_dir = getenv ("TEMP");

  if (!tmp_dir)
    {
#ifdef OS_WIN32
      tmp_dir = "C:\\";
#else  
      tmp_dir = "/tmp";
#endif	/* !OS_WIN32 */
    }

  strncpy (buffer, tmp_dir, size);
  return 0;
}



int GWEN_Directory_GetAllEntries(const char *folder,
				 GWEN_STRINGLIST *sl,
				 const char *mask) {
  GWEN_DIRECTORY *d;
  int rv;
  char buffer[256];

  d=GWEN_Directory_new();
  rv=GWEN_Directory_Open(d, folder);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Directory_free(d);
    return rv;
  }

  while(0==GWEN_Directory_Read(d, buffer, sizeof(buffer))) {
    if (strcmp(buffer, ".")!=0 &&
	strcmp(buffer, "..")!=0 &&
	(mask==NULL ||
	 GWEN_Text_ComparePattern(buffer+1, mask, 0)!=-1))
      GWEN_StringList_AppendString(sl, buffer, 0, 1);
  }

  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  return 0;
}



int GWEN_Directory_GetFileEntriesWithType(const char *folder,
					  GWEN_STRINGLIST *sl,
					  const char *mask) {
  GWEN_DIRECTORY *d;
  int rv;
  char buffer[256];
  GWEN_BUFFER *pbuf;
  uint32_t pos;

  d=GWEN_Directory_new();
  rv=GWEN_Directory_Open(d, folder);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Directory_free(d);
    return rv;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf, folder);
  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  pos=GWEN_Buffer_GetPos(pbuf);

  while(0==GWEN_Directory_Read(d, buffer+1, sizeof(buffer)-2)) {
    if (strcmp(buffer, ".")!=0 &&
	strcmp(buffer, "..")!=0 &&
	(mask==NULL ||
	 GWEN_Text_ComparePattern(buffer+1, mask, 0)!=-1)) {
      struct stat st;

      GWEN_Buffer_AppendString(pbuf, buffer+1);
      if (stat(GWEN_Buffer_GetStart(pbuf), &st)==0) {
	if (S_ISREG(st.st_mode))
	  buffer[0]='f';
	else if (S_ISDIR(st.st_mode))
	  buffer[0]='d';
	else
	  buffer[0]='?';
	GWEN_StringList_AppendString(sl, buffer, 0, 1);
      }
      GWEN_Buffer_Crop(pbuf, 0, pos);
    }
  }

  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  return 0;
}




int GWEN_Directory_GetFileEntries(const char *folder, GWEN_STRINGLIST *sl,
				  const char *mask) {
  GWEN_DIRECTORY *d;
  int rv;
  char buffer[256];
  GWEN_BUFFER *pbuf;
  uint32_t pos;

  d=GWEN_Directory_new();
  rv=GWEN_Directory_Open(d, folder);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Directory_free(d);
    return rv;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf, folder);
  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  pos=GWEN_Buffer_GetPos(pbuf);

  while(0==GWEN_Directory_Read(d, buffer, sizeof(buffer))) {
    if (strcmp(buffer, ".")!=0 &&
	strcmp(buffer, "..")!=0 &&
	(mask==NULL ||
	 GWEN_Text_ComparePattern(buffer+1, mask, 0)!=-1)) {
      struct stat st;

      GWEN_Buffer_AppendString(pbuf, buffer);
      if (stat(GWEN_Buffer_GetStart(pbuf), &st)==0) {
	if (S_ISREG(st.st_mode))
	  GWEN_StringList_AppendString(sl, buffer, 0, 1);
      }
      GWEN_Buffer_Crop(pbuf, 0, pos);
    }
  }

  GWEN_Buffer_free(pbuf);
  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  return 0;
}



int GWEN_Directory_GetDirEntries(const char *folder, GWEN_STRINGLIST *sl,
				 const char *mask) {
  GWEN_DIRECTORY *d;
  int rv;
  char buffer[256];
  GWEN_BUFFER *pbuf;
  uint32_t pos;

  d=GWEN_Directory_new();
  rv=GWEN_Directory_Open(d, folder);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Directory_free(d);
    return rv;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf, folder);
  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  pos=GWEN_Buffer_GetPos(pbuf);

  while(0==GWEN_Directory_Read(d, buffer, sizeof(buffer))) {
    if (strcmp(buffer, ".")!=0 &&
	strcmp(buffer, "..")!=0 &&
	(mask==NULL ||
	 GWEN_Text_ComparePattern(buffer+1, mask, 0)!=-1)) {
      struct stat st;

      GWEN_Buffer_AppendString(pbuf, buffer);
      if (stat(GWEN_Buffer_GetStart(pbuf), &st)==0) {
	if (S_ISDIR(st.st_mode))
	  GWEN_StringList_AppendString(sl, buffer, 0, 1);
      }
      GWEN_Buffer_Crop(pbuf, 0, pos);
    }
  }

  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  return 0;
}



int GWEN_Directory_GetMatchingFilesRecursively(const char *folder,
					       GWEN_STRINGLIST *sl,
					       const char *mask) {
  GWEN_DIRECTORY *d;
  int rv;
  char buffer[256];
  GWEN_BUFFER *pbuf;
  uint32_t pos;
  GWEN_STRINGLIST *folderList;

  folderList=GWEN_StringList_new();

  d=GWEN_Directory_new();
  rv=GWEN_Directory_Open(d, folder);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Directory_free(d);
    GWEN_StringList_free(folderList);
    return rv;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf, folder);
  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  pos=GWEN_Buffer_GetPos(pbuf);

  while(0==GWEN_Directory_Read(d, buffer, sizeof(buffer)-2)) {
    if (strcmp(buffer, ".")!=0 &&
	strcmp(buffer, "..")!=0 &&
	(mask==NULL ||
	 GWEN_Text_ComparePattern(buffer, mask, 0)!=-1)) {
      struct stat st;

      GWEN_Buffer_AppendString(pbuf, buffer);
      if (stat(GWEN_Buffer_GetStart(pbuf), &st)==0) {
	if (S_ISDIR(st.st_mode))
          /* add folders to the folder list */
	  GWEN_StringList_AppendString(folderList, GWEN_Buffer_GetStart(pbuf), 0, 1);
	else
	  GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(pbuf), 0, 1);
      }
      GWEN_Buffer_Crop(pbuf, 0, pos);
    }
  }

  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);

  if (GWEN_StringList_Count(folderList)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(folderList);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s)
	GWEN_Directory_GetMatchingFilesRecursively(s, sl, mask);
      se=GWEN_StringListEntry_Next(se);
    }
  }
  GWEN_StringList_free(folderList);
  GWEN_Buffer_free(pbuf);

  return 0;
}





