/***************************************************************************
 begin       : Wed Jun 22 2011
 copyright   : (C) 2011 by Martin Preuss
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


#include "sar_p.h"
#include "gwen_sar_fileheader_l.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/tlv.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/cryptmgrkeys.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>



GWEN_SAR *GWEN_Sar_new(void) {
  GWEN_SAR *sr;

  GWEN_NEW_OBJECT(GWEN_SAR, sr);
  sr->refCount=1;

  sr->headers=GWEN_SarFileHeader_List_new();

  return sr;
}



void GWEN_Sar_Attach(GWEN_SAR *sr) {
  assert(sr);
  assert(sr->refCount);
  sr->refCount++;
}



void GWEN_Sar_free(GWEN_SAR *sr) {
  if (sr) {
    assert(sr->refCount);
    if (sr->refCount==1) {
      free(sr->archiveName);
      GWEN_SarFileHeader_List_free(sr->headers);
      GWEN_SyncIo_free(sr->archiveSio);
      sr->refCount=0;
      GWEN_FREE_OBJECT(sr);
    }
    else {
      sr->refCount--;
    }
  }
}



int GWEN_Sar_CreateArchive(GWEN_SAR *sr, const char *aname) {
  GWEN_SYNCIO *sio;
  int rv;

  assert(sr);
  assert(sr->refCount);

  assert(aname);
  if (sr->openMode!=GWEN_Sar_OpenMode_Closed) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Archive already open");
    return GWEN_ERROR_OPEN;
  }

  free(sr->archiveName);
  sr->archiveName=strdup(aname);
  sio=GWEN_SyncIo_File_new(aname, GWEN_SyncIo_File_CreationMode_CreateAlways);
  GWEN_SyncIo_AddFlags(sio,
                       GWEN_SYNCIO_FILE_FLAGS_READ |
                       GWEN_SYNCIO_FILE_FLAGS_WRITE |
                       GWEN_SYNCIO_FILE_FLAGS_UREAD |
                       GWEN_SYNCIO_FILE_FLAGS_UWRITE |
                       GWEN_SYNCIO_FILE_FLAGS_GREAD |
                       GWEN_SYNCIO_FILE_FLAGS_GWRITE);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  sr->archiveSio=sio;
  sr->openMode=GWEN_Sar_OpenMode_Created;

  sr->signaturePos=0;
  sr->signatureSize=0;

  return 0;
}



int GWEN_Sar_OpenArchive(GWEN_SAR *sr, const char *aname,
                         GWEN_SYNCIO_FILE_CREATIONMODE cm,
                         uint32_t acc) {
  GWEN_SYNCIO *sio;
  int rv;

  assert(sr);
  assert(sr->refCount);

  assert(aname);
  if (sr->openMode!=GWEN_Sar_OpenMode_Closed) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Archive already open");
    return GWEN_ERROR_OPEN;
  }

  free(sr->archiveName);
  sr->archiveName=strdup(aname);
  sio=GWEN_SyncIo_File_new(aname, cm);
  GWEN_SyncIo_AddFlags(sio, acc);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  sr->archiveSio=sio;
  sr->openMode=GWEN_Sar_OpenMode_Opened;

  sr->signaturePos=0;
  sr->signatureSize=0;

  rv=GWEN_Sar_ScanFile(sr);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    sr->archiveSio=NULL;
    sr->openMode=GWEN_Sar_OpenMode_Closed;
    GWEN_SyncIo_free(sio);
    return rv;
  }

  return 0;
}



int GWEN_Sar_CloseArchive(GWEN_SAR *sr, int abandon) {
  int rv;

  assert(sr);
  assert(sr->refCount);

  if (sr->openMode!=GWEN_Sar_OpenMode_Opened &&
      sr->openMode!=GWEN_Sar_OpenMode_Created) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Archive not open");
    return GWEN_ERROR_NOT_OPEN;
  }

  if (!abandon) {
    /* flush */
    rv=GWEN_SyncIo_Flush(sr->archiveSio);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sr->archiveSio);
      GWEN_SyncIo_free(sr->archiveSio);
      sr->archiveSio=NULL;
      free(sr->archiveName);
      sr->archiveName=NULL;
      sr->openMode=GWEN_Sar_OpenMode_Closed;
      return rv;
    }
  }

  /* disconnect */
  rv=GWEN_SyncIo_Disconnect(sr->archiveSio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sr->archiveSio);
    sr->archiveSio=NULL;
    free(sr->archiveName);
    sr->archiveName=NULL;
    sr->openMode=GWEN_Sar_OpenMode_Closed;
    return rv;
  }

  GWEN_SyncIo_free(sr->archiveSio);
  sr->archiveSio=NULL;
  free(sr->archiveName);
  sr->archiveName=NULL;
  sr->openMode=GWEN_Sar_OpenMode_Closed;
  return 0;
}



int GWEN_Sar_FileHeaderToTlv(const GWEN_SAR_FILEHEADER *fh, GWEN_BUFFER *tbuf) {
  const char *s;
  uint16_t v8;
  uint32_t v32;
  uint64_t v64;
  uint8_t hbuf[8];
  const GWEN_TIME *ti;
  int rv;

  /* header version */
  v32=GWEN_SAR_HEADER_VERSION;
  hbuf[0]=(v32>>24) & 0xff;
  hbuf[1]=(v32>>16) & 0xff;
  hbuf[2]=(v32>>8) & 0xff;
  hbuf[3]=v32 & 0xff;
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_VERSION, 0x00, hbuf, 4, 1, tbuf);

  /* status */
  v8=GWEN_SarFileHeader_GetStatus(fh) & 0xff;
  hbuf[0]=v8;
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_STATUS, 0x00, hbuf, 1, 1, tbuf);

  /* flags */
  v32=GWEN_SarFileHeader_GetFlags(fh);
  hbuf[0]=(v32>>24) & 0xff;
  hbuf[1]=(v32>>16) & 0xff;
  hbuf[2]=(v32>>8) & 0xff;
  hbuf[3]=v32 & 0xff;
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_FLAGS, 0x00, hbuf, 4, 1, tbuf);

  /* path */
  s=GWEN_SarFileHeader_GetPath(fh);
  if (s && *s)
    GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_PATH, 0x00, s, strlen(s)+1, 1, tbuf);

  /* file type */
  v8=GWEN_SarFileHeader_GetFileType(fh) & 0xff;
  hbuf[0]=v8;
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_TYPE, 0x00, hbuf, 1, 1, tbuf);

  /* permissions */
  v32=GWEN_SarFileHeader_GetPermissions(fh);
  hbuf[0]=(v32>>24) & 0xff;
  hbuf[1]=(v32>>16) & 0xff;
  hbuf[2]=(v32>>8) & 0xff;
  hbuf[3]=v32 & 0xff;
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_PERM, 0x00, hbuf, 4, 1, tbuf);

  /* atime */
  ti=GWEN_SarFileHeader_GetAtime(fh);
  if (ti) {
    GWEN_BUFFER *xbuf;

    xbuf=GWEN_Buffer_new(0, 32, 0, 1);
    rv=GWEN_Time_toUtcString(ti, "YYYYMMDDhhmmss", xbuf);
    if (rv>=0)
      GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_ATIME, 0x00,
                                GWEN_Buffer_GetStart(xbuf),
                                GWEN_Buffer_GetUsedBytes(xbuf),
                                1,
                                tbuf);
    GWEN_Buffer_free(xbuf);
  }

  /* mtime */
  ti=GWEN_SarFileHeader_GetMtime(fh);
  if (ti) {
    GWEN_BUFFER *xbuf;

    xbuf=GWEN_Buffer_new(0, 32, 0, 1);
    rv=GWEN_Time_toUtcString(ti, "YYYYMMDDhhmmss", xbuf);
    if (rv>=0)
      GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_MTIME, 0x00,
                                GWEN_Buffer_GetStart(xbuf),
                                GWEN_Buffer_GetUsedBytes(xbuf),
                                1,
                                tbuf);
    GWEN_Buffer_free(xbuf);
  }

  /* ctime */
  ti=GWEN_SarFileHeader_GetCtime(fh);
  if (ti) {
    GWEN_BUFFER *xbuf;

    xbuf=GWEN_Buffer_new(0, 32, 0, 1);
    rv=GWEN_Time_toUtcString(ti, "YYYYMMDDhhmmss", xbuf);
    if (rv>=0)
      GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_CTIME, 0x00,
                                GWEN_Buffer_GetStart(xbuf),
                                GWEN_Buffer_GetUsedBytes(xbuf),
                                1,
                                tbuf);
    GWEN_Buffer_free(xbuf);
  }

  /* file size */
  v64=GWEN_SarFileHeader_GetFileSize(fh);
  hbuf[0]=(v64>>56) & 0xff;
  hbuf[1]=(v64>>48) & 0xff;
  hbuf[2]=(v64>>40) & 0xff;
  hbuf[3]=(v64>>32) & 0xff;
  hbuf[4]=(v64>>24) & 0xff;
  hbuf[5]=(v64>>16) & 0xff;
  hbuf[6]=(v64>>8) & 0xff;
  hbuf[7]=v64 & 0xff;
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HEADER_SIZE, 0x00, hbuf, 8, 1, tbuf);

  return 0;
}



