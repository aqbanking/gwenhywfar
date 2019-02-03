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




GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_FILE)



GWEN_SYNCIO *GWEN_SyncIo_File_new(const char *path, GWEN_SYNCIO_FILE_CREATIONMODE cm)
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_FILE *xio;

  assert(path);
  sio=GWEN_SyncIo_new(GWEN_SYNCIO_FILE_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_FILE, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio, xio, GWEN_SyncIo_File_FreeData);

  xio->creationMode=cm;
  xio->path=strdup(path);
  xio->fh=INVALID_HANDLE_VALUE;

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_File_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_File_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_File_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_File_Write);

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromHandle(HANDLE hd)
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_FILE *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_FILE_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_FILE, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio, xio, GWEN_SyncIo_File_FreeData);

  xio->fh=hd;

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_File_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_File_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_File_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_File_Write);

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStdHandle(HANDLE hd, const char *hname)
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_FILE *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_FILE_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_FILE, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio, xio, GWEN_SyncIo_File_FreeData);

  xio->path=strdup(hname);
  xio->fh=hd;
  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_File_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_File_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_File_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_File_Write);

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStdin(void)
{
  return GWEN_SyncIo_File_fromStdHandle(GetStdHandle(STD_INPUT_HANDLE), "stdin");
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStdout(void)
{
  return GWEN_SyncIo_File_fromStdHandle(GetStdHandle(STD_OUTPUT_HANDLE), "stdout");
}



GWEN_SYNCIO *GWEN_SyncIo_File_fromStderr(void)
{
  return GWEN_SyncIo_File_fromStdHandle(GetStdHandle(STD_ERROR_HANDLE), "stderr");
}



const char *GWEN_SyncIo_File_GetPath(const GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  return xio->path;
}



void GWENHYWFAR_CB GWEN_SyncIo_File_FreeData(void *bp, void *p)
{
  GWEN_SYNCIO_FILE *xio;

  xio=(GWEN_SYNCIO_FILE *) p;
  free(xio->path);
  if (xio->fh!=INVALID_HANDLE_VALUE)
    CloseHandle(xio->fh);
  GWEN_FREE_OBJECT(xio);
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Connect(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_FILE *xio;
  uint32_t flags;
  DWORD dwDesiredAccess=0;
  DWORD dwFlagsAndAttrs=0;
  DWORD dwCreationDistribution;
  HANDLE fh;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fh!=INVALID_HANDLE_VALUE)
    /* already open */
    return 0;

  flags=GWEN_SyncIo_GetFlags(sio);
  if (flags & GWEN_SYNCIO_FILE_FLAGS_APPEND)
    dwDesiredAccess=FILE_APPEND_DATA;
  else {
    if (flags & GWEN_SYNCIO_FILE_FLAGS_READ)
      dwDesiredAccess|=GENERIC_READ;
    if (flags & GWEN_SYNCIO_FILE_FLAGS_WRITE)
      dwDesiredAccess|=GENERIC_WRITE;
  }

  dwFlagsAndAttrs=FILE_ATTRIBUTE_NORMAL;
  if (flags & GWEN_SYNCIO_FILE_FLAGS_RANDOM)
    dwFlagsAndAttrs|=FILE_FLAG_RANDOM_ACCESS;

  switch (xio->creationMode) {
  case GWEN_SyncIo_File_CreationMode_OpenExisting:
    dwCreationDistribution=OPEN_EXISTING;
    break;
  case GWEN_SyncIo_File_CreationMode_CreateNew:
    dwCreationDistribution=CREATE_NEW;
    break;
  case GWEN_SyncIo_File_CreationMode_OpenAlways:
    dwCreationDistribution=OPEN_ALWAYS;
    break;
  case GWEN_SyncIo_File_CreationMode_CreateAlways:
    dwCreationDistribution=CREATE_ALWAYS;
    break;
  case GWEN_SyncIo_File_CreationMode_TruncateExisting:
    dwCreationDistribution=TRUNCATE_EXISTING;
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid creation mode %d", xio->creationMode);
    return GWEN_ERROR_INVALID;
  }

  fh=CreateFile(xio->path,
                dwDesiredAccess,                    /* dwDesiredAccess */
                FILE_SHARE_READ | FILE_SHARE_WRITE, /* dwShareMode */
                NULL,                               /* pSecurityAttributes */
                dwCreationDistribution,             /* dwCreationDistribution */
                dwFlagsAndAttrs,                    /* dwFlagsAndAttrs */
                NULL);                              /* hTemplateFile */
  if (fh==INVALID_HANDLE_VALUE) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Error opening file[%s]: %d",
              xio->path,
              (int)GetLastError());
    return GWEN_ERROR_IO;
  }

  xio->fh=fh;
  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);
  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Disconnect(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fh!=INVALID_HANDLE_VALUE) {
    if (!(GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_DONTCLOSE)) {
      if (!CloseHandle(xio->fh)) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Error closing file[%s]: %d",
                  xio->path,
                  (int)GetLastError());
        xio->fh=INVALID_HANDLE_VALUE;
        return GWEN_ERROR_IO;
      }
    }
    xio->fh=INVALID_HANDLE_VALUE;
  }

  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Read(GWEN_SYNCIO *sio,
                                        uint8_t *buffer,
                                        uint32_t size)
{
  GWEN_SYNCIO_FILE *xio;
  DWORD bytesRead=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fh==INVALID_HANDLE_VALUE) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File is not open");
    return GWEN_ERROR_NOT_CONNECTED;
  }


  if (!ReadFile(xio->fh, buffer, size, &bytesRead, NULL)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on ReadFile(%s): %d",
              xio->path,
              (int)GetLastError());
    return GWEN_ERROR_IO;
  }

  return (int) bytesRead;
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Write(GWEN_SYNCIO *sio,
                                         const uint8_t *buffer,
                                         uint32_t size)
{
  GWEN_SYNCIO_FILE *xio;
  DWORD bytesWritten=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fh==INVALID_HANDLE_VALUE) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File is not open");
    return GWEN_ERROR_NOT_CONNECTED;
  }


  if (!WriteFile(xio->fh, buffer, size, &bytesWritten, NULL)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on WriteFile(%s): %d",
              xio->path,
              (int)GetLastError());
    return GWEN_ERROR_IO;
  }

  return (int) bytesWritten;
}



