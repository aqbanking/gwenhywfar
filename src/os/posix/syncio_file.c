/***************************************************************************
 begin       : Tue Apr 27 2010
 copyright   : (C) 2010 by Martin Preuss
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



#include "syncio_file_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>




GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_FILE)



GWEN_SYNCIO *GWEN_SyncIo_File_new(const char *path, GWEN_SYNCIO_FILE_CREATIONMODE cm) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_FILE *xio;

  assert(path);
  sio=GWEN_SyncIo_new(GWEN_SYNCIO_FILE_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_FILE, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio, xio, GWEN_SyncIo_File_FreeData);

  xio->creationMode=cm;
  xio->path=strdup(path);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_File_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_File_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_File_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_File_Write);

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromFd(int fd) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_FILE *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_FILE_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_FILE, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio, xio, GWEN_SyncIo_File_FreeData);

  xio->fd=fd;

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_File_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_File_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_File_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_File_Write);

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStdHandle(int fd, const char *hname) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_FILE *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_FILE_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_FILE, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio, xio, GWEN_SyncIo_File_FreeData);

  xio->path=strdup(hname);
  xio->fd=fd;

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_File_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_File_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_File_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_File_Write);

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStdin() {
  return GWEN_SyncIo_File_fromStdHandle(fileno(stdin), "stdin");
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStdout() {
  return GWEN_SyncIo_File_fromStdHandle(fileno(stdout), "stdout");
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStderr() {
  return GWEN_SyncIo_File_fromStdHandle(fileno(stderr), "stderr");
}







const char *GWEN_SyncIo_File_GetPath(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  return xio->path;
}



void GWENHYWFAR_CB GWEN_SyncIo_File_FreeData(void *bp, void *p) {
  GWEN_SYNCIO_FILE *xio;

  xio=(GWEN_SYNCIO_FILE*) p;
  free(xio->path);
  GWEN_FREE_OBJECT(xio);
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Connect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_FILE *xio;
  int fd;
  uint32_t flags;
  uint32_t accflags;
  mode_t mode=0;
  int acc=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  flags=GWEN_SyncIo_GetFlags(sio);
  accflags=flags & (GWEN_SYNCIO_FILE_FLAGS_READ | GWEN_SYNCIO_FILE_FLAGS_WRITE);
  if (accflags==(GWEN_SYNCIO_FILE_FLAGS_READ | GWEN_SYNCIO_FILE_FLAGS_WRITE))
    acc=O_RDWR;
  else if (accflags==GWEN_SYNCIO_FILE_FLAGS_READ)
    acc=O_RDONLY;
  else if (accflags==GWEN_SYNCIO_FILE_FLAGS_WRITE)
    acc=O_WRONLY;

  if (flags & GWEN_SYNCIO_FILE_FLAGS_APPEND)
    acc|=O_APPEND;

#ifdef O_BINARY
  /* always assume binary mode */
  acc|=O_BINARY;
#endif

  if (flags & GWEN_SYNCIO_FILE_FLAGS_UREAD)
    mode|=S_IRUSR;
  if (flags & GWEN_SYNCIO_FILE_FLAGS_UWRITE)
    mode|=S_IWUSR;
  if (flags & GWEN_SYNCIO_FILE_FLAGS_UEXEC)
    mode|=S_IXUSR;

#ifdef S_IRGRP
  if (flags & GWEN_SYNCIO_FILE_FLAGS_GREAD)
    mode|=S_IRGRP;
#endif
#ifdef S_IWGRP
  if (flags & GWEN_SYNCIO_FILE_FLAGS_GWRITE)
    mode|=S_IWGRP;
#endif
#ifdef S_IXGRP
  if (flags & GWEN_SYNCIO_FILE_FLAGS_GEXEC)
    mode|=S_IXGRP;
#endif

#ifdef S_IROTH
  if (flags & GWEN_SYNCIO_FILE_FLAGS_OREAD)
    mode|=S_IROTH;
#endif
#ifdef S_IWOTH
  if (flags & GWEN_SYNCIO_FILE_FLAGS_OWRITE)
    mode|=S_IWOTH;