uint64_t GWEN_Sar_ReadUint64(const uint8_t *p, uint32_t bs) {
  uint64_t v=0;

  switch(bs) {
  case 8:
    v=(((uint64_t)(p[0]))<<56)+
      (((uint64_t)(p[1]))<<48)+
      (((uint64_t)(p[2]))<<40)+
      (((uint64_t)(p[3]))<<32)+
      (((uint64_t)(p[4]))<<24)+
      (((uint64_t)(p[5]))<<16)+
      (((uint64_t)(p[6]))<<8)+
      ((uint64_t)(p[7]));
    break;
  case 7:
    v=(((uint64_t)(p[0]))<<48)+
      (((uint64_t)(p[1]))<<40)+
      (((uint64_t)(p[2]))<<32)+
      (((uint64_t)(p[3]))<<24)+
      (((uint64_t)(p[4]))<<16)+
      (((uint64_t)(p[5]))<<8)+
      ((uint64_t)(p[6]));
    break;
  case 6:
    v=(((uint64_t)(p[0]))<<40)+
      (((uint64_t)(p[1]))<<32)+
      (((uint64_t)(p[2]))<<24)+
      (((uint64_t)(p[3]))<<16)+
      (((uint64_t)(p[4]))<<8)+
      ((uint64_t)(p[5]));
    break;
  case 5:
    v=(((uint64_t)(p[0]))<<32)+
      (((uint64_t)(p[1]))<<24)+
      (((uint64_t)(p[2]))<<16)+
      (((uint64_t)(p[3]))<<8)+
      ((uint64_t)(p[4]));
    break;
  case 4:
    v=(((uint64_t)(p[0]))<<24)+
      (((uint64_t)(p[1]))<<16)+
      (((uint64_t)(p[2]))<<8)+
      ((uint64_t)(p[3]));
    break;
  case 3:
    v=(((uint64_t)(p[0]))<<16)+
      (((uint64_t)(p[1]))<<8)+
      ((uint64_t)(p[2]));
    break;
  case 2:
    v=(((uint64_t)(p[0]))<<8)+
      ((uint64_t)(p[1]));
    break;
  case 1:
    v=((uint64_t)(p[0]));
    break;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unsupported size of %d for uint32", bs);
    break;
  }

  return v;
}



int GWEN_Sar_TlvToFileHeader(GWEN_BUFFER *mbuf, GWEN_SAR_FILEHEADER *fh) {
  while(GWEN_Buffer_GetBytesLeft(mbuf)) {
    GWEN_TLV *tlv;

    tlv=GWEN_TLV_fromBuffer(mbuf, 1);
    if (tlv) {
      const uint8_t *p;
      uint32_t bs;
      uint64_t v;
      GWEN_TIME *ti;

      p=GWEN_TLV_GetTagData(tlv);
      bs=GWEN_TLV_GetTagLength(tlv);
      switch(GWEN_TLV_GetTagType(tlv)) {
      case GWEN_SAR_TAG_HEADER_VERSION:
        v=GWEN_Sar_ReadUint64(p, bs);
        DBG_DEBUG(GWEN_LOGDOMAIN, "File Header Version: %08x", (unsigned int) (v & 0xffffffff));
        break;

      case GWEN_SAR_TAG_HEADER_STATUS:
        v=GWEN_Sar_ReadUint64(p, bs);
        GWEN_SarFileHeader_SetStatus(fh, v & 0xffffffff);
        break;

      case GWEN_SAR_TAG_HEADER_FLAGS:
        v=GWEN_Sar_ReadUint64(p, bs);
        GWEN_SarFileHeader_SetFlags(fh, v & 0xffffffff);
        break;

      case GWEN_SAR_TAG_HEADER_PATH:
        GWEN_SarFileHeader_SetPath(fh, (const char*)p);
        break;

      case GWEN_SAR_TAG_HEADER_TYPE:
        v=GWEN_Sar_ReadUint64(p, bs);
        GWEN_SarFileHeader_SetFileType(fh, v & 0xffffffff);
        break;

      case GWEN_SAR_TAG_HEADER_PERM:
        v=GWEN_Sar_ReadUint64(p, bs);
        GWEN_SarFileHeader_SetPermissions(fh, v & 0xffffffff);
        break;

      case GWEN_SAR_TAG_HEADER_ATIME:
        ti=GWEN_Time_fromUtcString((const char*) p, "YYYYMMDDhhmmss");
        GWEN_SarFileHeader_SetAtime(fh, ti);
        break;

      case GWEN_SAR_TAG_HEADER_MTIME:
        ti=GWEN_Time_fromUtcString((const char*) p, "YYYYMMDDhhmmss");
        GWEN_SarFileHeader_SetMtime(fh, ti);
        break;

      case GWEN_SAR_TAG_HEADER_CTIME:
        ti=GWEN_Time_fromUtcString((const char*) p, "YYYYMMDDhhmmss");
        GWEN_SarFileHeader_SetCtime(fh, ti);
        break;

      case GWEN_SAR_TAG_HEADER_SIZE:
        v=GWEN_Sar_ReadUint64(p, bs);
        GWEN_SarFileHeader_SetFileSize(fh, v);
        break;
      default:
        DBG_WARN(GWEN_LOGDOMAIN, "Ignoring unknown tag %d", GWEN_TLV_GetTagType(tlv));
        break;
      }

    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "No TLV in buffer");
      return GWEN_ERROR_BAD_DATA;
    }
  }
  return 0;
}



int GWEN_Sar_AddAndDigestFileReg(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, GWEN_MDIGEST *md) {
  int rv;
  const char *fname;
  uint64_t fsize;
  uint64_t bytesDone;
  GWEN_BUFFER *hbuf;

  assert(sr);
  assert(sr->refCount);

  fname=GWEN_SarFileHeader_GetPath(fh);
  assert(fname);
  fsize=GWEN_SarFileHeader_GetFileSize(fh);

  /* create TLV header */
  hbuf=GWEN_Buffer_new(0, 32, 0, 1);
  rv=GWEN_TLV_WriteHeader(GWEN_SAR_TAG_FILE, 0x00, fsize, 1, hbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* write TLV header */
  rv=GWEN_SyncIo_WriteForced(sr->archiveSio,
                             (const uint8_t*) GWEN_Buffer_GetStart(hbuf),
                             GWEN_Buffer_GetUsedBytes(hbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  GWEN_Buffer_free(hbuf);

  /* copy file if fsize>0 */
  if (fsize>0) {
    GWEN_SYNCIO *sio;
    uint32_t pid;

    /* open input file */
    sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sio);
      return rv;
    }

    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_SHOW_ABORT |
                               GWEN_GUI_PROGRESS_ALLOW_EMBED |
                               GWEN_GUI_PROGRESS_SHOW_PROGRESS,
                               I18N("File Operation"),
                               I18N("Copying file into archive"),
                               fsize,
                               0);
    bytesDone=0;
    while(fsize) {
      uint8_t fbuf[10240];
      uint64_t bs;

      bs=fsize;
      if (bs>sizeof(fbuf))
        bs=sizeof(fbuf);

      /* read from input */
      rv=GWEN_SyncIo_Read(sio, fbuf, bs);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_SyncIo_Disconnect(sio);
        GWEN_SyncIo_free(sio);
        return rv;
      }
      bs=rv;

      /* digest data */
      rv=GWEN_MDigest_Update(md, fbuf, bs);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_SyncIo_Disconnect(sio);
        GWEN_SyncIo_free(sio);
        return rv;
      }

      /* write to archive */
      rv=GWEN_SyncIo_WriteForced(sr->archiveSio, fbuf, bs);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_SyncIo_Disconnect(sio);
        GWEN_SyncIo_free(sio);
        return rv;
      }

      if (bs>fsize) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: bs>fsize (%lu>%lu)",
                  (unsigned long int)bs, (unsigned long int) fsize);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_SyncIo_Disconnect(sio);
        GWEN_SyncIo_free(sio);
        return rv;
      }

      bytesDone+=bs;
      fsize-=bs;

      /* advance progress bar */
      rv=GWEN_Gui_ProgressAdvance(pid, bytesDone);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_SyncIo_Disconnect(sio);
        GWEN_SyncIo_free(sio);
        return rv;
      }

    } /* while */
    GWEN_Gui_ProgressEnd(pid);

    /* close input file */
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
  }

  return 0;
}



