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

#define DISABLE_DEBUGLOG


#include "syncio_p.h"
#include "syncio_file.h"
#include "syncio_buffered.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>



GWEN_INHERIT_FUNCTIONS(GWEN_SYNCIO)
GWEN_LIST_FUNCTIONS(GWEN_SYNCIO, GWEN_SyncIo)




GWEN_SYNCIO *GWEN_SyncIo_new(const char *typeName, GWEN_SYNCIO *baseIo)
{
  GWEN_SYNCIO *sio;

  assert(typeName);
  GWEN_NEW_OBJECT(GWEN_SYNCIO, sio);
  sio->refCount=1;
  GWEN_INHERIT_INIT(GWEN_SYNCIO, sio);
  GWEN_LIST_INIT(GWEN_SYNCIO, sio);

  sio->typeName=strdup(typeName);
  sio->baseIo=baseIo;

  return sio;
}



void GWEN_SyncIo_Attach(GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  sio->refCount++;
}



void GWEN_SyncIo_free(GWEN_SYNCIO *sio)
{
  if (sio) {
    assert(sio->refCount);
    if (sio->refCount==1) {
      GWEN_LIST_FINI(GWEN_SYNCIO, sio);
      GWEN_INHERIT_FINI(GWEN_SYNCIO, sio);
      GWEN_SyncIo_free(sio->baseIo);
      free(sio->typeName);
      sio->refCount=0;
      GWEN_FREE_OBJECT(sio);
    }
    else
      sio->refCount--;
  }
}



int GWEN_SyncIo_Connect(GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  if (sio->connectFn)
    return sio->connectFn(sio);
  else
    return 0;
}



int GWEN_SyncIo_Disconnect(GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  if (sio->disconnectFn)
    return sio->disconnectFn(sio);
  else
    return 0;
}



int GWEN_SyncIo_Flush(GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  if (sio->flushFn)
    return sio->flushFn(sio);
  else
    return 0;
}



int GWEN_SyncIo_Read(GWEN_SYNCIO *sio,
                     uint8_t *buffer,
                     uint32_t size)
{
  assert(sio);
  assert(sio->refCount);
  if (sio->readFn)
    return sio->readFn(sio, buffer, size);
  else
    return GWEN_ERROR_EOF;
}



int GWEN_SyncIo_Write(GWEN_SYNCIO *sio,
                      const uint8_t *buffer,
                      uint32_t size)
{
  assert(sio);
  assert(sio->refCount);
  if (sio->writeFn)
    return sio->writeFn(sio, buffer, size);
  else
    return GWEN_ERROR_BROKEN_PIPE;
}



uint32_t GWEN_SyncIo_GetFlags(const GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  return sio->flags;
}



void GWEN_SyncIo_SetFlags(GWEN_SYNCIO *sio, uint32_t fl)
{
  assert(sio);
  assert(sio->refCount);
  sio->flags=fl;
}



void GWEN_SyncIo_AddFlags(GWEN_SYNCIO *sio, uint32_t fl)
{
  assert(sio);
  assert(sio->refCount);
  sio->flags|=fl;
}



void GWEN_SyncIo_SubFlags(GWEN_SYNCIO *sio, uint32_t fl)
{
  assert(sio);
  assert(sio->refCount);
  sio->flags&=~fl;
}



GWEN_SYNCIO_STATUS GWEN_SyncIo_GetStatus(const GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  return sio->status;
}



void GWEN_SyncIo_SetStatus(GWEN_SYNCIO *sio, GWEN_SYNCIO_STATUS st)
{
  assert(sio);
  assert(sio->refCount);
  sio->status=st;
}



const char *GWEN_SyncIo_GetTypeName(const GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  return sio->typeName;
}



GWEN_SYNCIO *GWEN_SyncIo_GetBaseIo(const GWEN_SYNCIO *sio)
{
  assert(sio);
  assert(sio->refCount);
  return sio->baseIo;
}



GWEN_SYNCIO *GWEN_SyncIo_GetBaseIoByTypeName(const GWEN_SYNCIO *sio, const char *typeName)
{
  GWEN_SYNCIO *baseIo;

  assert(sio);
  assert(sio->refCount);

  baseIo=sio->baseIo;
  while (baseIo) {
    if (baseIo->typeName && strcasecmp(baseIo->typeName, typeName)==0)
      return baseIo;
    baseIo=baseIo->baseIo;
  }

  return NULL;
}



GWEN_SYNCIO_CONNECT_FN GWEN_SyncIo_SetConnectFn(GWEN_SYNCIO *sio, GWEN_SYNCIO_CONNECT_FN fn)
{
  GWEN_SYNCIO_CONNECT_FN of;

  assert(sio);
  assert(sio->refCount);
  of=sio->connectFn;
  sio->connectFn=fn;
  return of;
}



