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



GWEN_DIRECTORY *GWEN_Directory_new(void)
{
  GWEN_DIRECTORY *d;

  GWEN_NEW_OBJECT(GWEN_DIRECTORY, d);
  return d;
}



void GWEN_Directory_free(GWEN_DIRECTORY *d)
{
  if (d) {
    if (d->handle!=INVALID_HANDLE_VALUE)
      FindClose(d->handle);
    d->handle=INVALID_HANDLE_VALUE;
    GWEN_FREE_OBJECT(d);
  }
}



int GWEN_Directory_Open(GWEN_DIRECTORY *d, const char *n)
{
  assert(d);
  assert(n);
  if ((strlen(n)+5)>=sizeof(d->pattern)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Directory name too long");
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  strcpy(d->pattern, n);
  strcat(d->pattern, "\\*.*");

  return GWEN_Directory_Rewind(d);;
}



int GWEN_Directory_Close(GWEN_DIRECTORY *d)
{
  int rv;

  rv=0;
  if (d->handle!=INVALID_HANDLE_VALUE)
    rv=!FindClose(d->handle);
  d->handle=INVALID_HANDLE_VALUE;
  return rv;
}



int GWEN_Directory_Read(GWEN_DIRECTORY *d,
                        char *buffer,
                        unsigned int len)
{
  WIN32_FIND_DATA wd;

  assert(d);
  assert(buffer);
  assert(len);
  assert(d->handle);

  if (d->lastName[0]==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No more entries");
    return GWEN_ERROR_NOT_FOUND;
  }

  /* copy existing entry */
  if ((strlen(d->lastName)>=len)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  strcpy(buffer, d->lastName);

  /* read next entry */
  d->lastName[0]=0;
  if (FindNextFile(d->handle, &wd)) {
    if ((strlen(wd.cFileName)+1)>=sizeof(d->lastName)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Entry too long");
    }

    strcpy(d->lastName, wd.cFileName);
  }
  return 0;
}



int GWEN_Directory_Rewind(GWEN_DIRECTORY *d)
{
  WIN32_FIND_DATA wd;

  assert(d);
  d->handle=FindFirstFile(d->pattern, &wd);
  if (d->handle==INVALID_HANDLE_VALUE) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "No entry for \"%s\"", d->pattern);
    return GWEN_ERROR_NOT_FOUND;
  }
  if ((strlen(wd.cFileName)+1)>=sizeof(d->lastName)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry name too long");
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  strcpy(d->lastName, wd.cFileName);
  return 0;
}



int GWEN_Directory_Create(const char *path)
{

  if (_mkdir(path)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error on _mkdir(%s): %s",
             path, strerror(errno));
    return -1;
  }
  return 0;
}



int GWEN_Directory_CreatePublic(const char *path)
{
  /* same as above, since on WIN32 all folders are public (gulp) */
  return GWEN_Directory_Create(path);
}


/** Returns TRUE (nonzero) if the given path is an absolute one. */
static int path_is_absolute(const char *path)
{
  return path &&
         ((path[0] == '\\') ||
          (path[0] == '/') ||
          ((strlen(path) > 2) && (path[1] == ':')));
}

int GWEN_Directory_GetHomeDirectory(char *buffer, unsigned int size)
{
  int rv;
  char *home_dir;

  /* Taken from
     http://svn.gnome.org/viewcvs/glib/trunk/glib/gutils.c, see
     the lookup of the g_home_dir variable in
     g_get_any_init_do(). */

  /* We check $HOME first for Win32, though it is a last resort for Unix
   * where we prefer the results of getpwuid().
   */
  home_dir = getenv("HOME");

  /* Only believe HOME if it is an absolute path and exists */
  if (home_dir) {
    if (!(path_is_absolute(home_dir)
          /* && g_file_test (home_dir, G_FILE_TEST_IS_DIR) */
         )) {
      home_dir = NULL;
    }
  }

  if (!home_dir) {
    /* USERPROFILE is probably the closest equivalent to $HOME? */
    if (getenv("USERPROFILE") != NULL)
      home_dir = getenv("USERPROFILE");
  }

  /* Did we find any home_dir? Copy it to buffer. */
  if (home_dir) {
    char *p;

    rv = strlen(home_dir);
    strncpy(buffer, home_dir, size);

    /* In case HOME is Unix-style (it happens), convert it to
     * Windows style.
     */
    while ((p = strchr(buffer, '/')) != NULL)
      *p = '\\';
  }
  else {
    rv=GetWindowsDirectory(buffer, size);
    if (rv==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error on GetWindowsDirectory");
      return -1;
    }
  }

  if (rv>=size) {
    DBG_INFO(GWEN_LOGDOMAIN, "Buffer too small");
    return -1;
  }
  return 0;
}



int GWEN_Directory_GetPrefixDirectory(char *buffer, unsigned int size)
{
  DWORD rv;
  char *p;
  char cwd[256];

  /* Get the absolute path to the executable, including its name */
  rv=GetModuleFileName(NULL, cwd, sizeof(cwd)-1);
  if (rv==0) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "GetModuleFileName(): %d",
              (int)GetLastError());
    return GWEN_ERROR_IO;
  }

  /* Find the last DIRSEP and set it to NULL so that we now have the
     bindir. */
  p=strrchr(cwd, '\\');
  if (p) {
    *p=0;
  }

  /* Find again the last DIRSEP to check whether the path ends in
     "bin" or "lib". */
  p=strrchr(cwd, '\\');
  if (p) {
    /* DIRSEP was found and p points to it. p+1 points either to the
       rest of the string or the '\0' byte, so we can use it
       here. */
    if ((strcmp(p+1, "bin") == 0) || (strcmp(p+1, "lib") == 0)) {
      /* The path ends in "bin" or "lib", hence we strip that suffix
      so that we now only have the prefix. */
      *p=0;
    }
  }

  if ((strlen(cwd)+1)>=size)
    return GWEN_ERROR_BUFFER_OVERFLOW;

  strcpy(buffer, cwd);
  return 0;
}