int GWEN_Sar_AddAndDigestFileLink(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, GWEN_MDIGEST *md) {
#if ((_BSD_SOURCE || _XOPEN_SOURCE >= 500 || (_XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED) || _POSIX_C_SOURCE >= 200112L) && !defined(__MINGW32__)) || defined(OS_DARWIN)

  int rv;
  const char *fname;
  GWEN_BUFFER *hbuf;
  char lbuf[300];
  int len;

  assert(sr);
  assert(sr->refCount);

  fname=GWEN_SarFileHeader_GetPath(fh);
  assert(fname);

  /* read link content */
  rv=readlink(fname, lbuf, sizeof(lbuf)-1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "readlink(%s): %d (%s)",
              fname, errno, strerror(errno));
    return GWEN_ERROR_IO;
  }
  len=rv;
  lbuf[len]=0;

  /* create TLV header */
  hbuf=GWEN_Buffer_new(0, rv+8, 0, 1);
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_FILE, 0x00,
                            lbuf, len+1,
                            1, hbuf);


  /* write TLV */
  rv=GWEN_SyncIo_WriteForced(sr->archiveSio,
                             (const uint8_t*) GWEN_Buffer_GetStart(hbuf),
                             GWEN_Buffer_GetUsedBytes(hbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return rv;
  }
  GWEN_Buffer_free(hbuf);

  /* digest data */
  rv=GWEN_MDigest_Update(md, (const uint8_t*) lbuf, len+1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
#else
# warning "Function readlink() is not available"
  DBG_ERROR(GWEN_LOGDOMAIN, "Function readlink() is not available");
  return GWEN_ERROR_IO;
#endif
}



int GWEN_Sar_AddAndDigestFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, GWEN_MDIGEST *md) {
  int rv;

  switch(GWEN_SarFileHeader_GetFileType(fh)) {
  case GWEN_SarFileHeader_FType_File:
    rv=GWEN_Sar_AddAndDigestFileReg(sr, fh, md);
    break;
  case GWEN_SarFileHeader_FType_Dir:
    rv=0;
    break;
  case GWEN_SarFileHeader_FType_SymLink:
    rv=GWEN_Sar_AddAndDigestFileLink(sr, fh, md);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "File type %d not supported", GWEN_SarFileHeader_GetFileType(fh));
    return GWEN_ERROR_INVALID;
  }

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_Sar_AddFile(GWEN_SAR *sr, const char *fname) {
  struct stat st;
  int rv;
  GWEN_SAR_FILEHEADER *fh;
  GWEN_TIME *ti;
  GWEN_BUFFER *hbuf;
  GWEN_BUFFER *xbuf;
  GWEN_MDIGEST *md;
  int64_t pos;

  assert(sr);
  assert(sr->refCount);

  /* stat file to be added */
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED
  rv=lstat(fname, &st);
#else
  rv=stat(fname, &st);
#endif
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "stat(%s): %d (%s)",
              fname, errno, strerror(errno));
    return GWEN_ERROR_IO;
  }

  /* create and setup file header */
  fh=GWEN_SarFileHeader_new();

  /* path */
  GWEN_SarFileHeader_SetPath(fh, fname);

  /* file type */
  switch(st.st_mode & S_IFMT) {
#ifdef S_IFLNK
  case S_IFLNK:
    GWEN_SarFileHeader_SetFileType(fh, GWEN_SarFileHeader_FType_SymLink);
    break;
#endif
  case S_IFREG:
    GWEN_SarFileHeader_SetFileType(fh, GWEN_SarFileHeader_FType_File);
    break;
  case S_IFDIR:
    GWEN_SarFileHeader_SetFileType(fh, GWEN_SarFileHeader_FType_Dir);
    break;
  default:
    GWEN_SarFileHeader_free(fh);
    return GWEN_ERROR_INVALID;
  }

  /* permissions */
  if (st.st_mode & S_IRUSR) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_UREAD);
  if (st.st_mode & S_IWUSR) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_UWRITE);
  if (st.st_mode & S_IXUSR) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_UEXEC);

#ifdef S_IRGRP
  if (st.st_mode & S_IRGRP) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_GREAD);
#endif
#ifdef S_IWGRP
  if (st.st_mode & S_IWGRP) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_GWRITE);
#endif
#ifdef S_IXGRP
  if (st.st_mode & S_IXGRP) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_GEXEC);
#endif

#ifdef S_IROTH
  if (st.st_mode & S_IROTH) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_OREAD);
#endif
#ifdef S_IWOTH
  if (st.st_mode & S_IWOTH) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_OWRITE);
#endif
#ifdef S_IXOTH
  if (st.st_mode & S_IXOTH) GWEN_SarFileHeader_AddPermissions(fh, GWEN_SYNCIO_FILE_FLAGS_OEXEC);