GWEN_SYNCIO_DISCONNECT_FN GWEN_SyncIo_SetDisconnectFn(GWEN_SYNCIO *sio, GWEN_SYNCIO_DISCONNECT_FN fn)
{
  GWEN_SYNCIO_DISCONNECT_FN of;

  assert(sio);
  assert(sio->refCount);
  of=sio->disconnectFn;
  sio->disconnectFn=fn;
  return of;
}



GWEN_SYNCIO_FLUSH_FN GWEN_SyncIo_SetFlushFn(GWEN_SYNCIO *sio, GWEN_SYNCIO_FLUSH_FN fn)
{
  GWEN_SYNCIO_FLUSH_FN of;

  assert(sio);
  assert(sio->refCount);
  of=sio->flushFn;
  sio->flushFn=fn;
  return of;
}



GWEN_SYNCIO_READ_FN GWEN_SyncIo_SetReadFn(GWEN_SYNCIO *sio, GWEN_SYNCIO_READ_FN fn)
{
  GWEN_SYNCIO_READ_FN of;

  assert(sio);
  assert(sio->refCount);
  of=sio->readFn;
  sio->readFn=fn;
  return of;
}



GWEN_SYNCIO_WRITE_FN GWEN_SyncIo_SetWriteFn(GWEN_SYNCIO *sio, GWEN_SYNCIO_WRITE_FN fn)
{
  GWEN_SYNCIO_WRITE_FN of;

  assert(sio);
  assert(sio->refCount);
  of=sio->writeFn;
  sio->writeFn=fn;
  return of;
}



int GWEN_SyncIo_WriteForced(GWEN_SYNCIO *sio,
                            const uint8_t *buffer,
                            uint32_t size)
{
  if (size==0) {
    int rv;

    do {
      rv=GWEN_SyncIo_Write(sio, buffer, size);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    return 0;
  }
  else {
    uint32_t todo;

    todo=size;
    while (todo) {
      int rv;

      do {
        rv=GWEN_SyncIo_Write(sio, buffer, todo);
      }
      while (rv==GWEN_ERROR_INTERRUPTED);

      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      todo-=rv;
      buffer+=rv;
    }

    return size;
  }
}



int GWEN_SyncIo_ReadForced(GWEN_SYNCIO *sio,
                           uint8_t *buffer,
                           uint32_t size)
{
  uint32_t todo;

  todo=size;
  while (todo) {
    int rv;

    do {
      rv=GWEN_SyncIo_Read(sio, buffer, todo);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv==0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "EOF met");
      return GWEN_ERROR_EOF;
    }
    todo-=rv;
    buffer+=rv;
  }

  return size;
}



int GWEN_SyncIo_WriteString(GWEN_SYNCIO *sio, const char *s)
{
  int rv;

  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t *) s, s?strlen(s):0);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_SyncIo_WriteLine(GWEN_SYNCIO *sio, const char *s)
{
  int rv;

  rv=GWEN_SyncIo_WriteString(sio, s);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_DOSMODE)
    rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t *) "\r\n", 2);
  else
    rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t *) "\n", 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_SyncIo_WriteChar(GWEN_SYNCIO *sio, char s)
{
  return GWEN_SyncIo_WriteForced(sio, (const uint8_t *) &s, 1);
}



int GWEN_SyncIo_Helper_ReadFileToStringList(const char *fname,
                                            int maxLines,
                                            GWEN_STRINGLIST *sl)
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseSio;
  int rv;

  /* open checksums from file */
  baseSio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_SetFlags(baseSio, GWEN_SYNCIO_FILE_FLAGS_READ);
  sio=GWEN_SyncIo_Buffered_new(baseSio);

  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not open file [%s]", fname?fname:"<no filename>");
    GWEN_SyncIo_free(sio);
    return rv;
  }

  /* read up to maxlines lines from file */
  rv=GWEN_SyncIo_Buffered_ReadLinesToStringList(sio, maxLines, sl);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not open file [%s]", fname?fname:"<no filename>");
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  /* close file */
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);
  return 0;
}




int GWEN_SyncIo_Helper_PartiallyReadFile(const char *fName, uint8_t *buffer, uint32_t size)
{
  GWEN_SYNCIO *sio;
  uint32_t todo;
  int rv;

  /* open file */
  sio=GWEN_SyncIo_File_new(fName, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_SetFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);

  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not open file [%s]", fName?fName:"<no filename>");
    GWEN_SyncIo_free(sio);
    return rv;
  }

  /* read file */
  todo=size;
  while (todo) {
    do {
      rv=GWEN_SyncIo_Read(sio, buffer, todo);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return rv;
    }
    else if (rv==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
      break;
    }
    todo-=rv;
    buffer+=rv;
  }

  /* close file */
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return size-todo;
}