#endif
#ifdef S_IXOTH
  if (flags & GWEN_SYNCIO_FILE_FLAGS_OEXEC)
    mode|=S_IXOTH;
#endif

  switch(xio->creationMode) {
  case GWEN_SyncIo_File_CreationMode_OpenExisting:
    fd=open(xio->path, acc);
    break;
  case GWEN_SyncIo_File_CreationMode_CreateNew:
    fd=open(xio->path, acc | O_CREAT | O_EXCL, mode);
    break;
  case GWEN_SyncIo_File_CreationMode_OpenAlways:
    fd=open(xio->path, acc | O_CREAT, mode);
    break;
  case GWEN_SyncIo_File_CreationMode_CreateAlways:
    fd=open(xio->path, acc | O_CREAT | O_TRUNC, mode);
    break;
  case GWEN_SyncIo_File_CreationMode_TruncateExisting:
    fd=open(xio->path, acc | O_TRUNC, mode);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid creation mode %d", xio->creationMode);
    fd=-1;
    break;
  }

  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "open(%s, %d): %s", xio->path, xio->creationMode, strerror(errno));
    switch(errno) {
    case EEXIST: return GWEN_ERROR_FOUND;
    case EACCES: return GWEN_ERROR_PERMISSIONS;
    case ENOENT: return GWEN_ERROR_NOT_FOUND;
    default:     return GWEN_ERROR_IO;
    }
  }

  xio->fd=fd;
  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);

  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Disconnect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_FILE *xio;
  int rv=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File (%s) not open", xio->path);
    return GWEN_ERROR_NOT_OPEN;
  }

  if (!(GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_DONTCLOSE)) {
    do {
      rv=close(xio->fd);
    } while (rv==-1 && errno==EINTR);

    if (rv==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "close(%s): %s", xio->path, strerror(errno));
      switch(errno) {
      case ENOSPC: return GWEN_ERROR_MEMORY_FULL;
      default:     return GWEN_ERROR_IO;
      }
    }
  }

  xio->fd=-1;

  return (int)rv;
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Read(GWEN_SYNCIO *sio,
					uint8_t *buffer,
					uint32_t size) {
  GWEN_SYNCIO_FILE *xio;
  ssize_t rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File (%s) not open", xio->path);
    return GWEN_ERROR_NOT_OPEN;
  }

  do {
    rv=read(xio->fd, buffer, size);
  } while (rv==-1 && errno==EINTR);

  if (rv==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "read(%s, %lu): %s", xio->path, (long unsigned int) size, strerror(errno));
    return GWEN_ERROR_IO;
  }

  return (int)rv;
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Write(GWEN_SYNCIO *sio,
					 const uint8_t *buffer,
					 uint32_t size) {
  GWEN_SYNCIO_FILE *xio;
  ssize_t rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File (%s) not open", xio->path);
    return GWEN_ERROR_NOT_OPEN;
  }

  do {
    rv=write(xio->fd, buffer, size);
  } while (rv==-1 && errno==EINTR);

  if (rv==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "write(%s, %lu): %s", xio->path, (long unsigned int) size, strerror(errno));
    switch(errno) {
    case ENOSPC: return GWEN_ERROR_MEMORY_FULL;
    default:     return GWEN_ERROR_IO;
    }
  }

  return (int)rv;
}



int64_t GWEN_SyncIo_File_Seek(GWEN_SYNCIO *sio, int64_t pos, GWEN_SYNCIO_FILE_WHENCE whence) {
  GWEN_SYNCIO_FILE *xio;
  off_t rv;
  int w;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  switch(whence) {
  case GWEN_SyncIo_File_Whence_Set:     w=SEEK_SET; break;
  case GWEN_SyncIo_File_Whence_Current: w=SEEK_CUR; break;
  case GWEN_SyncIo_File_Whence_End:     w=SEEK_END; break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid seek mode (%d)", whence);
    return GWEN_ERROR_INVALID;
  }

  rv=lseek(xio->fd, pos, w);
  if (rv==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "lseek(%s, %lli): %s",
	      xio->path, (long long int) pos, strerror(errno));
    return GWEN_ERROR_IO;
  }
  return rv;
}