#endif

  /* atime */
  ti=GWEN_Time_fromSeconds(st.st_atime);
  GWEN_SarFileHeader_SetAtime(fh, ti);

  /* mtime */
  ti=GWEN_Time_fromSeconds(st.st_mtime);
  GWEN_SarFileHeader_SetMtime(fh, ti);

  /* ctime */
  ti=GWEN_Time_fromSeconds(st.st_ctime);
  GWEN_SarFileHeader_SetCtime(fh, ti);

  /* file size */
  GWEN_SarFileHeader_SetFileSize(fh, st.st_size);


  /* prepare header */
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_ReserveBytes(hbuf, 16);
  rv=GWEN_Sar_FileHeaderToTlv(fh, hbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  /* create TLV header for file header */
  xbuf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=GWEN_TLV_WriteHeader(GWEN_SAR_TAG_HEADER, 0xe0,
                          GWEN_Buffer_GetUsedBytes(hbuf), 1, xbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  /* insert TLV header into file header buffer */
  GWEN_Buffer_SetPos(hbuf, 0);
  GWEN_Buffer_InsertBytes(hbuf,
                          GWEN_Buffer_GetStart(xbuf),
                          GWEN_Buffer_GetUsedBytes(xbuf));
  GWEN_Buffer_SetPos(hbuf, GWEN_Buffer_GetUsedBytes(hbuf));
  GWEN_Buffer_free(xbuf);

  /* seek to end of file */
  pos=GWEN_SyncIo_File_Seek(sr->archiveSio, 0, GWEN_SyncIo_File_Whence_End);
  if (pos<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return (int) pos;
  }

  /* write header into archive file */
  rv=GWEN_SyncIo_WriteForced(sr->archiveSio,
                             (const uint8_t*) GWEN_Buffer_GetStart(hbuf),
                             GWEN_Buffer_GetUsedBytes(hbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  /* prepare digest */
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  /* digest header */
  rv=GWEN_MDigest_Update(md, (const uint8_t*) GWEN_Buffer_GetStart(hbuf), GWEN_Buffer_GetUsedBytes(hbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  GWEN_Buffer_Reset(hbuf);

  /* copy file into archive */
  rv=GWEN_Sar_AddAndDigestFile(sr, fh, md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  /* finish hash */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  /* create hash TLV */
  GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_HASH, 0x00,
                            GWEN_MDigest_GetDigestPtr(md),
                            GWEN_MDigest_GetDigestSize(md),
                            1, hbuf);
  GWEN_MDigest_free(md);

  /* write hash into archive file */
  rv=GWEN_SyncIo_WriteForced(sr->archiveSio,
                             (const uint8_t*) GWEN_Buffer_GetStart(hbuf),
                             GWEN_Buffer_GetUsedBytes(hbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_SarFileHeader_free(fh);
    return rv;
  }

  /* done */
  GWEN_Buffer_free(hbuf);
  GWEN_SarFileHeader_free(fh);
  return 0;
}




int GWEN_Sar_ScanFile(GWEN_SAR *sr) {
  int rv;
  int64_t pos;
  GWEN_BUFFER *mbuf;
  GWEN_SAR_FILEHEADER *lastHeader=NULL;

  assert(sr);
  assert(sr->refCount);

  sr->signaturePos=0;
  sr->signatureSize=0;

  /* scan all TLV elements */
  pos=0;
  mbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  for (;;) {
    int64_t startOfTagHeader;
    int64_t startOfTagData;
    unsigned int tagType;
    uint32_t tagLength;
    uint32_t fullTagSize;
    uint32_t bs;
    GWEN_TLV *tlv;
    uint8_t buffer[32];

    startOfTagHeader=pos;
    rv=GWEN_SyncIo_Read(sr->archiveSio, buffer, sizeof(buffer));
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF)
        break;
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      return rv;
    }
    else if (rv==0) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "EOF met");
      break;
    }
    bs=rv;

    /* read start of fileheader TLV */
    tlv=GWEN_TLV_new();
    rv=GWEN_TLV_ReadHeader(tlv, buffer, bs, 1);
    if (rv<2) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_TLV_free(tlv);
      GWEN_Buffer_free(mbuf);
      return rv;
    }
    /* got it, now calculate start of tag data */
    startOfTagData=pos+rv;
    tagLength=GWEN_TLV_GetTagLength(tlv);
    tagType=GWEN_TLV_GetTagType(tlv);
    fullTagSize=GWEN_TLV_GetTagSize(tlv);
    GWEN_TLV_free(tlv);

    /* seek to start of header data */
    pos=GWEN_SyncIo_File_Seek(sr->archiveSio, startOfTagData, GWEN_SyncIo_File_Whence_Set);
    if (pos<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
      GWEN_Buffer_free(mbuf);
      return (int) pos;
    }

    /* now read or skip info */
    if (tagType==GWEN_SAR_TAG_HEADER) {
      GWEN_SAR_FILEHEADER *fh;
      const char *s;

      /* alloc memory for data */
      GWEN_Buffer_AllocRoom(mbuf, tagLength);

      /* read header data */
      rv=GWEN_SyncIo_ReadForced(sr->archiveSio, (uint8_t*) GWEN_Buffer_GetStart(mbuf), tagLength);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(mbuf);
        return rv;
      }
      GWEN_Buffer_IncrementPos(mbuf, tagLength);
      GWEN_Buffer_AdjustUsedBytes(mbuf);
      GWEN_Buffer_Rewind(mbuf);

      /* now parse header */
      fh=GWEN_SarFileHeader_new();
      rv=GWEN_Sar_TlvToFileHeader(mbuf, fh);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_SarFileHeader_free(fh);
        GWEN_Buffer_free(mbuf);
        return rv;
      }

      GWEN_SarFileHeader_SetHeaderStartPos(fh, startOfTagHeader);
      GWEN_SarFileHeader_SetHeaderSize(fh, fullTagSize);

      s=GWEN_SarFileHeader_GetPath(fh);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Got entry [%s]", s?s:"(empty)");

      GWEN_SarFileHeader_List_Add(fh, sr->headers);
      lastHeader=fh;
    }
    else if (tagType==GWEN_SAR_TAG_FILE) {
      if (lastHeader==NULL) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad file structure: No file header before data");
        GWEN_Buffer_free(mbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      if (GWEN_SarFileHeader_GetFileType(lastHeader)!=GWEN_SarFileHeader_FType_SymLink &&
          GWEN_SarFileHeader_GetFileSize(lastHeader)!=tagLength) {
        DBG_ERROR(GWEN_LOGDOMAIN, "File size in header and in archive differ (%s: hs=%lu, ts=%lu)",
                  GWEN_SarFileHeader_GetPath(lastHeader),
                  (unsigned long int) GWEN_SarFileHeader_GetFileSize(lastHeader),
                  (unsigned long int) tagLength);
        GWEN_Buffer_free(mbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      /* only store position of file data */
      GWEN_SarFileHeader_SetDataPos(lastHeader, startOfTagData);
      GWEN_SarFileHeader_SetDataSize(lastHeader, tagLength);

      /* skip data */
      pos=GWEN_SyncIo_File_Seek(sr->archiveSio, tagLength, GWEN_SyncIo_File_Whence_Current);
      if (pos<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
        GWEN_Buffer_free(mbuf);
        return (int) pos;
      }
    }
    else if (tagType==GWEN_SAR_TAG_HASH)  {
      if (lastHeader==NULL) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad file structure: No file header before data");
        GWEN_Buffer_free(mbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      /* only store position of file data */
      GWEN_SarFileHeader_SetHashPos(lastHeader, startOfTagData);

      /* skip data */
      pos=GWEN_SyncIo_File_Seek(sr->archiveSio, tagLength, GWEN_SyncIo_File_Whence_Current);
      if (pos<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
        GWEN_Buffer_free(mbuf);
        return (int) pos;
      }
    }
    else if (tagType==GWEN_SAR_TAG_SIGNATURE)  {
      /* only store position of file data */
      DBG_INFO(GWEN_LOGDOMAIN, "Signature found");
      sr->signaturePos=startOfTagData;
      sr->signatureSize=tagLength;

      /* skip data */
      pos=GWEN_SyncIo_File_Seek(sr->archiveSio, tagLength, GWEN_SyncIo_File_Whence_Current);
      if (pos<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
        GWEN_Buffer_free(mbuf);
        return (int) pos;
      }
    }
    else {
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown TAG %d, ignoring", (int) tagType);
      /* just skip data */
      pos=GWEN_SyncIo_File_Seek(sr->archiveSio, tagLength, GWEN_SyncIo_File_Whence_Current);
      if (pos<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
        GWEN_Buffer_free(mbuf);
        return (int) pos;
      }
    }

    GWEN_Buffer_Reset(mbuf);
    pos=startOfTagData+tagLength;
  } /* for */

  /* done */
  GWEN_Buffer_free(mbuf);
  return 0;
}



int GWEN_Sar_ExtractAndDigestFileReg(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly) {
  int rv;
  const char *fname;
  uint32_t perms;
  uint64_t dpos;
  uint64_t fsize;
  uint64_t bytesDone;
  uint64_t hsize;
  uint64_t hpos;
  uint64_t mpos;
  int64_t pos;
  GWEN_MDIGEST *md;

  assert(sr);
  assert(sr->refCount);

  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  fname=GWEN_SarFileHeader_GetPath(fh);
  assert(fname);
  perms=GWEN_SarFileHeader_GetPermissions(fh);
  fsize=GWEN_SarFileHeader_GetFileSize(fh);
  dpos=GWEN_SarFileHeader_GetDataPos(fh);

  hpos=GWEN_SarFileHeader_GetHeaderStartPos(fh);
  hsize=GWEN_SarFileHeader_GetHeaderSize(fh);
  if (hsize>0) {
    GWEN_BUFFER *mbuf;

    /* seek to start of header */
    pos=GWEN_SyncIo_File_Seek(sr->archiveSio, hpos, GWEN_SyncIo_File_Whence_Set);
    if (pos<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
      GWEN_MDigest_free(md);
      return (int) pos;
    }

    mbuf=GWEN_Buffer_new(0, hsize, 0, 1);
    rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                              (uint8_t*) GWEN_Buffer_GetStart(mbuf),
                              hsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_Buffer_IncrementPos(mbuf, hsize);
    GWEN_Buffer_AdjustUsedBytes(mbuf);

    /* digest TLV */
    rv=GWEN_MDigest_Update(md, (const uint8_t*) GWEN_Buffer_GetStart(mbuf), GWEN_Buffer_GetUsedBytes(mbuf));
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_Buffer_free(mbuf);
  }

  /* copy file if fsize>0 */
  if (1) {
    GWEN_SYNCIO *sio=NULL;
    uint32_t pid;

    /* open input file */
    if (!checkOnly) {
      sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_CreateNew);
      GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ | GWEN_SYNCIO_FILE_FLAGS_WRITE);
      GWEN_SyncIo_AddFlags(sio, perms);
      rv=GWEN_SyncIo_Connect(sio);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_SyncIo_free(sio);
        GWEN_MDigest_free(md);
        return rv;
      }
    }

    if (fsize>0) {
      /* seek to start of data */
      pos=GWEN_SyncIo_File_Seek(sr->archiveSio, dpos, GWEN_SyncIo_File_Whence_Set);
      if (pos<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
        if (!checkOnly) {
          GWEN_SyncIo_Disconnect(sio);
          GWEN_SyncIo_free(sio);
        }
        GWEN_MDigest_free(md);
        return (int) pos;
      }

      /* start extracting */
      pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                                 GWEN_GUI_PROGRESS_SHOW_ABORT |
                                 GWEN_GUI_PROGRESS_ALLOW_EMBED |
                                 GWEN_GUI_PROGRESS_SHOW_PROGRESS,
                                 I18N("File Operation"),
                                 I18N("Extracting file from archive"),
                                 fsize,
                                 0);
      bytesDone=0;
      while(fsize) {
        uint8_t fbuf[10240];
        uint64_t bs;

        bs=fsize;
        if (bs>sizeof(fbuf))
          bs=sizeof(fbuf);

        /* read from input */
        rv=GWEN_SyncIo_Read(sr->archiveSio, fbuf, bs);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Gui_ProgressEnd(pid);
          if (!checkOnly) {
            GWEN_SyncIo_Disconnect(sio);
            GWEN_SyncIo_free(sio);
          }
          GWEN_MDigest_free(md);
          return rv;
        }
        bs=rv;

        /* digest data */
        rv=GWEN_MDigest_Update(md, fbuf, bs);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Gui_ProgressEnd(pid);
          if (!checkOnly) {
            GWEN_SyncIo_Disconnect(sio);
            GWEN_SyncIo_free(sio);
          }
          GWEN_MDigest_free(md);
          return rv;
        }

        if (!checkOnly) {
          /* write to archive */
          rv=GWEN_SyncIo_WriteForced(sio, fbuf, bs);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Gui_ProgressEnd(pid);
            if (!checkOnly) {
              GWEN_SyncIo_Disconnect(sio);
              GWEN_SyncIo_free(sio);
            }
            GWEN_MDigest_free(md);
            return rv;
          }
        }

        if (bs>fsize) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: bs>fsize (%lu>%lu)",
                    (unsigned long int)bs, (unsigned long int) fsize);
          GWEN_Gui_ProgressEnd(pid);
          if (!checkOnly) {
            GWEN_SyncIo_Disconnect(sio);
            GWEN_SyncIo_free(sio);
          }
          GWEN_MDigest_free(md);
          return rv;
        }

        bytesDone+=bs;
        fsize-=bs;

        /* advance progress bar */
        rv=GWEN_Gui_ProgressAdvance(pid, bytesDone);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Gui_ProgressEnd(pid);
          if (!checkOnly) {
            GWEN_SyncIo_Disconnect(sio);
            GWEN_SyncIo_free(sio);
          }
          GWEN_MDigest_free(md);
          return rv;
        }

      } /* while */
      GWEN_Gui_ProgressEnd(pid);
    }

    if (!checkOnly) {
      /* close output file */
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
    }

    /* finish hash */
    rv=GWEN_MDigest_End(md);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_MDigest_free(md);
      return rv;
    }

    /* read and check hash */
    mpos=GWEN_SarFileHeader_GetHashPos(fh);
    if (mpos) {
      GWEN_BUFFER *mbuf;

      /* seek to end of file */
      pos=GWEN_SyncIo_File_Seek(sr->archiveSio, mpos, GWEN_SyncIo_File_Whence_Set);
      if (pos<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
        GWEN_MDigest_free(md);
        return (int) pos;
      }

      /* read 20 bytes of hash */
      mbuf=GWEN_Buffer_new(0, 20, 0, 1);
      rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                                (uint8_t*) GWEN_Buffer_GetStart(mbuf),
                                20);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(mbuf);
        GWEN_MDigest_free(md);
        return rv;
      }
      GWEN_Buffer_IncrementPos(mbuf, 20);
      GWEN_Buffer_AdjustUsedBytes(mbuf);

      if (memcmp(GWEN_MDigest_GetDigestPtr(md),
                 GWEN_Buffer_GetStart(mbuf),
                 20)!=0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        DBG_ERROR(0, "Hash don't match:");
        GWEN_Text_LogString((const char*) GWEN_MDigest_GetDigestPtr(md), 20,
                            GWEN_LOGDOMAIN, GWEN_LoggerLevel_Error);

        GWEN_Text_LogString(GWEN_Buffer_GetStart(mbuf), 20,
                            GWEN_LOGDOMAIN, GWEN_LoggerLevel_Error);
        GWEN_Buffer_free(mbuf);
        GWEN_MDigest_free(md);
        return GWEN_ERROR_BAD_DATA;
      }

      GWEN_Buffer_free(mbuf);
    }

    GWEN_MDigest_free(md);
  }

  return 0;
}



