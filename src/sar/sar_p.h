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


#ifndef GWEN_SAR_P_H
#define GWEN_SAR_P_H

#include <gwenhywfar/sar.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/mdigest.h>

#include <inttypes.h>


#define GWEN_SAR_HEADER_VERSION ((uint32_t)0x01000000)


#define GWEN_SAR_TAG_HEADER         0x01
# define GWEN_SAR_TAG_HEADER_VERSION 0x01
# define GWEN_SAR_TAG_HEADER_STATUS  0x02
# define GWEN_SAR_TAG_HEADER_FLAGS   0x03
# define GWEN_SAR_TAG_HEADER_PATH    0x04
# define GWEN_SAR_TAG_HEADER_TYPE    0x05
# define GWEN_SAR_TAG_HEADER_PERM    0x06
# define GWEN_SAR_TAG_HEADER_ATIME   0x07
# define GWEN_SAR_TAG_HEADER_MTIME   0x08
# define GWEN_SAR_TAG_HEADER_CTIME   0x09
# define GWEN_SAR_TAG_HEADER_SIZE    0x0a

#define GWEN_SAR_TAG_FILE           0x02
#define GWEN_SAR_TAG_HASH           0x03



enum {
  GWEN_Sar_OpenMode_Closed=0,
  GWEN_Sar_OpenMode_Created,
  GWEN_Sar_OpenMode_Opened
};


struct GWEN_SAR {
  char *archiveName;
  GWEN_SYNCIO *archiveSio;
  uint64_t archiveSize;

  GWEN_SAR_FILEHEADER_LIST *headers;

  int openMode;
  int refCount;
};


static uint64_t GWEN_Sar_ReadUint64(const uint8_t *p, uint32_t bs);
static int GWEN_Sar_FileHeaderToTlv(const GWEN_SAR_FILEHEADER *fh, GWEN_BUFFER *tbuf);
static int GWEN_Sar_TlvToFileHeader(GWEN_BUFFER *mbuf, GWEN_SAR_FILEHEADER *fh);

static int GWEN_Sar_AddAndDigestFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, GWEN_MDIGEST *md);
static int GWEN_Sar_AddAndDigestFileReg(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, GWEN_MDIGEST *md);
static int GWEN_Sar_AddAndDigestFileLink(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, GWEN_MDIGEST *md);

static int GWEN_Sar_ExtractAndDigestFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly);
static int GWEN_Sar_ExtractAndDigestFileReg(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly);
static int GWEN_Sar_ExtractAndDigestFileLink(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly);
static int GWEN_Sar_ExtractAndDigestFileDir(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh, int checkOnly);


static int GWEN_Sar_ScanFile(GWEN_SAR *sr);



#endif