int GWEN_SyncIo_Helper_ReadFile(const char *fName, GWEN_BUFFER *dbuf)
{
  GWEN_SYNCIO *sio;
  int rv;
  int bytesRead=0;
  int64_t fileSize=0;

  /* open file */
  sio=GWEN_SyncIo_File_new(fName, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_SetFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);

  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not open file [%s]", fName?fName:"<no filename>");
    GWEN_SyncIo_free(sio);
    return rv;
  }

  fileSize=GWEN_SyncIo_File_Seek(sio, 0, GWEN_SyncIo_File_Whence_End);
  GWEN_SyncIo_File_Seek(sio, 0, GWEN_SyncIo_File_Whence_Set);
  if (fileSize>GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf))
    GWEN_Buffer_AllocRoom(dbuf, (uint32_t) fileSize);

  /* read file */
  while (1) {
    uint32_t l;
    uint8_t *p;

    GWEN_Buffer_AllocRoom(dbuf, 4096);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
    p=(uint8_t *) GWEN_Buffer_GetPosPointer(dbuf);

    do {
      rv=GWEN_SyncIo_Read(sio, p, l);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return rv;
    }
    else if (rv==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
      break;
    }
    bytesRead+=rv;

    GWEN_Buffer_IncrementPos(dbuf, rv);
    GWEN_Buffer_AdjustUsedBytes(dbuf);
  }

  /* close file */
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return bytesRead;
}



int GWEN_SyncIo_Helper_WriteFile(const char *fName, const uint8_t *ptrSource, uint64_t lenSource)
{
  GWEN_SYNCIO *sio;
  int rv;
  int64_t bytesWritten=0;
  int64_t bytesLeft;

  /* open file */
  sio=GWEN_SyncIo_File_new(fName, GWEN_SyncIo_File_CreationMode_CreateNew);
  GWEN_SyncIo_SetFlags(sio,
                       GWEN_SYNCIO_FILE_FLAGS_WRITE | GWEN_SYNCIO_FILE_FLAGS_READ |
                       GWEN_SYNCIO_FILE_FLAGS_UREAD | GWEN_SYNCIO_FILE_FLAGS_UWRITE);

  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not open file [%s]", fName?fName:"<no filename>");
    GWEN_SyncIo_free(sio);
    return rv;
  }

  /* write file */
  bytesLeft=lenSource;
  while (bytesLeft>0) {
    do {
      rv=GWEN_SyncIo_Write(sio, ptrSource, bytesLeft);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return rv;
    }
    else if (rv==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Nothing written");
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return GWEN_ERROR_IO;
    }
    ptrSource+=rv;
    bytesWritten+=rv;
    bytesLeft-=rv;
  }

  /* close file */
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return bytesWritten;
}



int GWEN_SyncIo_Helper_CopyFile(const char *srcPath, const char *destPath)
{
  GWEN_SYNCIO *sioRead;
  GWEN_SYNCIO *sioWrite;
  int rv;
  int bytesRead=0;

  sioRead=GWEN_SyncIo_File_new(srcPath, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_SetFlags(sioRead, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sioRead);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not open file [%s]", srcPath?srcPath:"<no filename>");
    GWEN_SyncIo_free(sioRead);
    return rv;
  }

  sioWrite=GWEN_SyncIo_File_new(destPath, GWEN_SyncIo_File_CreationMode_TruncateExisting);
  GWEN_SyncIo_SetFlags(sioWrite, GWEN_SYNCIO_FILE_FLAGS_WRITE);
  rv=GWEN_SyncIo_Connect(sioWrite);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not create destination file [%s]", destPath?destPath:"<no filename>");
    GWEN_SyncIo_free(sioWrite);
    GWEN_SyncIo_Disconnect(sioRead);
    GWEN_SyncIo_free(sioRead);
    return rv;
  }

  /* read file */
  while (1) {
    uint8_t transferBuffer[1024];
    uint32_t l;
    uint8_t *p;

    p=transferBuffer;
    l=sizeof(transferBuffer);
    do {
      rv=GWEN_SyncIo_Read(sioRead, p, l);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sioWrite);
      GWEN_SyncIo_free(sioWrite);
      GWEN_SyncIo_Disconnect(sioRead);
      GWEN_SyncIo_free(sioRead);
      return rv;
    }
    else if (rv==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
      break;
    }
    bytesRead+=rv;

    rv=GWEN_SyncIo_WriteForced(sioWrite, transferBuffer, rv);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sioWrite);
      GWEN_SyncIo_free(sioWrite);
      GWEN_SyncIo_Disconnect(sioRead);
      GWEN_SyncIo_free(sioRead);
      return rv;
    }
  }

  /* close file */
  rv=GWEN_SyncIo_Disconnect(sioWrite);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sioWrite);
    GWEN_SyncIo_free(sioWrite);
    GWEN_SyncIo_Disconnect(sioRead);
    GWEN_SyncIo_free(sioRead);
    return rv;
  }

  GWEN_SyncIo_free(sioWrite);
  GWEN_SyncIo_Disconnect(sioRead);
  GWEN_SyncIo_free(sioRead);

  return bytesRead;
}