int GWEN_Sar_ExtractAndDigestFileLink(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly) {
#if ((_BSD_SOURCE || _XOPEN_SOURCE >= 500 || (_XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED) || _POSIX_C_SOURCE >= 200112L) && !defined(__MINGW32__)) || defined(OS_DARWIN)
  int rv;
  const char *fname;
  uint32_t perms;
  uint64_t dpos;
  uint64_t fsize;
  uint64_t hsize;
  uint64_t hpos;
  uint64_t mpos;
  int64_t pos;
  GWEN_MDIGEST *md;

  assert(sr);
  assert(sr->refCount);

  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  fname=GWEN_SarFileHeader_GetPath(fh);
  perms=GWEN_SarFileHeader_GetPermissions(fh);
  assert(fname);
  fsize=GWEN_SarFileHeader_GetDataSize(fh); /* not FileSize!! */
  dpos=GWEN_SarFileHeader_GetDataPos(fh);

  hpos=GWEN_SarFileHeader_GetHeaderStartPos(fh);
  hsize=GWEN_SarFileHeader_GetHeaderSize(fh);
  if (hsize>0) {
    GWEN_BUFFER *mbuf;

    /* seek to header pos */
    pos=GWEN_SyncIo_File_Seek(sr->archiveSio, hpos, GWEN_SyncIo_File_Whence_Set);
    if (pos<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
      GWEN_MDigest_free(md);
      return (int) pos;
    }

    mbuf=GWEN_Buffer_new(0, hsize, 0, 1);
    rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                              (uint8_t*) GWEN_Buffer_GetStart(mbuf),
                              hsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_Buffer_IncrementPos(mbuf, hsize);
    GWEN_Buffer_AdjustUsedBytes(mbuf);

    /* digest header TLV */
    rv=GWEN_MDigest_Update(md, (const uint8_t*) GWEN_Buffer_GetStart(mbuf), GWEN_Buffer_GetUsedBytes(mbuf));
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_Buffer_free(mbuf);
  }

  /* seek to data pos */
  pos=GWEN_SyncIo_File_Seek(sr->archiveSio, dpos, GWEN_SyncIo_File_Whence_Set);
  if (pos<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
    GWEN_MDigest_free(md);
    return (int) pos;
  }

  /* copy file if fsize>0 */
  if (fsize>0) { /* fsize is the size of the file tag data */
    GWEN_BUFFER *mbuf;

    mbuf=GWEN_Buffer_new(0, fsize, 0, 1);
    rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                              (uint8_t*) GWEN_Buffer_GetStart(mbuf),
                              fsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_Buffer_IncrementPos(mbuf, fsize);
    GWEN_Buffer_AdjustUsedBytes(mbuf);

    /* digest TLV */
    rv=GWEN_MDigest_Update(md, (const uint8_t*) GWEN_Buffer_GetStart(mbuf), GWEN_Buffer_GetUsedBytes(mbuf));
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      return rv;
    }

    if (!checkOnly) {
      mode_t mode=0;

      if (symlink(GWEN_Buffer_GetStart(mbuf), fname)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "symlink(%s, %s): %d (%s)",
                  GWEN_Buffer_GetStart(mbuf),
                  fname, errno, strerror(errno));
        GWEN_Buffer_free(mbuf);
        GWEN_MDigest_free(md);
        return GWEN_ERROR_IO;
      }
      /* owner perms */
      if (perms & GWEN_SYNCIO_FILE_FLAGS_UREAD)
        mode|=S_IRUSR;
      if (perms & GWEN_SYNCIO_FILE_FLAGS_UWRITE)
        mode|=S_IWUSR;
      if (perms & GWEN_SYNCIO_FILE_FLAGS_UEXEC)
        mode|=S_IXUSR;

#if 0 /* CHMOD on symlinks doesn't work */

      /* group perms */
#ifdef S_IRGRP
      if (perms & GWEN_SYNCIO_FILE_FLAGS_GREAD) mode|=S_IRGRP;
#endif
#ifdef S_IWGRP
      if (perms & GWEN_SYNCIO_FILE_FLAGS_GWRITE) mode|=S_IWGRP;
#endif
#ifdef S_IXGRP
      if (perms & GWEN_SYNCIO_FILE_FLAGS_GEXEC) mode|=S_IXGRP;
#endif

      /* other perms */
#ifdef S_IROTH
      if (perms & GWEN_SYNCIO_FILE_FLAGS_OREAD) mode|=S_IROTH;
#endif
#ifdef S_IWOTH
      if (perms & GWEN_SYNCIO_FILE_FLAGS_OWRITE) mode|=S_IWOTH;
#endif
#ifdef S_IXOTH
      if (perms & GWEN_SYNCIO_FILE_FLAGS_OEXEC) mode|=S_IXOTH;
#endif

      rv=chmod(fname, mode);
      if (rv<0) {
        DBG_WARN(GWEN_LOGDOMAIN, "chmod(%s): %d (%s), ignoring",
                 fname, errno, strerror(errno));
      }

#endif
    }
    GWEN_Buffer_free(mbuf);
  }

  /* finish hash */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  /* read and check hash */
  mpos=GWEN_SarFileHeader_GetHashPos(fh);
  if (mpos) {
    GWEN_BUFFER *mbuf;

    /* seek to end of file */
    pos=GWEN_SyncIo_File_Seek(sr->archiveSio, mpos, GWEN_SyncIo_File_Whence_Set);
    if (pos<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
      GWEN_MDigest_free(md);
      return (int) pos;
    }

    /* read 20 bytes of hash */
    mbuf=GWEN_Buffer_new(0, 20, 0, 1);
    rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                              (uint8_t*) GWEN_Buffer_GetStart(mbuf),
                              20);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      if (!checkOnly)
        unlink(fname);
      return rv;
    }
    GWEN_Buffer_IncrementPos(mbuf, 20);
    GWEN_Buffer_AdjustUsedBytes(mbuf);

    if (memcmp(GWEN_MDigest_GetDigestPtr(md),
               GWEN_Buffer_GetStart(mbuf),
               20)!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      if (!checkOnly)
        unlink(fname);
      return GWEN_ERROR_BAD_DATA;
    }

    GWEN_Buffer_free(mbuf);
  }

  GWEN_MDigest_free(md);

  return 0;