int64_t GWEN_SyncIo_File_Seek(GWEN_SYNCIO *sio, int64_t pos, GWEN_SYNCIO_FILE_WHENCE whence)
{
  GWEN_SYNCIO_FILE *xio;
  DWORD w;
  LARGE_INTEGER li;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(xio);

  if (xio->fh==INVALID_HANDLE_VALUE) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File is not open");
    return GWEN_ERROR_NOT_CONNECTED;
  }

  switch (whence) {
  case GWEN_SyncIo_File_Whence_Set:
    w=FILE_BEGIN;
    break;
  case GWEN_SyncIo_File_Whence_Current:
    w=FILE_CURRENT;
    break;
  case GWEN_SyncIo_File_Whence_End:
    w=FILE_END;
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid seek mode (%d)", whence);
    return GWEN_ERROR_INVALID;
  }

  li.QuadPart = pos;
  li.LowPart=SetFilePointer(xio->fh,
                            li.LowPart,
                            &li.HighPart,
                            w);
  if (li.LowPart==INVALID_SET_FILE_POINTER) {
    DWORD rv;

    rv=GetLastError();
    if (rv!=NO_ERROR) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error seeking in file[%s] at %llu: %d",
                xio->path,
                (unsigned long long) pos,
                (int)rv);
      return GWEN_ERROR_IO;
    }
  }
  return (int64_t) li.QuadPart;
}



