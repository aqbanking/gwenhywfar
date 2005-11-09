/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
    copyright   : (C) 2004 by Martin Preuss
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


#include "nl_file_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_FILE)


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayerFile_new(int fdRead,
				     int fdWrite,
				     int closeOnFree) {
  GWEN_NETLAYER *nl;
  GWEN_NL_FILE *nld;

  nl=GWEN_NetLayer_new(GWEN_NL_FILE_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_FILE, nld)
  DBG_MEM_INC("GWEN_NL_FILE", 0);
  nld->fdRead=fdRead;
  nld->fdWrite=fdWrite;
  nld->closeOnFree=closeOnFree;

  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_FILE, nl, nld,
                       GWEN_NetLayerFile_FreeData);

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerFile_Work);
  GWEN_NetLayer_SetReadFn(nl, GWEN_NetLayerFile_Read);
  GWEN_NetLayer_SetWriteFn(nl, GWEN_NetLayerFile_Write);
  GWEN_NetLayer_SetConnectFn(nl, GWEN_NetLayerFile_Connect);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerFile_Disconnect);

  return nl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayerFile_FreeData(void *bp, void *p) {
  GWEN_NL_FILE *nld;

  nld=(GWEN_NL_FILE*)p;
  if (nld->closeOnFree) {
    if (nld->fdWrite!=-1) {
      if (close(nld->fdWrite)) {
	DBG_WARN(GWEN_LOGDOMAIN, "close(fdWrite): %s", strerror(errno));
      }
    }
    if (nld->fdRead!=-1)
      close(nld->fdRead);
  }
  DBG_MEM_DEC("GWEN_NL_FILE");
  GWEN_FREE_OBJECT(nld);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerFile_Connect(GWEN_NETLAYER *nl){
  GWEN_NL_FILE *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_FILE, nl);
  assert(nld);

  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Unconnected &&
      GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Disconnected){
    DBG_ERROR(GWEN_LOGDOMAIN, "File is not unconnected (status \"%s\")",
              GWEN_NetLayerStatus_toString(GWEN_NetLayer_GetStatus(nl)));
    return GWEN_ERROR_INVALID;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Connecting");
  if (nld->fdRead!=-1) {
    if ((off_t)-1==lseek(nld->fdRead, 0, SEEK_SET)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "lseek(fdRead, 0): %s",
		strerror(errno));
      return GWEN_ERROR_INVALID;
    }
  }

  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Connected);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerFile_Disconnect(GWEN_NETLAYER *nl){
  GWEN_NL_FILE *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_FILE, nl);
  assert(nld);

  /* adjust status (Disconnected) */
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
  GWEN_NetLayer_SubFlags(nl,
			 GWEN_NETLAYER_FLAGS_EOFMET |
			 GWEN_NETLAYER_FLAGS_BROKENPIPE |
			 GWEN_NETLAYER_FLAGS_WANTREAD |
			 GWEN_NETLAYER_FLAGS_WANTWRITE);

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerFile_Read(GWEN_NETLAYER *nl,
                             char *buffer,
                             int *bsize){
  GWEN_NL_FILE *nld;
  int rv;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Reading %d bytes", *bsize);

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_FILE, nl);
  assert(nld);

  /* check status */
  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File is not connected (%d)",
              GWEN_NetLayer_GetStatus(nl));
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_EOFMET) {
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
    return GWEN_ERROR_EOF;
  }

  if (nld->fdRead==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No file descriptor for reading");
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
    return GWEN_ERROR_GENERIC;
  }

  /* try to read */
  rv=read(nld->fdRead, buffer, *bsize);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "read(): %s", strerror(errno));
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
    return GWEN_ERROR_GENERIC;
  }
  *bsize=rv;

  if (*bsize==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_EOFMET);
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Read %d bytes", *bsize);
    GWEN_Text_LogString(buffer, *bsize, GWEN_LOGDOMAIN,
                        GWEN_LoggerLevel_Verbous);
  }
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTREAD);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerFile_Write(GWEN_NETLAYER *nl,
			    const char *buffer,
			    int *bsize){
  GWEN_NL_FILE *nld;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_FILE, nl);
  assert(nld);

  /* check status */
  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File is not connected (%d)",
              GWEN_NetLayer_GetStatus(nl));
    return GWEN_ERROR_INVALID;
  }

  /* try to write */
  rv=write(nld->fdWrite, buffer, *bsize);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "write(): %s", strerror(errno));
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
    return GWEN_ERROR_GENERIC;
  }
  *bsize=rv;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Written %d bytes", *bsize);
  GWEN_Text_LogString(buffer, *bsize, 0, GWEN_LoggerLevel_Verbous);
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerFile_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_FILE *nld;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_FILE, nl);
  assert(nld);

  st=GWEN_NetLayer_GetStatus(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Working with status \"%s\" (%d)",
	      GWEN_NetLayerStatus_toString(st), st);

  return GWEN_NetLayerResult_Idle;
}