#else
# warning "Function symlink() is not available"
  DBG_ERROR(GWEN_LOGDOMAIN, "Function symlink() is not available");
  return GWEN_ERROR_IO;
#endif
}



int GWEN_Sar_ExtractAndDigestFileDir(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly) {
  int rv;
  const char *fname;
  //uint64_t dpos;
  //uint64_t fsize;
  uint64_t hsize;
  uint64_t hpos;
  uint64_t mpos;
  int64_t pos;
  GWEN_MDIGEST *md;
  GWEN_BUFFER *mbuf;
  uint32_t perms;

  assert(sr);
  assert(sr->refCount);

  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  fname=GWEN_SarFileHeader_GetPath(fh);
  assert(fname);
  perms=GWEN_SarFileHeader_GetPermissions(fh);
  //fsize=GWEN_SarFileHeader_GetDataSize(fh); /* not FileSize!! */
  //dpos=GWEN_SarFileHeader_GetDataPos(fh);

  hpos=GWEN_SarFileHeader_GetHeaderStartPos(fh);
  hsize=GWEN_SarFileHeader_GetHeaderSize(fh);
  assert(hsize);

  /* seek to end of file */
  pos=GWEN_SyncIo_File_Seek(sr->archiveSio, hpos, GWEN_SyncIo_File_Whence_Set);
  if (pos<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
    GWEN_MDigest_free(md);
    return (int) pos;
  }

  mbuf=GWEN_Buffer_new(0, hsize, 0, 1);
  rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                            (uint8_t*) GWEN_Buffer_GetStart(mbuf),
                            hsize);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(mbuf);
    GWEN_MDigest_free(md);
    return rv;
  }
  GWEN_Buffer_IncrementPos(mbuf, hsize);
  GWEN_Buffer_AdjustUsedBytes(mbuf);

  /* digest TLV */
  rv=GWEN_MDigest_Update(md, (const uint8_t*) GWEN_Buffer_GetStart(mbuf), GWEN_Buffer_GetUsedBytes(mbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(mbuf);
    GWEN_MDigest_free(md);
    return rv;
  }
  GWEN_Buffer_free(mbuf);

  /* finish hash */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  /* read and check hash */
  mpos=GWEN_SarFileHeader_GetHashPos(fh);
  if (mpos) {
    GWEN_BUFFER *mbuf;

    /* seek to end of file */
    pos=GWEN_SyncIo_File_Seek(sr->archiveSio, mpos, GWEN_SyncIo_File_Whence_Set);
    if (pos<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
      GWEN_MDigest_free(md);
      return (int) pos;
    }

    /* read 20 bytes of hash */
    mbuf=GWEN_Buffer_new(0, 20, 0, 1);
    rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                              (uint8_t*) GWEN_Buffer_GetStart(mbuf),
                              20);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      unlink(fname);
      return rv;
    }
    GWEN_Buffer_IncrementPos(mbuf, 20);
    GWEN_Buffer_AdjustUsedBytes(mbuf);

    if (memcmp(GWEN_MDigest_GetDigestPtr(md),
               GWEN_Buffer_GetStart(mbuf),
               20)!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_MDigest_free(md);
      unlink(fname);
      return GWEN_ERROR_BAD_DATA;
    }

    GWEN_Buffer_free(mbuf);
  }

  GWEN_MDigest_free(md);

  if (!checkOnly) {
    mode_t mode=0;

    /* owner perms */
    if (perms & GWEN_SYNCIO_FILE_FLAGS_UREAD)
      mode|=S_IRUSR;
    if (perms & GWEN_SYNCIO_FILE_FLAGS_UWRITE)
      mode|=S_IWUSR;
    if (perms & GWEN_SYNCIO_FILE_FLAGS_UEXEC)
      mode|=S_IXUSR;

    /* group perms */
#ifdef S_IRGRP
    if (perms & GWEN_SYNCIO_FILE_FLAGS_GREAD) mode|=S_IRGRP;
#endif
#ifdef S_IWGRP
    if (perms & GWEN_SYNCIO_FILE_FLAGS_GWRITE) mode|=S_IWGRP;
#endif
#ifdef S_IXGRP
    if (perms & GWEN_SYNCIO_FILE_FLAGS_GEXEC) mode|=S_IXGRP;
#endif

    /* other perms */
#ifdef S_IROTH
    if (perms & GWEN_SYNCIO_FILE_FLAGS_OREAD) mode|=S_IROTH;
#endif
#ifdef S_IWOTH
    if (perms & GWEN_SYNCIO_FILE_FLAGS_OWRITE) mode|=S_IWOTH;
#endif
#ifdef S_IXOTH
    if (perms & GWEN_SYNCIO_FILE_FLAGS_OEXEC) mode|=S_IXOTH;
#endif

    /* create folder */
#ifndef OS_WIN32
    rv=mkdir(fname, mode);
#else
    rv=mkdir(fname);
#endif
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "mkdir(%s): %d (%s)",
                fname, errno, strerror(errno));
      return GWEN_ERROR_IO;
    }
  }

  return 0;
}



int GWEN_Sar_ExtractAndDigestFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly) {
  int rv;

  switch(GWEN_SarFileHeader_GetFileType(fh)) {
  case GWEN_SarFileHeader_FType_File:
    rv=GWEN_Sar_ExtractAndDigestFileReg(sr, fh, checkOnly);
    break;
  case GWEN_SarFileHeader_FType_Dir:
    rv=GWEN_Sar_ExtractAndDigestFileDir(sr, fh, checkOnly);
    break;
  case GWEN_SarFileHeader_FType_SymLink:
    rv=GWEN_Sar_ExtractAndDigestFileLink(sr, fh, checkOnly);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "File type %d not supported", GWEN_SarFileHeader_GetFileType(fh));
    return GWEN_ERROR_INVALID;
  }

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_Sar_ExtractFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh) {
  int rv;

  rv=GWEN_Sar_ExtractAndDigestFile(sr, fh, 0);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int GWEN_Sar_CheckFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh) {
  int rv;

  rv=GWEN_Sar_ExtractAndDigestFile(sr, fh, 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



const GWEN_SAR_FILEHEADER_LIST *GWEN_Sar_GetHeaders(GWEN_SAR *sr) {
  assert(sr);
  assert(sr->refCount);
  return sr->headers;
}



int GWEN_Sar__UnpackArchive(const char *inFile, const char *where) {
  GWEN_SAR *sr;
  int rv;
  const GWEN_SAR_FILEHEADER_LIST *fhl;

  /* open archive file */
  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, inFile,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* change to "where" */
  if (chdir(where)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "chdir(%s): %s", where, strerror(errno));
    GWEN_Sar_CloseArchive(sr, 1);
    GWEN_Sar_free(sr);
    return GWEN_ERROR_IO;
  }

  fhl=GWEN_Sar_GetHeaders(sr);
  if (fhl) {
    const GWEN_SAR_FILEHEADER *fh;
    uint32_t pid;

    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_SHOW_ABORT |
                               GWEN_GUI_PROGRESS_ALLOW_EMBED |
                               GWEN_GUI_PROGRESS_SHOW_PROGRESS,
                               I18N("File Operation"),
                               I18N("Unpacking archive file"),
                               GWEN_SarFileHeader_List_GetCount(fhl),
                               0);

    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      //const char *s;

      //s=GWEN_SarFileHeader_GetPath(fh);
      rv=GWEN_Sar_ExtractFile(sr, fh);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_Sar_CloseArchive(sr, 1);
        GWEN_Sar_free(sr);
      }

      rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_Sar_CloseArchive(sr, 1);
        GWEN_Sar_free(sr);
        return rv;
      }

      fh=GWEN_SarFileHeader_List_Next(fh);
    }
    GWEN_Gui_ProgressEnd(pid);
  }

  rv=GWEN_Sar_CloseArchive(sr, 0);
  if (rv<0) {
    fprintf(stderr, "Error closing archive (%d)\n", rv);
    return 2;
  }

  return 0;
}



int GWEN_Sar_UnpackArchive(const char *inFile, const char *where) {
  char savedPwd[300];
  int rv;

  /* get current working dir */
  if (getcwd(savedPwd, sizeof(savedPwd)-1)==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "getcwd(): %s", strerror(errno));
    return GWEN_ERROR_IO;
  }
  savedPwd[sizeof(savedPwd)-1]=0;

  rv=GWEN_Sar__UnpackArchive(inFile, where);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }

  /* change back to previous pwd */
  if (chdir(savedPwd)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "chdir(%s): %s", savedPwd, strerror(errno));
    return GWEN_ERROR_IO;
  }

  return rv;
}



int GWEN_Sar_Sign(GWEN_SAR *sr, GWEN_CRYPTMGR *cm) {
  int rv;
  GWEN_SAR_FILEHEADER_LIST *fhl;

  assert(sr);
  assert(sr->refCount);

  if (sr->openMode!=GWEN_Sar_OpenMode_Opened &&
      sr->openMode!=GWEN_Sar_OpenMode_Created) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Archive not open");
    return GWEN_ERROR_NOT_OPEN;
  }

  if (sr->signaturePos!=0 || sr->signatureSize!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "There already is a signature in the archive file");
    return GWEN_ERROR_INVALID;
  }

  fhl=sr->headers;
  if (fhl) {
    GWEN_SAR_FILEHEADER *fh;
    uint32_t pid;
    GWEN_MDIGEST *md;
    uint8_t hashBuf[21];
    GWEN_BUFFER *sbuf;
    GWEN_BUFFER *tbuf;
    int64_t pos;

    md=GWEN_MDigest_Rmd160_new();
    rv=GWEN_MDigest_Begin(md);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_MDigest_free(md);
      return rv;
    }

    /* clear SIGNED flags */
    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      GWEN_SarFileHeader_SubFlags(fh, GWEN_SAR_FILEHEADER_FLAGS_SIGNED);
      fh=GWEN_SarFileHeader_List_Next(fh);
    }

    /* calculate hash over all file hashes */
    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_SHOW_ABORT |
                               GWEN_GUI_PROGRESS_ALLOW_EMBED |
                               GWEN_GUI_PROGRESS_SHOW_PROGRESS,
                               I18N("File Operation"),
                               I18N("Signing archive file"),
                               GWEN_SarFileHeader_List_GetCount(fhl),
                               0);
    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      const char *s;
      uint64_t hpos;

      s=GWEN_SarFileHeader_GetPath(fh);
      hpos=GWEN_SarFileHeader_GetHashPos(fh);
      if (hpos==0) {
        DBG_WARN(GWEN_LOGDOMAIN, "File %s has no valid hash", s?s:"(unnamed)");
      }
      else {
        /* seek to start of hash */
        pos=GWEN_SyncIo_File_Seek(sr->archiveSio, hpos, GWEN_SyncIo_File_Whence_Set);
        if (pos<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
          GWEN_Gui_ProgressEnd(pid);
          GWEN_MDigest_free(md);
          return (int) pos;
        }

        /* read hash */
        rv=GWEN_SyncIo_ReadForced(sr->archiveSio, hashBuf, 20);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Gui_ProgressEnd(pid);
          GWEN_MDigest_free(md);
          return rv;
        }

        /* digest hash */
        rv=GWEN_MDigest_Update(md, hashBuf, 20);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Gui_ProgressEnd(pid);
          GWEN_MDigest_free(md);
          return rv;
        }


        GWEN_SarFileHeader_AddFlags(fh, GWEN_SAR_FILEHEADER_FLAGS_SIGNED);
      }

      rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_MDigest_free(md);
        return rv;
      }

      fh=GWEN_SarFileHeader_List_Next(fh);
    }

    /* finish hash */
    rv=GWEN_MDigest_End(md);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return rv;
    }

    /* sign hash */
    sbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_CryptMgr_Sign(cm,
                          GWEN_MDigest_GetDigestPtr(md),
                          GWEN_MDigest_GetDigestSize(md),
                          sbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(sbuf);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_MDigest_free(md);

    /* create signature TLV */
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_TLV_DirectlyToBuffer(GWEN_SAR_TAG_SIGNATURE, 0x00,
                                 GWEN_Buffer_GetStart(sbuf),
                                 GWEN_Buffer_GetUsedBytes(sbuf),
                                 1, tbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      GWEN_Buffer_free(sbuf);
      GWEN_Gui_ProgressEnd(pid);
      return rv;
    }

    /* seek to end of file */
    pos=GWEN_SyncIo_File_Seek(sr->archiveSio, 0, GWEN_SyncIo_File_Whence_End);
    if (pos<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
      GWEN_Buffer_free(tbuf);
      GWEN_Buffer_free(sbuf);
      GWEN_Gui_ProgressEnd(pid);
      return (int) pos;
    }

    /* write TLV into archive file */
    rv=GWEN_SyncIo_WriteForced(sr->archiveSio,
                               (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
                               GWEN_Buffer_GetUsedBytes(tbuf));
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      GWEN_Buffer_free(sbuf);
      GWEN_Gui_ProgressEnd(pid);
      return rv;
    }


    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(sbuf);

    GWEN_Gui_ProgressEnd(pid);
  }

  return 0;
}



int GWEN_Sar_Verify(GWEN_SAR *sr, GWEN_CRYPTMGR *cm) {
  int rv;
  GWEN_SAR_FILEHEADER_LIST *fhl;

  assert(sr);
  assert(sr->refCount);

  if (sr->openMode!=GWEN_Sar_OpenMode_Opened &&
      sr->openMode!=GWEN_Sar_OpenMode_Created) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Archive not open");
    return GWEN_ERROR_NOT_OPEN;
  }

  if (sr->signaturePos==0 || sr->signatureSize==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No valid signature data in the archive file");
    return GWEN_ERROR_INVALID;
  }

  fhl=sr->headers;
  if (fhl) {
    GWEN_SAR_FILEHEADER *fh;
    uint32_t pid;
    GWEN_MDIGEST *md;
    uint8_t hashBuf[21];
    GWEN_BUFFER *sbuf;
    GWEN_BUFFER *hbuf;
    int64_t pos;

    md=GWEN_MDigest_Rmd160_new();
    rv=GWEN_MDigest_Begin(md);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_MDigest_free(md);
      return rv;
    }

    /* clear SIGNED flags */
    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      GWEN_SarFileHeader_SubFlags(fh, GWEN_SAR_FILEHEADER_FLAGS_SIGNED);
      fh=GWEN_SarFileHeader_List_Next(fh);
    }

    /* calculate hash over all file hashes */
    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_SHOW_ABORT |
                               GWEN_GUI_PROGRESS_ALLOW_EMBED |
                               GWEN_GUI_PROGRESS_SHOW_PROGRESS,
                               I18N("File Operation"),
                               I18N("Signing archive file"),
                               GWEN_SarFileHeader_List_GetCount(fhl),
                               0);
    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      const char *s;
      uint64_t hpos;

      s=GWEN_SarFileHeader_GetPath(fh);
      hpos=GWEN_SarFileHeader_GetHashPos(fh);
      if (hpos==0) {
        DBG_WARN(GWEN_LOGDOMAIN, "File %s has no valid hash", s?s:"(unnamed)");
      }
      else {
        /* seek to start of hash */
        pos=GWEN_SyncIo_File_Seek(sr->archiveSio, hpos, GWEN_SyncIo_File_Whence_Set);
        if (pos<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
          GWEN_Gui_ProgressEnd(pid);
          GWEN_MDigest_free(md);
          return (int) pos;
        }

        /* read hash */
        rv=GWEN_SyncIo_ReadForced(sr->archiveSio, hashBuf, 20);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Gui_ProgressEnd(pid);
          GWEN_MDigest_free(md);
          return rv;
        }

        /* digest hash */
        rv=GWEN_MDigest_Update(md, hashBuf, 20);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Gui_ProgressEnd(pid);
          GWEN_MDigest_free(md);
          return rv;
        }


        GWEN_SarFileHeader_AddFlags(fh, GWEN_SAR_FILEHEADER_FLAGS_SIGNED);
      }

      rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_MDigest_free(md);
        return rv;
      }

      fh=GWEN_SarFileHeader_List_Next(fh);
    }

    /* finish hash */
    rv=GWEN_MDigest_End(md);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return rv;
    }

    /* seek to start of signature data */
    pos=GWEN_SyncIo_File_Seek(sr->archiveSio, sr->signaturePos, GWEN_SyncIo_File_Whence_Set);
    if (pos<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) pos);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return (int) pos;
    }

    /* read signature data */
    sbuf=GWEN_Buffer_new(0, sr->signatureSize, 0, 1);
    rv=GWEN_SyncIo_ReadForced(sr->archiveSio,
                              (uint8_t*) GWEN_Buffer_GetStart(sbuf),
                              sr->signatureSize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(sbuf);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_Buffer_IncrementPos(sbuf, sr->signatureSize);
    GWEN_Buffer_AdjustUsedBytes(sbuf);

    /* verify signature */
    hbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_CryptMgr_Verify(cm,
                            (const uint8_t*) GWEN_Buffer_GetStart(sbuf),
                            GWEN_Buffer_GetUsedBytes(sbuf),
                            hbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Buffer_free(sbuf);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_Buffer_free(sbuf);

    /* verify hash */
    if (GWEN_Buffer_GetUsedBytes(hbuf)!=20) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid size of signed hash (%d)", GWEN_Buffer_GetUsedBytes(hbuf));
      GWEN_Buffer_free(hbuf);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return GWEN_ERROR_BAD_DATA;
    }
    if (memcmp(GWEN_Buffer_GetStart(hbuf), GWEN_MDigest_GetDigestPtr(md), 20)!=0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid hash, data is invalid!");
      GWEN_Buffer_free(hbuf);
      GWEN_Gui_ProgressEnd(pid);
      GWEN_MDigest_free(md);
      return GWEN_ERROR_VERIFY;
    }
    DBG_INFO(GWEN_LOGDOMAIN, "Signature is valid");

    GWEN_MDigest_free(md);
    GWEN_Buffer_free(hbuf);

    GWEN_Gui_ProgressEnd(pid);
  }

  return 0;
}



int GWEN_Sar_VerifyArchive(const char *inFile, const char *signer, GWEN_CRYPT_KEY *key) {
  GWEN_SAR *sr;
  int rv;

  /* open archive file */
  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, inFile,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Sar_free(sr);
    return rv;
  }
  else {
    GWEN_CRYPTMGR *cm;

    cm=GWEN_CryptMgrKeys_new(NULL, NULL, signer, key, 0);

    /* verify */
    rv=GWEN_Sar_Verify(sr, cm);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_CryptMgr_free(cm);
      GWEN_Sar_CloseArchive(sr, 1);
      GWEN_Sar_free(sr);
      return rv;
    }
    GWEN_CryptMgr_free(cm);

    /* close archive */
    rv=GWEN_Sar_CloseArchive(sr, 0);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Sar_CloseArchive(sr, 1);
      GWEN_Sar_free(sr);
      return rv;
    }
    GWEN_Sar_free(sr);
    return 0;
  }
}



int GWEN_Sar_SignArchive(const char *inFile, const char *signer, GWEN_CRYPT_KEY *key) {
  GWEN_SAR *sr;
  int rv;

  /* open archive file */
  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, inFile,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Sar_free(sr);
    return rv;
  }
  else {
    GWEN_CRYPTMGR *cm;

    cm=GWEN_CryptMgrKeys_new(NULL, NULL, signer, key, 0);

    /* verify */
    rv=GWEN_Sar_Sign(sr, cm);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_CryptMgr_free(cm);
      GWEN_Sar_CloseArchive(sr, 1);
      GWEN_Sar_free(sr);
      return rv;
    }
    GWEN_CryptMgr_free(cm);

    /* close archive */
    rv=GWEN_Sar_CloseArchive(sr, 0);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Sar_CloseArchive(sr, 1);
      GWEN_Sar_free(sr);
      return rv;
    }
    GWEN_Sar_free(sr);
    return 0;
  }
}



int GWEN_Sar_CheckArchive(const char *inFile) {
  GWEN_SAR *sr;
  int rv;
  const GWEN_SAR_FILEHEADER_LIST *fhl;

  /* open archive file */
  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, inFile,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  fhl=GWEN_Sar_GetHeaders(sr);
  if (fhl) {
    const GWEN_SAR_FILEHEADER *fh;
    uint32_t pid;

    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_SHOW_ABORT |
                               GWEN_GUI_PROGRESS_ALLOW_EMBED |
                               GWEN_GUI_PROGRESS_SHOW_PROGRESS,
                               I18N("File Operation"),
                               I18N("Checking archive file"),
                               GWEN_SarFileHeader_List_GetCount(fhl),
                               0);

    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      //const char *s;

      //s=GWEN_SarFileHeader_GetPath(fh);
      rv=GWEN_Sar_CheckFile(sr, fh);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_Sar_CloseArchive(sr, 1);
        GWEN_Sar_free(sr);
      }

      rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Gui_ProgressEnd(pid);
        GWEN_Sar_CloseArchive(sr, 1);
        GWEN_Sar_free(sr);
        return rv;
      }

      fh=GWEN_SarFileHeader_List_Next(fh);
    }
    GWEN_Gui_ProgressEnd(pid);
  }

  rv=GWEN_Sar_CloseArchive(sr, 0);
  if (rv<0) {
    fprintf(stderr, "Error closing archive (%d)\n", rv);
    return 2;
  }
  GWEN_Sar_free(sr);

  return 0;
}



